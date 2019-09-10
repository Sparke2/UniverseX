#include <universe_x.hpp>
#include <resolvers.hpp>

void universe::eraseplayer(eosio::name acc) {
   if(acc == ADMIN)
   {
      require_auth(ADMIN);
   }
   else
   {
      require_auth(acc);
   }
   
   auto player = _players.find(acc.value);
   _players.erase(player);
}

void universe::spawnplayer(eosio::name acc, uint64_t sector_id) {
   require_auth(acc);
    
   auto player = _players.find(acc.value);

   eosio_assert(player == _players.end(), "Can't spawn an existing player.");

   if(player == _players.end())
   {
      _players.emplace(get_self(), [&](auto& p) {
         p.account = acc;
         p.last_spawned = now();
         p.owned_planet_ids[0] = sector_id;
         p.num_owned_planets = 1;
      });
      
      auto planet = _sectors.find(sector_id);

      init_planet(sector_id,  // ID of the point at the map
              (*planet).x, // x
              (*planet).y, // y
              120,         // temperature
              20,          // buildable size
              1,           // owner (system variable:: number of active owners of a planet)
              acc,         // eosio::name of the owner (only 1 owner is supported in this release)

              0,           // Com. center  level
              0,           // Assembly     level
              1,           // Metal mine   level
              1,           // Crystal mine level
              0,           // Gas mine     level

              250,         // Metal     resource
              750,         // Crystal   resource
              300,         // Gas       resource
              0,           // Battleships in orbital fleet
              0            // Cargoships  in orbital fleet
              );

   }   
   eosio::print("\n Player joined! Congratulations ", acc);
}

void universe::setcyclic(eosio::name acc, bool allowed) {
   require_auth(acc);

   auto _state = _gstate.find(0);
   _gstate.modify(_state, get_self(), [&](auto& s) {
      s.cyclic_updates_allowed = allowed;
         //eosio::print("\n Updating global state CYCLIC ALLOWD:   ", allowed);
   });
}

void universe::updatemap(eosio::name acc, uint64_t min_cycles, uint64_t iterations, uint64_t delay, bool repeat)
{
   auto _state = _gstate.find(0);
   if(_gstate.begin() == _gstate.end())
   {
      init_state();
   }
   else
   {
      _gstate.modify(_state, get_self(), [&](auto& s) {
         s.last_update_cycle = now();
      });
   }
   
   // commented out due to excess CPU consumption
   //auto num_sectors = std::distance(_sectors.cbegin(),_sectors.cend());

   uint64_t num_sectors = (*_gstate.find(0)).active_sectors;
   //   eosio::print("\n STATE UUP id:  ", (*_state).last_updated_id);
   uint64_t count = 0;
   for (uint64_t i = (*_state).last_updated_id; i <= num_sectors && count < iterations; i++)
   {
      count++;
      auto planet = _sectors.find(i);
      if((*planet).has_planet)
      {
         uint16_t passed_cycles = (now() - (*planet).last_updated[update_type::event_update]) / time_cycle_length;
         if(passed_cycles >= min_cycles)
         {
            updateplanet(i);
         }
      }
      if(i == num_sectors)
      {
         // Reset the updating position and update the planet 0
         // because i will be incremented in the next iteration.
         i = 0;
         planet = _sectors.find(i);
         if((*planet).has_planet)
         {
            uint16_t passed_cycles = (now() - (*planet).last_updated[update_type::event_update]) / time_cycle_length;
            if(passed_cycles >= min_cycles)
            {
               updateplanet(i);
            }
         }
      }
   }

   // *************************************
   // Queue the update deferred TX if necessary

   if(repeat && (*_state).cyclic_updates_allowed)
   {
      require_auth(_self);
        // always double check the action name as it will fail silently
        // in the deferred transaction
      eosio::transaction t{};
      t.actions.emplace_back(
            eosio::permission_level(_self, "active"_n),
            _self, // who will receive this call
            "updatemap"_n, // function [action] call name
            std::make_tuple(acc, min_cycles, iterations, delay, repeat)); // arguments for the action
      t.delay_sec = delay;
      t.send(now(), _self, true);
   }
}

