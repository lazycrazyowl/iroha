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

#ifndef IROHA_SHARED_MODEL_PAGER_HPP
#define IROHA_SHARED_MODEL_PAGER_HPP

#include "cryptography/blob.hpp"
#include "interfaces/base/primitive.hpp"
#include "interfaces/transaction.hpp"
#include "model/queries/pager.hpp"
#include "utils/string_builder.hpp"

namespace shared_model {
  namespace interface {

    /**
     * Class represents pager for get account/account asset transactions api
     */
    class Pager : public Primitive<Pager, iroha::model::Pager> {
     public:
      /**
       * Type of limit
       */
      using Limit = uint16_t;

      /**
       * Max limit
       */
      static constexpr Limit MAX_PAGER_LIMIT = 100;
      static_assert(MAX_PAGER_LIMIT == iroha::model::Pager::MAX_PAGER_LIMIT,
                    "Should be equal to old fashioned model's");

      /**
       * @return transaction hash of pager
       */
      virtual const Transaction::HashType &transactionHash() const = 0;

      /**
       * @return limit of pager
       */
      virtual const Limit &limit() const = 0;

      bool operator==(const Pager &rhs) const override {
        return transactionHash() == rhs.transactionHash()
            and limit() == rhs.limit();
      }

      OldModelType *makeOldModel() const override {
        iroha::model::Pager *oldStylePager = new iroha::model::Pager();
        using OldStyleTxHash =
            decltype(std::declval<iroha::model::Pager>().tx_hash);
        oldStylePager->tx_hash =
            transactionHash().template makeOldModel<OldStyleTxHash>();
        oldStylePager->limit = limit();
        return oldStylePager;
      }

      std::string toString() const override {
        return detail::PrettyStringBuilder()
            .init("Pager")
            .append("transactionHash", transactionHash().toString())
            .append("limit", std::to_string(limit()))
            .finalize();
      }
    };
  }  // namespace interface
}  // namespace shared_model
#endif  // IROHA_SHARED_MODEL_PAGER_HPP
