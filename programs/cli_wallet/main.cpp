#include <algorithm>
#include <iomanip>
#include <iostream>
#include <map>

#include <fc/io/json.hpp>
#include <fc/io/stdio.hpp>
#include <fc/network/http/server.hpp>
#include <fc/network/http/websocket.hpp>
#include <fc/rpc/cli.hpp>
#include <fc/rpc/http_api.hpp>
#include <fc/rpc/websocket_api.hpp>
#include <fc/smart_ref_impl.hpp>

#include <golos/utilities/key_conversion.hpp>

#include <golos/application/api.hpp>
#include <golos/wallet/wallet.hpp>

#include <fc/interprocess/signals.hpp>
#include <boost/algorithm/string.hpp>

#include <fc/log/console_appender.hpp>
#include <fc/log/file_appender.hpp>
#include <fc/log/logger_config.hpp>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/regex.hpp>
#include <fc/variant.hpp>

#ifdef WIN32
# include <signal.h>
#else

# include <csignal>

#endif


using namespace graphene::utilities;
using namespace golos::application;
using namespace golos::chain;
using namespace golos::wallet;
using namespace std;

void daemon_mode();

void nom_daemon_mode (
    const boost::program_options::variables_map & options,
    const std::vector < std::string > & commands,
    const bool & interactive,
    std::shared_ptr<fc::rpc::cli> wallet_cli,
    const fc::api<wallet_api> & wapi
);

void parse_commands (
    const boost::program_options::variables_map & options,
    std::vector < std::string > & commands,
    bool & interactive
);

