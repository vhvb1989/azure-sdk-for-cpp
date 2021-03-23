// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/core/credentials/credentials.hpp>
#include <azure/core/http/http.hpp>
#include <azure/core/http/policies/policy.hpp>

#include "azure/keyvault/keys/details/key_backup.hpp"
#include "azure/keyvault/keys/details/key_constants.hpp"
#include "azure/keyvault/keys/details/key_request_parameters.hpp"
#include "azure/keyvault/keys/key_client.hpp"

#include <memory>
#include <string>
#include <vector>

using namespace Azure::Security::KeyVault::Keys;
using namespace Azure::Core::Http;
using namespace Azure::Core::Http::Policies;

KeyClient::KeyClient(
    std::string const& vaultUrl,
    std::shared_ptr<Core::Credentials::TokenCredential const> credential,
    KeyClientOptions options)
{
  auto apiVersion = options.GetVersionString();

  std::vector<std::unique_ptr<HttpPolicy>> perRetrypolicies;
  {
    Azure::Core::Credentials::TokenRequestContext const tokenContext
        = {{"https://vault.azure.net/.default"}};

    perRetrypolicies.emplace_back(
        std::make_unique<BearerTokenAuthenticationPolicy>(credential, tokenContext));
  }

  m_pipeline = std::make_shared<Azure::Security::KeyVault::Common::_internal::KeyVaultPipeline>(
      Azure::Core::Url(vaultUrl),
      apiVersion,
      Azure::Core::Http::_internal::HttpPipeline(
          options, "KeyVault", apiVersion, std::move(perRetrypolicies), {}));
}

Azure::Response<KeyVaultKey> KeyClient::GetKey(
    std::string const& name,
    GetKeyOptions const& options,
    Azure::Core::Context const& context) const
{
  return m_pipeline->SendRequest<KeyVaultKey>(
      context,
      Azure::Core::Http::HttpMethod::Get,
      [&name](Azure::Core::Http::RawResponse const& rawResponse) {
        return _detail::KeyVaultKeyDeserialize(name, rawResponse);
      },
      {_detail::KeysPath, name, options.Version});
}

Azure::Response<KeyVaultKey> KeyClient::CreateKey(
    std::string const& name,
    JsonWebKeyType keyType,
    CreateKeyOptions const& options,
    Azure::Core::Context const& context) const
{
  return m_pipeline->SendRequest<KeyVaultKey>(
      context,
      Azure::Core::Http::HttpMethod::Post,
      _detail::KeyRequestParameters(keyType, options),
      [&name](Azure::Core::Http::RawResponse const& rawResponse) {
        return _detail::KeyVaultKeyDeserialize(name, rawResponse);
      },
      {_detail::KeysPath, name, "create"});
}

Azure::Response<KeyVaultKey> KeyClient::CreateEcKey(
    CreateEcKeyOptions const& ecKeyOptions,
    Azure::Core::Context const& context) const
{
  std::string const& keyName = ecKeyOptions.GetName();
  return m_pipeline->SendRequest<KeyVaultKey>(
      context,
      Azure::Core::Http::HttpMethod::Post,
      _detail::KeyRequestParameters(ecKeyOptions),
      [&keyName](Azure::Core::Http::RawResponse const& rawResponse) {
        return _detail::KeyVaultKeyDeserialize(keyName, rawResponse);
      },
      {_detail::KeysPath, keyName, "create"});
}

Azure::Response<KeyVaultKey> KeyClient::CreateRsaKey(
    CreateRsaKeyOptions const& rsaKeyOptions,
    Azure::Core::Context const& context) const
{
  std::string const& keyName = rsaKeyOptions.GetName();
  return m_pipeline->SendRequest<KeyVaultKey>(
      context,
      Azure::Core::Http::HttpMethod::Post,
      _detail::KeyRequestParameters(rsaKeyOptions),
      [&keyName](Azure::Core::Http::RawResponse const& rawResponse) {
        return _detail::KeyVaultKeyDeserialize(keyName, rawResponse);
      },
      {_detail::KeysPath, keyName, "create"});
}

Azure::Response<KeyVaultKey> KeyClient::CreateOctKey(
    CreateOctKeyOptions const& octKeyOptions,
    Azure::Core::Context const& context) const
{
  std::string const& keyName = octKeyOptions.GetName();
  return m_pipeline->SendRequest<KeyVaultKey>(
      context,
      Azure::Core::Http::HttpMethod::Post,
      _detail::KeyRequestParameters(octKeyOptions),
      [&keyName](Azure::Core::Http::RawResponse const& rawResponse) {
        return _detail::KeyVaultKeyDeserialize(keyName, rawResponse);
      },
      {_detail::KeysPath, keyName, "create"});
}

