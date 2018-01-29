#pragma once

#include <functional>
#include <map>
#include <memory>
#include <vector>

#include <fc/optional.hpp>
#include <fc/variant_object.hpp>
#include <golos/plugins/json_rpc/utility.hpp>
#include <golos/plugins/json_rpc/plugin.hpp>
#include <golos/plugins/database_api/applied_operation.hpp>
#include <golos/plugins/database_api/state.hpp>
#include <golos/plugins/database_api/api_objects/feed_history_api_object.hpp>
#include <golos/plugins/database_api/api_objects/owner_authority_history_api_object.hpp>
#include <golos/plugins/database_api/api_objects/account_recovery_request_api_object.hpp>
#include <golos/plugins/database_api/api_objects/savings_withdraw_api_object.hpp>
#include <golos/plugins/chain/plugin.hpp>

#include "forward.hpp"

namespace golos {
    namespace plugins {
        namespace database_api {
            using namespace golos::chain;
            using namespace golos::protocol;
            using fc::variant;
            using std::vector;
            using plugins::json_rpc::void_type;
            using plugins::json_rpc::msg_pack;

            struct scheduled_hardfork {
                hardfork_version hf_version;
                fc::time_point_sec live_time;
            };

            struct withdraw_route {
                std::string from_account;
                std::string to_account;
                uint16_t percent;
                bool auto_vest;
            };

            enum withdraw_route_type {
                incoming, outgoing, all
            };


            struct tag_count_object {
                string tag;
                uint32_t count;
            };

            struct get_tags_used_by_author {
                vector<tag_count_object> tags;
            };

            struct signed_block_api_object : public signed_block {
                signed_block_api_object(const signed_block &block) : signed_block(block) {
                    block_id = id();
                    signing_key = signee();
                    transaction_ids.reserve(transactions.size());
                    for (const signed_transaction &tx : transactions) {
                        transaction_ids.push_back(tx.id());
                    }
                }

                signed_block_api_object() {
                }

                block_id_type block_id;
                public_key_type signing_key;
                vector<transaction_id_type> transaction_ids;
            };


            ///account_history_api
            struct operation_api_object {
                operation_api_object() {
                }

                operation_api_object(const golos::chain::operation_object &op_obj) : trx_id(op_obj.trx_id),
                        block(op_obj.block), trx_in_block(op_obj.trx_in_block), virtual_op(op_obj.virtual_op),
                        timestamp(op_obj.timestamp) {
                    op = fc::raw::unpack<golos::protocol::operation>(op_obj.serialized_op);
                }

                golos::protocol::transaction_id_type trx_id;
                uint32_t block = 0;
                uint32_t trx_in_block = 0;
                uint16_t op_in_trx = 0;
                uint64_t virtual_op = 0;
                fc::time_point_sec timestamp;
                golos::protocol::operation op;
            };


            using get_account_history_return_type = std::map<uint32_t, applied_operation>;

            using chain_properties_17 = chain_properties;
            using price_17 = price;
            using asset_17 = asset;