int main(int argc, char **argv) {
    try {

        boost::program_options::options_description opts;
        opts.add_options()
                ("help,h", "Print this help message and exit.")
                ("server-rpc-endpoint,s", boost::program_options::value<string>()->implicit_value("ws://127.0.0.1:8090"), "Server websocket RPC endpoint")
                ("server-rpc-user,u", boost::program_options::value<string>(), "Server Username")
                ("server-rpc-password,p", boost::program_options::value<string>(), "Server Password")
                ("cert-authority,a", boost::program_options::value<string>()->default_value("_default"), "Trusted CA bundle file for connecting to wss:// TLS server")
                ("rpc-endpoint,r", boost::program_options::value<string>()->implicit_value("127.0.0.1:8091"), "Endpoint for wallet websocket RPC to listen on")
                ("rpc-tls-endpoint,t", boost::program_options::value<string>()->implicit_value("127.0.0.1:8092"), "Endpoint for wallet websocket TLS RPC to listen on")
                ("rpc-tls-certificate,c", boost::program_options::value<string>()->implicit_value("server.pem"), "PEM certificate for wallet websocket TLS RPC")
                ("rpc-http-endpoint,H", boost::program_options::value<string>()->implicit_value("127.0.0.1:8093"), "Endpoint for wallet HTTP RPC to listen on")
                ("daemon,d", "Run the wallet in daemon mode")
                ("rpc-http-allowip", boost::program_options::value<vector<string>>()->multitoken(), "Allows only specified IPs to connect to the HTTP endpoint")
                ("wallet-file,w", boost::program_options::value<string>()->implicit_value("wallet.json"), "wallet to load")
                ("chain-id", boost::program_options::value<string>(), "chain ID to connect to")
                ("commands,c", boost::program_options::value<string>(), "Enable non-interactive mode")
                ;

        vector<string> allowed_ips;

        std::vector < std::string > commands;                
        bool interactive = true;

        boost::program_options::variables_map options;

        boost::program_options::store(boost::program_options::parse_command_line(argc, argv, opts), options);

        if (options.count("help")) {
            std::cout << opts << "\n";
            return 0;
        }

        parse_commands(options, commands, interactive);

        if (options.count("rpc-http-allowip") &&
            options.count("rpc-http-endpoint")) {
            allowed_ips = options["rpc-http-allowip"].as<vector<string>>();
            wdump((allowed_ips));
        }

        fc::path data_dir;
        fc::logging_config cfg;
        fc::path log_dir = data_dir / "logs";

        fc::file_appender::config ac;
        ac.filename = log_dir / "rpc" / "rpc.log";
        ac.flush = true;
        ac.rotate = true;
        ac.rotation_interval = fc::hours(1);
        ac.rotation_limit = fc::days(1);

        std::cout << "Logging RPC to file: "
                  << (data_dir / ac.filename).preferred_string() << "\n";

        cfg.appenders.push_back(fc::appender_config("default", "console", fc::variant(fc::console_appender::config())));
        cfg.appenders.push_back(fc::appender_config("rpc", "file", fc::variant(ac)));

        cfg.loggers = {fc::logger_config("default"), fc::logger_config("rpc")};
        cfg.loggers.front().level = fc::log_level::info;
        cfg.loggers.front().appenders = {"default"};
        cfg.loggers.back().level = fc::log_level::debug;
        cfg.loggers.back().appenders = {"rpc"};


        //
        // TODO:  We read wallet_data twice, once in main() to grab the
        //    socket info, again in wallet_api when we do
        //    load_wallet_file().  Seems like this could be better
        //    designed.
        //
        wallet_data wdata;

        fc::path wallet_file(options.count("wallet-file")
                             ? options.at("wallet-file").as<string>()
                             : "wallet.json");
        if (fc::exists(wallet_file)) {
            wdata = fc::json::from_file(wallet_file).as<wallet_data>();
        } else {
            std::cout << "Starting a new wallet\n";
        }

        // but allow CLI to override
        if (options.count("server-rpc-endpoint")) {
            wdata.ws_server = options.at("server-rpc-endpoint").as<std::string>();
        }
        if (options.count("server-rpc-user")) {
            wdata.ws_user = options.at("server-rpc-user").as<std::string>();
        }
        if (options.count("server-rpc-password")) {
            wdata.ws_password = options.at("server-rpc-password").as<std::string>();
        }


        fc::http::websocket_client client(options["cert-authority"].as<std::string>());
        idump((wdata.ws_server));
        auto con = client.connect(wdata.ws_server);
        auto apic = std::make_shared<fc::rpc::websocket_api_connection>(*con);

        auto remote_api = apic->get_remote_api<login_api>(1);
        edump((wdata.ws_user)(wdata.ws_password));
        // TODO:  Error message here
        FC_ASSERT(remote_api->login(wdata.ws_user, wdata.ws_password));

        auto wapiptr = std::make_shared<wallet_api>(wdata, remote_api);
        wapiptr->set_wallet_filename(wallet_file.generic_string());
        wapiptr->load_wallet_file();

        fc::api<wallet_api> wapi(wapiptr);

        auto wallet_cli = std::make_shared<fc::rpc::cli>();
        for (auto &name_formatter : wapiptr->get_result_formatters()) {
            wallet_cli->format_result(name_formatter.first, name_formatter.second);
        }

        boost::signals2::scoped_connection closed_connection(con->closed.connect([=] {
            cerr << "Server has disconnected us.\n";
            wallet_cli->stop();
        }));
        (void)(closed_connection);

        if (wapiptr->is_new()) {
            std::cout << "Please use the set_password method to initialize a new wallet before continuing\n";
            wallet_cli->set_prompt("new >>> ");
        } else {
            wallet_cli->set_prompt("locked >>> ");
        }

        boost::signals2::scoped_connection locked_connection(wapiptr->lock_changed.connect([&](bool locked) {
            wallet_cli->set_prompt(locked ? "locked >>> " : "unlocked >>> ");
        }));

        auto _websocket_server = std::make_shared<fc::http::websocket_server>();
        if (options.count("rpc-endpoint")) {
            _websocket_server->on_connection([&](const fc::http::websocket_connection_ptr &c) {
                std::cout << "here... \n";
                wlog(".");
                auto wsc = std::make_shared<fc::rpc::websocket_api_connection>(*c);
                wsc->register_api(wapi);
                c->set_session_data(wsc);
            });
            ilog("Listening for incoming RPC requests on ${p}", ("p", options.at("rpc-endpoint").as<string>()));
            _websocket_server->listen(fc::ip::endpoint::from_string(options.at("rpc-endpoint").as<string>()));
            _websocket_server->start_accept();
        }

        string cert_pem = "server.pem";
        if (options.count("rpc-tls-certificate")) {
            cert_pem = options.at("rpc-tls-certificate").as<string>();
        }

        auto _websocket_tls_server = std::make_shared<fc::http::websocket_tls_server>(cert_pem);
        if (options.count("rpc-tls-endpoint")) {
            _websocket_tls_server->on_connection([&](const fc::http::websocket_connection_ptr &c) {
                auto wsc = std::make_shared<fc::rpc::websocket_api_connection>(*c);
                wsc->register_api(wapi);
                c->set_session_data(wsc);
            });
            ilog("Listening for incoming TLS RPC requests on ${p}", ("p", options.at("rpc-tls-endpoint").as<string>()));
            _websocket_tls_server->listen(fc::ip::endpoint::from_string(options.at("rpc-tls-endpoint").as<string>()));
            _websocket_tls_server->start_accept();
        }

        set<fc::ip::address> allowed_ip_set;

        auto _http_server = std::make_shared<fc::http::server>();
        if (options.count("rpc-http-endpoint")) {
            ilog("Listening for incoming HTTP RPC requests on ${p}", ("p", options.at("rpc-http-endpoint").as<string>()));
            for (const auto &ip : allowed_ips) {
                allowed_ip_set.insert(fc::ip::address(ip));
            }

            _http_server->listen(fc::ip::endpoint::from_string(options.at("rpc-http-endpoint").as<string>()));
            //
            // due to implementation, on_request() must come AFTER listen()
            //
            _http_server->on_request(
                    [&](const fc::http::request &req, const fc::http::server::response &resp) {
                        auto itr = allowed_ip_set.find(fc::ip::endpoint::from_string(req.remote_endpoint).get_address());
                        if (itr == allowed_ip_set.end()) {
                            elog("rejected connection from ${ip} because it isn't in allowed set ${s}", ("ip", req.remote_endpoint)("s", allowed_ip_set));
                            resp.set_status(fc::http::reply::NotAuthorized);
                            return;
                        }
                        std::shared_ptr<fc::rpc::http_api_connection> conn =
                                std::make_shared<fc::rpc::http_api_connection>();
                        conn->register_api(wapi);
                        conn->on_request(req, resp);
                    });
        }

        if (!options.count("daemon")) {
            nom_daemon_mode ( options, commands, interactive, wallet_cli, wapi );
        } else {
            daemon_mode();
        }
        wapi->save_wallet_file(wallet_file.generic_string());
        locked_connection.disconnect();
        closed_connection.disconnect();
    }
    catch (const fc::exception &e) {
        std::cout << e.to_detail_string() << "\n";
        return -1;
    }
    return 0;
}

