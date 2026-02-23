#pragma once

#include "Commandlets/Commandlet.h"
#include "BlueprintToYamlCommandlet.generated.h"

UCLASS()
class UEBLUEPRINTYAMLCONVERTER_API UBlueprintToYamlCommandlet : public UCommandlet
{
    GENERATED_BODY()
public:
    UBlueprintToYamlCommandlet();
    virtual int32 Main(const FString& Params) override;
};