            ///               API,                                    args,                return
            DEFINE_API_ARGS(get_active_witnesses, msg_pack, std::vector<account_name_type>)
            DEFINE_API_ARGS(get_block_header, msg_pack, optional<block_header>)
            DEFINE_API_ARGS(get_block, msg_pack, optional<block_header>)
            DEFINE_API_ARGS(get_ops_in_block, msg_pack, std::vector<applied_operation>)
            DEFINE_API_ARGS(get_config, msg_pack, variant_object)
            DEFINE_API_ARGS(get_dynamic_global_properties, msg_pack, dynamic_global_property_api_object)
            DEFINE_API_ARGS(get_chain_properties, msg_pack, chain_properties_17)
            DEFINE_API_ARGS(get_current_median_history_price, msg_pack, price_17)
            DEFINE_API_ARGS(get_feed_history, msg_pack, feed_history_api_object)
            DEFINE_API_ARGS(get_witness_schedule, msg_pack, witness_schedule_api_object)
            DEFINE_API_ARGS(get_hardfork_version, msg_pack, hardfork_version)
            DEFINE_API_ARGS(get_next_scheduled_hardfork, msg_pack, scheduled_hardfork)
            DEFINE_API_ARGS(get_key_references, msg_pack, std::vector<vector<account_name_type> >)
            DEFINE_API_ARGS(get_accounts, msg_pack, std::vector<extended_account>)
            DEFINE_API_ARGS(lookup_account_names, msg_pack, std::vector<optional<account_api_object> >)
            DEFINE_API_ARGS(lookup_accounts, msg_pack, std::set<std::string>)
            DEFINE_API_ARGS(get_account_count, msg_pack, uint64_t)
            DEFINE_API_ARGS(get_owner_history, msg_pack, std::vector<owner_authority_history_api_object>)
            DEFINE_API_ARGS(get_recovery_request, msg_pack, optional<account_recovery_request_api_object>)
            DEFINE_API_ARGS(get_escrow, msg_pack, optional<escrow_api_object>)
            DEFINE_API_ARGS(get_withdraw_routes, msg_pack, std::vector<withdraw_route>)
            DEFINE_API_ARGS(get_account_bandwidth, msg_pack, optional<account_bandwidth_api_object>)
            DEFINE_API_ARGS(get_savings_withdraw_from, msg_pack, std::vector<savings_withdraw_api_object>)
            DEFINE_API_ARGS(get_savings_withdraw_to, msg_pack, std::vector<savings_withdraw_api_object>)
            DEFINE_API_ARGS(get_witnesses, msg_pack, std::vector<optional<witness_api_object> >)
            DEFINE_API_ARGS(get_conversion_requests, msg_pack, std::vector<convert_request_api_object>)
            DEFINE_API_ARGS(get_witness_by_account, msg_pack, optional<witness_api_object>)
            DEFINE_API_ARGS(get_witnesses_by_vote, msg_pack, std::vector<witness_api_object>)
            DEFINE_API_ARGS(lookup_witness_accounts, msg_pack, std::set<account_name_type>)
            DEFINE_API_ARGS(get_open_orders, msg_pack, std::vector<extended_limit_order>)
            DEFINE_API_ARGS(get_witness_count, msg_pack, uint64_t)
            DEFINE_API_ARGS(get_transaction_hex, msg_pack, std::string)
            DEFINE_API_ARGS(get_transaction, msg_pack, annotated_signed_transaction)
            DEFINE_API_ARGS(get_required_signatures, msg_pack, std::set<public_key_type>)
            DEFINE_API_ARGS(get_potential_signatures, msg_pack, std::set<public_key_type>)
            DEFINE_API_ARGS(verify_authority, msg_pack, bool)
            DEFINE_API_ARGS(verify_account_authority, msg_pack, bool)
            DEFINE_API_ARGS(get_account_history, msg_pack, get_account_history_return_type)
            DEFINE_API_ARGS(get_account_balances, msg_pack, std::vector<asset_17>)
            DEFINE_API_ARGS(get_miner_queue, msg_pack, std::vector<account_name_type>)


            /**
             * @brief The database_api class implements the RPC API for the chain database.
             *
             * This API exposes accessors on the database which query state tracked by a blockchain validating node. This API is
             * read-only; all modifications to the database must be performed via transactions. Transactions are broadcast via
             * the @ref network_broadcast_api.
             */
            class plugin final : public appbase::plugin<plugin> {
            public:
                constexpr static const char *plugin_name = "database_api";

                static const std::string &name() {
                    static std::string name = plugin_name;
                    return name;
                }

                APPBASE_PLUGIN_REQUIRES(
                        (json_rpc::plugin)
                        (chain::plugin)
                )

                void set_program_options(boost::program_options::options_description &cli, boost::program_options::options_description &cfg) override{}

                void plugin_initialize(const boost::program_options::variables_map &options) override;

