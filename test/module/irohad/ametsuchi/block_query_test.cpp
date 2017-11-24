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

#include <boost/optional.hpp>
#include "ametsuchi/impl/redis_block_index.hpp"
#include "ametsuchi/impl/redis_block_query.hpp"
#include "cryptography/ed25519_sha3_impl/internal/sha3_hash.hpp"
#include "framework/test_subscriber.hpp"
#include "module/irohad/ametsuchi/ametsuchi_fixture.hpp"

using namespace iroha::ametsuchi;
using namespace iroha::model;
using namespace framework::test_subscriber;

class BlockQueryTest : public AmetsuchiTest {
 protected:
  void SetUp() override {
    AmetsuchiTest::SetUp();

    file = FlatFile::create(block_store_path);
    ASSERT_TRUE(file);
    index = std::make_shared<RedisBlockIndex>(client);
    blocks = std::make_shared<RedisBlockQuery>(client, *file);

    // First transaction in block1
    Transaction txn1_1;
    txn1_1.creator_account_id = creator1;
    tx_hashes.push_back(iroha::hash(txn1_1));

    // Second transaction in block1
    Transaction txn1_2;
    txn1_2.creator_account_id = creator1;
    tx_hashes.push_back(iroha::hash(txn1_2));

    Block block1;
    block1.height = 1;
    block1.transactions.push_back(txn1_1);
    block1.transactions.push_back(txn1_2);
    auto block1hash = iroha::hash(block1);

    // First tx in block 1
    Transaction txn2_1;
    txn2_1.creator_account_id = creator1;
    tx_hashes.push_back(iroha::hash(txn2_1));

    // Second tx in block 2
    Transaction txn2_2;
    // this tx has another creator
    txn2_2.creator_account_id = creator2;
    tx_hashes.push_back(iroha::hash(txn2_2));

    Block block2;
    block2.height = 2;
    block2.prev_hash = block1hash;
    block2.transactions.push_back(txn2_1);
    block2.transactions.push_back(txn2_2);

    for (const auto &b : {block1, block2}) {
      file->add(b.height, iroha::stringToBytes(converters::jsonToString(
          converters::JsonBlockFactory().serialize(b))));
      index->index(b);
    }
  }

  std::vector<iroha::hash256_t> tx_hashes;
  std::shared_ptr<BlockQuery> blocks;
  std::shared_ptr<BlockIndex> index;
  std::unique_ptr<FlatFile> file;
  std::string creator1 = "user1@test";
  std::string creator2 = "user2@test";
};

/**
 * @given block store with 2 blocks totally containing 3 txs created by
 * user1@test
 * AND 1 tx created by user2@test
 * @when query to get transactions with existing transaction hashes
 * @then queried transactions
 */
TEST_F(BlockQueryTest, GetTransactionsExistingTxHashes) {
  auto wrapper = make_test_subscriber<CallExact>(
      blocks->getTransactions({tx_hashes[1], tx_hashes[3]}), 2);
  wrapper.subscribe([this](auto tx) {
    static auto subs_cnt = 0;
    subs_cnt++;
    if (subs_cnt == 1) {
      EXPECT_TRUE(tx);
      EXPECT_EQ(this->tx_hashes[1], iroha::hash(*tx));
    } else {
      EXPECT_TRUE(tx);
      EXPECT_EQ(this->tx_hashes[3], iroha::hash(*tx));
    }
  });
  ASSERT_TRUE(wrapper.validate());
}

/**
 * @given block store with 2 blocks totally containing 3 txs created by
 * user1@test
 * AND 1 tx created by user2@test
 * @when query to get transactions with non-existing transaction hashes
 * @then nullopt values are retrieved
 */
TEST_F(BlockQueryTest, GetTransactionsIncludesNonExistingTxHashes) {
  iroha::hash256_t invalid_tx_hash_1, invalid_tx_hash_2;
  invalid_tx_hash_1[0] = 1;
  invalid_tx_hash_2[0] = 2;
  auto wrapper = make_test_subscriber<CallExact>(
      blocks->getTransactions({invalid_tx_hash_1, invalid_tx_hash_2}), 2);
  wrapper.subscribe(
      [](auto transaction) { EXPECT_EQ(boost::none, transaction); });
  ASSERT_TRUE(wrapper.validate());
}

/**
 * @given block store with 2 blocks totally containing 3 txs created by
 * user1@test
 * AND 1 tx created by user2@test
 * @when query to get transactions with empty vector
 * @then no transactions are retrieved
 */
TEST_F(BlockQueryTest, GetTransactionsWithEmpty) {
  // transactions' hashes are empty.
  auto wrapper =
      make_test_subscriber<CallExact>(blocks->getTransactions({}), 0);
  wrapper.subscribe();
  ASSERT_TRUE(wrapper.validate());
}

/**
 * @given block store with 2 blocks totally containing 3 txs created by
 * user1@test
 * AND 1 tx created by user2@test
 * @when query to get transactions with non-existing txhash and existing txhash
 * @then queried transactions and empty transaction
 */
TEST_F(BlockQueryTest, GetTransactionsWithInvalidTxAndValidTx) {
  // TODO 15/11/17 motxx - Use EqualList VerificationStrategy
  iroha::hash256_t invalid_tx_hash_1;
  invalid_tx_hash_1[0] = 1;
  auto wrapper = make_test_subscriber<CallExact>(
      blocks->getTransactions({invalid_tx_hash_1, tx_hashes[0]}), 2);
  wrapper.subscribe([this](auto tx) {
    static auto subs_cnt = 0;
    subs_cnt++;
    if (subs_cnt == 1) {
      EXPECT_EQ(boost::none, tx);
    } else {
      EXPECT_TRUE(tx);
      EXPECT_EQ(this->tx_hashes[0], iroha::hash(*tx));
    }
  });
  ASSERT_TRUE(wrapper.validate());
}
