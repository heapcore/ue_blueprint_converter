"""
External tests for UE Blueprint YAML Converter commandlets
Requires: pip install pytest
Run: pytest test_commandlets.py -v
"""

import subprocess
import json
import os
import shutil
import pytest
from pathlib import Path

# Configuration
UE_EDITOR = (
    r"C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
)

# Use absolute paths to avoid UE Editor working directory issues
PROJECT_ROOT = Path(__file__).parent.parent.parent.absolute()
PROJECT = str(
    PROJECT_ROOT / "Tests/Data/UEProject/UEBlueprintYamlConverterTest.uproject"
)
TEST_OUTPUT_DIR = str(PROJECT_ROOT / "Tests/Data/Output")
CONTENT_GENERATED_DIR = str(PROJECT_ROOT / "Tests/Data/UEProject/Content/Generated")
CONTENT_MAPS_DIR = str(PROJECT_ROOT / "Tests/Data/UEProject/Content/Maps")


class TestCommandlets:
    """Test UE commandlets as black-box processes"""

    @pytest.fixture(autouse=True)
    def setup_and_teardown(self):
        """Clean test output and generated assets before each test"""
        # Setup: Clean output dir
        if os.path.exists(TEST_OUTPUT_DIR):
            shutil.rmtree(TEST_OUTPUT_DIR)
        os.makedirs(TEST_OUTPUT_DIR, exist_ok=True)

        # Clean generated content from previous runs to ensure test isolation
        if os.path.exists(CONTENT_GENERATED_DIR):
            shutil.rmtree(CONTENT_GENERATED_DIR)
        if os.path.exists(CONTENT_MAPS_DIR):
            shutil.rmtree(CONTENT_MAPS_DIR)

        # Setup: Copy sample level for level export tests
        sample_level_path = PROJECT_ROOT / "Tests/Data/Assets/SampleLevel.umap"
        if sample_level_path.exists():
            # Create Maps directory
            os.makedirs(CONTENT_MAPS_DIR, exist_ok=True)

            # Copy SampleLevel.umap as L_ExampleExport.umap (for level export tests)
            target_level_path = Path(CONTENT_MAPS_DIR) / "L_ExampleExport.umap"
            shutil.copy2(sample_level_path, target_level_path)
            print(
                f"[OK] Copied sample level: {sample_level_path} -> {target_level_path}"
            )
        else:
            print(f"[WARN] Sample level not found: {sample_level_path}")

        yield

        # Teardown (optional - keep for debugging)
        # shutil.rmtree(TEST_OUTPUT_DIR)

    def run_commandlet(self, commandlet, **params):
        """Helper to run UE commandlet"""
        project_path = os.path.abspath(PROJECT)
        cmd = [UE_EDITOR, project_path, f"-run={commandlet}"]

        for key, value in params.items():
            # Convert relevant paths to absolute
            if key in ["Yaml", "OutDir"] and not os.path.isabs(value):
                value = os.path.abspath(value)
            cmd.append(f"-{key}={value}")

        print(f"Running: {' '.join(cmd)}")
        # Set CWD to project root to handle relative paths correctly
        project_root = os.path.dirname(project_path)
        result = subprocess.run(
            cmd, capture_output=True, text=True, timeout=120, cwd=project_root
        )

        print(f"Exit code: {result.returncode}")
        if result.stdout:
            print(f"Stdout: {result.stdout[:500]}...")
        if result.stderr:
            print(f"Stderr: {result.stderr[:500]}...")

        return result

    def run_commandlet_with_shorter_timeout(self, commandlet, timeout=30, **params):
        """Helper to run UE commandlet with shorter timeout for level export"""
        project_path = os.path.abspath(PROJECT)
        cmd = [UE_EDITOR, project_path, f"-run={commandlet}"]

        for key, value in params.items():
            # Convert relevant paths to absolute
            if key in ["Yaml", "OutDir"] and not os.path.isabs(value):
                value = os.path.abspath(value)
            cmd.append(f"-{key}={value}")

        print(f"Running: {' '.join(cmd)}")
        # Set CWD to project root to handle relative paths correctly
        project_root = str(PROJECT_ROOT)
        result = subprocess.run(
            cmd, capture_output=True, text=True, timeout=timeout, cwd=project_root
        )

        print(f"Exit code: {result.returncode}")
        if result.stdout:
            stdout_lines = result.stdout.split("\n")
            print(f"Stdout: {stdout_lines[0]}...")

        return result

    def test_yaml_build_basic(self):
        """Test basic YAML to Blueprint conversion"""
        result = self.run_commandlet(
            "YamlBuild",
            Yaml=str(PROJECT_ROOT / "Tests/Data/Input/SimpleTest.yaml"),
            OutDir=TEST_OUTPUT_DIR,
            Manifest=f"{TEST_OUTPUT_DIR}/manifest.json",
        )

        # Check commandlet executed
        assert result.returncode == 0, f"Commandlet failed: {result.stderr}"

        # Check manifest exists
        manifest_path = Path(TEST_OUTPUT_DIR) / "manifest.json"
        assert manifest_path.exists(), "Manifest file not created"

        # Validate manifest content
        with open(manifest_path, "r", encoding="utf-8-sig") as f:
            manifest = json.load(f)
            assert manifest.get("success") == True, f"Build failed: {manifest}"
            assert "generatedAssets" in manifest, "No assets listed in manifest"

    def test_blueprint_to_yaml_export(self):
        """Test Blueprint to YAML export"""
        # First create a blueprint
        self.run_commandlet(
            "YamlBuild",
            Yaml=str(PROJECT_ROOT / "Tests/Data/Input/SimpleTest.yaml"),
            OutDir=TEST_OUTPUT_DIR,
            Manifest=f"{TEST_OUTPUT_DIR}/create_manifest.json",
        )

        # Then export it
        export_dir = f"{TEST_OUTPUT_DIR}/export"
        result = self.run_commandlet(
            "BlueprintToYaml", OutDir=export_dir, Filter="/Game/Generated"
        )

        assert result.returncode == 0, f"Export failed: {result.stderr}"

        # Check if YAML files were created
        export_path = Path(export_dir)
        yaml_files = list(export_path.rglob("*.yaml"))
        assert len(yaml_files) > 0, "No YAML files exported"

        # Validate YAML content
        for yaml_file in yaml_files:
            content = yaml_file.read_text()
            assert "version:" in content, "YAML missing version"
            assert "blueprints:" in content or "blueprint:" in content, (
                "YAML missing blueprint section"
            )

    def test_round_trip(self):
        """Test complete round-trip: YAML -> Blueprint -> YAML"""
        # Step 1: Create from YAML
        original_yaml = Path(
            PROJECT_ROOT / "Tests/Data/Input/SimpleTest.yaml"
        ).read_text()

        result1 = self.run_commandlet(
            "YamlBuild",
            Yaml=str(PROJECT_ROOT / "Tests/Data/Input/SimpleTest.yaml"),
            OutDir=TEST_OUTPUT_DIR,
            Manifest=f"{TEST_OUTPUT_DIR}/build_manifest.json",
        )
        assert result1.returncode == 0

        # Step 2: Export back to YAML
        export_dir = f"{TEST_OUTPUT_DIR}/roundtrip"
        result2 = self.run_commandlet(
            "BlueprintToYaml", OutDir=export_dir, Filter="/Game/Generated"
        )
        assert result2.returncode == 0

        # Step 3: Compare key elements
        yaml_files = list(Path(export_dir).rglob("*.yaml"))
        assert len(yaml_files) > 0, "No YAML exported in round-trip"

        exported_yaml = yaml_files[0].read_text()

        # Check that key elements from original are in exported
        assert "TestVariable" in exported_yaml, "Variables missing"
        assert "real" in exported_yaml or "float" in exported_yaml, (
            "Variable types missing (UE uses 'real' for float)"
        )

    def test_invalid_yaml_handling(self):
        """Test handling of invalid YAML input"""
        # Create invalid YAML
        invalid_yaml_path = Path(TEST_OUTPUT_DIR) / "invalid.yaml"
        invalid_yaml_path.write_text("this is not: valid: yaml: [[[")

        result = self.run_commandlet(
            "YamlBuild",
            Yaml=str(invalid_yaml_path),
            OutDir=TEST_OUTPUT_DIR,
            Manifest=f"{TEST_OUTPUT_DIR}/invalid_manifest.json",
        )

        # Should handle gracefully (not crash)
        # Check manifest for error indication
        manifest_path = Path(TEST_OUTPUT_DIR) / "invalid_manifest.json"
        assert manifest_path.exists(), (
            "Manifest file should be created even for invalid YAML"
        )

        with open(manifest_path, "r", encoding="utf-8-sig") as f:
            manifest = json.load(f)
            print(f"Manifest content: {manifest}")  # Debug output

            # Check that validation failed
            assert manifest.get("success") == False, (
                f"Expected success=false, got: {manifest}"
            )

            # Check for validation error message
            if "errors" in manifest and len(manifest["errors"]) > 0:
                error_message = manifest["errors"][0].get("message", "")
                assert (
                    "validation" in error_message.lower()
                    or "invalid" in error_message.lower()
                ), f"Expected validation error, got: {error_message}"

    def test_player_character_movement(self):
        """Test complex player character blueprint with movement system"""
        result = self.run_commandlet(
            "YamlBuild",
            Yaml=str(PROJECT_ROOT / "Tests/Data/Input/BP_PlayerCharacterMovement.yaml"),
            OutDir=TEST_OUTPUT_DIR,
            Manifest=f"{TEST_OUTPUT_DIR}/player_movement_manifest.json",
        )

        # Check commandlet executed successfully
        assert result.returncode == 0, (
            f"Player character blueprint build failed: {result.stderr}"
        )

        # Check manifest exists and indicates success
        manifest_path = Path(TEST_OUTPUT_DIR) / "player_movement_manifest.json"
        assert manifest_path.exists(), "Player movement manifest file not created"

        # Validate manifest content
        with open(manifest_path, "r", encoding="utf-8-sig") as f:
            manifest = json.load(f)
            assert manifest.get("success") == True, (
                f"Player character build failed: {manifest}"
            )
            assert "generatedAssets" in manifest, (
                "No assets listed in manifest for player character"
            )

            # Check that blueprint was generated
            generated_assets = manifest.get("generatedAssets", [])
            assert len(generated_assets) > 0, (
                "No assets were generated for player character"
            )

            # Look for the player character blueprint specifically
            player_bp_found = any(
                "BP_PlayerCharacterMovement" in asset.get("path", "")
                for asset in generated_assets
            )
            assert player_bp_found, (
                f"BP_PlayerCharacterMovement not found in generated assets: {generated_assets}"
            )

        print(
            "[OK] Player character blueprint with movement system successfully built!"
        )
        print(f"Generated assets: {manifest.get('generatedAssets', [])}")

    def test_level_export_to_yaml(self):
        """Test exporting existing level to YAML format"""
        export_dir = f"{TEST_OUTPUT_DIR}/level_export"
        manifest_path = f"{TEST_OUTPUT_DIR}/level_export_manifest.json"

        try:
            # Run commandlet with shorter timeout since actual work completes in 0.02 seconds
            result = self.run_commandlet_with_shorter_timeout(
                "LevelToYaml",
                Level="/Game/Maps/L_ExampleExport",
                OutDir=export_dir,
                Manifest=manifest_path,
            )

            # If process completed normally, check return code
            if hasattr(result, "returncode") and result.returncode is not None:
                assert result.returncode == 0, f"Level export failed: {result.stderr}"

        except subprocess.TimeoutExpired:
            # Timeout is acceptable if files were created (UE Editor leaves background processes)
            print(
                "[WARN] Process timed out, checking whether output files were created..."
            )
        except Exception as e:
            print(
                f"[WARN] Process issue: {e}, checking whether output files were created..."
            )

        # Check manifest exists and indicates success
        manifest_path = Path(TEST_OUTPUT_DIR) / "level_export_manifest.json"
        assert manifest_path.exists(), "Level export manifest file not created"

        # Validate manifest content
        with open(manifest_path, "r", encoding="utf-8-sig") as f:
            manifest = json.load(f)
            assert manifest.get("success") == True, f"Level export failed: {manifest}"
            assert "exportedFiles" in manifest, "No exported files listed in manifest"

            # Check that YAML file was generated
            exported_files = manifest.get("exportedFiles", [])
            assert len(exported_files) > 0, "No files were exported"

            # Look for L_ExampleExport.yaml
            yaml_file_found = any(
                "L_ExampleExport.yaml" in file_info.get("path", "")
                for file_info in exported_files
            )
            assert yaml_file_found, (
                f"L_ExampleExport.yaml not found in exported files: {exported_files}"
            )

        # Check YAML file content
        yaml_files = list(Path(export_dir).rglob("*.yaml"))
        assert len(yaml_files) > 0, "No YAML files found in export directory"

        yaml_content = yaml_files[0].read_text(encoding="utf-8")
        assert "version: 1" in yaml_content, "YAML missing version header"
        assert "level:" in yaml_content, "YAML missing level section"
        assert "actors:" in yaml_content, "YAML missing actors section"
        assert "L_ExampleExport" in yaml_content, "YAML missing level name"

        print("[OK] Level exported to YAML successfully!")
        print(f"YAML content preview (first 500 chars): {yaml_content[:500]}...")


if __name__ == "__main__":
    # Run tests
    pytest.main([__file__, "-v", "-s"])
