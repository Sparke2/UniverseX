#pragma once

#include <eosiolib/eosio.hpp>
#include <eosiolib/system.hpp>
#include <vector>
#include <eosiolib/transaction.hpp>
#include <consts.hpp>
#include <math.h>

// Include random: https://github.com/bada-studio/knights_contract/blob/master/knights/table/outchain/random_val.hpp

using namespace eosio;

class [[eosio::contract("universe_x")]] universe : public eosio::contract {

    public:
    
        universe( eosio::name receiver, eosio::name code, eosio::datastream<const char*> ds ): eosio::contract(receiver, code, ds),  _sectors(receiver, code.value), _players(receiver, code.value), _gstate(receiver, code.value)
    {}

    struct [[eosio::table]] player
    {
        eosio::name account;

        std::vector<uint64_t> owned_planet_ids;
        std::string           pgp_key;                       // Public key of the assimetric encryption,
                                                             // necessary for on-chain private messages.
        uint64_t              last_spawned;
        uint64_t              num_owned_planets;

        uint64_t primary_key() const { return account.value; }

        EOSLIB_SERIALIZE( player, (account)(owned_planet_ids)(pgp_key)(last_spawned)(num_owned_planets))
    };

    struct [[eosio::table]] globalstate
    {
        uint64_t universe_id;
        uint64_t last_update_cycle;     // Timestam of the last update cycle.
        uint64_t last_updated_id;       // ID of the last updated planet. Needed for iterated updates.
        bool     cyclic_updates_allowed;

        uint64_t active_sectors; // The number of map cells presented in the game currently.
        uint64_t primary_key() const { return universe_id; }

        EOSLIB_SERIALIZE( globalstate, (universe_id)(last_update_cycle)(last_updated_id)(cyclic_updates_allowed)(active_sectors))
    };

    const uint16_t update_types = 4; // Preserves the length of the types of updates.
    enum update_type { income_update, task_update, fleet_update, event_update };

    enum task_type_num { building_task, assembling_task };

    struct [[eosio::class]] task
    {
        uint8_t task_goal_id;  // Identifies the ID of a structure being build
                               // or ID of a ship being assembled.

        uint64_t time_required;
        uint64_t start_time;

        uint64_t quantity; // The amount of ships being processed if the task is a ship processing.
    };

    enum building_num { com_center, assembly, shield, metal_mine, crystal_mine, gas_mine, metal_storage, crystal_storage, gas_storage };
    struct [[eosio::class]] building
    {
        uint8_t type; 
        // 0 com. center
        // 1 assembly
        // 2 shield
        // 3 metal mine
        // 4 crystal mine
        // 5 gas mine
        // 6 metal storage
        // 7 crystal storage
        // 8 gas storage

        uint8_t level; // Up to 255 level due to uint8
    };

    enum resource_num { metal, crystal, gas };
    struct [[eosio::class]] resource
    {
        uint8_t type; 
        // 0 metal
        // 1 crystal
        // 2 gas

        uint64_t quantity;
    };


    enum order_num { idle, travel_home, attack, transport, relocate, colonize };
    struct [[eosio::class]] fleet
    {
        std::string id;

        // For task-driven fleets:
   
        //_fleet.id = "";
        //_fleet.id += now();
        //_fleet.id += planet_id;
        //_fleet.id += destination_id;
        //_fleet.id += (*home).fleets.size();

        // For orbital planetary fleets:

        // _fleet.id = "0";

        std::vector<uint8_t> ships;

        std::vector<uint8_t> cargo;

        uint32_t home_id;
        uint32_t destination_id;
        uint64_t leave_time;
        uint64_t travel_time;

        uint8_t order;
    };

    struct [[eosio::table]] sector 
    {
        uint64_t id; // primary key

        // Coordinates at the map
        uint64_t x;
        uint64_t y;

        // Planet persistence and upgradeability
        std::vector<uint64_t> last_updated;

        // Habitability status
        bool has_planet;

        // Planetary constants. Empty for non-planet sectors.
        uint16_t temperature;    // 0 to 200
        uint16_t size;           // 7 to 40
        //uint64_t building_slots; // 0 to 40 => Not yet implemented. For further updates.

        // Decorative planet states
        uint8_t type;            // Visual type of the planet
                                 // 0 - Yellow
                                 // 1 - Red
                                 // 2 - Green
                                 // 3 - Ice white

        std::string name;        // Assigned by the owner of the planet.

        // Ownership status
        uint16_t owner; // Number of owners; reserved for future updates.

        uint64_t colonization_start;
        uint64_t colonization_duration;

        eosio::name owner_name;

        // Structures
        std::vector<building> planetary_buildings;

        // Currently stored resources
        std::vector<uint64_t> resources;

        // Active fleets (proceeding to orders)
        std::vector<fleet> fleets;

        // Orbital fleet (idle)
        fleet planetary_fleet;

        // Task queue
        std::vector<task> building_queue;
        std::vector<task> assembling_queue;

        uint64_t primary_key() const { return id; }
    };

    typedef eosio::multi_index<"sector"_n, sector> sectortable;
    typedef eosio::multi_index<"player"_n, player> playertable;
    typedef eosio::multi_index<"globalstate"_n, globalstate> statetable;


    //// local instances of the multi indexes
    sectortable _sectors;
    playertable _players;
    statetable _gstate;



