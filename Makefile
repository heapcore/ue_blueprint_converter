.PHONY: test test-manual clean help build test-cpp test-python test-all test-minimal debug-build show-logs

# Windows-only Makefile for UE Blueprint YAML Converter
UE_EDITOR := "C:/Program Files/Epic Games/UE_5.6/Engine/Binaries/Win64/UnrealEditor-Cmd.exe"
PROJECT_PATH := $(shell cd)

help: ## Show this help message
	@echo UE Blueprint YAML Converter - Windows Commands:
	@echo UE Editor: $(UE_EDITOR)
	@echo.
	@echo Quick Start:
	@echo   make test-minimal     # Check compilation (FASTEST)
	@echo   make test-python      # Run Python tests
	@echo   make test-cpp         # Run C++ tests
	@echo   make test-all         # Run all tests
	@echo.
	@echo Available commands:
	@echo   build                 # Build the plugin
	@echo   test                  # Run UE automation tests
	@echo   test-manual           # Manual YAML test
	@echo   export-test           # Export blueprints to YAML
	@echo   clean                 # Alias for clean-generated (default cleanup)
	@echo   clean-logs            # Clean only logs and cache files
	@echo   clean-generated       # Clean ALL generated files (includes logs)
	@echo   clean-all             # NUCLEAR cleanup including saved settings
	@echo   check-env             # Check environment
	@echo   install-pytest        # Install pytest
	@echo   debug-build           # Build with detailed logs and show results
	@echo   show-logs             # Show recent compilation/build logs

test: ## Run automation tests
	@echo Running UE Blueprint YAML Converter automation tests...
	@if not exist Tests/Data/Output mkdir Tests\Data\Output
	$(UE_EDITOR) "$(PROJECT_PATH)/Tests/Data/UEProject/UEBlueprintYamlConverterTest.uproject" -ExecCmds="Automation RunTests UEBlueprintYamlConverter" -TestExit="Automation Test Queue Empty" -ReportOutputDir="$(PROJECT_PATH)/Tests/Data/Output" -log
	@echo Test results saved to Tests/Data/Output/

test-manual: ## Run manual round-trip test with test YAML
	@echo Running manual round-trip test...
	@if not exist Tests\Data\Output mkdir Tests\Data\Output
	$(UE_EDITOR) "$(PROJECT_PATH)/Tests/Data/UEProject/UEBlueprintYamlConverterTest.uproject" -run=YamlBuild -Yaml="$(PROJECT_PATH)/Tests/Data/Input/SimpleTest.yaml" -OutDir="$(PROJECT_PATH)/Tests/Data/Output" -Manifest="$(PROJECT_PATH)/Tests/Data/Output/manifest.json"
	@echo Manual test completed. Check Tests/Data/Output/ for results.

export-test: ## Export test blueprint to YAML
	@echo Exporting test blueprints to YAML...
	@if not exist Tests\Data\Output mkdir Tests\Data\Output
	$(UE_EDITOR) "$(PROJECT_PATH)/Tests/Data/UEProject/UEBlueprintYamlConverterTest.uproject" -run=BlueprintToYaml -OutDir="$(PROJECT_PATH)/Tests/Data/Output" -Filter="/Game/Generated"
	@echo Export completed. Check Tests/Data/Output/ for YAML files.

clean-logs: ## Clean only logs, cache, and temp files (keep builds)
	@echo [1/4] Cleaning logs and crashes...
	@if exist Tests\Data\UEProject\Saved\Logs rmdir /s /q Tests\Data\UEProject\Saved\Logs 2>nul
	@if exist Tests\Data\UEProject\Saved\Crashes rmdir /s /q Tests\Data\UEProject\Saved\Crashes 2>nul
	@if exist Tests\Data\UEProject\Saved\AutoScreenshot.png del /f /q Tests\Data\UEProject\Saved\AutoScreenshot.png 2>nul

	@echo [2/4] Cleaning test outputs...
	@if exist Tests\Data\Output rmdir /s /q Tests\Data\Output 2>nul
	@if not exist Tests\Data\Output mkdir Tests\Data\Output

	@echo [3/4] Cleaning Python cache...
	@if exist Tests\Integration\__pycache__ rmdir /s /q Tests\Integration\__pycache__ 2>nul
	@if exist Tests\Unit\__pycache__ rmdir /s /q Tests\Unit\__pycache__ 2>nul
	@if exist .pytest_cache rmdir /s /q .pytest_cache 2>nul
	@if exist Tests\Unit\simple_cpp_test.exe del /f /q Tests\Unit\simple_cpp_test.exe 2>nul

	@echo [4/4] Cleaning temp files...
	@if exist *.tmp del /f /q *.tmp 2>nul
	@if exist Tests\Data\UEProject\*.tmp del /f /q Tests\Data\UEProject\*.tmp 2>nul
	@echo Logs and cache cleaned.

