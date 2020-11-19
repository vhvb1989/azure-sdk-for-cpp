// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/core/test/record_network_call_policy.hpp"

#include <azure/core/strings.hpp>
#include <map>
#include <unordered_set>

using namespace Azure::Core::Http;

namespace {
static std::unordered_set<std::string> GetRequestBaseHeader()
{
  static std::unordered_set<std::string> headers;
  headers.insert("x-ms-client-request-id");
  headers.insert("Content-Type");
  headers.insert("x-ms-version");
  headers.insert("User-Agent");
  return headers;
}

} // namespace

/**
 * @brief Records network request and response into RecordedData.
 *
 * @param ctx The context for canceling the request.
 * @param request The HTTP request that is sent.
 * @param nextHttpPolicy The next policy in the pipeline.
 * @return The HTTP raw response.
 */
std::unique_ptr<RawResponse> Azure::Core::Test::RecordNetworkCallPolicy::Send(
    Context const& ctx,
    Request& request,
    NextHttpPolicy nextHttpPolicy) const
{

  // Can't record streaming requests - Not supported
  if (request.IsDownloadViaStream())
  {
    throw std::invalid_argument("Record policy does not support streaming requests.");
  }

  NetworkCallRecord record;

  // Take base headers
  {
    std::map<std::string, std::string> headers;
    auto requestHeaders = request.GetHeaders();
    auto baseHeaders = GetRequestBaseHeader();
    for (std::pair<std::string, std::string> header : requestHeaders)
    {
      if (baseHeaders.count(header.first))
      {
        headers.insert(header);
      }
    }
    record.headers = std::move(headers);
    record.method = HttpMethodToString(request.GetMethod());
  }

  // Remove Account info from URL
  {
    auto& hostWithAccount = request.GetUrl().GetHost();
    auto firstDot = hostWithAccount.find('.');
    if (firstDot == std::string::npos)
    {
      throw std::runtime_error("unexpected host url with any dots!");
    }
    auto hostWithNoAccount = hostWithAccount.substr(firstDot);
    Azure::Core::Http::Url copyUrl(request.GetUrl());
    copyUrl.SetHost(hostWithNoAccount);
    auto qp = copyUrl.GetQueryParameters();
    if (qp.find("sig") != qp.end())
    {
      copyUrl.AppendQueryParameter("sig", "REDACTED");
    }
    record.uri = copyUrl.GetAbsoluteUrl();
  }

  auto response = nextHttpPolicy.Send(ctx, request);

  // Take response info
  {
    std::map<std::string, std::string> responseData;
    auto code = static_cast<typename std::underlying_type<Http::HttpStatusCode>::type>(
        response->GetStatusCode());
    std::string codeString = std::to_string(code);
    responseData.emplace(std::string("StatusCode"), codeString);

    bool retryHeaderAdded = false;
    for (auto& header : response->GetHeaders())
    {
      std::string value = header.second;
      if (Azure::Core::Strings::LocaleInvariantCaseInsensitiveEqual(header.first, "retry-after"))
      {
        value = "0";
        retryHeaderAdded = true;
      }
      else if (Azure::Core::Strings::LocaleInvariantCaseInsensitiveEqual(
                   header.first, "x-ms-encryption-key-sha256"))
      {
        value = "REDACTED";
      }
      responseData.emplace(header.first, value);
    }
    if (!retryHeaderAdded)
    {
      responseData.emplace(std::string("retry-after"), std::string("0"));
    }

    auto& body = response->GetBody();
    std::string bodyString(body.begin(), body.end());
    responseData.emplace(std::string("Body"), bodyString);

    record.response = std::move(responseData);
  }

  m_recordedData.AddNetworkCall(std::move(record));
  return response;
}
