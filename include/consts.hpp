#pragma once

using namespace eosio;

    enum ships_num { battleship, cargoship, colonizer };

    // Debugging constants
    const eosio::name ADMIN = "dexaraniiznx"_n;

    // Global constants.
    const uint64_t max_building_task_index = 9;
    const uint64_t max_assembling_task_index = 11;

    const uint64_t update_cycle = 1800; // (30 min) in seconds.
    const uint64_t time_cycle_length = 60;     // Resource incomes and time-consuming actions
                                        // are calculated against this variable.
                                        // (1 min) in seconds
    const double   time_per_distance = 25.0; // Global multiplier of a fleet movement speed,
                                             // the more the `time_per_distance` the longer it takes for a fleet
                                             // to reach its destination.
    const double   gas_per_distance  = 0.33; // Global multiplier of gas comsumption.
    const double   time_scale        = 1.0;  // Global multiplier applied to each time-consuming process.

    const uint64_t colonization_duration = 3600; // 30 minutes by default.

    // Resource incomes.
    // Incomes are calculated for 100 seconds period.
    const uint64_t metal_income_per_lvl = 10;
    const uint64_t crystal_income_per_lvl = 7; 
    const uint64_t gas_income_per_lvl = 5;

    const uint64_t metal_storage_per_lvl = 5000;
    const uint64_t crystal_storage_per_lvl = 5000;
    const uint64_t gas_storage_per_lvl = 5000;



    struct [[eosio::class]] ship
    {
        uint64_t type; 
        // TYPES:
        // 0 battleship
        // 1 cargoship
        // 2 colonizer
        // 3 researcher
        // 4 recycler

        /* Combat stats. */
        uint64_t power;
        uint64_t durability;
        uint64_t cargo;

        /* Not implemented. */
        //uint64_t crew; // Not yet implemented; reserved for further updates.
        //uint64_t passengers; // Not yet implemented; reserved for further updates.

        //uint64_t speed; // Not yet implemented; reserved for further updates.

        /* Building costs. */
        std::vector<uint64_t> cost;
        uint64_t time_cost;

        /* Flight costs    */
        uint64_t gas_cost_launch;
        uint64_t gas_cost_travel;
        uint64_t speed;
    };

    struct [[eosio::class]] base_building
    {
        std::vector<uint64_t> cost;
        uint16_t time_cost;
    };

    struct [[eosio::class]] base_resource
    {
        uint16_t type;
        //uint16_t weight;
        uint16_t volume;
    };

    std::vector<ship> base_ships = { 
        {
            // Battleship
            .type            = 0,
            .power           = 100,
            .durability      = 150,
            .cargo           = 20,
            .cost            = { 150, 30, 0 },
            .time_cost       = 60,
            .gas_cost_launch = 5,
            .gas_cost_travel = 10,
            .speed = 100
        },
        {
            // Cargoship
            .type            = 1,
            .power           = 10,
            .durability      = 250,
            .cargo           = 500,
            .cost            = { 200, 150, 0 },
            .time_cost       = 180,
            .gas_cost_launch = 80,
            .gas_cost_travel = 10,
            .speed = 100
        },
        {
            // Colonizer
            .type            = 2,
            .power           = 10,
            .durability      = 700,
            .cargo           = 350,
            .cost            = { 400, 750, 150 },
            .time_cost       = 360,
            .gas_cost_launch = 200,
            .gas_cost_travel = 8,
            .speed = 100
        }
    };

    std::vector<base_resource> base_resources = { 
        {
            // Metal
            .type            = 0,
            .volume          = 1
        },
        {
            // Crystal
            .type            = 1,
            .volume          = 1
        },
        {
            // Metal
            .type            = 2,
            .volume          = 1
        }
    };

    std::vector<base_building> base_buildings = { 
        {
            // Com. center
            .time_cost       = 150,
            .cost            = { 150, 30, 0 }
        },
        {
            // Assembly
            .time_cost       = 150,
            .cost            = { 150, 30, 0 }
        },
        {
            // Shield
            .time_cost       = 150,
            .cost            = { 150, 30, 0 }
        },
        {
            // Metal mine
            .time_cost       = 150,
            .cost            = { 150, 30, 0 }
        },
        {
            // Crystal mine
            .time_cost       = 150,
            .cost            = { 150, 30, 0 }
        },
        {
            // Gas mine
            .time_cost       = 150,
            .cost            = { 150, 30, 0 }
        },
        {
            // Metal storage
            .time_cost       = 150,
            .cost            = { 150, 30, 0 }
        },
        {
            // Crystal storage
            .time_cost       = 150,
            .cost            = { 150, 30, 0 }
        },
        {
            // Gas storage
            .time_cost       = 150,
            .cost            = { 150, 30, 0 }
        }
    };