clean-generated: ## Clean ALL generated files for fresh rebuild (includes logs)
	@echo ============================================
	@echo   CLEANING ALL GENERATED FILES
	@echo ============================================

	@echo [1/4] Running log cleanup first...
	@$(MAKE) -s clean-logs

	@echo [2/4] Cleaning plugin binaries and intermediate files...
	@if exist Plugins\UEBlueprintYamlConverter\Binaries rmdir /s /q Plugins\UEBlueprintYamlConverter\Binaries 2>nul
	@if exist Plugins\UEBlueprintYamlConverter\Intermediate rmdir /s /q Plugins\UEBlueprintYamlConverter\Intermediate 2>nul

	@echo [3/4] Cleaning UE project generated files...
	@if exist Tests\Data\UEProject\Binaries rmdir /s /q Tests\Data\UEProject\Binaries 2>nul
	@if exist Tests\Data\UEProject\Intermediate rmdir /s /q Tests\Data\UEProject\Intermediate 2>nul
	@if exist Tests\Data\UEProject\DerivedDataCache rmdir /s /q Tests\Data\UEProject\DerivedDataCache 2>nul
	@if exist Tests\Data\UEProject\Content\Generated rmdir /s /q Tests\Data\UEProject\Content\Generated 2>nul
	@if not exist Tests\Data\UEProject\Content\Generated mkdir Tests\Data\UEProject\Content\Generated

	@echo [4/4] Cleaning VS/MSBuild artifacts...
	@if exist .vs rmdir /s /q .vs 2>nul
	@if exist Tests\Data\UEProject\.vs rmdir /s /q Tests\Data\UEProject\.vs 2>nul
	@if exist *.sdf del /f /q *.sdf 2>nul
	@if exist Tests\Data\UEProject\*.sdf del /f /q Tests\Data\UEProject\*.sdf 2>nul

	@echo ============================================
	@echo   ALL GENERATED FILES CLEANED
	@echo   Ready for fresh build/test cycle!
	@echo ============================================

clean-all: ## NUCLEAR cleanup - removes EVERYTHING including saved editor settings
	@echo ============================================
	@echo   NUCLEAR CLEANUP - EVERYTHING WILL BE REMOVED
	@echo ============================================
	@echo This will remove ALL generated files including editor settings!
	@echo Press Ctrl+C to cancel or any key to continue...
	@pause >nul

	@echo [1/6] Standard generated files cleanup...
	@$(MAKE) -s clean-generated

	@echo [2/6] Removing ALL UE saved data...
	@if exist Tests\Data\UEProject\Saved rmdir /s /q Tests\Data\UEProject\Saved 2>nul

	@echo [3/6] Removing ALL generated UE content...
	@if exist Tests\Data\UEProject\Content\Generated rmdir /s /q Tests\Data\UEProject\Content\Generated 2>nul
	@if exist Tests\Data\UEProject\Content\Maps rmdir /s /q Tests\Data\UEProject\Content\Maps 2>nul

	@echo [4/6] Removing build and project files...
	@if exist Tests\Data\UEProject\*.sln del /f /q Tests\Data\UEProject\*.sln 2>nul
	@if exist *.sln del /f /q *.sln 2>nul

	@echo [5/6] Removing lock files...
	@if exist *.lock del /f /q *.lock 2>nul
	@if exist Tests\Data\UEProject\*.lock del /f /q Tests\Data\UEProject\*.lock 2>nul

	@echo [6/6] Recreating essential directories...
	@if not exist Tests\Data\Output mkdir Tests\Data\Output
	@if not exist Tests\Data\UEProject\Content\Generated mkdir Tests\Data\UEProject\Content\Generated
	@if not exist Tests\Data\UEProject\Content\Maps mkdir Tests\Data\UEProject\Content\Maps

	@echo ============================================
	@echo   NUCLEAR CLEANUP COMPLETED
	@echo   Project is now PRISTINE - like fresh git clone
	@echo ============================================

build: ## Build the plugin (requires UE project to be set up)
	@echo Building UE Blueprint YAML Converter plugin...
	$(UE_EDITOR) "$(PROJECT_PATH)/Tests/Data/UEProject/UEBlueprintYamlConverterTest.uproject" -run=CompileAllBlueprints -ShowResultsOnly
	@echo Build completed.

