#include <boost/test/unit_test.hpp>
#include <eosio/chain/abi_serializer.hpp>
#include <eosio/chain/permission_object.hpp>
#include <eosio/chain/resource_limits.hpp>
#include <eosio/testing/tester.hpp>
#include "contracts.hpp"

using namespace eosio;
using namespace fc;

using namespace testing;
using namespace chain;


BOOST_AUTO_TEST_SUITE(kata_tests)


BOOST_AUTO_TEST_CASE(post) try {

    tester t{setup_policy::none};

   // Create accounts and load contracts
   	t.create_account(N(eosio.token), config::system_account_name, false, true);
   	t.set_code(N(eosio.token), eosio::testing::contracts::token_wasm());
   	t.set_abi(N(eosio.token), eosio::testing::contracts::token_abi().data());

    t.create_account(N(myaccount), config::system_account_name, false, true);
    t.set_code(N(myaccount), eosio::testing::contracts::kata_wasm());
    t.set_abi(N(myaccount), eosio::testing::contracts::kata_abi().data());
   
    t.produce_block();

    // create SYS tokens
    t.push_action(
        N(eosio.token), N(create), N(eosio.token),
        mutable_variant_object("issuer", "eosio.token")(
        "maximum_supply", "10000000.0000 SYS")
    );
    
    //issue SYS tokens
    t.push_action(
        N(eosio.token), N(issue), N(eosio.token),
        mutable_variant_object
        ("to", "eosio.token")
        ("quantity", "1000.0000 SYS")
        ("memo", "")
    );
    
    // Send symbols to myaccount
    t.push_action(N(eosio.token), N(transfer), N(eosio.token),
        mutable_variant_object("from", "eosio.token")("to", "myaccount")(
        "quantity", "100.0000 SYS")("memo", "memo"));

    t.push_action(
            N(myaccount), N(newcategory), N(myaccount),
            mutable_variant_object("name", "checking")
        );

    t.push_action(
            N(myaccount), N(newcategory), N(myaccount),
            mutable_variant_object("name", "saving")
        );

    BOOST_CHECK_THROW(
        [&] {
            t.push_action(
                        N(myaccount), N(create), N(myaccount),
                        mutable_variant_object("name", "checking")
                    );
        }(),
        fc::exception);

    BOOST_CHECK_THROW(
        [&] {
            t.push_action(
                        N(myaccount), N(move), N(myaccount),
                        mutable_variant_object("from", "default")("to", "checking")("amount", "101.0000 SYS")
                    );
        }(),
        fc::exception);

    t.push_action(
            N(myaccount), N(move), N(myaccount),
            mutable_variant_object("from", "default")("to", "checking")("amount", "40.0000 SYS")
        );

    BOOST_CHECK_THROW(
        [&] {
            t.push_action(
                        N(myaccount), N(move), N(myaccount),
                        mutable_variant_object("from", "default")("to", "checking")("amount", "60.0001 SYS")
                    );
        }(),
        fc::exception);

    t.push_action(
            N(myaccount), N(move), N(myaccount),
            mutable_variant_object("from", "default")("to", "checking")("amount", "60.0000 SYS")
        );

    BOOST_CHECK_THROW(
        [&] {
            t.push_action(
                        N(myaccount), N(move), N(myaccount),
                        mutable_variant_object("from", "checking")("to", "saving")("amount", "-10.0000 SYS")
                    );
        }(),
        fc::exception);
        
    t.push_action(
            N(myaccount), N(move), N(myaccount),
            mutable_variant_object("from", "checking")("to", "saving")("amount", "60.0000 SYS")
        );

    BOOST_CHECK_THROW(
        [&] {
            t.push_action(
                        N(myaccount), N(move), N(myaccount),
                        mutable_variant_object("from", "checking")("to", "saving")("amount", "40.0005 SYS")
                    );
        }(),
        fc::exception);

    t.push_action(
            N(myaccount), N(move), N(myaccount),
            mutable_variant_object("from", "checking")("to", "saving")("amount", "40.0000 SYS")
        );

    t.push_action(
            N(myaccount), N(move), N(myaccount),
            mutable_variant_object("from", "saving")("to", "default")("amount", "70.0000 SYS")
        );

    BOOST_CHECK_THROW(
        [&] {
            t.push_action(
                        N(myaccount), N(move), N(myaccount),
                        mutable_variant_object("from", "saving")("to", "default")("amount", "30.0001 SYS")
                    );
        }(),
        fc::exception);

    t.push_action(
            N(myaccount), N(move), N(myaccount),
            mutable_variant_object("from", "saving")("to", "default")("amount", "30.0000 SYS")
        );

    t.push_action(
            N(myaccount), N(move), N(myaccount),
            mutable_variant_object("from", "default")("to", "checking")("amount", "25.0000 SYS")
        );

    t.push_action(
            N(myaccount), N(move), N(myaccount),
            mutable_variant_object("from", "default")("to", "saving")("amount", "25.0000 SYS")
        );
        
    t.push_action(N(eosio.token), N(transfer), N(myaccount),
         mutable_variant_object("from", "myaccount")("to", "eosio.token")(
        "quantity", "10.0000 SYS")("memo", "memo"));

    BOOST_CHECK_THROW(
        [&] {
            t.push_action(
                        N(eosio.token), N(transfer), N(myaccount),
                        mutable_variant_object("from", "myaccount")("to", "eosio.token")("quantity", "50.0000 SYS")("memo", "memo")
                    );
        }(),
        fc::exception);

    t.push_action(
            N(myaccount), N(move), N(myaccount),
            mutable_variant_object("from", "saving")("to", "default")("amount", "10.0000 SYS")
        );
        
    // print categories
    t.push_action(
        N(myaccount), N(displayall), N(myaccount),
        mutable_variant_object()
    );

    t.push_action(N(eosio.token), N(transfer), N(myaccount),
         mutable_variant_object("from", "myaccount")("to", "eosio.token")(
        "quantity", "50.0000 SYS")("memo", "memo"));

    // Send symbols to myaccount
   	t.push_action(N(eosio.token), N(transfer), N(eosio.token),
         mutable_variant_object("from", "eosio.token")("to", "myaccount")(
        "quantity", "200.0000 SYS")("memo", "memo"));
 
    BOOST_CHECK_THROW(
        [&] {
            t.push_action(
                        N(myaccount), N(move), N(myaccount),
                        mutable_variant_object("from", "default")("to", "checking")("amount", "201.0000 SYS")
                    );
        }(),
        fc::exception);       
 
    t.push_action(
            N(myaccount), N(move), N(myaccount),
            mutable_variant_object("from", "default")("to", "checking")("amount", "200.0000 SYS")
        );       
}


FC_LOG_AND_RETHROW()

BOOST_AUTO_TEST_SUITE_END()