void universe::adminmodify(eosio::name acc,          // ADMIN account only
                             uint64_t  id,           // ID of the planet to submit a gift
                             uint64_t  mode,         // 1 - overwrite, 2 - add
                             uint64_t  type,         // Type of the gift i.e. resource, ship, building etc.
                                                     //                  0 - resources
                                                     //                  1 - building levels
                                                     //                  2 - planetary fleets (orbital)
                             uint64_t  gift_obj_id,  // ID of the gift (resource ID, ship ID or other)
                             int64_t   count)        // Quantity of the fungible gifted objects, Level of the building modified
{
   auto planet = _sectors.find(id);
   _sectors.modify(planet, get_self(), [&](auto& p) {
      if(type == 0)
      {
         // Modify planetary resources
         if (mode == 1)
         {
            p.resources[gift_obj_id] = count;
           // eosio::print("\n -- -- -- -- -- -- -- -- -- \n Planetary RESOURCE is modified \n Modify mode is OVERWRITE \n setted    ", count, " of units of ", gift_obj_id);
         }
         else if (mode == 2)
         {
            p.resources[gift_obj_id] += count;
           // eosio::print("\n -- -- -- -- -- -- -- -- -- \n Planetary RESOURCE is modified \n Modify mode is ADD \n added    ", count, " of units of ", gift_obj_id);
         }
      }
      if(type == 1)
      {
         // Modify planetary buildings
         if (mode == 1)
         {
            p.planetary_buildings[gift_obj_id].level = count;
           // eosio::print("\n -- -- -- -- -- -- -- -- -- \n Planetary BUILDING is modified \n Modify mode is OVERWRITE \n setted    ", count, " level of ", gift_obj_id);
         }
         else if (mode == 2)
         {
            p.planetary_buildings[gift_obj_id].level += count;
          //  eosio::print("\n -- -- -- -- -- -- -- -- -- \n Planetary BUILDING is modified \n Modify mode is ADD \n added    ", count, " level of ", gift_obj_id);
         }
      }
      if(type == 2)
      {
         // Modify planetary fleet (orbital)
         if (mode == 1)
         {
            p.planetary_fleet.ships[gift_obj_id] = count;
          //  eosio::print("\n -- -- -- -- -- -- -- -- -- \n Planetary FLEET is modified \n Modify mode is OVERWRITE \n setted    ", count, " number of ships ", gift_obj_id);
         }
         else if (mode == 2)
         {
            p.planetary_fleet.ships[gift_obj_id] += count;
          //  eosio::print("\n -- -- -- -- -- -- -- -- -- \n Planetary FLEET is modified \n Modify mode is ADD \n added    ", count, " number of ships ", gift_obj_id);
         }
      }
   });
}




