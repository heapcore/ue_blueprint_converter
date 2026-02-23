#include "LevelToYamlCommandlet.h"
#include "UEBlueprintYamlConverter.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFilemanager.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/Light.h"
#include "Engine/DirectionalLight.h"
#include "Engine/PointLight.h"
#include "Engine/SpotLight.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/Actor.h"
#include "GameFramework/WorldSettings.h"
#include "Components/StaticMeshComponent.h"
#include "Components/LightComponent.h"
#include "Components/PointLightComponent.h"
#include "UObject/UObjectGlobals.h"
#include "EngineUtils.h"

ULevelToYamlCommandlet::ULevelToYamlCommandlet()
{
	IsServer = false;
	IsClient = false;
	LogToConsole = true;
}

int32 ULevelToYamlCommandlet::Main(const FString& Params)
{
	UE_LOG(LogUEYaml, Log, TEXT("=== LevelToYaml Commandlet Started ==="));

	// Parse command line parameters
	FString LevelPath;
	FString OutDir;
	FString ManifestPath;

	if (!FParse::Value(*Params, TEXT("Level="), LevelPath) || LevelPath.IsEmpty())
	{
		UE_LOG(LogUEYaml, Error, TEXT("Missing or empty -Level parameter. Example: -Level=\"/Game/Maps/L_Example\""));
		return 1;
	}

	if (!FParse::Value(*Params, TEXT("OutDir="), OutDir) || OutDir.IsEmpty())
	{
		UE_LOG(LogUEYaml, Error, TEXT("Missing or empty -OutDir parameter. Example: -OutDir=\"C:/Temp\""));
		return 1;
	}

	if (!FParse::Value(*Params, TEXT("Manifest="), ManifestPath) || ManifestPath.IsEmpty())
	{
		UE_LOG(LogUEYaml, Error, TEXT("Missing or empty -Manifest parameter. Example: -Manifest=\"C:/Temp/manifest.json\""));
		return 1;
	}

	// Clean up paths (remove quotes)
	LevelPath = LevelPath.TrimQuotes();
	OutDir = OutDir.TrimQuotes();
	ManifestPath = ManifestPath.TrimQuotes();

	UE_LOG(LogUEYaml, Log, TEXT("Parameters: Level=%s, OutDir=%s, Manifest=%s"), *LevelPath, *OutDir, *ManifestPath);

	// Ensure output directory exists
	if (!IFileManager::Get().DirectoryExists(*OutDir))
	{
		if (!IFileManager::Get().MakeDirectory(*OutDir, true))
		{
			const FString ErrorMsg = FString::Printf(TEXT("Failed to create output directory: %s"), *OutDir);
			UE_LOG(LogUEYaml, Error, TEXT("%s"), *ErrorMsg);
			WriteManifest(ManifestPath, false, ErrorMsg, {});
			return 2;
		}
	}

	// Load the level
	FString LevelObjectPath = LevelPath;
	if (!LevelObjectPath.Contains(TEXT(".")))
	{
		// Add the object name if not present (e.g., "/Game/Maps/L_Example" -> "/Game/Maps/L_Example.L_Example")
		FString LevelName = FPaths::GetBaseFilename(LevelPath);
		LevelObjectPath = LevelPath + TEXT(".") + LevelName;
	}

	UObject* LevelAsset = StaticFindObject(UWorld::StaticClass(), nullptr, *LevelObjectPath);
	if (!LevelAsset)
	{
		// Try to load the asset
		LevelAsset = StaticLoadObject(UWorld::StaticClass(), nullptr, *LevelObjectPath);
	}

	UWorld* World = Cast<UWorld>(LevelAsset);
	if (!World)
	{
		const FString ErrorMsg = FString::Printf(TEXT("Failed to load level: %s (tried path: %s)"), *LevelPath, *LevelObjectPath);
		UE_LOG(LogUEYaml, Error, TEXT("%s"), *ErrorMsg);
		WriteManifest(ManifestPath, false, ErrorMsg, {});
		return 3;
	}

	UE_LOG(LogUEYaml, Log, TEXT("Successfully loaded level: %s"), *World->GetName());

	// Export level to YAML
	FString LevelName = FPaths::GetBaseFilename(LevelPath);
	FString YamlContent = ExportLevelToYaml(World, OutDir, LevelName);

	if (YamlContent.IsEmpty())
	{
		const FString ErrorMsg = TEXT("Failed to generate YAML content from level");
		UE_LOG(LogUEYaml, Error, TEXT("%s"), *ErrorMsg);
		WriteManifest(ManifestPath, false, ErrorMsg, {});
		return 4;
	}

	// Write YAML file
	FString YamlFilePath = FPaths::Combine(OutDir, LevelName + TEXT(".yaml"));
	if (!FFileHelper::SaveStringToFile(YamlContent, *YamlFilePath, FFileHelper::EEncodingOptions::ForceUTF8))
	{
		const FString ErrorMsg = FString::Printf(TEXT("Failed to write YAML file: %s"), *YamlFilePath);
		UE_LOG(LogUEYaml, Error, TEXT("%s"), *ErrorMsg);
		WriteManifest(ManifestPath, false, ErrorMsg, {});
		return 5;
	}

	UE_LOG(LogUEYaml, Log, TEXT("Successfully exported level to: %s"), *YamlFilePath);

	// Write success manifest
	TArray<FString> ExportedFiles;
	ExportedFiles.Add(YamlFilePath);

	if (!WriteManifest(ManifestPath, true, TEXT(""), ExportedFiles))
	{
		UE_LOG(LogUEYaml, Warning, TEXT("Failed to write manifest, but export succeeded"));
		return 6;
	}

	UE_LOG(LogUEYaml, Log, TEXT("=== LevelToYaml Commandlet Completed Successfully ==="));
	return 0;
}

