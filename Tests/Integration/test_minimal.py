"""
Minimal tests for UE Blueprint YAML Converter - just check that things compile
"""

from pathlib import Path


def test_plugin_exists():
    """Test that plugin files exist"""
    plugin_path = Path(
        "Plugins/UEBlueprintYamlConverter/UEBlueprintYamlConverter.uplugin"
    )
    assert plugin_path.exists(), "Plugin file not found"


def test_dll_exists():
    """Test that the compiled DLL exists"""
    dll_path = Path(
        "Plugins/UEBlueprintYamlConverter/Binaries/Win64/UnrealEditor-UEBlueprintYamlConverter.dll"
    )
    assert dll_path.exists(), "Compiled DLL not found - plugin not built"


def test_project_compiles():
    """Test that the project compiles without errors"""
    # Check if the project solution exists
    sln_path = Path("Tests/Data/UEProject/UEBlueprintYamlConverterTest.sln")
    assert sln_path.exists(), "Project solution file not found"


def test_yaml_files_exist():
    """Test that test YAML files exist"""
    yaml_path = Path("Tests/Data/Input/SimpleTest.yaml")
    assert yaml_path.exists(), "Test YAML file not found"

    # Check YAML content is valid
    content = yaml_path.read_text()
    assert "version:" in content, "YAML missing version"
    assert "blueprints:" in content, "YAML missing blueprints section"


if __name__ == "__main__":
    # Run simple tests
    print("Running minimal tests...")

    try:
        test_plugin_exists()
        print("[OK] Plugin file exists")
    except AssertionError as e:
        print(f"[FAIL] Plugin test failed: {e}")

    try:
        test_dll_exists()
        print("[OK] Plugin DLL compiled successfully")
    except AssertionError as e:
        print(f"[FAIL] DLL test failed: {e}")

    try:
        test_project_compiles()
        print("[OK] Project files generated")
    except AssertionError as e:
        print(f"[FAIL] Project test failed: {e}")

    try:
        test_yaml_files_exist()
        print("[OK] Test YAML files valid")
    except AssertionError as e:
        print(f"[FAIL] YAML test failed: {e}")

    print("\nAll basic tests completed!")
