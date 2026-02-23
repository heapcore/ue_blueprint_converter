#include "YamlBuildCommandlet.h"
#include "UEBlueprintYamlConverter.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/FileManagerGeneric.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"
#include "AssetToolsModule.h"
#include "Factories/WorldFactory.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Engine/World.h"
#include "UObject/SavePackage.h"
#include "EdGraphSchema_K2.h"

UYamlBuildCommandlet::UYamlBuildCommandlet()
{
    IsServer = false;
    IsClient = false;
    LogToConsole = true;
}

bool UYamlBuildCommandlet::ParseParams(const FString& Params, FString& YamlPath, FString& OutDir, FString& ManifestPath)
{
    FParse::Value(*Params, TEXT("-Yaml="), YamlPath);
    FParse::Value(*Params, TEXT("-OutDir="), OutDir);
    FParse::Value(*Params, TEXT("-Manifest="), ManifestPath);
    return !YamlPath.IsEmpty() && !OutDir.IsEmpty() && !ManifestPath.IsEmpty();
}

bool UYamlBuildCommandlet::WriteManifest(const FString& ManifestPath, bool bSuccess, const FString& Message, const TArray<TPair<FString, FString>>& GeneratedAssets)
{
    TSharedPtr<FJsonObject> Root = MakeShared<FJsonObject>();
    Root->SetBoolField(TEXT("success"), bSuccess);
    if (!Message.IsEmpty())
    {
        TArray<TSharedPtr<FJsonValue>> Errors;
        TSharedPtr<FJsonObject> Err = MakeShared<FJsonObject>();
        Err->SetStringField(TEXT("message"), Message);
        Errors.Add(MakeShared<FJsonValueObject>(Err));
        Root->SetArrayField(TEXT("errors"), Errors);
    }
    if (GeneratedAssets.Num() > 0)
    {
        TArray<TSharedPtr<FJsonValue>> Assets;
        for (const auto& Pair : GeneratedAssets)
        {
            TSharedPtr<FJsonObject> Obj = MakeShared<FJsonObject>();
            Obj->SetStringField(TEXT("path"), Pair.Key);
            Obj->SetStringField(TEXT("type"), Pair.Value);
            Assets.Add(MakeShared<FJsonValueObject>(Obj));
        }
        Root->SetArrayField(TEXT("generatedAssets"), Assets);
    }

    FString OutputString;
    TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&OutputString);
    FJsonSerializer::Serialize(Root.ToSharedRef(), JsonWriter);
    return FFileHelper::SaveStringToFile(OutputString, *ManifestPath);
}

