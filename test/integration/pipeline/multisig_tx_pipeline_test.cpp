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

/**
 * @given create peer with default genesis block
 * @when sent valid transaction with quorum 2 and one signature
 * AND sent same transaction with another signature
 * @then check that block contains transaction
 */
TEST(MST, OnePeerSendsTest) {
  // TODO 21/12/2017 muratovv rework with auto erased crypto manager
  auto seed = iroha::create_seed("foo");
  auto key_pair = iroha::create_keypair(seed);

  integration_framework::IntegrationTestFramework()
      .setInitialState(
          key_pair,
          iroha::model::Block{})  // todo replace with valid genesis block
      .sendTx(iroha::model::Transaction{})  // todo replace with tx with sign 1
      .sendTx(iroha::model::Transaction{})  // todo replace with tx with sign 2
      .skipProposal()
      .checkBlock([](const auto &block) {
      })  // todo check that transactions with both signatures appears
      .done();
}