void universe::addtask(eosio::name acc,         // Owner of the planet.
                           uint64_t id,         // ID of the planet to add a task to.
                           uint64_t type,       // Task type: 0 - building, 1 - assembling ships
                           uint64_t task_id,    // Task ID: id of the buildable structure or assembled ship
                           uint64_t quantity,   // For assembling tasks only! Quantity of the ships being assembled.
                           bool     autoupdate) // When set to TRUE the contract will queue a deferred TX to complete task.
{
   // Function for user-driven task placement.
   require_auth(acc);

   auto planet = _sectors.find(id);
   eosio_assert((*planet).owner > 0, "No owner presented");
   eosio_assert((*planet).colonization_start != 0, "Can not place order for a planet that is currently in colonization stage");
   eosio_assert((*planet).owner_name == acc, "Unable to access someone elses planet");
   task _task;
   _task.time_required = 0;

   _sectors.modify(planet, get_self(), [&](auto& p) {
      // 0 - 8 are building tasks while 8 - 10 are assemble tasks
      _task.task_goal_id = task_id;
      _task.start_time = now();
      _task.quantity = quantity;

      if (type == task_type_num::building_task) {
         // Building task.

         uint64_t current_level = p.planetary_buildings[_task.task_goal_id].level;
         eosio_assert(quantity == 0, "Quantity is not available for BUILDING tasks");
         eosio_assert(p.resources[resource_num::metal]   >= base_buildings[_task.task_goal_id].cost[resource_num::metal] + (current_level * base_buildings[_task.task_goal_id].cost[resource_num::metal] / 2), "Not enough metal");
         eosio_assert(p.resources[resource_num::crystal] >= base_buildings[_task.task_goal_id].cost[resource_num::crystal] + (current_level * base_buildings[_task.task_goal_id].cost[resource_num::crystal] / 2), "Not enough crystals");
         eosio_assert(p.resources[resource_num::gas]     >= base_buildings[_task.task_goal_id].cost[resource_num::gas] + (current_level * base_buildings[_task.task_goal_id].cost[resource_num::gas] / 2), "Not enough gas");

         p.resources[resource_num::metal]   -= base_buildings[_task.task_goal_id].cost[resource_num::metal] + (current_level * base_buildings[_task.task_goal_id].cost[resource_num::metal] / 2);
         p.resources[resource_num::crystal] -= base_buildings[_task.task_goal_id].cost[resource_num::crystal] + (current_level * base_buildings[_task.task_goal_id].cost[resource_num::crystal] / 2);
         p.resources[resource_num::gas]     -= base_buildings[_task.task_goal_id].cost[resource_num::gas] + (current_level * base_buildings[_task.task_goal_id].cost[resource_num::gas] / 2);

         _task.time_required = time_scale * (base_buildings[_task.task_goal_id].time_cost + base_buildings[_task.task_goal_id].time_cost * current_level);

         p.building_queue.push_back(_task);
      } 
      else if (type == task_type_num::assembling_task){
         // Assemble ships task.
         eosio_assert(p.planetary_buildings[building_num::assembly].level > 0, "Planet does not have an assembly to build ships");
         eosio_assert(quantity > 0, "Quantity must be >0; assembling 0 ships make unnecessary CPU usage");

         eosio_assert(p.resources[resource_num::metal]   >= base_ships[_task.task_goal_id].cost[resource_num::metal] * quantity, "Not enough metal");
         eosio_assert(p.resources[resource_num::crystal] >= base_ships[_task.task_goal_id].cost[resource_num::crystal] * quantity, "Not enough crystals");
         eosio_assert(p.resources[resource_num::gas]     >= base_ships[_task.task_goal_id].cost[resource_num::gas] * quantity, "Not enough gas");

         p.resources[resource_num::metal]   -= base_ships[_task.task_goal_id].cost[resource_num::metal] * quantity;

         p.resources[resource_num::crystal] -= base_ships[_task.task_goal_id].cost[resource_num::crystal] * quantity;
         p.resources[resource_num::gas]     -= base_ships[_task.task_goal_id].cost[resource_num::gas] * quantity;

         _task.time_required = time_scale * (base_ships[_task.task_goal_id].time_cost * quantity);

         p.assembling_queue.push_back(_task);
      }
      else {
         //eosio_assert(0 > 1, "Task type not found \n Try ", task_type_num::building_task, " for building\n", task_type_num::assembling_task, " for assembling");
      }
   });


   print("Task placed  ", now() ," type:   ", type);
   if(_task.time_required > 0 && autoupdate)
   {
      setupdate(acc, _self, id, _task.time_required + 10, now());
   }
}

void universe::setupdate(eosio::name from,    // Sender of the transaction.
                         eosio::name payer, // The account that will pay for update.
                              uint64_t id,    // ID of a planet to update.
                              uint64_t delay, // Delay of the deferred tx.
                              uint64_t tx_id) // ID of the deferred tx (used for unnecessary tx replacement).
{
   require_auth(from);
   eosio::transaction t{};
        // always double check the action name as it will fail silently
        // in the deferred transaction
   t.actions.emplace_back(
            // when sending to _self a different authorization can be used
            // otherwise _self must be used

            eosio::permission_level(from, "active"_n),
                                                _self, // who will receive this call
                                     "updateplanet"_n, // function [action] call name
                           std::make_tuple(id)); // arguments for the action

        // set delay in seconds
        t.delay_sec = delay;

        // first argument is a unique sender id
        // second argument is account paying for RAM/CPU/NET
        // third argument can specify whether an in-flight transaction
        // with this senderId should be replaced
        // if set to false and this senderId already exists
        // this action will fail
        t.send(tx_id, payer, true);

  // eosio::print("Scheduled PLANET UPDATE with a delay of ", delay);
}