int32 UYamlBuildCommandlet::Main(const FString& Params)
{
    FString YamlPath, OutDir, ManifestPath;
    if (!ParseParams(Params, YamlPath, OutDir, ManifestPath))
    {
        UE_LOG(LogUEYaml, Error, TEXT("Missing parameters. Expected -Yaml= -OutDir= -Manifest="));
        return 1;
    }

    UE_LOG(LogUEYaml, Log, TEXT("YamlBuild: Yaml=%s OutDir=%s Manifest=%s"), *YamlPath, *OutDir, *ManifestPath);

    // Convert relative paths to absolute paths
    if (FPaths::IsRelative(ManifestPath))
    {
        ManifestPath = FPaths::ConvertRelativePathToFull(ManifestPath);
    }
    UE_LOG(LogUEYaml, Log, TEXT("Absolute Manifest path: %s"), *ManifestPath);

    IFileManager::Get().MakeDirectory(*OutDir, true);

    // Parse YAML for blueprint info and variables
    FString YamlText;
    FString LevelName = TEXT("L_Example");
    FString BlueprintName = TEXT("BP_Door");
    TArray<FBPVariableDescription> Variables;

    if (FPaths::FileExists(YamlPath))
    {
        FFileHelper::LoadFileToString(YamlText, *YamlPath);

        // Validate YAML structure before processing
        FString ValidationError;
        if (!ValidateYamlStructure(YamlText, ValidationError))
        {
            UE_LOG(LogUEYaml, Error, TEXT("YAML validation failed: %s"), *ValidationError);
            WriteManifest(ManifestPath, false, FString::Printf(TEXT("YAML validation error: %s"), *ValidationError), {});
            return 4;
        }

        auto FindValue = [](const FString& Source, const FString& Key, const FString& DefaultValue) -> FString
        {
            int32 Pos = Source.Find(Key, ESearchCase::IgnoreCase, ESearchDir::FromStart);
            if (Pos != INDEX_NONE)
            {
                int32 Colon = Source.Find(TEXT(":"), ESearchCase::IgnoreCase, ESearchDir::FromStart, Pos + Key.Len());
                if (Colon != INDEX_NONE)
                {
                    int32 LineEnd = Source.Find(TEXT("\n"), ESearchCase::IgnoreCase, ESearchDir::FromStart, Colon + 1);
                    FString Raw = Source.Mid(Colon + 1, (LineEnd == INDEX_NONE ? Source.Len() : LineEnd) - (Colon + 1)).TrimStartAndEnd();
                    Raw = Raw.Replace(TEXT("\""), TEXT(""));
                    return Raw;
                }
            }
            return DefaultValue;
        };

        // Extract blueprint name.
        // Canonical schema: "blueprints: - name: ...", legacy compatibility: "blueprint: name: ..."
        FString ParsedBlueprintName = FindValue(YamlText, TEXT("- name"), TEXT(""));
        if (ParsedBlueprintName.IsEmpty())
        {
            ParsedBlueprintName = FindValue(YamlText, TEXT("name"), TEXT(""));
        }
        if (!ParsedBlueprintName.IsEmpty())
        {
            BlueprintName = ParsedBlueprintName;
        }

        // Parse variables section
        Variables = ParseVariablesFromYaml(YamlText);

        UE_LOG(LogUEYaml, Log, TEXT("Parsed blueprint name: %s with %d variables"), *BlueprintName, Variables.Num());
    }
    else
    {
        WriteManifest(ManifestPath, false, FString::Printf(TEXT("YAML not found: %s"), *YamlPath), {});
        return 2;
    }

    // Create World asset if missing
    const FString MapsPath = TEXT("/Game/Maps");
    const FString WorldAssetPath = MapsPath + TEXT("/") + LevelName;
    const FString WorldObjectPath = WorldAssetPath + TEXT(".") + LevelName;
    UWorld* CreatedWorld = nullptr;
    if (!StaticFindObject(UWorld::StaticClass(), nullptr, *WorldObjectPath))
    {
        FAssetToolsModule& AssetToolsModule = FAssetToolsModule::GetModule();
        UWorldFactory* Factory = NewObject<UWorldFactory>();
        UObject* NewAsset = AssetToolsModule.Get().CreateAsset(*LevelName, *MapsPath, UWorld::StaticClass(), Factory);
        CreatedWorld = Cast<UWorld>(NewAsset);
        if (CreatedWorld)
        {
            UPackage* Pkg = CreatedWorld->GetOutermost();
            FSavePackageArgs SaveArgs;
            SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
            FString Filename = FPackageName::LongPackageNameToFilename(WorldAssetPath, FPackageName::GetMapPackageExtension());
            IFileManager::Get().MakeDirectory(*FPaths::GetPath(Filename), true);
            UPackage::SavePackage(Pkg, CreatedWorld, *Filename, SaveArgs);
        }
    }

    // Create simple Actor Blueprint if missing
    const FString GeneratedPath = TEXT("/Game/Generated");
    const FString BPAssetPath = GeneratedPath + TEXT("/") + BlueprintName;
    const FString BPObjectPath = BPAssetPath + TEXT(".") + BlueprintName;
    if (!StaticFindObject(UBlueprint::StaticClass(), nullptr, *BPObjectPath))
    {
        UPackage* Pkg = CreatePackage(*BPAssetPath);
        Pkg->FullyLoad();
        UBlueprint* BP = FKismetEditorUtilities::CreateBlueprint(AActor::StaticClass(), Pkg, *BlueprintName, BPTYPE_Normal, UBlueprint::StaticClass(), UBlueprintGeneratedClass::StaticClass());
        if (BP)
        {
            // Add variables from YAML
            for (const FBPVariableDescription& Variable : Variables)
            {
                BP->NewVariables.Add(Variable);
                UE_LOG(LogUEYaml, Log, TEXT("Added variable: %s (%s)"), *Variable.VarName.ToString(), *Variable.VarType.PinCategory.ToString());
            }

            // TODO: Advanced features temporarily disabled for debugging
            // ParseAndCreateComponents(YamlText, BP);
            // ParseAndCreateFunctions(YamlText, BP);
            // ParseAndCreateEvents(YamlText, BP);

            // Compile the blueprint to make changes take effect
            FKismetEditorUtilities::CompileBlueprint(BP);

            FSavePackageArgs SaveArgs;
            SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
            FString Filename = FPackageName::LongPackageNameToFilename(BPAssetPath, FPackageName::GetAssetPackageExtension());
            IFileManager::Get().MakeDirectory(*FPaths::GetPath(Filename), true);
            UPackage::SavePackage(Pkg, BP, *Filename, SaveArgs);
        }
    }

    TArray<TPair<FString, FString>> Assets;
    Assets.Emplace(WorldAssetPath, TEXT("World"));
    Assets.Emplace(BPAssetPath, TEXT("Blueprint"));

    const bool bOk = WriteManifest(ManifestPath, true, TEXT(""), Assets);
    UE_LOG(LogUEYaml, Log, TEXT("WriteManifest result: %s for path: %s"), bOk ? TEXT("SUCCESS") : TEXT("FAILED"), *ManifestPath);
    return bOk ? 0 : 3;
}

