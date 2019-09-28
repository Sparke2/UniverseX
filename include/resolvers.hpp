#include <universe_x.hpp>


void universe::resolve_income(uint64_t id)
{
   auto planet = _sectors.find(id);
   eosio_assert((*planet).has_planet, "The destination must be a planet!");
   
   // Determine how much time passed since the last update
   // of the planet.

   uint64_t update_cycles = (now() - (*planet).last_updated[update_type::income_update]) / time_cycle_length;

   if(update_cycles > 0)
   {

      _sectors.modify(planet, get_self(), [&](auto& p) {
         for (uint16_t i = 0; i < base_resources.size(); i++)
         if (p.resources[i] < get_storage_max(id, i))
         {
            p.resources[i] += get_resource_income(id, i) * update_cycles;
            if(p.resources[i] > get_storage_max(id, i))
            {
               p.resources[i] = get_storage_max(id, i);
            }
         }
         p.last_updated[update_type::income_update] = now();
      });
   }
}

void universe::resolve_building(uint64_t planet_id)
{
   auto planet = _sectors.find(planet_id);
   eosio_assert((*planet).has_planet, "The destination must be a planet!");

   task _task;
   
   if(!(*planet).building_queue.empty())
   {
      _task = (*planet).building_queue[0];
   }

   for (uint64_t i = 0; (!(*planet).building_queue.empty()) && (_task.start_time + _task.time_required <= now()); i++)
   {
      if(_task.task_goal_id <= max_building_task_index)
      {
         eosio::print("\n Resolving a building task <", _task.task_goal_id, "> \n");
         // The task is a building request.
         if(_task.start_time + _task.time_required <= now())
         {
            // Complete the task then.
            _sectors.modify(planet, get_self(), [&](auto& p) {
               eosio::print("\n Building ", _task.task_goal_id);
               p.planetary_buildings[_task.task_goal_id].level++;

               p.building_queue.erase(p.building_queue.begin());
            });
         }
      }

      if(!(*planet).building_queue.empty())
      {
         _task = (*planet).building_queue[0];
      }
   }
   /*
   _sectors.modify(planet, get_self(), [&](auto& p) {
      p.last_updated[update_type::task_update] = now();
   });
   */
}

void universe::resolve_assembling(uint64_t planet_id)
{
   auto planet = _sectors.find(planet_id);
   eosio_assert((*planet).has_planet, "The destination must be a planet!");
   task _task;
   
   if(!(*planet).assembling_queue.empty())
   {
      _task = (*planet).assembling_queue[0];
   }

   for (uint64_t i = 0; (!(*planet).assembling_queue.empty()) && (_task.start_time + _task.time_required <= now()); i++)
   {
      if (_task.task_goal_id <= max_assembling_task_index)
      {
         eosio::print("\n Resolving an assembling task <", _task.task_goal_id, "> \n");
         // The task is a ship assembling request.
         if(_task.start_time + _task.time_required <= now())
        {
            _sectors.modify(planet, get_self(), [&](auto& p) {
               p.planetary_fleet.ships[_task.task_goal_id] += _task.quantity;
               p.assembling_queue.erase(p.assembling_queue.begin());
            });
         }
      }

      if(!(*planet).assembling_queue.empty())
      {
         _task = (*planet).assembling_queue[0];
      }
   }
   /*
   _sectors.modify(planet, get_self(), [&](auto& p) {
      p.last_updated[update_type::task_update] = now();
   });
   */
}

