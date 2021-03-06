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

#include "interactive/interactive_query_cli.hpp"

#include <boost/algorithm/string.hpp>
#include <fstream>

#include "byteutils.hpp"
#include "client.hpp"
#include "cryptography/ed25519_sha3_impl/internal/ed25519_impl.hpp"
#include "cryptography/ed25519_sha3_impl/internal/sha3_hash.hpp"
#include "crypto/keys_manager_impl.hpp"
#include "datetime/time.hpp"
#include "grpc_response_handler.hpp"
#include "model/converters/json_query_factory.hpp"
#include "model/queries/get_asset_info.hpp"
#include "model/queries/get_roles.hpp"

using namespace iroha::model;

namespace iroha_cli {
  namespace interactive {

    void InteractiveQueryCli::create_queries_menu() {
      description_map_ = {
          {GET_ACC, "Get Account Information"},
          {GET_ACC_AST, "Get Account's Assets"},
          {GET_ACC_TX, "Get Account's Transactions"},
          {GET_TX, "Get Transactions by transactions' hashes"},
          {GET_ACC_SIGN, "Get Account's Signatories"},
          {GET_ROLES, "Get all current roles in the system"},
          {GET_AST_INFO, "Get information about asset"},
          {GET_ROLE_PERM, "Get all permissions related to role"}
          // description_map_
      };

      const auto acc_id = "Requested account Id";
      const auto ast_id = "Requested asset Id";
      const auto role_id = "Requested role name";
      const auto tx_hashes = "Requested tx hashes";

      query_params_descriptions_ = {
          {GET_ACC, {acc_id}},
          {GET_ACC_AST, {acc_id, ast_id}},
          {GET_ACC_TX, {acc_id}},
          {GET_TX, {tx_hashes}},
          {GET_ACC_SIGN, {acc_id}},
          {GET_ROLES, {}},
          {GET_AST_INFO, {ast_id}},
          {GET_ROLE_PERM, {role_id}}
          // query_params_descriptions_
      };

      query_handlers_ = {
          {GET_ACC, &InteractiveQueryCli::parseGetAccount},
          {GET_ACC_AST, &InteractiveQueryCli::parseGetAccountAssets},
          {GET_ACC_TX, &InteractiveQueryCli::parseGetAccountTransactions},
          {GET_TX, &InteractiveQueryCli::parseGetTransactions},
          {GET_ACC_SIGN, &InteractiveQueryCli::parseGetSignatories},
          {GET_ROLE_PERM, &InteractiveQueryCli::parseGetRolePermissions},
          {GET_ROLES, &InteractiveQueryCli::parseGetRoles},
          {GET_AST_INFO, &InteractiveQueryCli::parseGetAssetInfo}
          // query_handlers_
      };

      menu_points_ = formMenu(
          query_handlers_, query_params_descriptions_, description_map_);
      // Add "go back" option
      addBackOption(menu_points_);
    }

    void InteractiveQueryCli::create_result_menu() {
      result_handlers_ = {{SAVE_CODE, &InteractiveQueryCli::parseSaveFile},
                          {SEND_CODE, &InteractiveQueryCli::parseSendToIroha}};
      result_params_descriptions_ = getCommonParamsMap();

      result_points_ = formMenu(result_handlers_,
                                result_params_descriptions_,
                                getCommonDescriptionMap());
      addBackOption(result_points_);
    }

    InteractiveQueryCli::InteractiveQueryCli(
        const std::string &account_name,
        uint64_t query_counter,
        const std::shared_ptr<iroha::model::ModelCryptoProvider> &provider)
        : current_context_(MAIN),
          creator_(account_name),
          counter_(query_counter),
          provider_(provider) {
      log_ = logger::log("InteractiveQueryCli");
      create_queries_menu();
      create_result_menu();
    }

    void InteractiveQueryCli::run() {
      std::string line;
      bool is_parsing = true;
      current_context_ = MAIN;
      printMenu("Choose query: ", menu_points_);
      // Creating a new query, increment local counter
      ++counter_;
      // Init timestamp for a new query
      local_time_ = iroha::time::now();

      while (is_parsing) {
        line = promtString("> ");
        switch (current_context_) {
          case MAIN:
            is_parsing = parseQuery(line);
            break;
          case RESULT:
            is_parsing = parseResult(line);
            break;
        }
      }
    }