Azure::Response<KeyPropertiesSinglePage> KeyClient::GetPropertiesOfKeysSinglePage(
    GetPropertiesOfKeysSinglePageOptions const& options,
    Azure::Core::Context const& context) const
{
  if (!options.ContinuationToken) // First page when no continuation token //
  {
    if (options.MaxResults) // Update max-results //
    {
      return m_pipeline->SendRequest<KeyPropertiesSinglePage>(
          context,
          Azure::Core::Http::HttpMethod::Get,
          [](Azure::Core::Http::RawResponse const& rawResponse) {
            return _detail::KeyPropertiesSinglePageDeserialize(rawResponse);
          },
          {_detail::KeysPath},
          {{"maxResults", std::to_string(options.MaxResults.GetValue())}});
    }
    // let server choose max-results //
    return m_pipeline->SendRequest<KeyPropertiesSinglePage>(
        context,
        Azure::Core::Http::HttpMethod::Get,
        [](Azure::Core::Http::RawResponse const& rawResponse) {
          return _detail::KeyPropertiesSinglePageDeserialize(rawResponse);
        },
        {_detail::KeysPath});
  }
  // Get next page //
  return m_pipeline->SendRequest<KeyPropertiesSinglePage>(
      context,
      Azure::Core::Http::HttpMethod::Get,
      [](Azure::Core::Http::RawResponse const& rawResponse) {
        return _detail::KeyPropertiesSinglePageDeserialize(rawResponse);
      },
      {_detail::KeysPath});
}

Azure::Response<KeyPropertiesSinglePage> KeyClient::GetPropertiesOfKeyVersions(
    std::string const& name,
    GetPropertiesOfKeyVersionsOptions const& options,
    Azure::Core::Context const& context) const
{
  if (!options.ContinuationToken) // First page when no continuation token //
  {
    if (options.MaxResults) // Update max-results //
    {
      return m_pipeline->SendRequest<KeyPropertiesSinglePage>(
          context,
          Azure::Core::Http::HttpMethod::Get,
          [](Azure::Core::Http::RawResponse const& rawResponse) {
            return _detail::KeyPropertiesSinglePageDeserialize(rawResponse);
          },
          {_detail::KeysPath, name, "versions"},
          {{"maxResults", std::to_string(options.MaxResults.GetValue())}});
    }
    // let server choose max-results //
    return m_pipeline->SendRequest<KeyPropertiesSinglePage>(
        context,
        Azure::Core::Http::HttpMethod::Get,
        [](Azure::Core::Http::RawResponse const& rawResponse) {
          return _detail::KeyPropertiesSinglePageDeserialize(rawResponse);
        },
        {_detail::KeysPath, name, "versions"});
  }
  // Get next page //
  return m_pipeline->SendRequest<KeyPropertiesSinglePage>(
      context,
      Azure::Core::Http::HttpMethod::Get,
      [](Azure::Core::Http::RawResponse const& rawResponse) {
        return _detail::KeyPropertiesSinglePageDeserialize(rawResponse);
      },
      {_detail::KeysPath, name, "versions"});
}

Azure::Security::KeyVault::Keys::DeleteKeyOperation KeyClient::StartDeleteKey(
    std::string const& name,
    Azure::Core::Context const& context) const
{
  return Azure::Security::KeyVault::Keys::DeleteKeyOperation(
      m_pipeline,
      m_pipeline->SendRequest<Azure::Security::KeyVault::Keys::DeletedKey>(
          context,
          Azure::Core::Http::HttpMethod::Delete,
          [&name](Azure::Core::Http::RawResponse const& rawResponse) {
            return _detail::DeletedKeyDeserialize(name, rawResponse);
          },
          {_detail::KeysPath, name}));
}

Azure::Security::KeyVault::Keys::RecoverDeletedKeyOperation KeyClient::StartRecoverDeletedKey(
    std::string const& name,
    Azure::Core::Context const& context) const
{
  return Azure::Security::KeyVault::Keys::RecoverDeletedKeyOperation(
      m_pipeline,
      m_pipeline->SendRequest<Azure::Security::KeyVault::Keys::KeyVaultKey>(
          context,
          Azure::Core::Http::HttpMethod::Post,
          [&name](Azure::Core::Http::RawResponse const& rawResponse) {
            return _detail::KeyVaultKeyDeserialize(name, rawResponse);
          },
          {_detail::DeletedKeysPath, name, "recover"}));
}

Azure::Response<DeletedKey> KeyClient::GetDeletedKey(
    std::string const& name,
    Azure::Core::Context const& context) const
{
  return m_pipeline->SendRequest<DeletedKey>(
      context,
      Azure::Core::Http::HttpMethod::Get,
      [&name](Azure::Core::Http::RawResponse const& rawResponse) {
        return _detail::DeletedKeyDeserialize(name, rawResponse);
      },
      {_detail::DeletedKeysPath, name});
}

