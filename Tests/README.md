# Tests Directory

Organized structure of all tests for the UE Blueprint YAML Converter project.

## Structure

```
Tests/
+-- Integration/             # Integration tests (full UE pipeline)
|   +-- test_commandlets.py  # UE commandlet tests via subprocess
+-- Unit/                    # Unit tests (isolated components)
|   +-- test_minimal.py      # Basic compilation/existence checks
|   +-- simple_cpp_test.cpp  # C++ tests without dependencies
+-- Data/                    # All test data centralized
|   +-- Input/               # Input test data
|   |   +-- SimpleTest.yaml  # Simple YAML for testing
|   +-- Expected/            # Expected test results
|   |   +-- expected_manifest.json
|   |   +-- expected_simple_blueprint.yaml
|   +-- Output/              # Generated test outputs
|   +-- UEProject/           # Self-contained test UE project
|       +-- UEBlueprintYamlConverterTest.uproject
|       +-- UEBlueprintYamlConverterTest.sln
|       +-- Source/
+-- README.md                # This file
```

## Setup

Before running tests, ensure:
1. Unreal Engine 5.6 is installed
2. Python with pytest is available (`pip install pytest`)
3. **Plugin symbolic link exists**:
   If `Tests/Data/UEProject/Plugins/UEBlueprintYamlConverter` doesn't work, recreate the symbolic link:
   ```powershell
   # Run PowerShell as Administrator from project root
   New-Item -ItemType SymbolicLink -Path "Tests\Data\UEProject\Plugins\UEBlueprintYamlConverter" -Target "Plugins\UEBlueprintYamlConverter"
   ```

## How to Run Tests

### Integration Tests
```bash
cd Tests/Integration
pytest test_commandlets.py -v
```

### Unit Tests
```bash
# Python tests
cd Tests/Unit
python test_minimal.py

# C++ tests
cd Tests/Unit
cl simple_cpp_test.cpp /std:c++17 /EHsc  # Windows
# or
g++ simple_cpp_test.cpp -std=c++17 -o simple_cpp_test  # Linux
./simple_cpp_test
```

### All Tests from Project Root
```bash
make test  # if corresponding target exists in Makefile
```

## Adding New Tests

1. **Integration tests** - add to `Integration/` for testing full pipeline
2. **Unit tests** - add to `Unit/` for testing isolated components
3. **Test data** - place in `Data/Input/`
4. **Expected results** - place in `Data/Expected/`

## Conventions

- Test files should start with `test_`
- Test data should have meaningful names
- Each test should be independent and isolated
- Use relative paths from the test directory