void universe::resolve_fleets(uint64_t planet_id)
{
   auto planet = _sectors.find(planet_id);
   eosio_assert((*planet).has_planet, "The destination must be a planet!");
   if((*planet).fleets.size() > 0)
   {

   std::vector<uint64_t> erase_candidate;
   for( uint64_t i = 0; (*planet).fleets.begin() + i != (*planet).fleets.end(); i++) 
   {
      // First check if the fleet is empty or not.
      // Remove fleet if it is empty.
      uint64_t ships_remaining = 0;
      for (uint64_t ship_type = 0; ship_type < base_ships.size(); ship_type++)
      {
         ships_remaining += (*planet).fleets[i].ships[ship_type];
      }
      if(ships_remaining > 0)
      {
      // @@print("\n Resolving fleets: battleships ", (*planet).fleets[i].ships[ships_num::battleship], "   ++++++++++");

      // Mark finished fleets tasks for erase later.


      // DEBUG ONLY!
      // DELETE FROM DEPLOY VERSION
      /*
         if((*planet).fleets[i].order == order_num::travel_home)
         {
            fleet_action_arrive_home((*planet).fleets[i]);
         }
      */
      
      /*
      if((*planet).fleets[i].leave_time + 2 * (*planet).fleets[i].travel_time < now())
      {
         // If the amount of time passed is greater than the amount of travel time to the destination
         // and back to the planet, consider that the fleet task is finished.

         

         //erase_candidate.push_back(i); << Arrive_Home will erase fleet on its own.
         fleet_action_arrive((*planet).fleets[i], true);
      } 
      
      else
      */
      if((*planet).fleets[i].leave_time + (*planet).fleets[i].travel_time <= now())
      {
         // The fleet reached its destination.
            switch ( (*planet).fleets[i].order )
            {
               case order_num::idle:
                  break;
               case order_num::travel_home:
                  fleet_action_arrive((*planet).fleets[i], true);
                  break;
               case order_num::attack:
                  fleet_action_attack((*planet).fleets[i]);
                  break;
               case order_num::transport:
                  fleet_action_transport((*planet).fleets[i]);
                  break;
               case order_num::relocate:
                  fleet_action_arrive((*planet).fleets[i], false);
                  break;
               case order_num::colonize:
                  fleet_action_colonize((*planet).fleets[i]);
                  break;
               default:
                  break;
            }
         }
      }
      else
      {
         // @@eosio::print("Erase candidate added ", i);
         erase_candidate.push_back(i);
      }
   }

   // Unlike Bulding and Assembling tasks
   // fleets may leave/arrive at a random time.
   // It is not guaranteed that a fleet with greater index will be resolved after
   // a fleet with lesser index.

   // Thats why we mark fleets for erase and erase them later.
   
   // @@eosio::print("\n \n erase candidate size:", erase_candidate.size());

   uint64_t already_erased = 0;
   for (uint64_t i=0; i < erase_candidate.size(); i++)
   {
      // @@eosio::print("\n i:", i);
      // @@eosio::print("Erasing fleet <", erase_candidate[i], ">");

      _sectors.modify(planet, get_self(), [&](auto& p) {
         p.fleets.erase(p.fleets.begin() + erase_candidate[i] - already_erased); // Erases fleet entity at `erase_candidate[i]` position.
         already_erased++; // Each erase introduces an offset -1 for remaining elements.
      });
   }

   }
}

uint64_t calculate_remaining_ships(universe::fleet _fleet)
{
   uint64_t ships_remaining = 0;
   for (uint64_t ship_type = 0; ship_type < base_ships.size(); ship_type++)  
   {
      ships_remaining += _fleet.ships[ship_type];
   }
   return ships_remaining;
}

uint64_t calculate_fleet_cargo(universe::fleet _fleet)
{
   uint64_t cargo = 0;
   for (uint64_t ship_type = 0; ship_type < base_ships.size(); ship_type++)  
   {
      cargo += _fleet.ships[ship_type] * base_ships[ship_type].cargo;
   }
   return cargo;
}

void universe::fleet_action_arrive(fleet _fleet, bool _home)
{
   // Consider that the fleet is travelling home by default
   // most of the actions are fleet home arrivals.
   auto planet = _sectors.find(_fleet.home_id);

   if(!_home)
   {
      // Consider that the fleet is changing location and approaching its target planet.
      planet = _sectors.find(_fleet.destination_id);
   }

   _sectors.modify(planet, get_self(), [&](auto& p) {
      p.resources[resource_num::metal] += _fleet.cargo[resource_num::metal];
      p.resources[resource_num::crystal] += _fleet.cargo[resource_num::crystal];
      p.resources[resource_num::gas] += _fleet.cargo[resource_num::gas];

      for ( uint64_t i=0; i<p.fleets.size(); i++ )
         {
            if(p.fleets[i].id == _fleet.id)
            {
               for (uint64_t ship_type = 0; ship_type < base_ships.size(); ship_type++)
                  {
                     // Copy ships from the Task-driven fleet
                     // to the planetary orbital fleet.
                     p.planetary_fleet.ships[ship_type] += p.fleets[i].ships[ship_type];
                     p.fleets[i].ships[ship_type] = 0;
                  }
            }
         }
   });
}