TArray<FBPVariableDescription> UYamlBuildCommandlet::ParseVariablesFromYaml(const FString& YamlText)
{
    TArray<FBPVariableDescription> Variables;

    // Find variables section
    int32 VariablesPos = YamlText.Find(TEXT("variables:"), ESearchCase::IgnoreCase);
    if (VariablesPos == INDEX_NONE)
    {
        return Variables;
    }

    // Parse each variable entry (very basic parsing)
    FString VariablesSection = YamlText.Mid(VariablesPos);
    TArray<FString> Lines;
    VariablesSection.ParseIntoArrayLines(Lines);

    FBPVariableDescription CurrentVar;
    bool bParsingVariable = false;

    for (const FString& Line : Lines)
    {
        FString TrimmedLine = Line.TrimStartAndEnd();

        // Check if this is a new variable entry
        if (TrimmedLine.StartsWith(TEXT("- name:")))
        {
            // Save previous variable if we were parsing one
            if (bParsingVariable && !CurrentVar.VarName.IsNone())
            {
                Variables.Add(CurrentVar);
            }

            // Start new variable
            CurrentVar = FBPVariableDescription();
            bParsingVariable = true;

            // Extract variable name
            FString VarName = TrimmedLine.Replace(TEXT("- name:"), TEXT("")).TrimStartAndEnd();
            VarName = VarName.Replace(TEXT("\""), TEXT(""));
            CurrentVar.VarName = FName(*VarName);
        }
        else if (bParsingVariable)
        {
            // Parse variable properties
            if (TrimmedLine.StartsWith(TEXT("type:")))
            {
                FString TypeStr = TrimmedLine.Replace(TEXT("type:"), TEXT("")).TrimStartAndEnd();
                TypeStr = TypeStr.Replace(TEXT("\""), TEXT(""));

                // Map YAML types to UE types
                if (TypeStr == TEXT("float"))
                {
                    CurrentVar.VarType.PinCategory = UEdGraphSchema_K2::PC_Real;
                    CurrentVar.VarType.PinSubCategory = UEdGraphSchema_K2::PC_Float;
                }
                else if (TypeStr == TEXT("bool"))
                {
                    CurrentVar.VarType.PinCategory = UEdGraphSchema_K2::PC_Boolean;
                }
                else if (TypeStr == TEXT("string"))
                {
                    CurrentVar.VarType.PinCategory = UEdGraphSchema_K2::PC_String;
                }
                else if (TypeStr == TEXT("int"))
                {
                    CurrentVar.VarType.PinCategory = UEdGraphSchema_K2::PC_Int;
                }
                else
                {
                    // Default to string for unknown types
                    CurrentVar.VarType.PinCategory = UEdGraphSchema_K2::PC_String;
                }
            }
            else if (TrimmedLine.StartsWith(TEXT("default:")))
            {
                FString DefaultValue = TrimmedLine.Replace(TEXT("default:"), TEXT("")).TrimStartAndEnd();
                DefaultValue = DefaultValue.Replace(TEXT("\""), TEXT(""));
                CurrentVar.DefaultValue = DefaultValue;
            }
            else if (TrimmedLine.StartsWith(TEXT("editable: true")))
            {
                CurrentVar.PropertyFlags |= CPF_Edit;
            }
            // Stop parsing this variable when we hit a new section or end
            else if (TrimmedLine.EndsWith(TEXT(":")))
            {
                if (bParsingVariable && !CurrentVar.VarName.IsNone())
                {
                    Variables.Add(CurrentVar);
                    bParsingVariable = false;
                }
                break;
            }
        }
    }

    // Add the last variable if we were parsing one
    if (bParsingVariable && !CurrentVar.VarName.IsNone())
    {
        Variables.Add(CurrentVar);
    }

    UE_LOG(LogUEYaml, Log, TEXT("Parsed %d variables from YAML"), Variables.Num());
    return Variables;
}