FString ULevelToYamlCommandlet::ExportLevelToYaml(UWorld* World, const FString& OutDir, const FString& LevelName)
{
	if (!World)
	{
		return FString();
	}

	FString YamlContent;

	// YAML header
	YamlContent += TEXT("# Exported from Unreal Engine level\n");
	YamlContent += TEXT("version: 1\n");
	YamlContent += TEXT("\n");

	// Level metadata
	YamlContent += TEXT("level:\n");
	YamlContent += FString::Printf(TEXT("  name: \"%s\"\n"), *LevelName);
	YamlContent += FString::Printf(TEXT("  description: \"Exported from %s\"\n"), *World->GetName());
	YamlContent += TEXT("  world_settings:\n");
	YamlContent += FString::Printf(TEXT("    gravity: [0, 0, %.1f]\n"), World->GetGravityZ());
	YamlContent += TEXT("\n");

	// Export actors
	FString ActorsYaml = ExportActorsToYaml(World);
	YamlContent += ActorsYaml;

	return YamlContent;
}

FString ULevelToYamlCommandlet::ExportActorsToYaml(UWorld* World)
{
	if (!World)
	{
		return FString();
	}

	FString ActorsYaml;
	ActorsYaml += TEXT("actors:\n");

	int32 ActorCount = 0;

	// Iterate through all actors in the level
	UE_LOG(LogUEYaml, Log, TEXT("=== Scanning actors in world ==="));

	for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
	{
		AActor* Actor = *ActorIterator;
		if (!Actor)
		{
			continue; // Skip null actors
		}

		UE_LOG(LogUEYaml, Log, TEXT("Found actor: %s (class: %s)"), *Actor->GetName(), *Actor->GetClass()->GetName());

		// Skip only WorldSettings, but export everything else
		if (Actor->IsA<AWorldSettings>())
		{
			UE_LOG(LogUEYaml, Log, TEXT("Skipping WorldSettings actor: %s"), *Actor->GetName());
			continue;
		}

		FString ActorYaml = ExportActorToYaml(Actor);
		if (!ActorYaml.IsEmpty())
		{
			ActorsYaml += ActorYaml;
			ActorCount++;
			UE_LOG(LogUEYaml, Log, TEXT("Exported actor %d: %s"), ActorCount, *Actor->GetName());
		}
	}

	if (ActorCount == 0)
	{
		ActorsYaml += TEXT("  # No actors found in level\n");
	}

	UE_LOG(LogUEYaml, Log, TEXT("Exported %d actors from level"), ActorCount);

	return ActorsYaml;
}