    // DEBUG actions
    [[eosio::action]] void setupdate(eosio::name from,       // Sender of the transaction.
                                     eosio::name payer,      // The account that will pay for update.
                                        uint64_t id,         // ID of a planet to update.
                                        uint64_t delay,      // Delay of the deferred tx.
                                        uint64_t tx_id);     // ID of the deferred tx (used for unnecessary tx replacement).


    [[eosio::action]] void adminmodify(eosio::name acc,      // ADMIN account only
                                       uint64_t id,          // ID of the planet to submit a gift
                                       uint64_t mode,        // Mode: 1 - overwrite, 2 - add
                                       uint64_t type,        // Type of the gift i.e. resource, ship, building etc.
                                       uint64_t gift_obj_id, // ID of the gift (resource ID, ship ID or other)
                                       int64_t  count);      // Quantity of the fungible gifted objects, Level of the building modified.

    [[eosio::action]] void cleargame(eosio::name owner);

    [[eosio::action]] void addplanet(eosio::name acc, uint64_t x, uint64_t y, uint64_t id);
    [[eosio::action]] void initmap(eosio::name acc, uint64_t height, uint64_t width, uint64_t offsetx, uint64_t offsety, bool init_planets);
    [[eosio::action]] void erasemap(eosio::name acc, uint64_t iterations);

    [[eosio::action]] void eraseplayer(eosio::name acc);
    [[eosio::action]] void setstate(eosio::name owner);
    [[eosio::action]] void updateplanet(uint64_t id);
    [[eosio::action]] void updatemap(    eosio::name acc, // Account name. Arg is passed to `require_auth` in deferred tx.
                                     uint64_t min_cycles, // Minimal amount of `update_cycle`s that should pass since the last update
                                                          // to consider that a planet needs updating.
                                    uint64_t iterations,  // Max number of update iterations to process. Needed for iterated update due to 30ms tx limit.
                                          uint64_t delay, // Interval of sending a replicated auto- updatemap tx.
                                            bool repeat);
    [[eosio::action]] void setcyclic(eosio::name acc, bool allowed);
    [[eosio::action]] void getstats(uint64_t planet_id);

    [[eosio::action]] void fleetorder(eosio::name acc, uint64_t planet_id, uint64_t destination_id, uint64_t order_type, uint64_t battleship_count, uint64_t cargoship_count, uint64_t colonizer_count, uint64_t cargo_metal,  uint64_t cargo_crystal,  uint64_t cargo_gas);

    // Player actions
    [[eosio::action]] void buildtask(eosio::name acc,      // Owner of the planet.
                           uint64_t id,                    // ID of the planet to add a task to.
                           uint64_t task_id,               // Task ID: id of the buildable structure or assembled ship
                                                           //          watch `building_num::` for building IDs
                                                           //          watch `ships_num::`    for shio IDs (located at consts.hpp)
                           bool autoupdate);               // When set to TRUE the contract will queue a deferred TX to complete task.

    [[eosio::action]] void assembletask(eosio::name acc,   // Owner of the planet.
                           uint64_t id,                    // ID of the planet to add a task to.
                           uint64_t task_id,               // Task ID: id of the buildable structure or assembled ship
                                                           //          watch `building_num::` for building IDs
                                                           //          watch `ships_num::`    for shio IDs (located at consts.hpp)
                           uint64_t quantity,              // For assembling tasks only! Quantity of the ships being assembled.
                           bool autoupdate);               // When set to TRUE the contract will queue a deferred TX to complete task.
    [[eosio::action]] void spawnplayer(eosio::name acc, uint64_t planet_id);
    [[eosio::action]] void despwnplayer(eosio::name acc);

    [[eosio::action]] void adminit(eosio::name acc, uint64_t planet_id, uint64_t x, uint64_t y, uint64_t battleships, uint64_t cargoships, uint64_t resource_metal, uint64_t resource_gas);

    // Function for internal use only
    void resolve_building(uint64_t planet_id);
    void resolve_assembling(uint64_t planet_id);
    void resolve_fleets(uint64_t planet_id);
    void resolve_income(uint64_t id);

    void fleet_action_arrive(fleet _fleet, bool _home);
    void fleet_action_attack(fleet _attackers);
    void fleet_action_transport(fleet _fleet);
    void fleet_action_relocate(fleet _fleet);
    void fleet_action_colonize(fleet _fleet);

    void clear_state();
    void clear_players();
    void init_state();

    void mechanic_apply_damage(fleet damaged_fleet, uint64_t damage_amount, bool is_attacking);
    void mechanic_start_colonization(uint64_t planet_id, eosio::name colonizator);
    void mechanic_finish_colonization(uint64_t planet_id);
    
    void init_planet(uint64_t sector_id,
                 uint64_t x,
                 uint64_t y,
                 uint64_t temperature,
                 uint64_t size,
                 uint64_t owner,
                 eosio::name owner_name,

                 uint64_t com_center,
                 uint64_t assembly,
                 uint64_t metal_mine,
                 uint64_t crystal_mine,
                 uint64_t gas_mine,

                 uint64_t metal,
                 uint64_t crystal,
                 uint64_t gas,

                 uint64_t battleship,
                 uint64_t cargoship                 
                 );

    uint64_t get_storage_max(uint64_t planet_id, uint64_t resource_type);
    uint64_t get_resource_income(uint64_t planet_id, uint64_t resource_type);
    uint64_t get_fuel_cost(fleet _fleet);
    uint64_t get_flight_time(fleet _fleet);
    double get_distance(uint64_t from, uint64_t to);
};
