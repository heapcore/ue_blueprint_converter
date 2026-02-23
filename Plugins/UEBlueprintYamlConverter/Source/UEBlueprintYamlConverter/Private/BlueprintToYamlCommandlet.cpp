#include "BlueprintToYamlCommandlet.h"
#include "UEBlueprintYamlConverter.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"
#include "EdGraphSchema_K2.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Engine/Blueprint.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

UBlueprintToYamlCommandlet::UBlueprintToYamlCommandlet()
{
    IsServer = false;
    IsClient = false;
    LogToConsole = true;
}

static FString ToYamlScalar(const FString& In)
{
    FString S = In;
    S.ReplaceInline(TEXT("\r"), TEXT(""));
    S.ReplaceInline(TEXT("\n"), TEXT("\\n"));
    if (S.Contains(TEXT(":")) || S.Contains(TEXT("#")) || S.Contains(TEXT("\"")) || S.Contains(TEXT("'")) || S.Contains(TEXT(" "))) {
        return FString::Printf(TEXT("\"%s\""), *S.Replace(TEXT("\""), TEXT("\\\"")));
    }
    return S;
}

int32 UBlueprintToYamlCommandlet::Main(const FString& Params)
{
    FString OutDir;
    FString AbsLog;
    FString FilterPath;
    FParse::Value(*Params, TEXT("-OutDir="), OutDir);
    FParse::Value(*Params, TEXT("-Filter="), FilterPath); // e.g. /Game/HorrorEngine
    FParse::Value(*Params, TEXT("-ABSLOG="), AbsLog);
    if (OutDir.IsEmpty())
    {
        UE_LOG(LogUEYaml, Error, TEXT("Missing -OutDir= path"));
        return 1;
    }
    IFileManager::Get().MakeDirectory(*OutDir, true);

    FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
    FARFilter Filter;
    Filter.ClassPaths.Add(UBlueprint::StaticClass()->GetClassPathName());
    Filter.bRecursivePaths = true;
    if (!FilterPath.IsEmpty())
    {
        // Normalize FilterPath to package path (e.g. /Game/MyPath)
        if (!FilterPath.StartsWith(TEXT("/")))
        {
            // Try convert filesystem path to UE path
            FString Normal = FilterPath;
            Normal.ReplaceInline(TEXT("\\"), TEXT("/"));
            int32 ContentIdx;
            if (Normal.FindChar(TEXT('/'), ContentIdx)) { }
            // Best-effort: if contains /Content/, clip to UE path
            int32 CIdx = Normal.ToLower().Find(TEXT("/content/"));
            if (CIdx != INDEX_NONE)
            {
                FilterPath = TEXT("/Game/") + Normal.Mid(CIdx + 9);
            }
        }
        Filter.PackagePaths.Add(*FilterPath);
    }
    TArray<FAssetData> Assets;
    AssetRegistryModule.Get().GetAssets(Filter, Assets);

    int32 NumOk = 0, NumFail = 0;
    FString LogBuf;
    for (const FAssetData& AD : Assets)
    {
        UBlueprint* BP = Cast<UBlueprint>(AD.GetAsset());
        if (!BP) { NumFail++; LogBuf += FString::Printf(TEXT("Skip (not BP): %s\n"), *AD.GetObjectPathString()); continue; }

        FString Yaml;
        Yaml += TEXT("version: 1\n");
        Yaml += TEXT("blueprints:\n");
        Yaml += FString::Printf(TEXT("  - name: %s\n"), *ToYamlScalar(BP->GetName()));
        // Convert BlueprintType enum to string
        FString TypeString = TEXT("Normal");
        switch (BP->BlueprintType)
        {
            case BPTYPE_MacroLibrary: TypeString = TEXT("MacroLibrary"); break;
            case BPTYPE_Interface: TypeString = TEXT("Interface"); break;
            case BPTYPE_FunctionLibrary: TypeString = TEXT("FunctionLibrary"); break;
            default: TypeString = TEXT("Normal"); break;
        }
        Yaml += FString::Printf(TEXT("    type: \"%s\"\n"), *TypeString);

        // Variables
        if (BP->NewVariables.Num() > 0)
        {
            Yaml += TEXT("    variables:\n");
            for (const FBPVariableDescription& Var : BP->NewVariables)
            {
                Yaml += FString::Printf(TEXT("      - name: %s\n"), *ToYamlScalar(Var.VarName.ToString()));
                Yaml += FString::Printf(TEXT("        type: \"%s\"\n"), *Var.VarType.PinCategory.ToString());
                if (Var.DefaultValue.Len() > 0)
                {
                    Yaml += FString::Printf(TEXT("        default: %s\n"), *ToYamlScalar(Var.DefaultValue));
                }
                if (Var.PropertyFlags & CPF_Edit)
                {
                    Yaml += TEXT("        editable: true\n");
                }
            }
        }

        // EventGraph nodes (names only, MVP)
        if (BP->UbergraphPages.Num() > 0 && BP->UbergraphPages[0])
        {
            Yaml += TEXT("    event_graph:\n");
            for (UEdGraphNode* Node : BP->UbergraphPages[0]->Nodes)
            {
                if (!Node) continue;
                Yaml += FString::Printf(TEXT("      - node: %s\n"), *ToYamlScalar(Node->GetNodeTitle(ENodeTitleType::ListView).ToString()));
            }
        }

        const FString PackagePath = AD.PackageName.ToString();
        const FString RelPath = PackagePath.Replace(TEXT("/Game/"), TEXT(""));
        const FString FilePath = FPaths::Combine(OutDir, RelPath + TEXT(".yaml"));
        IFileManager::Get().MakeDirectory(*FPaths::GetPath(FilePath), true);
        if (FFileHelper::SaveStringToFile(Yaml, *FilePath, FFileHelper::EEncodingOptions::ForceUTF8))
        { NumOk++; LogBuf += FString::Printf(TEXT("OK: %s -> %s\n"), *AD.GetObjectPathString(), *FilePath); }
        else { NumFail++; LogBuf += FString::Printf(TEXT("FAIL: %s -> %s\n"), *AD.GetObjectPathString(), *FilePath); }
    }

    UE_LOG(LogUEYaml, Log, TEXT("BlueprintToYaml exported %d OK, %d failed. OutDir=%s"), NumOk, NumFail, *OutDir);
    if (!AbsLog.IsEmpty()) { FFileHelper::SaveStringToFile(LogBuf, *AbsLog); }
    return (NumFail == 0) ? 0 : 2;
}