FString ULevelToYamlCommandlet::ExportActorToYaml(AActor* Actor)
{
	if (!Actor)
	{
		return FString();
	}

	FString ActorYaml;

	// Actor header
	ActorYaml += FString::Printf(TEXT("  - name: \"%s\"\n"), *Actor->GetName());
	ActorYaml += FString::Printf(TEXT("    class: \"%s\"\n"), *Actor->GetClass()->GetName());

	// Transform
	FTransform ActorTransform = Actor->GetActorTransform();
	ActorYaml += TransformToYaml(ActorTransform);

	// Properties
	ActorYaml += TEXT("    properties:\n");

	// Tags
	if (Actor->Tags.Num() > 0)
	{
		ActorYaml += TEXT("      Tags: [");
		for (int32 i = 0; i < Actor->Tags.Num(); i++)
		{
			if (i > 0) ActorYaml += TEXT(", ");
			ActorYaml += FString::Printf(TEXT("\"%s\""), *Actor->Tags[i].ToString());
		}
		ActorYaml += TEXT("]\n");
	}

	// Hidden state
	if (Actor->IsHidden())
	{
		ActorYaml += TEXT("      Hidden: true\n");
	}

	// Actor-specific properties

	// Static Mesh Actor
	if (AStaticMeshActor* StaticMeshActor = Cast<AStaticMeshActor>(Actor))
	{
		if (UStaticMeshComponent* MeshComp = StaticMeshActor->GetStaticMeshComponent())
		{
			if (UStaticMesh* StaticMesh = MeshComp->GetStaticMesh())
			{
				ActorYaml += FString::Printf(TEXT("    static_mesh: \"%s\"\n"), *StaticMesh->GetPathName());
			}

			// Mobility
			FString MobilityStr = TEXT("Static");
			if (MeshComp->Mobility == EComponentMobility::Stationary)
				MobilityStr = TEXT("Stationary");
			else if (MeshComp->Mobility == EComponentMobility::Movable)
				MobilityStr = TEXT("Movable");

			ActorYaml += FString::Printf(TEXT("      Mobility: \"%s\"\n"), *MobilityStr);
		}
	}

	// Light Actors
	if (ALight* LightActor = Cast<ALight>(Actor))
	{
		if (ULightComponent* LightComp = LightActor->GetLightComponent())
		{
			ActorYaml += TEXT("    light:\n");
			ActorYaml += FString::Printf(TEXT("      intensity: %.2f\n"), LightComp->Intensity);

			FLinearColor LightColor = LightComp->LightColor;
			ActorYaml += FString::Printf(TEXT("      color: [%.3f, %.3f, %.3f]\n"),
				LightColor.R, LightColor.G, LightColor.B);

			if (APointLight* PointLight = Cast<APointLight>(Actor))
			{
				if (UPointLightComponent* PointLightComp = Cast<UPointLightComponent>(PointLight->GetLightComponent()))
				{
					ActorYaml += FString::Printf(TEXT("      radius: %.2f\n"),
						PointLightComp->AttenuationRadius);
				}
			}
		}
	}

	ActorYaml += TEXT("\n");

	return ActorYaml;
}

FString ULevelToYamlCommandlet::TransformToYaml(const FTransform& Transform)
{
	FString TransformYaml;

	TransformYaml += TEXT("    transform:\n");

	// Location
	FVector Location = Transform.GetLocation();
	TransformYaml += FString::Printf(TEXT("      location: [%.2f, %.2f, %.2f]\n"),
		Location.X, Location.Y, Location.Z);

	// Rotation (convert to degrees)
	FRotator Rotation = Transform.GetRotation().Rotator();
	TransformYaml += FString::Printf(TEXT("      rotation: [%.2f, %.2f, %.2f]\n"),
		Rotation.Pitch, Rotation.Yaw, Rotation.Roll);

	// Scale
	FVector Scale = Transform.GetScale3D();
	TransformYaml += FString::Printf(TEXT("      scale: [%.3f, %.3f, %.3f]\n"),
		Scale.X, Scale.Y, Scale.Z);

	return TransformYaml;
}

bool ULevelToYamlCommandlet::WriteManifest(const FString& ManifestPath, bool bSuccess, const FString& ErrorMessage, const TArray<FString>& ExportedFiles)
{
	TSharedPtr<FJsonObject> ManifestJson = MakeShareable(new FJsonObject);

	// Basic info
	ManifestJson->SetBoolField(TEXT("success"), bSuccess);
	ManifestJson->SetStringField(TEXT("timestamp"), FDateTime::Now().ToString());
	ManifestJson->SetStringField(TEXT("commandlet"), TEXT("LevelToYaml"));

	if (!bSuccess && !ErrorMessage.IsEmpty())
	{
		TArray<TSharedPtr<FJsonValue>> ErrorArray;
		TSharedPtr<FJsonObject> ErrorObj = MakeShareable(new FJsonObject);
		ErrorObj->SetStringField(TEXT("message"), ErrorMessage);
		ErrorArray.Add(MakeShareable(new FJsonValueObject(ErrorObj)));
		ManifestJson->SetArrayField(TEXT("errors"), ErrorArray);
	}

	// Exported files
	if (ExportedFiles.Num() > 0)
	{
		TArray<TSharedPtr<FJsonValue>> FilesArray;
		for (const FString& FilePath : ExportedFiles)
		{
			TSharedPtr<FJsonObject> FileObj = MakeShareable(new FJsonObject);
			FileObj->SetStringField(TEXT("path"), FilePath);
			FileObj->SetStringField(TEXT("type"), TEXT("YAML"));
			FilesArray.Add(MakeShareable(new FJsonValueObject(FileObj)));
		}
		ManifestJson->SetArrayField(TEXT("exportedFiles"), FilesArray);
	}

	// Serialize to JSON string
	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(ManifestJson.ToSharedRef(), Writer);

	// Write to file
	return FFileHelper::SaveStringToFile(OutputString, *ManifestPath, FFileHelper::EEncodingOptions::ForceUTF8);
}
