// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/core/test/network_models.hpp"

void Azure::Core::Test::RecordedData::AddNetworkCall(Azure::Core::Test::NetworkCallRecord record)
{
  {
    std::lock_guard<std::mutex> lock(m_insertRecordMutex);
    m_networkCallRecords.emplace_back(record);
  }
  return;
}
