#pragma once

#include "Commandlets/Commandlet.h"
#include "Engine/Blueprint.h"
#include "YamlBuildCommandlet.generated.h"

UCLASS()
class UEBLUEPRINTYAMLCONVERTER_API UYamlBuildCommandlet : public UCommandlet
{
    GENERATED_BODY()

public:
    UYamlBuildCommandlet();

    virtual int32 Main(const FString& Params) override;

private:
    bool ParseParams(const FString& Params, FString& YamlPath, FString& OutDir, FString& ManifestPath);
    bool WriteManifest(const FString& ManifestPath, bool bSuccess, const FString& Message, const TArray<TPair<FString, FString>>& GeneratedAssets);
    TArray<FBPVariableDescription> ParseVariablesFromYaml(const FString& YamlText);
    bool ValidateYamlStructure(const FString& YamlText, FString& OutError);
};
