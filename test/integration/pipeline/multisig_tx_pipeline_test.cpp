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

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "crypto/keys_manager_impl.hpp"
#include "framework/integration_framework/integration_test_framework.hpp"
#include "model/generators/command_generator.hpp"
#include "model/model_crypto_provider_impl.hpp"

iroha::model::Transaction addSignature(const iroha::model::Transaction &t,
                                       iroha::keypair_t keypair) {
  iroha::model::Transaction tx = t;
  iroha::model::ModelCryptoProviderImpl(keypair).sign(tx);
  return tx;
}

/**
 * @given create peer with default genesis block
 * @when sent valid transaction with quorum 2 and one signature
 *   AND sent same transaction with another signature
 * @then check that block contains transaction
 */
TEST(MST, OnePeerSendsTest) {
  // TODO 21/12/2017 muratovv rework with auto erased crypto manager
  auto user1 = "user1", user2 = "user2";
  auto key_user1 = iroha::create_keypair(), key_user2 = iroha::create_keypair();
  iroha::model::generators::CommandGenerator gen;

  iroha::model::Transaction tx;
  tx.commands.push_back(gen.generateAddAssetQuantity(
      user1, "test", iroha::Amount().createFromString("0").value()));

  integration_framework::IntegrationTestFramework()
      .setInitialState()  // todo replace with valid genesis block
      .addUser(user1, key_user1)
      .addUser(user2, key_user2)
      .sendTx(addSignature(tx, key_user1))  // todo replace with tx with sign 1
      .sendTx(addSignature(tx, key_user2))  // todo replace with tx with sign 2
      .skipProposal()
      .checkBlock([](const auto &block) {
      })  // todo check that transactions with both signatures appears
      .done();
}
