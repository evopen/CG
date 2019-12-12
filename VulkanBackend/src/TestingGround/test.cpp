// Copyright 2016 The Shaderc Authors. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// The program demonstrates basic shader compilation using the Shaderc C++ API.
// For clarity, each method is deliberately self-contained.
//
// Techniques demonstrated:
//  - Preprocessing GLSL source text
//  - Compiling a shader to SPIR-V assembly text
//  - Compliing a shader to a SPIR-V binary module
//  - Performing optimization with compilation
//  - Setting basic options: setting a preprocessor symbol.
//  - Checking compilation status and extracting an error message.

#include <cstring>
#include <iostream>
#include <string>
#include <vector>

#include <shaderc/shaderc.hpp>


// Compiles a shader to a SPIR-V binary. Returns the binary as
// a vector of 32-bit words.
std::vector<uint32_t> compile_file(const std::string& source_name,
                                   shaderc_shader_kind kind,
                                   const std::string& source,
                                   bool optimize = false) {
  shaderc::Compiler compiler;
  shaderc::CompileOptions options;

  // Like -DMY_DEFINE=1
  options.AddMacroDefinition("MY_DEFINE", "1");
  if (optimize) options.SetOptimizationLevel(shaderc_optimization_level_size);

  shaderc::SpvCompilationResult module =
      compiler.CompileGlslToSpv(source, kind, source_name.c_str(), options);

  if (module.GetCompilationStatus() != shaderc_compilation_status_success) {
    std::cerr << module.GetErrorMessage();
    return std::vector<uint32_t>();
  }

  return {module.cbegin(), module.cend()};
}

int main() {
  const char kShaderSource[] =
      "#version 310 es\n"
      "void main() { int x = MY_DEFINE; }\n";


  {  // Compiling

    auto spirv =
        compile_file("shader_src", shaderc_glsl_vertex_shader, kShaderSource);
    std::cout << "Compiled to a binary module with " << spirv.size()
              << " words." << std::endl;
  }

  {  // Compiling with optimizing
    auto spirv = compile_file("shader_src", shaderc_glsl_vertex_shader,
                              kShaderSource, /* optimize = */ true);
    std::cout << "Compiled to an optimized binary module with " << spirv.size()
              << " words." << std::endl;
  }

  {  // Error case
    const char kBadShaderSource[] =
        "#version 310 es\nint main() { int main_should_be_void; }\n";

    std::cout << std::endl << "Compiling a bad shader:" << std::endl;
    compile_file("bad_src", shaderc_glsl_vertex_shader, kBadShaderSource);
  }


  return 0;
}