Azure::Response<DeletedKeySinglePage> KeyClient::GetDeletedKeysSinglePage(
    GetDeletedKeysOptions const& options,
    Azure::Core::Context const& context) const
{
  if (!options.ContinuationToken) // First page when no continuation token //
  {
    if (options.MaxResults) // Update max-results //
    {
      return m_pipeline->SendRequest<DeletedKeySinglePage>(
          context,
          Azure::Core::Http::HttpMethod::Get,
          [](Azure::Core::Http::RawResponse const& rawResponse) {
            return _detail::DeletedKeySinglePageDeserialize(rawResponse);
          },
          {_detail::DeletedKeysPath},
          {{"maxResults", std::to_string(options.MaxResults.GetValue())}});
    }
    // let server choose max-results //
    return m_pipeline->SendRequest<DeletedKeySinglePage>(
        context,
        Azure::Core::Http::HttpMethod::Get,
        [](Azure::Core::Http::RawResponse const& rawResponse) {
          return _detail::DeletedKeySinglePageDeserialize(rawResponse);
        },
        {_detail::DeletedKeysPath});
  }
  // Get next page //
  return m_pipeline->SendRequest<DeletedKeySinglePage>(
      context,
      Azure::Core::Http::HttpMethod::Get,
      [](Azure::Core::Http::RawResponse const& rawResponse) {
        return _detail::DeletedKeySinglePageDeserialize(rawResponse);
      },
      {_detail::DeletedKeysPath});
}

Azure::Response<PurgedKey> KeyClient::PurgeDeletedKey(
    std::string const& name,
    Azure::Core::Context const& context) const
{
  return m_pipeline->SendRequest<PurgedKey>(
      context,
      Azure::Core::Http::HttpMethod::Delete,
      [](Azure::Core::Http::RawResponse const&) { return PurgedKey(); },
      {_detail::DeletedKeysPath, name});
}

Azure::Response<KeyVaultKey> KeyClient::UpdateKeyProperties(
    KeyProperties const& properties,
    Azure::Nullable<std::list<KeyOperation>> const& keyOperations,
    Azure::Core::Context const& context) const
{
  return m_pipeline->SendRequest<KeyVaultKey>(
      context,
      Azure::Core::Http::HttpMethod::Patch,
      _detail::KeyRequestParameters(properties, keyOperations),
      [&properties](Azure::Core::Http::RawResponse const& rawResponse) {
        return _detail::KeyVaultKeyDeserialize(properties.Name, rawResponse);
      },
      {_detail::KeysPath, properties.Name, properties.Version});
}

Azure::Response<std::vector<uint8_t>> KeyClient::BackupKey(
    std::string const& name,
    Azure::Core::Context const& context) const
{
  // Use the internal model KeyBackup to parse from Json
  auto response = m_pipeline->SendRequest<_detail::KeyBackup>(
      context,
      Azure::Core::Http::HttpMethod::Post,
      [](Azure::Core::Http::RawResponse const& rawResponse) {
        return _detail::KeyBackup::Deserialize(rawResponse);
      },
      {_detail::KeysPath, name, "/backup"});

  // Convert the internal KeyBackup model to a raw vector<uint8_t>.
  return Azure::Response<std::vector<uint8_t>>(
      response.ExtractValue().Value, response.ExtractRawResponse());
}

Azure::Response<KeyVaultKey> KeyClient::RestoreKeyBackup(
    std::vector<uint8_t> const& backup,
    Azure::Core::Context const& context) const
{
  _detail::KeyBackup backupModel;
  backupModel.Value = backup;
  return m_pipeline->SendRequest<KeyVaultKey>(
      context,
      Azure::Core::Http::HttpMethod::Post,
      backupModel,
      [](Azure::Core::Http::RawResponse const& rawResponse) {
        return _detail::KeyVaultKeyDeserialize(rawResponse);
      },
      {_detail::KeysPath, "restore"});
}

Azure::Response<KeyVaultKey> KeyClient::ImportKey(
    std::string const& name,
    JsonWebKey const& keyMaterial,
    Azure::Core::Context const& context) const
{
  return m_pipeline->SendRequest<KeyVaultKey>(
      context,
      Azure::Core::Http::HttpMethod::Put,
      ImportKeyOptions(name, keyMaterial),
      [&name](Azure::Core::Http::RawResponse const& rawResponse) {
        return _detail::KeyVaultKeyDeserialize(name, rawResponse);
      },
      {_detail::KeysPath, name});
}

Azure::Response<KeyVaultKey> KeyClient::ImportKey(
    ImportKeyOptions const& importKeyOptions,
    Azure::Core::Context const& context) const
{
  return m_pipeline->SendRequest<KeyVaultKey>(
      context,
      Azure::Core::Http::HttpMethod::Put,
      importKeyOptions,
      [&importKeyOptions](Azure::Core::Http::RawResponse const& rawResponse) {
        return _detail::KeyVaultKeyDeserialize(importKeyOptions.Name(), rawResponse);
      },
      {_detail::KeysPath, importKeyOptions.Name()});
}
