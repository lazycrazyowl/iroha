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

#ifndef IROHA_PROTO_PAGER_HPP
#define IROHA_PROTO_PAGER_HPP

#include "backend/protobuf/common_objects/trivial_proto.hpp"
#include "interfaces/common_objects/pager.hpp"
#include "interfaces/transaction.hpp"
#include "primitive.pb.h"

namespace shared_model {
  namespace proto {
    class Pager final : public CopyableProto<interface::Pager,
                                             iroha::protocol::Pager,
                                             Pager> {
     public:
      template <typename PagerType>
      explicit Pager(PagerType &&pager)
          : CopyableProto(std::forward<PagerType>(pager)),
            transaction_hash_(proto_->tx_hash()),
            limit_(proto_->limit()) {}

      Pager(const Pager &o) : Pager(o.proto_) {}

      Pager(Pager &&o) noexcept : Pager(std::move(o.proto_)) {}

      const interface::Transaction::HashType &transactionHash() const override {
        return transaction_hash_;
      }

      const Limit &limit() const override {
        return limit_;
      }

     private:
      // lazy
      template <typename T>
      using Lazy = detail::LazyInitializer<T>;

      interface::Transaction::HashType transaction_hash_;
      Pager::Limit limit_;
    };
  }  // namespace proto
}  // namespace shared_model

#endif  // IROHA_PROTO_PAGER_HPP
