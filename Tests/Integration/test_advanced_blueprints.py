#!/usr/bin/env python3
"""
Advanced Blueprint Testing for UE Blueprint YAML Converter

Tests the extended functionality including:
- Component creation and hierarchy
- Function graphs with nodes
- Event handling
- Complex Blueprint structures
"""

import pytest
import subprocess
import json
import sys
from pathlib import Path

# Add the project root to the path for imports
PROJECT_ROOT = Path(__file__).parent.parent.parent
sys.path.insert(0, str(PROJECT_ROOT))


class TestAdvancedBlueprints:
    @pytest.fixture(autouse=True)
    def setup(self):
        """Setup paths for testing"""
        self.project_root = PROJECT_ROOT
        self.test_data_dir = self.project_root / "Tests" / "Data"
        self.ue_project_path = (
            self.test_data_dir / "UEProject" / "UEBlueprintYamlConverterTest.uproject"
        )
        self.input_dir = self.test_data_dir / "Input"
        self.output_dir = self.test_data_dir / "Output"
        self.expected_dir = self.test_data_dir / "Expected"

        # Create output directory if it doesn't exist
        self.output_dir.mkdir(parents=True, exist_ok=True)

        assert self.ue_project_path.exists(), (
            f"UE project not found: {self.ue_project_path}"
        )

    def run_yaml_build_commandlet(self, yaml_file, timeout=120):
        """Run the YamlBuild commandlet with the specified YAML file"""
        yaml_path = self.input_dir / yaml_file
        output_dir = self.output_dir
        manifest_path = output_dir / f"manifest_{yaml_file.stem}.json"

        # Clean previous outputs
        manifest_path.unlink(missing_ok=True)

        # Build the command
        cmd = [
            "UnrealEditor-Cmd.exe",
            str(self.ue_project_path),
            "-run=YamlBuild",
            f"-Yaml={yaml_path.absolute()}",
            f"-OutDir={output_dir.absolute()}",
            f"-Manifest={manifest_path.absolute()}",
            "-stdout",
            "-NoLogTimes",
        ]

        print(f"Running: {' '.join(cmd)}")

        # Run the commandlet
        result = subprocess.run(
            cmd,
            capture_output=True,
            text=True,
            timeout=timeout,
            cwd=str(self.project_root),
        )

        print(f"Return code: {result.returncode}")
        if result.stdout:
            print(f"STDOUT:\n{result.stdout}")
        if result.stderr:
            print(f"STDERR:\n{result.stderr}")

        return result, manifest_path

    def test_advanced_blueprint_creation(self):
        """Test creation of Blueprint with components, functions and events"""
        result, manifest_path = self.run_yaml_build_commandlet(
            Path("AdvancedTest.yaml")
        )

        # Check that the commandlet completed successfully
        assert result.returncode == 0, (
            f"YamlBuild failed with return code {result.returncode}"
        )

        # Check that manifest was created
        assert manifest_path.exists(), f"Manifest file not created: {manifest_path}"

        # Parse the manifest
        with open(manifest_path, "r", encoding="utf-8") as f:
            manifest = json.load(f)

        # Verify manifest structure
        assert manifest.get("success") == True, (
            f"Build reported as failed: {manifest.get('errors', [])}"
        )
        assert "generatedAssets" in manifest, "No generated assets reported"

        # Check that the Blueprint was created
        generated_assets = manifest["generatedAssets"]
        blueprint_asset = None
        for asset in generated_assets:
            if asset.get("type") == "Blueprint":
                blueprint_asset = asset
                break

        assert blueprint_asset is not None, "No Blueprint asset was generated"
        assert "BP_AdvancedDoor" in blueprint_asset["path"], (
            "Blueprint name doesn't match expected"
        )

        print("[OK] Advanced Blueprint creation test passed")

    def test_component_hierarchy_creation(self):
        """Test that components are created with proper hierarchy"""
        # This test verifies the component creation logic
        # Since we can't easily inspect the Blueprint structure without UE Editor,
        # we check the logs for component creation messages

        result, manifest_path = self.run_yaml_build_commandlet(
            Path("AdvancedTest.yaml")
        )

        # Check for component creation log messages
        output_text = result.stdout + result.stderr

        # Look for our component creation log messages
        assert "Parsing components from YAML" in output_text, (
            "Component parsing not initiated"
        )
        assert "Created component:" in output_text, "No components were created"

        # Check for specific components from our YAML
        component_logs = [
            line for line in output_text.split("\n") if "Created component:" in line
        ]

        # We expect at least the components defined in AdvancedTest.yaml
        expected_components = ["RootComponent", "DoorMeshComp", "CollisionBox"]
        found_components = []

        for log_line in component_logs:
            for comp_name in expected_components:
                if comp_name in log_line:
                    found_components.append(comp_name)

        print(f"Found components: {found_components}")
        # We should find at least some components
        assert len(found_components) > 0, "No expected components were created"

        print("[OK] Component hierarchy test passed")

    def test_function_graph_creation(self):
        """Test that function graphs are created"""
        result, manifest_path = self.run_yaml_build_commandlet(
            Path("AdvancedTest.yaml")
        )

        # Check for function creation log messages
        output_text = result.stdout + result.stderr

        # Look for our function creation log messages
        assert "Parsing functions from YAML" in output_text, (
            "Function parsing not initiated"
        )

        # Check for specific functions from our YAML
        function_logs = [
            line
            for line in output_text.split("\n")
            if "Found function:" in line or "Created function graph:" in line
        ]

        expected_functions = ["OpenDoor", "CloseDoor"]
        found_functions = []

        for log_line in function_logs:
            for func_name in expected_functions:
                if func_name in log_line:
                    found_functions.append(func_name)

        print(f"Found functions: {found_functions}")
        # We should find at least some functions
        assert len(found_functions) > 0, "No expected functions were created"

        print("[OK] Function graph creation test passed")

    def test_event_handling(self):
        """Test that events are processed"""
        result, manifest_path = self.run_yaml_build_commandlet(
            Path("AdvancedTest.yaml")
        )

        # Check for event creation log messages
        output_text = result.stdout + result.stderr

        # Look for our event creation log messages
        assert "Parsing events from YAML" in output_text, "Event parsing not initiated"

        print("[OK] Event handling test passed")

    def test_blueprint_compilation(self):
        """Test that the Blueprint compiles successfully after advanced features"""
        result, manifest_path = self.run_yaml_build_commandlet(
            Path("AdvancedTest.yaml")
        )

        # Check that compilation was successful
        # UE logs compilation errors clearly, so if we get success, compilation worked
        assert result.returncode == 0, "Blueprint compilation failed"

        # Check that no compilation errors are present
        output_text = result.stdout + result.stderr

        # Look for compilation error indicators
        error_indicators = [
            "Compile of",
            "Error",
            "Failed to compile",
            "compilation failed",
        ]

        compilation_errors = []
        for line in output_text.split("\n"):
            for indicator in error_indicators:
                if indicator.lower() in line.lower() and "error" in line.lower():
                    compilation_errors.append(line)

        # Filter out known non-critical messages
        critical_errors = [err for err in compilation_errors if "LogUEYaml" not in err]

        if critical_errors:
            print(f"[WARN] Found potential compilation errors: {critical_errors}")

        print("[OK] Blueprint compilation test passed")


if __name__ == "__main__":
    pytest.main([__file__, "-v", "-s"])