                void plugin_startup() override{}

                void plugin_shutdown() override{}

                plugin();

                ~plugin();

                ///////////////////
                // Subscriptions //
                ///////////////////

                void set_subscribe_callback(std::function<void(const variant &)> cb, bool clear_filter);

                void set_pending_transaction_callback(std::function<void(const variant &)> cb);

                void set_block_applied_callback(std::function<void(const variant &block_header)> cb);

                /**
                 * @brief Stop receiving any notifications
                 *
                 * This unsubscribes from all subscribed markets and objects.
                 */
                void cancel_all_subscriptions();

                DECLARE_API(

                                    /**
                                     *  This API is a short-cut for returning all of the state required for a particular URL
                                     *  with a single query.
                                     */



                                    (get_active_witnesses)

                                    (get_miner_queue)

                                    /////////////////////////////
                                    // Blocks and transactions //
                                    /////////////////////////////

                                    /**
                                     * @brief Retrieve a block header
                                     * @param block_num Height of the block whose header should be returned
                                     * @return header of the referenced block, or null if no matching block was found
                                     */

                                    (get_block_header)

                                    /**
                                     * @brief Retrieve a full, signed block
                                     * @param block_num Height of the block to be returned
                                     * @return the referenced block, or null if no matching block was found
                                     */
                                    (get_block)

                                    /**
                                     *  @brief Get sequence of operations included/generated within a particular block
                                     *  @param block_num Height of the block whose generated virtual operations should be returned
                                     *  @param only_virtual Whether to only include virtual operations in returned results (default: true)
                                     *  @return sequence of operations included/generated within the block
                                     */
                                    (get_ops_in_block)

                                    /////////////
                                    // Globals //
                                    /////////////

                                    /**
                                     * @brief Retrieve compile-time constants
                                     */
                                    (get_config)

                                    /**
                                     * @brief Retrieve the current @ref dynamic_global_property_object
                                     */
                                    (get_dynamic_global_properties)

                                    (get_chain_properties)

                                    (get_current_median_history_price)

                                    (get_feed_history)

                                    (get_witness_schedule)

                                    (get_hardfork_version)

                                    (get_next_scheduled_hardfork)


                                    //////////////
                                    // Accounts //
                                    //////////////

                                    (get_accounts)

                                    /**
                                     * @brief Get a list of accounts by name
                                     * @param account_names Names of the accounts to retrieve
                                     * @return The accounts holding the provided names
                                     *
                                     * This function has semantics identical to @ref get_objects
                                     */
                                    (lookup_account_names)

                                    /**
                                     * @brief Get names and IDs for registered accounts
                                     * @param lower_bound_name Lower bound of the first name to return
                                     * @param limit Maximum number of results to return -- must not exceed 1000
                                     * @return Map of account names to corresponding IDs
                                     */
                                    (lookup_accounts)

                                    //////////////
                                    // Balances //
                                    //////////////

                                    /**
                                     * @brief Get an account's balances in various assets
                                     * @param name of the account to get balances for
                                     * @param assets names of the assets to get balances of; if empty, get all assets account has a balance in
                                     * @return Balances of the account
                                     */
                                    //(get_account_balances)

                                    /**
                                     * @brief Get the total number of accounts registered with the blockchain
                                     */
                                    (get_account_count)

                                    (get_owner_history)

                                    (get_recovery_request)

                                    (get_escrow)

                                    (get_withdraw_routes)

                                    (get_account_bandwidth)

                                    (get_savings_withdraw_from)

                                    (get_savings_withdraw_to)


                                    ///////////////
                                    // Witnesses //
                                    ///////////////

                                    /**
                                     * @brief Get a list of witnesses by ID
                                     * @param witness_ids IDs of the witnesses to retrieve
                                     * @return The witnesses corresponding to the provided IDs
                                     *
                                     * This function has semantics identical to @ref get_objects
                                     */
                                    (get_witnesses)

                                    (get_conversion_requests)

                                    /**
                                     * @brief Get the witness owned by a given account
                                     * @param account The name of the account whose witness should be retrieved
                                     * @return The witness object, or null if the account does not have a witness
                                     */
                                    (get_witness_by_account)

                                    /**
                                     *  This method is used to fetch witnesses with pagination.
                                     *
                                     *  @return an array of `count` witnesses sorted by total votes after witness `from` with at most `limit' results.
                                     */
                                    (get_witnesses_by_vote)

                                    /**
                                     * @brief Get names and IDs for registered witnesses
                                     * @param lower_bound_name Lower bound of the first name to return
                                     * @param limit Maximum number of results to return -- must not exceed 1000
                                     * @return Map of witness names to corresponding IDs
                                     */
                                    (lookup_witness_accounts)

                                    /**
                                     * @brief Get the total number of witnesses registered with the blockchain
                                     */
                                    (get_witness_count)

                                    ////////////
                                    // Assets //
                                    ////////////




                                    ////////////////////////////
                                    // Authority / Validation //
                                    ////////////////////////////

                                    /// @brief Get a hexdump of the serialized binary form of a transaction
                                    (get_transaction_hex)

                                    (get_transaction)

                                    /**
                                     *  This API will take a partially signed transaction and a set of public keys that the owner has the ability to sign for
                                     *  and return the minimal subset of public keys that should add signatures to the transaction.
                                     */
                                    (get_required_signatures)

                                    /**
                                     *  This method will return the set of all public keys that could possibly sign for a given transaction.  This call can
                                     *  be used by wallets to filter their set of public keys to just the relevant subset prior to calling @ref get_required_signatures
                                     *  to get the minimum subset.
                                     */
                                    (get_potential_signatures)

                                    /**
                                     * @return true of the @ref trx has all of the required signatures, otherwise throws an exception
                                     */
                                    (verify_authority)

                                    /*
                                     * @return true if the signers have enough authority to authorize an account
                                     */
                                    (verify_account_authority)


                                    /**
                                     *  Account operations have sequence numbers from 0 to N where N is the most recent operation. This method
                                     *  returns operations in the range [from-limit, from]
                                     *
                                     *  @param from - the absolute sequence number, -1 means most recent, limit is the number of operations before from.
                                     *  @param limit - the maximum number of items that can be queried (0 to 1000], must be less than from
                                     */
                                    (get_account_history)


                )

