#include "UEBlueprintYamlConverter.h"

DEFINE_LOG_CATEGORY(LogUEYaml);

void FUEBlueprintYamlConverterModule::StartupModule()
{
    UE_LOG(LogUEYaml, Log, TEXT("UEBlueprintYamlConverter: Startup"));
}

void FUEBlueprintYamlConverterModule::ShutdownModule()
{
    UE_LOG(LogUEYaml, Log, TEXT("UEBlueprintYamlConverter: Shutdown"));
}

IMPLEMENT_MODULE(FUEBlueprintYamlConverterModule, UEBlueprintYamlConverter)