void universe::init_planet(uint64_t sector_id,
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

                 uint64_t battleships,
                 uint64_t cargoships                 
                 )
{
   auto planet = _sectors.find(sector_id);
   //eosio_assert((*planet).owner < 1, "Unable to change already owned planed");

   _sectors.modify(planet, get_self(), [&](auto& p) {
      p.has_planet = true;
      p.x = x;
      p.y = y;
      p.temperature = temperature;
      p.size = size;
      p.owner = owner;
      p.owner_name = owner_name;

/*
      eosio::print("\n X  ", x);
      eosio::print("\n Y  ", y);
      eosio::print("\n temperature  ", temperature);
      eosio::print("\n size  ", size);
      eosio::print("\n owner  ", owner);
      eosio::print("\n owner_name  ", owner_name);


      eosio::print("\n metal_mine  ", metal_mine);
      eosio::print("\n crystal_mine  ", crystal_mine);
      eosio::print("\n gas_mine  ", gas_mine);
*/
      p.last_updated.clear();
      p.planetary_buildings.clear();
      p.resources.clear();
      p.planetary_fleet.ships.clear();
      p.planetary_fleet.cargo.clear();

      p.planetary_fleet.id = "0";

      for(uint16_t i = 0; i < update_types; i++)
      {
         p.last_updated.push_back(now());
      }

      for (uint64_t i = 0; i<max_building_task_index; i++)
      {
         building b;
         b.type = i;
         b.level = 0;
         p.planetary_buildings.push_back(b);
      }

      p.planetary_buildings[building_num::com_center].level   = com_center;
      p.planetary_buildings[building_num::assembly].level     = assembly;
      p.planetary_buildings[building_num::metal_mine].level   = metal_mine;
      p.planetary_buildings[building_num::crystal_mine].level = crystal_mine;
      p.planetary_buildings[building_num::gas_mine].level     = gas_mine;

      p.resources.push_back(0);  // Metal 
      p.resources.push_back(0);  // Crystal 
      p.resources.push_back(0);  // Gas 

      p.resources[resource_num::metal]   = metal;
      p.resources[resource_num::crystal] = crystal;
      p.resources[resource_num::gas]     = gas;


      for (uint64_t ship_type = 0; ship_type < base_ships.size(); ship_type++)
      {
         p.planetary_fleet.ships.push_back(0);
      }
      

      p.planetary_fleet.ships[ships_num::battleship] = battleships;
      p.planetary_fleet.ships[ships_num::cargoship] = cargoships;
   });
}

void universe::addplanet(eosio::name s, uint64_t x, uint64_t y, uint64_t id) {
    // require_auth(s);
    eosio::print("Add planet ", id);
}

void universe::updateplanet(uint64_t id)
{
   auto planet = _sectors.find(id);
   if((*planet).colonization_start != 0)
   {
      mechanic_finish_colonization(id);
   }
   resolve_building(id);
   resolve_assembling(id);
   resolve_fleets(id);
   resolve_income(id);

   // Resolving upkeep.

   // Resolving population & events.

   _sectors.modify(planet, get_self(), [&](auto& p) {
      p.last_updated[update_type::event_update] = now();
   });

   _gstate.modify(_gstate.find(0), get_self(), [&](auto& s) {
      s.last_updated_id = id;
      eosio::print("\n Last up id:  ", s.last_updated_id);
   });
}

