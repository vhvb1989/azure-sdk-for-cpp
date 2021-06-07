// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/keyvault/keys/list_keys_single_page_result.hpp"
#include "azure/keyvault/keys/key_client.hpp"
#include "private/key_constants.hpp"
#include "private/key_serializers.hpp"

#include <azure/keyvault/common/internal/unix_time_helper.hpp>

#include <azure/core/internal/json/json.hpp>
#include <azure/core/internal/json/json_optional.hpp>
#include <azure/core/internal/json/json_serializable.hpp>
#include <azure/core/url.hpp>

using namespace Azure::Security::KeyVault::Keys;
using namespace Azure::Core::Json::_internal;
using Azure::Security::KeyVault::_internal::UnixTimeConverter;

KeyPropertiesPageResult
_detail::KeyPropertiesPageResultSerializer::KeyPropertiesPageResultDeserialize(
    Azure::Core::Http::RawResponse const& rawResponse)
{
  KeyPropertiesPageResult result;
  auto const& body = rawResponse.GetBody();
  auto jsonParser = json::parse(body);

  JsonOptional::SetIfExists(result.NextPageToken, jsonParser, "nextLink");

  // Key properties
  auto keyPropertiesJson = jsonParser["value"];
  for (auto const& key : keyPropertiesJson)
  {
    KeyProperties keyProperties;
    keyProperties.Id = key[_detail::KeyIdPropertyName].get<std::string>();
    _detail::KeyVaultKeySerializer::ParseKeyUrl(keyProperties, keyProperties.Id);
    // "Attributes"
    if (key.contains(_detail::AttributesPropertyName))
    {
      auto attributes = key[_detail::AttributesPropertyName];

      JsonOptional::SetIfExists(keyProperties.Enabled, attributes, _detail::EnabledPropertyName);
      JsonOptional::SetIfExists<int64_t, Azure::DateTime>(
          keyProperties.NotBefore,
          attributes,
          _detail::NbfPropertyName,
          UnixTimeConverter::UnixTimeToDatetime);
      JsonOptional::SetIfExists<int64_t, Azure::DateTime>(
          keyProperties.ExpiresOn,
          attributes,
          _detail::ExpPropertyName,
          UnixTimeConverter::UnixTimeToDatetime);
      JsonOptional::SetIfExists<int64_t, Azure::DateTime>(
          keyProperties.CreatedOn,
          attributes,
          _detail::CreatedPropertyName,
          UnixTimeConverter::UnixTimeToDatetime);
      JsonOptional::SetIfExists<int64_t, Azure::DateTime>(
          keyProperties.UpdatedOn,
          attributes,
          _detail::UpdatedPropertyName,
          UnixTimeConverter::UnixTimeToDatetime);
    }

    // "Tags"
    if (key.contains(_detail::TagsPropertyName))
    {
      auto const& tags = key[_detail::TagsPropertyName];
      for (auto tag = tags.begin(); tag != tags.end(); ++tag)
      {
        keyProperties.Tags.emplace(tag.key(), tag.value().get<std::string>());
      }
    }

    // managed
    if (key.contains(_detail::ManagedPropertyName))
    {
      keyProperties.Managed = key[_detail::ManagedPropertyName].get<bool>();
    }

    result.Items.emplace_back(keyProperties);
  }

  return result;
}

DeletedKeyPageResult _detail::KeyPropertiesPageResultSerializer::DeletedKeyPageResultDeserialize(
    Azure::Core::Http::RawResponse const& rawResponse)
{
  auto const& body = rawResponse.GetBody();
  auto jsonParser = Azure::Core::Json::_internal::json::parse(body);

  DeletedKeyPageResult deletedKeyPageResult;

  JsonOptional::SetIfExists(deletedKeyPageResult.NextPageToken, jsonParser, "nextLink");

  auto deletedKeys = jsonParser["value"];
  for (auto const& key : deletedKeys)
  {
    DeletedKey deletedKey;
    deletedKey.Properties.Id = key[_detail::KeyIdPropertyName].get<std::string>();
    _detail::KeyVaultKeySerializer::ParseKeyUrl(deletedKey.Properties, deletedKey.Properties.Id);

    if (!key[_detail::RecoveryIdPropertyName].is_null())
    {
      deletedKey.RecoveryId = key[_detail::RecoveryIdPropertyName].get<std::string>();
    }
    if (!key[_detail::AttributesPropertyName][_detail::RecoveryLevelPropertyName].is_null())
    {
      deletedKey.Properties.RecoveryLevel
          = key[_detail::AttributesPropertyName][_detail::RecoveryLevelPropertyName]
                .get<std::string>();
    }
    JsonOptional::SetIfExists<int64_t, Azure::DateTime>(
        deletedKey.DeletedDate,
        key,
        _detail::DeletedOnPropertyName,
        UnixTimeConverter::UnixTimeToDatetime);
    JsonOptional::SetIfExists<int64_t, Azure::DateTime>(
        deletedKey.ScheduledPurgeDate,
        key,
        _detail::ScheduledPurgeDatePropertyName,
        UnixTimeConverter::UnixTimeToDatetime);

    deletedKeyPageResult.Items.emplace_back(deletedKey);
  }

  return deletedKeyPageResult;
}

void DeletedKeyPageResult::OnNextPage(const Azure::Core::Context& context)
{
  // Before calling `OnNextPage` pagedResponse validates there is a next page, so we are sure
  // NextPageToken is valid.
  GetDeletedKeysOptions options;
  options.NextPageToken = NextPageToken;
  *this = m_keyClient->GetDeletedKeys(options, context);
  CurrentPageToken = options.NextPageToken.Value();
}

void KeyPropertiesPageResult::OnNextPage(const Azure::Core::Context& context)
{
  // Notes
  // - Before calling `OnNextPage` pagedResponse validates there is a next page, so we are sure
  // NextPageToken is valid.
  // - KeyPropertiesPageResult is used to list keys from a Key Vault and also to list the key
  // versions from a specific key. When KeyPropertiesPageResult is listing keys, the `m_keyName`
  // fields will be empty, but for listing the key versions, the KeyPropertiesPageResult needs to
  // keep the name of the key in `m_keyName` because it is required to get more pages.
  //
  if (m_keyName.empty())
  {
    GetPropertiesOfKeysOptions options;
    options.NextPageToken = NextPageToken;
    *this = m_keyClient->GetPropertiesOfKeys(options, context);
    CurrentPageToken = options.NextPageToken.Value();
  }
  else
  {
    GetPropertiesOfKeyVersionsOptions options;
    options.NextPageToken = NextPageToken;
    *this = m_keyClient->GetPropertiesOfKeyVersions(m_keyName, options, context);
    CurrentPageToken = options.NextPageToken.Value();
  }
}
