# Changelog

## 2026-02-23

### Documentation
- Consolidated documentation into four files:
  - `ARCHITECTURE.md`
  - `QUICKSTART.md`
  - `TROUBLESHOOTING.md`
  - `CHANGELOG.md`
- Removed outdated and duplicated docs.
- Standardized docs to English-only content with no emoji usage.

### Tooling and Tests
- Fixed TypeScript runner plugin name:
  - `UEYamlPlugin` -> `UEBlueprintYamlConverter`
- Added a minimal preview server implementation used by CLI `preview` command.
- Added missing `open` dependency in `ue_yaml_tools`.
- Updated TypeScript imports for current library APIs (`yaml.parse`, `http.Server` typing).
- Fixed Python test console output to avoid non-ASCII symbols that can break CP1251 terminals.
- Fixed subprocess argument handling for `.uproject` path (removed embedded quote characters).

### YAML Compatibility
- Kept `blueprints:` as canonical format.
- Added backward compatibility support for legacy singular `blueprint:` in validation/import flow.

### Repository Hygiene
- Moved script `Examples/generate_advanced_blueprint.py` to `Scripts/generate_advanced_blueprint.py`.
- Updated `.gitignore` to exclude Node tooling artifacts in `ue_yaml_tools`:
  - `node_modules`
  - local npm cache
  - build output