            private:
/*
                template<typename DatabaseIndex, typename DiscussionIndex>
                std::vector<discussion> feed(
                        const std::set<string> &select_set, const discussion_query &query,
                                             const std::string &start_author, const std::string &start_permlink) const;

                template<typename DatabaseIndex, typename DiscussionIndex>
                std::vector<discussion> blog(
                        const std::set<string> &select_set,
                        const discussion_query &query,
                        const std::string &start_author,
                        const std::string &start_permlink
                ) const;

*/
                struct api_impl;
                std::shared_ptr<api_impl> my;
            };


            inline void register_database_api(){
                appbase::app().register_plugin<plugin>();
            }
        }
    }
}







FC_REFLECT((golos::plugins::database_api::scheduled_hardfork), (hf_version)(live_time))
FC_REFLECT((golos::plugins::database_api::withdraw_route), (from_account)(to_account)(percent)(auto_vest))

FC_REFLECT_ENUM(golos::plugins::database_api::withdraw_route_type, (incoming)(outgoing)(all))

FC_REFLECT((golos::plugins::database_api::tag_count_object), (tag)(count))

FC_REFLECT((golos::plugins::database_api::get_tags_used_by_author), (tags))

FC_REFLECT((golos::plugins::database_api::signed_block_api_object), (block_id)(signing_key)(transaction_ids))

FC_REFLECT((golos::plugins::database_api::operation_api_object),
           (trx_id)(block)(trx_in_block)(op_in_trx)(virtual_op)(timestamp)(op))