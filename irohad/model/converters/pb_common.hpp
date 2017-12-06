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

#ifndef IROHA_PB_COMMON_HPP
#define IROHA_PB_COMMON_HPP

#include "amount/amount.hpp"
#include "commands.pb.h"
#include "common/types.hpp"
#include "cryptography/ed25519_sha3_impl/internal/ed25519_impl.hpp"
#include "cryptography/ed25519_sha3_impl/internal/sha3_hash.hpp"
#include "model/queries/pager.hpp"
#include "model/signature.hpp"
#include "queries.pb.h"

namespace iroha {
  namespace model {
    namespace converters {
      // amount
      protocol::Amount serializeAmount(iroha::Amount iroha_amount);
      iroha::Amount deserializeAmount(protocol::Amount pb_amount);

      /**
       * Serialize pager from model to proto
       * @param pager - model Pager
       * @return protocol Pager
       */
      protocol::Pager serializePager(const model::Pager &pager);

      /**
       * Deserialize pager from proto to model
       * @param pb_pager - model Pager. If the size of tx_hash is invalid,
       * regards hash as empty.
       * @return model Pager
       */
      model::Pager deserializePager(const protocol::Pager &pb_pager);

      /**
       * Calculate hash from protobuf model object
       * @tparam T - protobuf model type
       * @param pb - protobuf model object
       * @return hash of object payload
       */
      template <typename T>
      hash256_t hash(const T &pb) {
        return sha3_256(pb.payload().SerializeAsString());
      }
    }  // namespace converters
  }    // namespace model
}  // namespace iroha

#endif  // IROHA_PB_COMMON_HPP