void universe::fleet_action_attack(fleet _attackers)
{
   updateplanet(_attackers.destination_id);

   // Fight.
   uint64_t defender_id = _attackers.destination_id;
   auto defender_planet = _sectors.find(defender_id);

   fleet _defenders = (*defender_planet).planetary_fleet;

   uint64_t attackers_power = _attackers.ships[ships_num::battleship] * base_ships[ships_num::battleship].power + _attackers.ships[ships_num::cargoship] * base_ships[ships_num::cargoship].power + _attackers.ships[ships_num::colonizer] + base_ships[ships_num::colonizer].power;
   uint64_t defenders_power = _defenders.ships[ships_num::battleship] * base_ships[ships_num::battleship].power + _defenders.ships[ships_num::cargoship] * base_ships[ships_num::cargoship].power + _defenders.ships[ships_num::colonizer] + base_ships[ships_num::colonizer].power;

   // Apply damage to attackers.

   mechanic_apply_damage(_attackers, defenders_power, true);   // This fleet is attacking. 
                                                               // It will not receive bonuses from its home_planet
                                                               // because it is far from its home planet.

   // Apply damage to defenders.

   mechanic_apply_damage((*defender_planet).planetary_fleet, attackers_power, false);  // This fleet is defending. 
                                                               // It will receive bonus from planetary shields.
   // Load attackers cargo if attackers win.

   uint64_t attackers_remaining = calculate_remaining_ships(_attackers);
   uint64_t defenders_remaining = calculate_remaining_ships((*defender_planet).planetary_fleet);

   if(attackers_remaining > 0 && defenders_remaining == 0)
   {
      // Attackers win

      // Calculate attackers cargo and planet overall resources quantity.
      // Calculate "resource ratio" which is equal to planet_resources/cargo.
      // Withdraw "resource ratio" * each_resource_type quantity of resources.

      uint64_t cargo = calculate_fleet_cargo(_attackers);
      uint64_t planetary_resources = (*defender_planet).resources[resource_num::metal] + (*defender_planet).resources[resource_num::crystal] + (*defender_planet).resources[resource_num::gas];
      double resource_withdrawal_ratio = 1.0;
      if(cargo < planetary_resources)
      {
         resource_withdrawal_ratio = (double)cargo / (double)planetary_resources;
      }

      // Load attackers with the rewarded resources from the defeated planet.
      auto attackers_planet = _sectors.find(_attackers.home_id);
      _sectors.modify(attackers_planet, get_self(), [&](auto& p) {
         for ( uint64_t i=0; i<p.fleets.size(); i++ )
         {
            if(p.fleets[i].id == _attackers.id)
            {
               p.fleets[i].cargo[resource_num::metal] = resource_withdrawal_ratio * (*defender_planet).resources[resource_num::metal];
               p.fleets[i].cargo[resource_num::crystal] = resource_withdrawal_ratio * (*defender_planet).resources[resource_num::crystal];
               p.fleets[i].cargo[resource_num::gas] = resource_withdrawal_ratio * (*defender_planet).resources[resource_num::gas];

               // DEBUG
               // REMOVE FROM REAL CONTRACT
               p.fleets[i].order = order_num::travel_home;
            }
         }
      });

      _sectors.modify(defender_planet, get_self(), [&](auto& p) {
         if(resource_withdrawal_ratio == 1.0)
         {
            p.resources[resource_num::metal] = 0;
            p.resources[resource_num::crystal] = 0;
            p.resources[resource_num::gas] = 0;
         }
         else
         {
            p.resources[resource_num::metal] -= resource_withdrawal_ratio * (*defender_planet).resources[resource_num::metal];
            p.resources[resource_num::crystal] -= resource_withdrawal_ratio * (*defender_planet).resources[resource_num::crystal];
            p.resources[resource_num::gas] -= resource_withdrawal_ratio * (*defender_planet).resources[resource_num::gas];
         }
      });
   }
   else if (attackers_remaining > 0 && defenders_remaining > 0)
   {
      // Draw

      // Send remaining attackers back home.
      
      auto attackers_planet = _sectors.find(_attackers.home_id);
      _sectors.modify(attackers_planet, get_self(), [&](auto& p) {
         for ( uint64_t i=0; i<p.fleets.size(); i++ )
         {
            if(p.fleets[i].id == _attackers.id)
            {
               p.fleets[i].order = order_num::travel_home;
            }
         }
      });
   }
   // Send attackers back home.
   updateplanet(_attackers.home_id);
}
    
    
void universe::mechanic_start_colonization(uint64_t planet_id, eosio::name colonizator)
{
   auto player = _players.find(colonizator.value);

   _players.modify(player, get_self(), [&](auto& p) {
      p.owned_planet_ids[p.num_owned_planets] = planet_id;
      p.num_owned_planets++;
   });


   auto planet = _sectors.find(planet_id);
   _sectors.modify(planet, get_self(), [&](auto& p) {
      p.owner = 1;
      p.owner_name = colonizator;
      p.colonization_start = now();

      // The more planets a player has
      // the more time each new planet takes to colonize.
      p.colonization_duration = time_scale * ((*player).num_owned_planets) * colonization_duration;
   });
}
    
    
void universe::mechanic_finish_colonization(uint64_t planet_id)
{
   auto planet = _sectors.find(planet_id);
   if((*planet).colonization_start + (*planet).colonization_duration <= now())
   {
      _sectors.modify(planet, get_self(), [&](auto& p) {
         p.colonization_start = 0;

         // Init starting structures... not in this release though.
      });
   }
}