void universe::fleetorder(eosio::name acc, uint64_t planet_id, uint64_t destination_id, uint64_t order_type, uint64_t battleship_count, uint64_t cargoship_count, uint64_t colonizer_count, uint64_t cargo_metal,  uint64_t cargo_crystal,  uint64_t cargo_gas)
{
   auto home   = _sectors.find(planet_id);
   auto target = _sectors.find(destination_id);
   require_auth(acc);
   
   eosio_assert((*home).has_planet, "Home sector must have a planet");
   eosio_assert((*target).has_planet, "Destination sector must have a planet");

   eosio_assert((*home).planetary_fleet.ships[ships_num::battleship] >= battleship_count, "Not enough battleships to send");
   eosio_assert((*home).planetary_fleet.ships[ships_num::cargoship]  >= cargoship_count, "Not enough cargoships to send");
   eosio_assert((*home).planetary_fleet.ships[ships_num::colonizer]  >= colonizer_count, "Not enough colonizers to send");

   eosio_assert((*home).resources[resource_num::metal]   >= cargo_metal, "Not enough metal");
   eosio_assert((*home).resources[resource_num::crystal] >= cargo_crystal, "Not enough crystals");
   eosio_assert((*home).resources[resource_num::gas]     >= cargo_gas, "Not enough gas");

   if(order_type == order_num::colonize)
   {
      eosio_assert(colonizer_count > 0, "At least 1 colonizer is required to proceed with the colonization task");
   }

   // Subtract ships.
   _sectors.modify(home, get_self(), [&](auto& p) {
      p.planetary_fleet.ships[ships_num::battleship] -= battleship_count;
      p.planetary_fleet.ships[ships_num::cargoship]  -= cargoship_count;
      p.planetary_fleet.ships[ships_num::colonizer]  -= colonizer_count;
   });
   



   // Create fleet.
   fleet _fleet;

   // Generate variables for ships:
   // 0 - battleship
   // 1 - cargo
   // 2 - colonizer
   _fleet.ships.push_back(0); // Battleships.
   _fleet.ships.push_back(0); // Cargoships.
   _fleet.ships.push_back(0); // Colonizers.

   _fleet.ships[ships_num::battleship] = battleship_count;
   _fleet.ships[ships_num::cargoship]  = cargoship_count;
   _fleet.ships[ships_num::colonizer]  = colonizer_count;

   _fleet.cargo.push_back(cargo_metal);   // Load metal into 0 position (resource_num::metal)
   _fleet.cargo.push_back(cargo_crystal); // Load crystal into 1 position (resource_num::crystal)
   _fleet.cargo.push_back(cargo_gas);     // Load gas into 2 position (resource_num::gas)

   _fleet.home_id = planet_id;
   _fleet.destination_id = destination_id;

   // Generating unique identifier to assign this fleer.
   _fleet.id = "";
   _fleet.id += now();
   _fleet.id += planet_id;
   _fleet.id += destination_id;
   _fleet.id += (*home).fleets.size();

   _fleet.leave_time = now();
   _fleet.travel_time = get_flight_time(_fleet);

   _fleet.order = order_type;

   // Init fleet in a planet.
   _sectors.modify(home, get_self(), [&](auto& p) {
      p.fleets.push_back(_fleet);

      // Subtract resources.
      p.resources[resource_num::metal] -= cargo_metal;
      p.resources[resource_num::crystal] -= cargo_crystal;
      p.resources[resource_num::gas] -= cargo_gas;
      
      eosio_assert(p.resources[resource_num::gas] >= get_fuel_cost(_fleet), "Not enough GAS to pay for fleet fuel");
      p.resources[resource_num::gas] -= get_fuel_cost(_fleet);
   });
}



