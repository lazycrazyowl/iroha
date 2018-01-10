/**
 * Copyright Soramitsu Co., Ltd. 2017 All Rights Reserved.
 * http://soramitsu.co.jp
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef IROHA_SHARED_MODEL_FIELD_VALIDATOR_HPP
#define IROHA_SHARED_MODEL_FIELD_VALIDATOR_HPP

#include <boost/format.hpp>
#include <regex>

#include "datetime/time.hpp"
#include "interfaces/commands/command.hpp"
#include "validator/address_validator.hpp"
#include "validators/answer.hpp"

namespace shared_model {
  namespace validation {

    /**
     * Class that validates fields of commands, concrete queries, transaction,
     * and query
     */
    class FieldValidator {
     public:
      FieldValidator()
          : account_id_regex_(R"([a-z]{1,9}\@[a-z]{1,9})"),
            asset_id_regex_(R"([a-z]{1,9}\#[a-z]{1,9})"),
            ip_address_regex_(
                "((([0-1]?\\d\\d?)|((2[0-4]\\d)|(25[0-5]))).){3}(([0-1]?\\d\\d?"
                ")|((2[0-4]\\d)|(25[0-5])))"),
            name_regex_(R"([a-z]{1,9})"),
            detail_key_regex_(R"([A-Za-z0-9_]{1,})") {}

      void validateAccountId(
          ReasonsGroupType &reason,
          const interface::types::AccountIdType &account_id) const {
        if (not std::regex_match(account_id, account_id_regex_)) {
          reason.second.push_back("Wrongly formed account_id");
        }
      }

      void validateAssetId(
          ReasonsGroupType &reason,
          const interface::types::AssetIdType &asset_id) const {
        if (not std::regex_match(asset_id, asset_id_regex_)) {
          reason.second.push_back("Wrongly formed asset_id");
        }
      }

      void validateAssetsId(
          ReasonsGroupType &reason,
          const interface::types::AssetIdCollectionType &assets_id) const {
        for (auto const& asset_id : assets_id) {
          if (not std::regex_match(asset_id, asset_id_regex_)) {
            reason.second.push_back("Wrongly formed asset_id in assets_id");
            break;
          }
        }
      }

      void validateAmount(ReasonsGroupType &reason,
                          const interface::Amount &amount) const {
        /* put here any validations*/
      }

      void validatePubkey(ReasonsGroupType &reason,
                          const interface::types::PubkeyType &pubkey) const {
        if (pubkey.blob().size() != 32) {
          reason.second.push_back("Public key has wrong size");
        }
      }

      void validatePeerAddress(
          ReasonsGroupType &reason,
          const interface::AddPeer::AddressType &address) const {
        if (not(iroha::validator::isValidIpV4(address)
                or iroha::validator::isValidHostname(address))) {
          reason.second.push_back("Wrongly formed PeerAddress: " + address);
        }
      }

      void validateRoleId(ReasonsGroupType &reason,
                          const interface::types::RoleIdType &role_id) const {
        if (not std::regex_match(role_id, name_regex_)) {
          reason.second.push_back("Wrongly formed role_id");
        }
      }

      void validateAccountName(
          ReasonsGroupType &reason,
          const interface::types::AccountNameType &account_name) const {
        if (not std::regex_match(account_name, name_regex_)) {
          reason.second.push_back("Wrongly formed account_name");
        }
      }

      void validateDomainId(
          ReasonsGroupType &reason,
          const interface::types::DomainIdType &domain_id) const {
        if (not std::regex_match(domain_id, name_regex_)) {
          reason.second.push_back("Wrongly formed domain_id");
        }
      }

      void validateAssetName(
          ReasonsGroupType &reason,
          const interface::types::AssetNameType &asset_name) const {
        if (not std::regex_match(asset_name, name_regex_)) {
          reason.second.push_back("Wrongly formed asset_name");
        }
      }

      void validateAccountDetailKey(
          ReasonsGroupType &reason,
          const interface::SetAccountDetail::AccountDetailKeyType &key) const {
        if (not std::regex_match(key, detail_key_regex_)) {
          reason.second.push_back("Wrongly formed key");
        }
      }

      void validatePrecision(
          ReasonsGroupType &reason,
          const interface::types::PrecisionType &precision) const {
        // define precision constraints
      }

      void validatePermission(
          ReasonsGroupType &reason,
          const interface::types::PermissionNameType &permission_name) const {
        // define permission constraints
      }

      void validatePermissions(
          ReasonsGroupType &reason,
          const interface::CreateRole::PermissionsType &permissions) const {
        if (permissions.empty()) {
          reason.second.push_back(
              "Permission set should contain at least one permission");
        }
      }

      void validateQuorum(ReasonsGroupType &reason,
                          const interface::types::QuorumType &quorum) const {
        // define quorum constraints
      }

      void validateCreatorAccountId(
          ReasonsGroupType &reason,
          const interface::types::AccountIdType &account_id) const {
        if (not std::regex_match(account_id, account_id_regex_)) {
          reason.second.push_back("Wrongly formed creator_account_id");
        }
      }

      void validateCreatedTime(
          ReasonsGroupType &reason,
          const interface::types::TimestampType &timestamp) const {
        iroha::ts64_t now = iroha::time::now();
        // TODO 06/08/17 Muratov: make future gap for passing timestamp, like
        // with old timestamps IR-511 #goodfirstissue
        if (now < timestamp) {
          reason.second.push_back(boost::str(
              boost::format(
                  "timestamp broken: send from future (%llu, now %llu)")
              % timestamp % now));
        }

        if (now - timestamp > MAX_DELAY) {
          reason.second.push_back(boost::str(
              boost::format("timestamp broken: too old (%llu, now %llu)")
              % timestamp % now));
        }
      }

      void validateCounter(ReasonsGroupType &reason,
                           const interface::types::CounterType &counter) const {
        if (counter == 0) {
          reason.second.push_back(
              boost::str(boost::format("Counter should be > 0")));
        }
      }

     private:
      std::regex account_id_regex_, asset_id_regex_, ip_address_regex_, name_regex_, detail_key_regex_;
      // max-delay between tx creation and validation
      static constexpr auto MAX_DELAY =
          std::chrono::hours(24) / std::chrono::milliseconds(1);
    };
  }  // namespace validation
}  // namespace shared_model

#endif  // IROHA_SHARED_MODEL_FIELD_VALIDATOR_HPP
