// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/core/credentials/credentials.hpp>
#include <azure/core/http/http.hpp>
#include <azure/core/http/policies/policy.hpp>

#include "azure/keyvault/keys/cryptography/cryptography_serializers.hpp"
#include "azure/keyvault/keys/cryptography/key_wrap_parameters.hpp"
#include "azure/keyvault/keys/cryptography/remote_cryptography_client.hpp"
#include "azure/keyvault/keys/details/key_constants.hpp"
#include "azure/keyvault/keys/details/key_serializers.hpp"

#include <memory>
#include <string>
#include <vector>

using namespace Azure::Security::KeyVault::Keys;
using namespace Azure::Security::KeyVault::Keys::Cryptography;
using namespace Azure::Security::KeyVault::Keys::Cryptography::_detail;
using namespace Azure::Core::Http;
using namespace Azure::Core::Http::Policies;
using namespace Azure::Core::Http::Policies::_internal;

RemoteCryptographyClient::RemoteCryptographyClient(
    std::string const& keyId,
    std::shared_ptr<Core::Credentials::TokenCredential const> credential,
    CryptographyClientOptions options)
{
  auto apiVersion = options.Version.ToString();
  // Remote client is init with the url to a key vault key.
  KeyId = Azure::Core::Url(keyId);
  std::vector<std::unique_ptr<HttpPolicy>> perRetrypolicies;
  {
    Azure::Core::Credentials::TokenRequestContext const tokenContext
        = {{"https://vault.azure.net/.default"}};

    perRetrypolicies.emplace_back(
        std::make_unique<BearerTokenAuthenticationPolicy>(credential, tokenContext));
  }

  Pipeline = std::make_shared<Azure::Security::KeyVault::_internal::KeyVaultPipeline>(
      Azure::Core::Url(keyId),
      apiVersion,
      Azure::Core::Http::_internal::HttpPipeline(
          options, "KeyVault", apiVersion, std::move(perRetrypolicies), {}));
}

Azure::Response<KeyVaultKey> RemoteCryptographyClient::GetKey(
    Azure::Core::Context const& context) const
{
  // The remote crypto client is created with a key vault key url, hence, no path is required to get
  // the key from the server.
  return Pipeline->SendRequest<KeyVaultKey>(
      context,
      Azure::Core::Http::HttpMethod::Get,
      [](Azure::Core::Http::RawResponse const& rawResponse) {
        return Azure::Security::KeyVault::Keys::_detail::KeyVaultKeySerializer::
            KeyVaultKeyDeserialize(rawResponse);
      },
      {});
}

Azure::Response<EncryptResult> RemoteCryptographyClient::EncryptWithResponse(
    EncryptParameters const& parameters,
    Azure::Core::Context const& context) const
{
  return Pipeline->SendRequest<EncryptResult>(
      context,
      Azure::Core::Http::HttpMethod::Post,
      [&parameters]() {
        return EncryptParametersSerializer::EncryptParametersSerialize(parameters);
      },
      [&parameters](Azure::Core::Http::RawResponse const& rawResponse) {
        auto result = EncryptResultSerializer::EncryptResultDeserialize(rawResponse);
        result.Algorithm = parameters.Algorithm;
        return result;
      },
      {"encrypt"});
}

EncryptResult RemoteCryptographyClient::Encrypt(
    EncryptParameters const& parameters,
    Azure::Core::Context const& context) const
{
  return EncryptWithResponse(parameters, context).Value;
}

Azure::Response<DecryptResult> RemoteCryptographyClient::DecryptWithResponse(
    DecryptParameters const& parameters,
    Azure::Core::Context const& context) const
{
  return Pipeline->SendRequest<DecryptResult>(
      context,
      Azure::Core::Http::HttpMethod::Post,
      [&parameters]() {
        return DecryptParametersSerializer::DecryptParametersSerialize(parameters);
      },
      [&parameters](Azure::Core::Http::RawResponse const& rawResponse) {
        auto result = DecryptResultSerializer::DecryptResultDeserialize(rawResponse);
        result.Algorithm = parameters.Algorithm;
        return result;
      },
      {"decrypt"});
}

DecryptResult RemoteCryptographyClient::Decrypt(
    DecryptParameters const& parameters,
    Azure::Core::Context const& context) const
{
  return DecryptWithResponse(parameters, context).Value;
}

Azure::Response<WrapResult> RemoteCryptographyClient::WrapKeyWithResponse(
    KeyWrapAlgorithm const& algorithm,
    std::vector<uint8_t> const& key,
    Azure::Core::Context const& context) const
{
  return Pipeline->SendRequest<WrapResult>(
      context,
      Azure::Core::Http::HttpMethod::Post,
      [&algorithm, &key]() {
        return KeyWrapParametersSerializer::KeyWrapParametersSerialize(
            KeyWrapParameters(algorithm.ToString(), key));
      },
      [&algorithm](Azure::Core::Http::RawResponse const& rawResponse) {
        auto result = WrapResultSerializer::WrapResultDeserialize(rawResponse);
        result.Algorithm = algorithm;
        return result;
      },
      {"wrapKey"});
}

WrapResult RemoteCryptographyClient::WrapKey(
    KeyWrapAlgorithm const& algorithm,
    std::vector<uint8_t> const& key,
    Azure::Core::Context const& context) const
{
  return WrapKeyWithResponse(algorithm, key, context).Value;
}

Azure::Response<UnwrapResult> RemoteCryptographyClient::UnwrapKeyWithResponse(
    KeyWrapAlgorithm const& algorithm,
    std::vector<uint8_t> const& key,
    Azure::Core::Context const& context) const
{
  return Pipeline->SendRequest<UnwrapResult>(
      context,
      Azure::Core::Http::HttpMethod::Post,
      [&algorithm, &key]() {
        return KeyWrapParametersSerializer::KeyWrapParametersSerialize(
            KeyWrapParameters(algorithm.ToString(), key));
      },
      [&algorithm](Azure::Core::Http::RawResponse const& rawResponse) {
        auto result = UnwrapResultSerializer::UnwrapResultDeserialize(rawResponse);
        result.Algorithm = algorithm;
        return result;
      },
      {"unwrapKey"});
}

UnwrapResult RemoteCryptographyClient::UnwrapKey(
    KeyWrapAlgorithm const& algorithm,
    std::vector<uint8_t> const& key,
    Azure::Core::Context const& context) const
{
  return UnwrapKeyWithResponse(algorithm, key, context).Value;
}