bool UYamlBuildCommandlet::ValidateYamlStructure(const FString& YamlText, FString& OutError)
{
    // Check for basic YAML structure and required fields
    if (YamlText.IsEmpty())
    {
        OutError = TEXT("YAML file is empty");
        return false;
    }

    // Check for invalid YAML syntax patterns
    if (YamlText.Contains(TEXT("[[[")))
    {
        OutError = TEXT("Invalid YAML syntax: malformed array brackets");
        return false;
    }

    // Check for required top-level fields
    if (!YamlText.Contains(TEXT("version:")))
    {
        OutError = TEXT("Missing required field: version");
        return false;
    }

    if (!YamlText.Contains(TEXT("blueprints:")) && !YamlText.Contains(TEXT("blueprint:")))
    {
        OutError = TEXT("Missing required field: blueprints (or legacy blueprint)");
        return false;
    }

    // Check for proper YAML key-value format
    TArray<FString> Lines;
    YamlText.ParseIntoArrayLines(Lines);

    for (int32 i = 0; i < Lines.Num(); i++)
    {
        FString Line = Lines[i].TrimStartAndEnd();
        if (Line.IsEmpty() || Line.StartsWith(TEXT("#")))
        {
            continue; // Skip empty lines and comments
        }

        // Check for multiple consecutive colons (invalid YAML)
        if (Line.Contains(TEXT(":")) && Line.Contains(TEXT(": :")))
        {
            OutError = FString::Printf(TEXT("Invalid YAML syntax on line %d: multiple consecutive colons"), i + 1);
            return false;
        }

        // Check for lines with colons that don't follow YAML format
        if (Line.Contains(TEXT(":")) && !Line.Contains(TEXT("- ")) && !Line.StartsWith(TEXT(" ")))
        {
            // Should be either "key: value" or "- item" format
            int32 ColonPos = Line.Find(TEXT(":"));
            if (ColonPos != INDEX_NONE)
            {
                FString AfterColon = Line.Mid(ColonPos + 1).TrimStartAndEnd();
                FString BeforeColon = Line.Mid(0, ColonPos).TrimStartAndEnd();

                // Allow "key:" (no value) or "key: value" format
                if (BeforeColon.IsEmpty())
                {
                    OutError = FString::Printf(TEXT("Invalid YAML syntax on line %d: empty key before colon"), i + 1);
                    return false;
                }
            }
        }
    }

    UE_LOG(LogUEYaml, Log, TEXT("YAML validation passed"));
    return true;
}