void daemon_mode() {
    fc::promise<int>::ptr exit_promise = new fc::promise<int>("UNIX Signal Handler");
    fc::set_signal_handler([&exit_promise](int signal) {
        exit_promise->set_value(signal);
    }, SIGINT);

    ilog("Entering Daemon Mode, ^C to exit");
    exit_promise->wait();
}

void nom_daemon_mode (
    const boost::program_options::variables_map & options,
    const std::vector < std::string > & commands,
    const bool & interactive,
    std::shared_ptr<fc::rpc::cli> wallet_cli,
    const fc::api<wallet_api> & wapi
) {
    wallet_cli->register_api(wapi);
    if (!interactive) {
        std::vector < std::pair < std::string, std::string > > commands_output;
        for (auto const &command : commands) {
            try {
                auto result = wallet_cli->exec_command ( command );
                commands_output.push_back ( {command, result} ) ;
            }
            catch ( const fc::exception& e ) {
                std::cout << e.to_detail_string() << '\n';
            }
        }
        for (auto i : commands_output) {
            // Format of output  
            std::cout << i.first << '\n' << fc::json::to_pretty_string( i.second ) << '\n';
        }
    }
    else {
        wallet_cli->start();
        wallet_cli->wait();
    }
}

void parse_commands (
    const boost::program_options::variables_map & options,
    std::vector < std::string > & commands, bool & interactive
) {
    if (options.count("commands")) {
        // If you would like to enable non-interactive mode, then you should
        // pass commands you like cli_wallet to execute via 'commands' program
        // option. All commands should be separated with "&&". The order does matter!
        // EXAMPLE: ./cli_wallet --commands="unlock verystrongpassword && some_command arg1 arg2 && another_command arg1 arg2 arg3"

        interactive = false;
        auto tmp_commmad_string = options["commands"].as<string>();

        // Here will be stored the strings that will be parsed by the "&&"
        std::vector < std::string > unchecked_commands;
        auto delims = "&&";
        boost::algorithm::split_regex( unchecked_commands, tmp_commmad_string, boost::regex( delims ) );

        for (auto x : unchecked_commands ) {
            boost::trim(x);
            if (x != "") {
                commands.push_back(x);
            }
        }
    }
}