void universe::mechanic_apply_damage(fleet damaged_fleet, uint64_t damage_amount, bool is_attacking)
{
   auto home_planet = _sectors.find(damaged_fleet.home_id);
   _sectors.modify(home_planet, get_self(), [&](auto& p) {

   for (uint64_t i = 0; i < base_ships.size(); i++)
   {
      // For each possible type of a ship in the game
      // calculate the amount of received damage.

      // Then destroy ships that were not durable enough to withstand the damage.
      if(damage_amount > 0)
      {
         uint64_t destroyed_ships = damage_amount / base_ships[i].durability;

         if(damage_amount > damaged_fleet.ships[i] * base_ships[i].durability)
         {
            damage_amount -= damaged_fleet.ships[i] * base_ships[i].durability;
         }
         else
         {
            damage_amount = 0;
         }  

         if (damaged_fleet.ships[i] > destroyed_ships)
         {
            damaged_fleet.ships[i] =  damaged_fleet.ships[i] - destroyed_ships;
         }
         else
         {
            damaged_fleet.ships[i] = 0;
         }

         // Place salvageable wreckages for each of `destroyed_ships`
         // ^ not yet implemented.    
      }
   }

   if(damaged_fleet.id != "0")
   {
      // Assign damage to attacking fleet
      for ( uint64_t i=0; i<p.fleets.size(); i++ )
      {
         if(p.fleets[i].id == damaged_fleet.id)
         {
            p.fleets[i] = damaged_fleet;
         }
      }
   }
   else
   {
      // Assign damage to planetary fleet
      p.planetary_fleet = damaged_fleet;
   }
   
   /*
   if( !is_attacking )
   {
      p.planetary_fleet = damaged_fleet;
   }
   */
   });
}

void universe::fleet_action_transport(fleet _fleet)
{
   auto planet = _sectors.find(_fleet.destination_id);

   _sectors.modify(planet, get_self(), [&](auto& p) {
      p.resources[resource_num::metal] += _fleet.cargo[resource_num::metal];
      p.resources[resource_num::crystal] += _fleet.cargo[resource_num::crystal];
      p.resources[resource_num::gas] += _fleet.cargo[resource_num::gas];
   });

   auto home_planet = _sectors.find(_fleet.home_id);
   for ( uint64_t i = 0; i < (*home_planet).fleets.size(); i++ )
   {
      if((*home_planet).fleets[i].id == _fleet.id)
      {
         _sectors.modify(home_planet, get_self(), [&](auto& h) {
            h.fleets[i].cargo[resource_num::metal] = 0;
            h.fleets[i].cargo[resource_num::crystal] = 0;
            h.fleets[i].cargo[resource_num::gas] = 0;
            h.fleets[i].order = order_num::travel_home;
         });
      }
   }
}

void universe::fleet_action_colonize(fleet _fleet)
{
   auto planet = _sectors.find(_fleet.destination_id);

   _sectors.modify(planet, get_self(), [&](auto& p) {
      p.resources[resource_num::metal] += _fleet.cargo[resource_num::metal];
      p.resources[resource_num::crystal] += _fleet.cargo[resource_num::crystal];
      p.resources[resource_num::gas] += _fleet.cargo[resource_num::gas];

      for ( uint64_t i=0; i<p.fleets.size(); i++ )
      {
         if(p.fleets[i].id == _fleet.id)
         {
            for (uint64_t ship_type = 0; ship_type < base_ships.size(); ship_type++)
               {
                  // Copy ships from the Task-driven fleet
                  // to the planetary orbital fleet.
                  p.planetary_fleet.ships[ship_type] += p.fleets[i].ships[ship_type];
                  if(ship_type == ships_num::colonizer)
                  {
                     // Destroy 1 colonizer
                     // that was used to colonize this planet.
                     p.planetary_fleet.ships[ships_num::colonizer] -= 1;
                  }
                  p.fleets[i].ships[ship_type] = 0;
                  p.fleets[i].order = order_num::relocate;
               }
            }
         }
   });

   // Now place the planet into colonization stage.
   mechanic_start_colonization((*planet).id, (*_sectors.find(_fleet.home_id)).owner_name);

   //fleet_action_arrive(_fleet, true);
}
