# Troubleshooting

## `make: command not found`

Install one of:
- Git for Windows (includes `make` in Git Bash)
- Chocolatey package `make`

Then run commands from a shell where `make` is available.

## UnrealEditor-Cmd not found

Check the path in `Makefile` and in tests:
- `Makefile` (`UE_EDITOR`)
- `Tests/Integration/test_commandlets.py` (`UE_EDITOR`)

If your Unreal install path differs, update both locations.

## Plugin not detected in test project

Ensure `Tests/Data/UEProject/Plugins/UEBlueprintYamlConverter` points to:
- `Plugins/UEBlueprintYamlConverter`

If needed, recreate the symlink with elevated permissions.

## Minimal test reports missing DLL or `.sln`

`Tests/Integration/test_minimal.py` checks for build artifacts. If they are missing:
- generate/build the UE test project first
- run commandlets once to trigger plugin compile

This is an environment readiness issue, not necessarily a code defect.

## pytest not found

Install pytest:

```bash
pip install pytest
```

Then rerun:

```bash
make test-python
```

## `tsc` not recognized in `ue_yaml_tools`

Install dependencies first:

```bash
cd ue_yaml_tools
npm install --cache .npm-cache
npm run build
```

## npm install permission errors on global cache (EPERM)

Use local cache inside the repo:

```bash
npm install --cache .npm-cache
```

## YAML validation/import mismatch

Preferred schema uses `blueprints:`. Legacy `blueprint:` is accepted for compatibility, but new files should use `blueprints:` to avoid ambiguity.

## Export/import type mismatch after round-trip

Current MVP behavior may map some Unreal types imperfectly (for example around `real` and `float`). Keep tests around your expected variable types and verify manifests/exports after changes.

## Output files are missing but commandlet started

Check:
- commandlet exit code
- generated manifest JSON
- UE log output in `Tests/Data/UEProject/Saved/Logs`
- output directory write permissions
