#ifndef GOLOS_DATABASE_HPP
#define GOLOS_DATABASE_HPP

#include <steemit/chain/evaluators/evaluator_registry.hpp>
#include <steemit/chain/index.hpp>

#include "database_policy.hpp"


namespace steemit {
    namespace chain {
        class database final : public database_t {
        public:
            database() = default;

            ~database() = default;

        protected:
            void initialize_indexes();

            void initialize_evaluators();

            void apply_operation(const operation &op);

            evaluator_registry<operation> registry;
        };


        shared_ptr<database> make_database();
    }
}
#endif //GOLOS_DATABASE_HPP