check-env: ## Check environment and display paths
	@echo === Environment Check ===
	@echo UE Editor Path: $(UE_EDITOR)
	@echo Project Path: $(PROJECT_PATH)
	@echo.
	@echo Testing UE Editor accessibility...
	@if exist $(UE_EDITOR) (echo [OK] UE Editor found) else (echo [FAIL] UE Editor not found at $(UE_EDITOR))

test-cpp: ## Build and run simple C++ tests
	@echo Building and running C++ tests...
	@cd Tests\Unit && cl simple_cpp_test.cpp /std:c++17 /EHsc /Fe:simple_cpp_test.exe 2>nul || echo Note: Install Visual Studio for C++ tests
	@cd Tests\Unit && if exist simple_cpp_test.exe simple_cpp_test.exe

test-python: ## Run Python pytest tests
	@echo Running Python tests...
	@python -m pytest Tests/Integration/test_commandlets.py -v -s || (echo Install pytest: pip install pytest && exit 1)

test-minimal: ## Run minimal tests that check basic functionality
	@echo Running minimal tests...
	@python Tests/Integration/test_minimal.py

test-all: test-minimal test-python test-cpp ## Run all tests

install-pytest: ## Install pytest for Python testing
	@echo Installing pytest...
	@pip install pytest || echo pip not found. Install Python with pip first.

debug-build: ## Build plugin with detailed logging and show compilation results
	@echo ============================================
	@echo   DEBUG BUILD WITH DETAILED LOGS
	@echo ============================================
	@echo Starting build with maximum verbosity...
	@echo.
	@if not exist Tests\Data\Output mkdir Tests\Data\Output
	@echo Build starting at %date% %time% > Tests\Data\Output\build_debug.log
	$(UE_EDITOR) "$(PROJECT_PATH)/Tests/Data/UEProject/UEBlueprintYamlConverterTest.uproject" -run=CompileAllBlueprints -ShowResultsOnly -Verbose -Log -Timestamp >> Tests\Data\Output\build_debug.log 2>&1
	@echo Build completed at %date% %time% >> Tests\Data\Output\build_debug.log
	@echo.
	@echo ============================================
	@echo   BUILD LOG SUMMARY
	@echo ============================================
	@if exist Tests\Data\Output\build_debug.log type Tests\Data\Output\build_debug.log
	@echo.
	@echo ============================================
	@echo   CHECKING FOR UE LOGS
	@echo ============================================
	@if exist Tests\Data\UEProject\Saved\Logs (dir Tests\Data\UEProject\Saved\Logs\*.log /od 2>nul && echo Found UE log files) else echo No UE logs found
	@echo.
	@echo To see all logs: make show-logs

show-logs: ## Display recent compilation and UE logs
	@echo ============================================
	@echo   RECENT COMPILATION LOGS
	@echo ============================================
	@echo.
	@echo === BUILD DEBUG LOG ===
	@if exist Tests\Data\Output\build_debug.log (type Tests\Data\Output\build_debug.log) else echo No build debug log found
	@echo.
	@echo === UE ENGINE LOGS ===
	@if exist Tests\Data\UEProject\Saved\Logs (powershell -Command "Get-ChildItem 'Tests\Data\UEProject\Saved\Logs\*.log' | Sort-Object LastWriteTime -Descending | Select-Object -First 3 | ForEach-Object { Write-Host '=== ' $$_.Name ' ==='; Get-Content $$_.FullName -Tail 50 }") else echo No UE logs found
	@echo.
	@echo === COMPILATION ERRORS (if any) ===
	@if exist Tests\Data\Output\build_debug.log findstr /i "error\|failed\|exception" Tests\Data\Output\build_debug.log || echo No compilation errors found in log
	@echo.
	@echo === PLUGIN-SPECIFIC LOGS ===
	@if exist Tests\Data\UEProject\Saved\Logs (findstr /i "UEBlueprintYamlConverter\|YamlBuild\|BlueprintToYaml" Tests\Data\UEProject\Saved\Logs\*.log 2>nul || echo No plugin-specific logs found) else echo No logs directory found
	@echo.
	@echo ============================================
	@echo   LOG LOCATIONS
	@echo ============================================
	@echo Build debug log: Tests\Data\Output\build_debug.log
	@echo UE engine logs:   Tests\Data\UEProject\Saved\Logs\
	@echo Test outputs:     Tests\Data\Output\
