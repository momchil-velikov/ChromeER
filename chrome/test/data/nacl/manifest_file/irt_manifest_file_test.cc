/*
 * Copyright 2014 The Chromium Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

//
// Test for resource open before PPAPI initialization.
//

#include <stdio.h>
#include <string.h>

#include <sstream>
#include <string>
#include <vector>

#include "native_client/src/untrusted/irt/irt.h"
#include "native_client/src/untrusted/nacl/nacl_irt.h"

#include "ppapi/cpp/instance.h"
#include "ppapi/cpp/module.h"
#include "ppapi/cpp/var.h"
#include "ppapi/cpp/var_array.h"
#include "ppapi/native_client/src/shared/ppapi_proxy/ppruntime.h"


std::vector<std::string> result;

std::string LoadManifestSuccess(TYPE_nacl_irt_query *query_func) {
  struct nacl_irt_resource_open nacl_irt_resource_open;
  if (sizeof(nacl_irt_resource_open) !=
      (*query_func)(
          NACL_IRT_RESOURCE_OPEN_v0_1,
          &nacl_irt_resource_open,
          sizeof(nacl_irt_resource_open))) {
    return "irt manifest api not found";
  }
  int desc;
  int error;
  error = nacl_irt_resource_open.open_resource("test_file", &desc);
  if (0 != error) {
    printf("Can't open file, error=%d", error);
    return "Can't open file";
  }

  std::string str;

  char buffer[4096];
  int len;
  while ((len = read(desc, buffer, sizeof buffer - 1)) > 0) {
    // Null terminate.
    buffer[len] = '\0';
    str += buffer;
  }

  if (str != "Test File Content") {
    printf("Wrong file content: \"%s\"\n", str.c_str());
    return "Wrong file content: " + str;
  }

  return "Pass";
}

std::string LoadManifestNonExistentEntry(
    TYPE_nacl_irt_query *query_func) {
  struct nacl_irt_resource_open nacl_irt_resource_open;
  if (sizeof(nacl_irt_resource_open) !=
      (*query_func)(
          NACL_IRT_RESOURCE_OPEN_v0_1,
          &nacl_irt_resource_open,
          sizeof(nacl_irt_resource_open))) {
    return "irt manifest api not found";
  }

  int desc;
  int error = nacl_irt_resource_open.open_resource("non_existent_entry", &desc);

  // We expect ENOENT here, as it does not exist.
  if (error != ENOENT) {
    printf("Unexpected error code: %d\n", error);
    char buf[80];
    snprintf(buf, sizeof(buf), "open_resource() result: %d", error);
    return std::string(buf);
  }

  return "Pass";
}

std::string LoadManifestNonExistentFile(
    TYPE_nacl_irt_query *query_func) {
  struct nacl_irt_resource_open nacl_irt_resource_open;
  if (sizeof(nacl_irt_resource_open) !=
      (*query_func)(
          NACL_IRT_RESOURCE_OPEN_v0_1,
          &nacl_irt_resource_open,
          sizeof(nacl_irt_resource_open))) {
    return "irt manifest api not found";
  }

  int desc;
  int error = nacl_irt_resource_open.open_resource("dummy_test_file", &desc);

  // We expect ENOENT here, as it does not exist.
  if (error != ENOENT) {
    printf("Unexpected error code: %d\n", error);
    char buf[80];
    snprintf(buf, sizeof(buf), "open_resource() result: %d", error);
    return std::string(buf);
  }

  return "Pass";
}

class TestInstance : public pp::Instance {
 public:
  explicit TestInstance(PP_Instance instance) : pp::Instance(instance) {}
  virtual ~TestInstance() {}
  virtual void HandleMessage(const pp::Var& var_message) {
    if (!var_message.is_string()) {
      return;
    }
    if (var_message.AsString() != "hello") {
      return;
    }
    pp::VarArray reply = pp::VarArray();
    for (size_t i = 0; i < result.size(); ++i) {
      reply.Set(i, pp::Var(result[i]));
    }
    PostMessage(reply);
  }
};

class TestModule : public pp::Module {
 public:
  TestModule() : pp::Module() {}
  virtual ~TestModule() {}

  virtual pp::Instance* CreateInstance(PP_Instance instance) {
    return new TestInstance(instance);
  }
};

namespace pp {
Module* CreateModule() {
  return new TestModule();
}
}

int main() {
  result.push_back(LoadManifestSuccess(&__nacl_irt_query));
  result.push_back(LoadManifestNonExistentEntry(&__nacl_irt_query));
  result.push_back(LoadManifestNonExistentFile(&__nacl_irt_query));
  return PpapiPluginMain();
}