    bool InteractiveQueryCli::parseQuery(std::string line) {
      if (isBackOption(line)) {
        // Stop parsing
        return false;
      }

      auto res = handleParse<std::shared_ptr<iroha::model::Query>>(
          this, line, query_handlers_, query_params_descriptions_);
      if (not res.has_value()) {
        // Continue parsing
        return true;
      }

      query_ = res.value();
      current_context_ = RESULT;
      printMenu("Query is formed. Choose what to do:", result_points_);
      // Continue parsing
      return true;
    }

    std::shared_ptr<iroha::model::Query> InteractiveQueryCli::parseGetAccount(
        QueryParams params) {
      auto account_id = params[0];
      return generator_.generateGetAccount(
          local_time_, creator_, counter_, account_id);
    }

    std::shared_ptr<iroha::model::Query>
    InteractiveQueryCli::parseGetAccountAssets(QueryParams params) {
      auto account_id = params[0];
      auto asset_id = params[1];
      return generator_.generateGetAccountAssets(
          local_time_, creator_, counter_, account_id, asset_id);
    }

    std::shared_ptr<iroha::model::Query>
    InteractiveQueryCli::parseGetAccountTransactions(QueryParams params) {
      auto account_id = params[0];
      return generator_.generateGetAccountTransactions(
          local_time_, creator_, counter_, account_id);
    }

    std::shared_ptr<iroha::model::Query>
    InteractiveQueryCli::parseGetTransactions(QueryParams params) {
      // Parser definition: hash1 hash2 ...
      GetTransactions::TxHashCollectionType tx_hashes;
      std::for_each(params.begin(), params.end(), [&tx_hashes](auto const& hex_hash){
        if (auto opt =
          iroha::hexstringToArray<GetTransactions::TxHashType::size()>(hex_hash)) {
          tx_hashes.push_back(*opt);
        }
      });
      return generator_.generateGetTransactions(
        local_time_, creator_, counter_, tx_hashes);
    }

    std::shared_ptr<iroha::model::Query>
    InteractiveQueryCli::parseGetSignatories(QueryParams params) {
      auto account_id = params[0];
      return generator_.generateGetSignatories(
          local_time_, creator_, counter_, account_id);
    }

    std::shared_ptr<iroha::model::Query> InteractiveQueryCli::parseGetAssetInfo(
        QueryParams params) {
      auto asset_id = params[0];
      auto query = std::make_shared<GetAssetInfo>(asset_id);
      //TODO 26/09/17 grimadas: remove duplicated code and move setQueryMetaData calls to private method IR-508 #goodfirstissue
      generator_.setQueryMetaData(query, local_time_, creator_, counter_);
      return query;
    }

    std::shared_ptr<iroha::model::Query> InteractiveQueryCli::parseGetRoles(
        QueryParams params) {
      auto query = std::make_shared<GetRoles>();
      generator_.setQueryMetaData(query, local_time_, creator_, counter_);
      return query;
    }

    std::shared_ptr<iroha::model::Query>
    InteractiveQueryCli::parseGetRolePermissions(QueryParams params) {
      auto role_name = params[0];
      auto query = std::make_shared<GetRolePermissions>(role_name);
      generator_.setQueryMetaData(query, local_time_, creator_, counter_);
      return query;
    }

    bool InteractiveQueryCli::parseResult(std::string line) {
      if (isBackOption(line)) {
        // Give up the last query and start a new one
        current_context_ = MAIN;
        printEnd();
        printMenu("Choose query: ", menu_points_);
        // Continue parsing
        return true;
      }

      auto res = handleParse<bool>(
          this, line, result_handlers_, result_params_descriptions_);

      return not res.has_value() ? true : res.value();
    }

    bool InteractiveQueryCli::parseSendToIroha(QueryParams params) {
      auto address = parseIrohaPeerParams(params);
      if (not address.has_value()) {
        return true;
      }

      provider_->sign(*query_);

      CliClient client(address.value().first, address.value().second);
      GrpcResponseHandler{}.handle(client.sendQuery(query_));
      printEnd();
      // Stop parsing
      return false;
    }

    bool InteractiveQueryCli::parseSaveFile(QueryParams params) {
      provider_->sign(*query_);

      auto path = params[0];
      iroha::model::converters::JsonQueryFactory json_factory;
      auto json_string = json_factory.serialize(query_);
      std::ofstream output_file(path);
      if (not output_file) {
        std::cout << "Cannot create file" << std::endl;
        // Continue parsing
        return true;
      }
      output_file << json_string;
      std::cout << "Successfully saved!" << std::endl;
      printEnd();
      // Stop parsing
      return false;
    }

  }  // namespace interactive
}  // namespace iroha_cli
