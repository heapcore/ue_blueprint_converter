# ue_blueprint_converter

> **WARNING:** This repository may be unstable or non-functional. Use at your own risk.

Unreal Engine editor plugin for converting Blueprint data to and from YAML.

## Scope

Current commandlets:
- `YamlBuild`: imports YAML and creates basic Blueprint/World assets with manifest output.
- `BlueprintToYaml`: exports Blueprint metadata, variables, and basic event graph node titles to YAML.
- `LevelToYaml`: exports level metadata and actor data to YAML.

Current status:
- Working for MVP workflows (metadata, variables, manifests, basic level export).
- Advanced graph/function/component reconstruction is partial and still evolving.

## Repository Layout

- `Plugins/UEBlueprintYamlConverter/`: plugin source code.
- `Tests/`: integration tests, unit tests, test data, and UE test project.
- `Examples/`: sample YAML files.
- `Scripts/`: helper scripts.
- `Docs/`: architecture, quickstart, troubleshooting, changelog.

## Requirements

- Unreal Engine 5.6
- Python 3 + `pytest`
- `make` (Git for Windows or Chocolatey)
- Visual Studio (for C++ test/build workflows)

## Quick Start

1. Ensure plugin symlink exists in test project:

```powershell
New-Item -ItemType SymbolicLink -Path "Tests\Data\UEProject\Plugins\UEBlueprintYamlConverter" -Target "Plugins\UEBlueprintYamlConverter"
```

2. Verify environment:

```bash
make check-env
```

3. Run tests:

```bash
make test-minimal
make test-python
```

See `Docs/QUICKSTART.md` for full commands.

## Manual Usage

YAML to Blueprint:

```bash
UnrealEditor-Cmd.exe "Tests/Data/UEProject/UEBlueprintYamlConverterTest.uproject" -run=YamlBuild -Yaml="D:/.../SimpleTest.yaml" -OutDir="D:/.../Output" -Manifest="D:/.../manifest.json"
```

Blueprint to YAML:

```bash
UnrealEditor-Cmd.exe "Tests/Data/UEProject/UEBlueprintYamlConverterTest.uproject" -run=BlueprintToYaml -OutDir="D:/.../export" -Filter="/Game/Generated"
```

Level to YAML:

```bash
UnrealEditor-Cmd.exe "Tests/Data/UEProject/UEBlueprintYamlConverterTest.uproject" -run=LevelToYaml -Level="/Game/Maps/L_ExampleExport" -OutDir="D:/.../level_export" -Manifest="D:/.../level_export_manifest.json"
```

## Documentation

- `Docs/ARCHITECTURE.md`
- `Docs/QUICKSTART.md`
- `Docs/TROUBLESHOOTING.md`
- `Docs/CHANGELOG.md`

## License

See `LICENSE`.
