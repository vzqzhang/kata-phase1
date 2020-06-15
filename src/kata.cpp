#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio.token/eosio.token.hpp>

class [[eosio::contract("kata")]] kata : eosio::contract {
    eosio::asset zero;
public:
    using eosio::contract::contract;

    kata(eosio::name receiver, eosio::name code, eosio::datastream<const char *> ds) : contract(receiver, code, ds)
    {
        eosio::symbol_code sym("SYS");
        zero = eosio::asset(0, eosio::symbol(sym, 4));
    }
    
    struct [[eosio::table]] category {
        uint64_t id;
        eosio::name unique_name;
        eosio::asset balance;
        
        uint64_t primary_key() const { return id; }
        uint64_t getname() const { return unique_name.value; }
    };
    
    using category_table = eosio::multi_index<"category"_n, category,
        eosio::indexed_by<"getname"_n, eosio::const_mem_fun<category, uint64_t, &category::getname>>>;
    
    [[eosio::action]]
    void sync() {
        category_table categories{get_self(), 0};
        auto default_it = categories.find(eosio::name("default").value);
        if (default_it != categories.end()) {
            return;
        }

        categories.emplace(get_self(), [&](auto & ct) {
            ct.id = categories.available_primary_key();
            ct.unique_name = eosio::name("default");
            ct.balance = eosio::token::get_balance("eosio.token"_n, get_self(), zero.symbol.code());
        });
    }

    [[eosio::action]]
    void newcategory(eosio::name name) {
        require_auth(get_self());
        category_table categories{get_self(), 0};
        auto ct_table = categories.get_index<"getname"_n>();

        auto default_it = ct_table.find(eosio::name("default").value);
        if (default_it == ct_table.end()) {
            sync();
            default_it = ct_table.find(eosio::name("default").value);
        }

        auto category_it = ct_table.find(name.value);
        eosio::check(category_it == ct_table.end(), "Category is already created.");
        
        categories.emplace(get_self(), [&](auto & ct) {
            ct.id = categories.available_primary_key();
            ct.unique_name = name;
            ct.balance = zero;
        });
    }
    
    [[eosio::action]]
    void displayall() {
        require_auth(get_self());
        
        category_table ct_table{get_self(), 0};
        for (auto it = ct_table.begin(); it != ct_table.end(); ++it) {
            eosio::print_f("%: %, %\n", it->id, it->unique_name, it->balance.to_string());
        }
    }
    
    [[eosio::action]]
    void move(eosio::name from, eosio::name to, eosio::asset amount) {
        require_auth(get_self());

        category_table categories{get_self(), 0};
        auto ct_table = categories.get_index<"getname"_n>();
        auto default_it = ct_table.find(eosio::name("default").value);
        if (default_it == ct_table.end()) {
            sync();
            default_it = ct_table.find(eosio::name("default").value);
        }

        eosio::check(amount > zero, "Amount is invalid.");

        auto from_it = ct_table.find(from.value);
        eosio::check(from_it != ct_table.end(), std::string("Category ") + from.to_string() + std::string(" does not exist."));
        
        eosio::check(from_it->balance >= amount, std::string("The balance in category ") + from.to_string() + std::string(" is less than ") + amount.to_string());

        auto to_it = ct_table.find(to.value);
        eosio::check(to_it != ct_table.end(), std::string("Category ") + to.to_string() + std::string(" does not exist."));
        
        ct_table.modify(from_it, get_self(), [&]( auto& ct ) {
            ct.balance = ct.balance - amount;
        });
        
        ct_table.modify(to_it, get_self(), [&]( auto& ct ) {
            ct.balance = ct.balance + amount;
        });
    }
    
    [[eosio::on_notify("eosio.token::transfer")]]
    void transfer(eosio::name from, eosio::name to, eosio::asset balance, std::string memo) {
        
        category_table categories{get_self(), 0};
        auto ct_table = categories.get_index<"getname"_n>();

        if (from == get_self()) {
            balance = -balance;
        }
        
        auto it = ct_table.find(eosio::name("default").value);
        if (it == ct_table.end()) {
            sync();
            it = ct_table.find(eosio::name("default").value);
        }

        if (from == get_self()) {
            eosio::check(it->balance >= -balance, "Insufficient symbols to transfer out.");
        }
        
        ct_table.modify(it, get_self(), [&]( auto& ct ) {
            ct.balance = ct.balance + balance;
        });
    }
    
    // cleos push action <contract_name> deletedata [ ] -p <contract_name>
    [[eosio::action]]
    void deletedata() {
        require_auth(_self);
        category_table categories{get_self(), 0};
        
        auto it = categories.begin();
        while(it != categories.end()) {
            it = categories.erase(it);
        }
    }
};