void universe::getstats(uint64_t planet_id)
{
   auto planet = _sectors.find( planet_id );

   eosio::print("\n Planet:   ", (*planet).has_planet);
   eosio::print("\n Com. center  lvl:   ", (*planet).planetary_buildings[universe::building_num::com_center].level);
   eosio::print("\n Assembly     lvl:   ", (*planet).planetary_buildings[universe::building_num::assembly].level);
   eosio::print("\n Metal Mine   lvl:   ", (*planet).planetary_buildings[universe::building_num::metal_mine].level);
   eosio::print("\n Crystal Mine lvl:   ", (*planet).planetary_buildings[universe::building_num::crystal_mine].level);
   eosio::print("\n Gas Mine     lvl:   ", (*planet).planetary_buildings[universe::building_num::gas_mine].level);

   eosio::print("\n Metal quantity:     ", (*planet).resources[universe::resource_num::metal]);
   eosio::print("\n Crystals quantity:  ", (*planet).resources[universe::resource_num::crystal]);
   eosio::print("\n Gas quantity:       ", (*planet).resources[universe::resource_num::gas]);

   eosio::print("\n Orbital fleet: Battleships: ", (*planet).planetary_fleet.ships[ships_num::battleship]);
   eosio::print("\n Orbital fleet: Cargoships:  ", (*planet).planetary_fleet.ships[ships_num::cargoship]);
   eosio::print("\n Orbital fleet: Colonizers:  ", (*planet).planetary_fleet.ships[ships_num::colonizer]);

   eosio::print("\n Active fleets:  ", (*planet).fleets.size());

   for(uint16_t i = 0; i<(*planet).fleets.size();i++)
   {
      eosio::print("\n Fleet[", i, "] : ", (*planet).fleets[i].id);
   }
}

void universe::adminit(eosio::name acc, uint64_t planet_id, uint64_t x, uint64_t y, uint64_t battleships, uint64_t cargoships, uint64_t resource_metal, uint64_t resource_gas)
{
   auto planet   = _sectors.find(planet_id);

   _sectors.modify(planet, get_self(), [&](auto& p) {
      p.x = x;
      p.y = y;
      p.has_planet = true;
      p.owner_name = acc;
      
      // Fill the buildings Vector (array).
      for (uint64_t i = 0; i<max_building_task_index; i++)
         {
            building b;
            b.type = i;
            b.level = 0;
            p.planetary_buildings.push_back(b);
         }
      // Fill the resources Vector (array).
      p.resources.push_back(0); // Generate 0 Metal.
      p.resources.push_back(0); // Generate 0 Crystal.
      p.resources.push_back(0); // Generate 0 Gas.

      // Fill resources (Doesn't work if using `push_back` directly)
      p.resources[resource_num::metal] = resource_metal;
      p.resources[resource_num::gas]   = resource_gas;

      p.planetary_fleet.ships.push_back(0); // Generate Ships[0] - battleship variable
      p.planetary_fleet.ships.push_back(0); // Generate Ships[1] - cargoship variable
      p.planetary_fleet.ships.push_back(0); // Generate Ships[2] - colonizer variable

      p.planetary_fleet.ships[ships_num::battleship] = battleships;
      p.planetary_fleet.ships[ships_num::cargoship] = cargoships;

      p.planetary_fleet.destination_id   = 0;
      p.planetary_fleet.leave_time       = 0;
      p.planetary_fleet.travel_time      = 0;

      p.planetary_fleet.order = universe::order_num::idle;
      p.planetary_fleet.home_id = planet_id;
      p.planetary_fleet.id = "0";
      });
}

void universe::erasemap(eosio::name owner, uint64_t iterations)
{
   uint64_t counter = 0;
   for( auto itr = _sectors.begin(); _sectors.begin() != _sectors.end() && counter != iterations; counter++) {
        itr = _sectors.erase(itr);
   }

/*
   _gstate.modify( (*_gstate.find(0)), get_self(), [&](auto& s) {
      s.active_sectors = 0;
   });
*/
}

void universe::setstate(eosio::name owner)
{   
   auto itr = _gstate.find(0);
   if(itr == _gstate.end())
   {
      _gstate.emplace(get_self(), [&](auto& s) {
         s.last_update_cycle = now();
         s.cyclic_updates_allowed = true;
         s.universe_id = 0;
         s.active_sectors = 0;
      });
   }
   else 
   {
      _gstate.modify( itr, get_self(), [&](auto& s) {
         s.cyclic_updates_allowed = true;
         s.active_sectors = 0;
      });
   }
}

