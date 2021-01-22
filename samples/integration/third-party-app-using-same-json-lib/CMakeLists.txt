# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

# Project set up
cmake_minimum_required(VERSION 3.13)
project(Application-using-storage-blobs LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

#
# Fetch content Azure SDK
#
include(FetchContent)
# FetchContent_Declare(azuresdkforcpp
#     # Set the SDK url path and release TAG
#     GIT_REPOSITORY      https://github.com/Azure/azure-sdk-for-cpp.git
#     GIT_TAG             azure-storage-files-datalake_12.0.0-beta.6)
# FetchContent_GetProperties(azuresdkforcpp)
# if(NOT azuresdkforcpp_POPULATED)
#     FetchContent_Populate(azuresdkforcpp)
#     add_subdirectory(${azuresdkforcpp_SOURCE_DIR} ${azuresdkforcpp_BINARY_DIR} EXCLUDE_FROM_ALL)
# endif()

#
# Fetch nlohmann Json
#
FetchContent_Declare(json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.8.0)

FetchContent_GetProperties(json)
if(NOT json_POPULATED)
  FetchContent_Populate(json)
  add_subdirectory(${json_SOURCE_DIR} ${json_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

# # application getting the SDK from source and nlohmann with fetch content
# add_executable (
#      application
#      src/main
# )
# # Link to Azure SDK
# target_link_libraries(application azure-storage-files-datalake nlohmann_json::nlohmann_json)

# application getting the SDK from source and nlohmann from local file
add_executable (
     applicationE
     src/main
)

# Link to Azure SDK
target_link_libraries(applicationE azure-storage-files-datalake)
