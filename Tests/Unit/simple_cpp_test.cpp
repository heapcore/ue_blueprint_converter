/**
 * Simple C++ test without external dependencies
 * Compile: cl simple_cpp_test.cpp /std:c++17 /EHsc (Windows)
 *          g++ simple_cpp_test.cpp -std=c++17 -o simple_cpp_test (Linux)
 */

#include <iostream>
#include <cstdlib>
#include <fstream>
#include <filesystem>
#include <string>
#include <vector>
#include <cassert>

namespace fs = std::filesystem;

// Simple test framework
class SimpleTest {
    int passed = 0;
    int failed = 0;

public:
    void run(const std::string& name, std::function<void()> test) {
        std::cout << "Running: " << name << " ... ";
        try {
            test();
            std::cout << "✅ PASSED" << std::endl;
            passed++;
        } catch (const std::exception& e) {
            std::cout << "❌ FAILED: " << e.what() << std::endl;
            failed++;
        }
    }

    void assert_true(bool condition, const std::string& msg) {
        if (!condition) {
            throw std::runtime_error(msg);
        }
    }

    void assert_equals(const std::string& expected, const std::string& actual) {
        if (expected != actual) {
            throw std::runtime_error("Expected: " + expected + ", got: " + actual);
        }
    }

    int summary() {
        std::cout << "\n=== Test Summary ===" << std::endl;
        std::cout << "Passed: " << passed << std::endl;
        std::cout << "Failed: " << failed << std::endl;
        return failed > 0 ? 1 : 0;
    }
};

// Helper to run commands
int runCommand(const std::string& command) {
    std::cout << "  Executing: " << command << std::endl;
    return std::system(command.c_str());
}

// Main tests
int main() {
    SimpleTest tests;

    std::cout << "=== Simple C++ Tests for UE Blueprint YAML Converter ===" << std::endl;

    // Configuration
    const std::string ueEditor = R"("C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealEditor-Cmd.exe")";
    const std::string project = "UEBlueprintYamlConverterTest.uproject";
    const std::string testOutput = "TestOutput";

    // Setup
    fs::remove_all(testOutput);
    fs::create_directories(testOutput);

    // Test 1: Check UE Editor exists
    tests.run("UE Editor Path Check", [&]() {
        std::string checkPath = R"(C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealEditor-Cmd.exe)";
        tests.assert_true(fs::exists(checkPath), "UE Editor not found at: " + checkPath);
    });

    // Test 2: Check project file exists
    tests.run("Project File Check", [&]() {
        tests.assert_true(fs::exists(project), "Project file not found: " + project);
    });

    // Test 3: Run YamlBuild commandlet
    tests.run("YamlBuild Commandlet", [&]() {
        std::string cmd = ueEditor + " " + project +
            " -run=YamlBuild" +
            " -Yaml=TestData\\SimpleTest.yaml" +
            " -OutDir=" + testOutput +
            " -Manifest=" + testOutput + "\\manifest.json";

        int result = runCommand(cmd);
        tests.assert_true(result == 0, "Commandlet failed with code: " + std::to_string(result));

        // Check manifest exists
        fs::path manifestPath = fs::path(testOutput) / "manifest.json";
        tests.assert_true(fs::exists(manifestPath), "Manifest not created");
    });

    // Test 4: Run BlueprintToYaml commandlet
    tests.run("BlueprintToYaml Commandlet", [&]() {
        std::string exportDir = testOutput + "\\export";
        std::string cmd = ueEditor + " " + project +
            " -run=BlueprintToYaml" +
            " -OutDir=" + exportDir +
            " -Filter=/Game/Generated";

        int result = runCommand(cmd);
        tests.assert_true(result == 0, "Export commandlet failed");

        // Check if any YAML files were created
        bool foundYaml = false;
        if (fs::exists(exportDir)) {
            for (const auto& entry : fs::recursive_directory_iterator(exportDir)) {
                if (entry.path().extension() == ".yaml") {
                    foundYaml = true;
                    std::cout << "  Found: " << entry.path() << std::endl;
                    break;
                }
            }
        }
        tests.assert_true(foundYaml, "No YAML files exported");
    });

    // Test 5: Unit test example (for future use)
    tests.run("String Escaping (Unit Test Example)", [&]() {
        // Example of testing a specific function
        auto escapeYamlString = [](const std::string& input) -> std::string {
            std::string result = input;
            // Simple YAML escaping
            if (result.find(':') != std::string::npos ||
                result.find('#') != std::string::npos ||
                result.find('"') != std::string::npos) {
                return "\"" + result + "\"";
            }
            return result;
        };

        tests.assert_equals("simple", escapeYamlString("simple"));
        tests.assert_equals("\"has:colon\"", escapeYamlString("has:colon"));
        tests.assert_equals("\"has#hash\"", escapeYamlString("has#hash"));
    });

    return tests.summary();
}
