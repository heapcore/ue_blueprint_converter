# Architecture

## Purpose

UE Blueprint YAML Converter is an Unreal Engine editor plugin for bidirectional conversion:
- YAML to Blueprint assets (`YamlBuild` commandlet)
- Blueprint assets to YAML (`BlueprintToYaml` commandlet)
- Level export to YAML (`LevelToYaml` commandlet)

The main use cases are version control diffs, automation, and scripted asset generation.

## Repository Layout

- `Plugins/UEBlueprintYamlConverter/`: Unreal plugin source and descriptor
- `Tests/Integration/`: pytest-based black-box commandlet tests
- `Tests/Unit/`: standalone C++ utility tests
- `Tests/Data/`: input YAML, expected output, generated output, and test UE project
- `Examples/`: sample YAML files
- `Scripts/`: helper scripts (for example, advanced YAML generation)
- `ue_yaml_tools/`: TypeScript CLI wrapper around commandlets

## Runtime Model

### YamlBuild

`YamlBuild` reads a YAML file and creates assets in the test project:
- validates required structure (`version`, blueprint section)
- extracts blueprint name and variables
- creates a world map if missing
- creates a basic actor blueprint if missing
- writes a JSON manifest with success/errors and generated assets

Current parser behavior is intentionally minimal and string-based. It is suitable for MVP workflows but not full YAML semantics.

### BlueprintToYaml

`BlueprintToYaml` scans blueprint assets and exports:
- blueprint metadata (name, type)
- variable list with Unreal pin categories
- event graph node titles (MVP-level graph output)

### LevelToYaml

`LevelToYaml` loads a level and exports:
- level metadata and gravity
- actor transforms
- basic actor/light/static-mesh properties
- export manifest with generated YAML file paths

## YAML Contract

Canonical blueprint format:

```yaml
version: 1
blueprints:
  - name: BP_Example
    parent: /Script/Engine.Actor
    variables: []
    components: []
    functions: []
    event_graph: {}
```

Backward compatibility:
- legacy singular `blueprint:` is accepted by validation/import paths
- prefer `blueprints:` for new files

Level export format:

```yaml
version: 1
level:
  name: L_Example
  world_settings:
    gravity: [0, 0, -980]
actors: []
```

## Testing Strategy

- Integration tests: run UnrealEditor-Cmd commandlets as external processes
- Unit tests: lightweight C++ checks with no UE runtime dependencies
- Minimal smoke test: project structure, YAML test data, and build artifacts presence

## Known Limitations

- Advanced blueprint graph reconstruction is not fully implemented.
- YAML parsing in `YamlBuild` is intentionally simplified.
- Type round-trip can still be lossy for some Unreal categories outside the current mapping.
- Docs and tests target UE 5.6 test environment.

## Development Direction

Short term:
- strengthen type mapping and schema consistency
- improve parser reliability and diagnostics
- keep commandlet behavior deterministic for CI

Mid term:
- richer node/function/component round-trip support
- broader level import/export coverage
