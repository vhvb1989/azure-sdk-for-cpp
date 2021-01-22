// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Application that consumes the Azure SDK for C++.
 *
 * @remark Set environment variable `STORAGE_CONNECTION_STRING` before running the application.
 *
 */
#include "nlohmann.hpp"

#include <azure/storage/files/datalake.hpp>

#include <exception>
#include <iostream>

using nlohmann::json;

int main(int argc, char* argv[])
{
  (void)argc;
  (void)argv;

  // Use nlohmann json
  json j;
  j["pi"] = 3.141;

  Azure::Core::Internal::Json::json cj;
  cj["pi"] = 3.141;

  using namespace Azure::Storage::Files::DataLake;
  auto serviceClient = DataLakeServiceClient::CreateFromConnectionString("");
  auto fileSystemClient = DataLakeFileSystemClient::CreateFromConnectionString("", "");

  std::cout << cj << " -- " << j;

  return 0;
}