void universe::clear_state()
{   
   for( auto itr = _gstate.begin(); _gstate.begin() != _gstate.end(); ) {
      itr = _gstate.erase(itr);
   }
}

void universe::clear_players()
{   
   for( auto itr = _players.begin(); _players.begin() != _players.end(); ) {
      itr = _players.erase(itr);
   }
}

void universe::cleargame(eosio::name owner)
{   
   require_auth(owner);

   //uint64_t counter = (*_gstate.find(0)).active_sectors;
   auto state_table = _gstate.find(0);
   if(state_table != _gstate.end())
   {
      erasemap(owner, (*state_table).active_sectors);
   }
   clear_state();
   clear_players();
}

void universe::init_state()
{   
   _gstate.emplace(get_self(), [&](auto& s) {
         s.last_update_cycle = now();
         s.cyclic_updates_allowed = true;
         s.universe_id = 0;

         s.active_sectors = 0;
   });
}

void universe::initmap(eosio::name acc, uint64_t height, uint64_t width, uint64_t offsetx, uint64_t offsety, bool init_planets) {
   
   //require_auth(acc);

   uint64_t counter;
   eosio::print("Init an EMPTY map");

   if(_gstate.begin() == _gstate.end())
   {
      // First check if the global state table exists
      // create it if not.
      counter = 0;
      init_state();
   }

   // Save the number of existing SECTORs for newly added ID offsets.
   counter = (*_gstate.find(0)).active_sectors;

   // Then add the newly created sectors to the global variable.
   _gstate.modify( (*_gstate.find(0)), get_self(), [&](auto& s) {
      s.active_sectors += (height * width);
   });

   eosio::print("\n Counter:    ", counter);
  // eosio::print("\n Global state ACTIVE SECTORS:   ", (*_gstate.find(0)).active_sectors);
    
   for( int i = 0; i < height; i++ ) 
   {
      for( int j = 0; j < width; j++ ) 
      {
         bool planet_presence = false;

         // Planet presence conditions. Exists for testing reasons.
         // Normally planets are uploaded and generated by ADMIN 
         // after the initialization of the empty map.

         // Generation is handled off-chain.

         // WARNING! Test only! Do not init that much planets in a real game!
         if (init_planets) 
         {
            planet_presence = true;
         }
         _sectors.emplace(get_self(), [&](auto& p) {
            p.id = counter;
            p.x = i + offsetx;
            p.y = j + offsety;
            p.owner = 0;
         });
         if(planet_presence)
         {   
            init_planet( counter,  // ID of the point at the map (unique)
                           i + offsetx, // x
                           j + offsety, // y
                           120,         // temperature
                           20,          // buildable size
                           0,           // owner (system variable:: number of active owners of a planet)
                           ""_n,         // eosio::name of the owner (only 1 owner is supported in this release)

                           0,           // Com. center  level
                           0,           // Assembly     level
                           0,           // Metal mine   level
                           0,           // Crystal mine level
                           0,           // Gas mine     level

                           0,           // Metal     resource
                           0,           // Crystal   resource
                           0,           // Gas       resource
                           0,           // Battleships in orbital fleet
                           0            // Cargoships  in orbital fleet
              );
         }
         counter++;
      }
   }
}

uint64_t universe::get_fuel_cost(fleet _fleet)
{
   double fuel_cost = 0;
   for (uint16_t i = 0; i < base_ships.size(); i++)
   {
      fuel_cost += (double)_fleet.ships[i] * (double)base_ships[i].gas_cost_launch;
      fuel_cost += (double)_fleet.ships[i] * (double)base_ships[i].gas_cost_travel * get_distance(_fleet.home_id, _fleet.destination_id) * (double)gas_per_distance;
   }
   return (uint64_t)fuel_cost;
}

uint64_t universe::get_flight_time(fleet _fleet)
{
   uint64_t travel_time = 0;
   for (uint16_t i = 0; i < base_ships.size(); i++)
   {
      if ((base_ships[i].speed / time_per_distance) * get_distance(_fleet.home_id, _fleet.destination_id) > travel_time)
      {
         travel_time = time_scale * ((base_ships[i].speed / time_per_distance) * get_distance(_fleet.home_id, _fleet.destination_id));
      }
   }
   return travel_time;
}

