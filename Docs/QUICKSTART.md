# Quickstart

## Prerequisites

- Windows machine
- Unreal Engine 5.6 installed
- Python 3
- `make` (Git for Windows or Chocolatey)
- Optional for CLI wrapper: Node.js 18+

Default editor path expected by tests and Makefile:

`C:/Program Files/Epic Games/UE_5.6/Engine/Binaries/Win64/UnrealEditor-Cmd.exe`

## 1. Prepare Plugin Link in Test Project

From project root, ensure this path exists:

`Tests/Data/UEProject/Plugins/UEBlueprintYamlConverter`

If it is missing, create a symlink in elevated PowerShell:

```powershell
New-Item -ItemType SymbolicLink `
  -Path "Tests\Data\UEProject\Plugins\UEBlueprintYamlConverter" `
  -Target "Plugins\UEBlueprintYamlConverter"
```

## 2. Verify Environment

```bash
make check-env
```

## 3. Run Fast Smoke Test

```bash
make test-minimal
```

## 4. Run Integration Tests

```bash
make test-python
```

Or directly:

```bash
python -m pytest Tests/Integration/test_commandlets.py -v -s
```

## 5. Manual Commandlet Runs

YAML to Blueprint:

```bash
"C:/Program Files/Epic Games/UE_5.6/Engine/Binaries/Win64/UnrealEditor-Cmd.exe" ^
  "Tests/Data/UEProject/UEBlueprintYamlConverterTest.uproject" ^
  -run=YamlBuild ^
  -Yaml="D:/YandexDisk/Projects/Legacy/ue_blueprint_converter/Tests/Data/Input/SimpleTest.yaml" ^
  -OutDir="D:/YandexDisk/Projects/Legacy/ue_blueprint_converter/Tests/Data/Output" ^
  -Manifest="D:/YandexDisk/Projects/Legacy/ue_blueprint_converter/Tests/Data/Output/manifest.json"
```

Blueprint to YAML:

```bash
"C:/Program Files/Epic Games/UE_5.6/Engine/Binaries/Win64/UnrealEditor-Cmd.exe" ^
  "Tests/Data/UEProject/UEBlueprintYamlConverterTest.uproject" ^
  -run=BlueprintToYaml ^
  -OutDir="D:/YandexDisk/Projects/Legacy/ue_blueprint_converter/Tests/Data/Output/export" ^
  -Filter="/Game/Generated"
```

Level to YAML:

```bash
"C:/Program Files/Epic Games/UE_5.6/Engine/Binaries/Win64/UnrealEditor-Cmd.exe" ^
  "Tests/Data/UEProject/UEBlueprintYamlConverterTest.uproject" ^
  -run=LevelToYaml ^
  -Level="/Game/Maps/L_ExampleExport" ^
  -OutDir="D:/YandexDisk/Projects/Legacy/ue_blueprint_converter/Tests/Data/Output/level_export" ^
  -Manifest="D:/YandexDisk/Projects/Legacy/ue_blueprint_converter/Tests/Data/Output/level_export_manifest.json"
```

## 6. Optional TypeScript CLI (`ue_yaml_tools`)

```bash
cd ue_yaml_tools
npm install --cache .npm-cache
npm run build
```

Then use:

```bash
node dist/cli.js --help
```
