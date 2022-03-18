// Protocol Buffers - Google's data interchange format
// Copyright 2022 Google Inc.  All rights reserved.
// https://developers.google.com/protocol-buffers/
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <memory>
#include <list>

#include <google/protobuf/compiler/js/js_generator.h>
#include <google/protobuf/compiler/command_line_interface.h>
#include <google/protobuf/io/zero_copy_stream.h>
#include <google/protobuf/io/printer.h>

#include <google/protobuf/testing/googletest.h>
#include <gtest/gtest.h>
#include <google/protobuf/testing/file.h>

namespace google {
namespace protobuf {
namespace compiler {
namespace js {
namespace {

std::string FindJSTestDir() {
  return TestSourceDir() + "/google/protobuf/compiler/js";
}

// This test is a simple golden-file test over the output of the JS code
// generator. When we make changes to the JS extension and alter the JS code
// generator to use those changes, we should (i) manually test the output of the
// code generator with the extension, and (ii) update the golden output above.
// Some day, we may integrate build systems between protoc and the language
// extensions to the point where we can do this test in a more automated way.

void JSTest(std::string import_style, std::string proto_file, std::string import_proto_file = "") {
  std::string js_tests = FindJSTestDir();

  google::protobuf::compiler::CommandLineInterface cli;
  cli.SetInputsAreProtoPathRelative(true);

  js::Generator js_generator;
  cli.RegisterGenerator("--js_out", &js_generator, "");

  // Copy generated_code.proto to the temporary test directory.
  std::string test_input;
  GOOGLE_CHECK_OK(File::GetContents(
      js_tests + proto_file + ".proto",
      &test_input,
      true));
  GOOGLE_CHECK_OK(File::SetContents(
      TestTempDir() + proto_file + ".proto",
      test_input,
      true));

  // Copy generated_code_import.proto to the temporary test directory.
  std::string test_import;
  if (!import_proto_file.empty()) {
    GOOGLE_CHECK_OK(File::GetContents(
        js_tests + import_proto_file + ".proto",
        &test_import,
        true));
    GOOGLE_CHECK_OK(File::SetContents(
        TestTempDir() + import_proto_file + ".proto",
        test_import,
        true));
  }

  // Invoke the proto compiler (we will be inside TestTempDir() at this point).
  std::string js_out = "--js_out=import_style=" + import_style + ":" + TestTempDir();
  std::string proto_path = "--proto_path=" + TestTempDir();

  std::string proto_target = TestTempDir() + proto_file + ".proto";
  const char* argv[] = {
    "protoc",
    js_out.c_str(),
    proto_path.c_str(),
    proto_target.c_str(),
  };

  EXPECT_EQ(0, cli.Run(4, argv));

  // Load the generated output and compare to the expected result.
  std::string output;
  GOOGLE_CHECK_OK(File::GetContentsAsText(
      TestTempDir() + proto_file + "_pb.js",
      &output,
      true));
  std::string expected_output;
  GOOGLE_CHECK_OK(File::GetContentsAsText(
      js_tests + proto_file + "_pb_" + import_style + ".js",
      &expected_output,
      true));
  EXPECT_EQ(expected_output, output);
}

TEST(JSGeneratorTest, Proto3GeneratorCommonjsStrictTest) {
  JSTest("commonjs_strict", "/test", "/test_import");
}

TEST(JSGeneratorTest, Proto3GeneratorCommonjsTest) {
  JSTest("commonjs", "/test", "/test_import");
}

}  // namespace
}  // namespace js
}  // namespace compiler
}  // namespace protobuf
}  // namespace google