double universe::get_distance(uint64_t _from,   // ID of the home sector
                                uint64_t  _to)    // ID of the target sector
{
   auto from = _sectors.find(_from);
   auto to = _sectors.find(_to);

/*
   eosio::print("\n From X ", (*from).x);
   eosio::print("\n From Y ", (*from).y);
   eosio::print("\n To   X ", (*to).x);
   eosio::print("\n To   Y ", (*to).y);
*/
   double distance = sqrt( pow((double)(*from).x - (double)(*to).x, 2) + pow((double)(*from).y - (double)(*to).y, 2) );
   //eosio::print("\n Distance from ", _from, " to ", _to, "  = ", distance);
   return distance;
}

uint64_t universe::get_storage_max(uint64_t planet_id, uint64_t resource_type)
{
   auto itr = _sectors.find(planet_id);

   if(resource_type == universe::resource_num::metal)
   {
      return ((*itr).planetary_buildings[universe::building_num::metal_storage].level * metal_storage_per_lvl + 1500);
   }
   if(resource_type == universe::resource_num::crystal)
   {
      return ((*itr).planetary_buildings[universe::building_num::crystal_storage].level * crystal_storage_per_lvl + 1500);
   }
   if(resource_type == universe::resource_num::gas)
   {
      return ((*itr).planetary_buildings[universe::building_num::gas_storage].level * gas_storage_per_lvl + 1500);
   }
}

uint64_t universe::get_resource_income(uint64_t planet_id, uint64_t resource_type)
{
   auto itr = _sectors.find(planet_id);

   if(resource_type == universe::resource_num::metal)
   {
      return ((*itr).planetary_buildings[universe::building_num::metal_mine].level * metal_income_per_lvl);
   }
   if(resource_type == universe::resource_num::crystal)
   {
      return ((*itr).planetary_buildings[universe::building_num::crystal_mine].level * crystal_income_per_lvl);
   }
   if(resource_type == universe::resource_num::gas)
   {
      return ((*itr).planetary_buildings[universe::building_num::gas_mine].level * gas_income_per_lvl);
   }
}


/*******************************************/
/****** COMMENTED HELPERS FOR FUTURE USE ***/
/*******************************************/

/*
    ACTION send(eosio::name from, const std::string &message, uint64_t delay)
    {
        require_auth(from);

        eosio::transaction t{};
        // always double check the action name as it will fail silently
        // in the deferred transaction
        t.actions.emplace_back(
            // when sending to _self a different authorization can be used
            // otherwise _self must be used
            eosio::permission_level(from, "active"_n),
            // account the action should be send to
            _self,
            // action to invoke
            "deferred"_n,
            // arguments for the action
            std::make_tuple(from, message));

        // set delay in seconds
        t.delay_sec = delay;

        // first argument is a unique sender id
        // second argument is account paying for RAM
        // third argument can specify whether an in-flight transaction
        // with this senderId should be replaced
        // if set to false and this senderId already exists
        // this action will fail
        t.send(1, from);

        eosio::print("Scheduled with a delay of ", delay);
    }
    */



   /*
   void validate_price(asset price, int grade) {
        assert_true(price.symbol == S(4,EOS) , "only EOS token allowed");
        assert_true(price.is_valid(), "invalid price");
        assert_true(price.amount > 0, "must price positive quantity");

        assert_true(price.amount >= get_min_market_price(grade), "too small price");
        assert_true(price.amount <= kv_max_market_price, "too big price");
    }

   void assert_true(bool test, const char* cstr) {
        eosio_assert(test ? 1 : 0, cstr);
        auto time_now = time_util::now_shifted();
    }
    */

EOSIO_DISPATCH( universe, (eraseplayer)(cleargame)(setstate)(updatemap)(setcyclic)(setupdate)(adminmodify)(addtask)(spawnplayer)(adminit)(addplanet)(initmap)(erasemap)(updateplanet)(getstats)(fleetorder))
