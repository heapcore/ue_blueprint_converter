#pragma once

#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "Engine/World.h"
#include "LevelToYamlCommandlet.generated.h"

/**
 * Commandlet to export Unreal Engine levels (.umap) to YAML format
 *
 * Usage: -run=LevelToYaml -Level="/Game/Maps/L_Example" -OutDir="C:/Temp" -Manifest="C:/Temp/export_manifest.json"
 *
 * Parameters:
 * -Level: Path to the level asset to export (e.g., "/Game/Maps/L_Example")
 * -OutDir: Output directory for generated YAML files (absolute path)
 * -Manifest: Path to output manifest JSON file (absolute path)
 */
UCLASS()
class UEBLUEPRINTYAMLCONVERTER_API ULevelToYamlCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	ULevelToYamlCommandlet();

	// UCommandlet interface
	virtual int32 Main(const FString& Params) override;

private:
	/**
	 * Export a single level to YAML format
	 * @param World The loaded world/level to export
	 * @param OutDir Output directory for YAML file
	 * @param LevelName Name of the level (for filename)
	 * @return Generated YAML content as string
	 */
	FString ExportLevelToYaml(UWorld* World, const FString& OutDir, const FString& LevelName);

	/**
	 * Export actors from the level to YAML format
	 * @param World The world containing actors
	 * @return YAML string representing all actors
	 */
	FString ExportActorsToYaml(UWorld* World);

	/**
	 * Export a single actor to YAML format
	 * @param Actor The actor to export
	 * @return YAML string for this actor
	 */
	FString ExportActorToYaml(AActor* Actor);

	/**
	 * Get actor transform as YAML string
	 * @param Transform Actor's transform
	 * @return YAML transform string
	 */
	FString TransformToYaml(const FTransform& Transform);

	/**
	 * Write success/failure manifest
	 * @param ManifestPath Path to manifest file
	 * @param bSuccess Whether operation succeeded
	 * @param ErrorMessage Error message if failed
	 * @param ExportedFiles List of exported files
	 * @return True if manifest written successfully
	 */
	bool WriteManifest(const FString& ManifestPath, bool bSuccess, const FString& ErrorMessage, const TArray<FString>& ExportedFiles);
};
