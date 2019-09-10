# UniverseX game developed for EOS

Author: [@Dexaran](https://github.com/Dexaran)


# Contract deployment and interactions sequences


################ Deploying the contract

cleos -u https://api.eosn.io set contract dexaraniiznx '/home/dex/Desktop/universe_x'




################ Beginning the game
### Generate `height` X `width` resolution map
### with X/Y offset of the left upper corner
### init_planet determines if empty sectors will be filled with planets (require RAM)
### or left empty (save your RAM) 
### 0 - false
### 1 - true

cleos -u https://api.eosn.io push action dexaraniiznx initmap '{"acc":"dexaraniiznx","height":"8","width":"7","offsetx":"0","offsety":"0","init_planets":"1"}' -p dexaraniiznx@active



################  and spawning yourself

cleos -u https://api.eosn.io push action dexaraniiznx spawnplayer '{"acc":"dexaraniiznx","planet_id":"3"}' -p dexaraniiznx@active


################ Placing your first task. Let's start with building of a mine.

cleos -u https://api.eosn.io push action dexaraniiznx addtask '{"acc":"dexaraniiznx","id":"3","type":"0","task_id":"6","quantity":"0","autoupdate":"1"}' -p dexaraniiznx@active


################ Watching what happens and updating tasks for a planet

cleos -u https://eos.greymass.com:443 push action dexaraniiznx updateplanet '{"id":"3"}' -p dexaraniiznx@active


################ Setting up a cyclic update

cleos -u https://api.eosn.io push action dexaraniiznx updatemap '{"acc":"dexaraniiznx","min_cycles":"0","iterations":"10","delay":"360","repeat":"1"}' -p dexaraniiznx@active


################ Stopping a cyclic update

cleos -u https://eos.greymass.com:443 push action dexaraniiznx setcyclic '{"acc":"dexaraniiznx","allowed":"0"}' -p dexaraniiznx@active

################ COMPLETE CLEAROUT (USE TO REDEPLOY THE CONTRACT)

cleos -u https://api.eosn.io push action dexaraniiznx cleargame '{"owner":"dexaraniiznx"}' -p dexaraniiznx@active


################ CLEARING OUT (map only)

cleos -u https://eos.greymass.com:443 push action dexaraniiznx erasemap '{"acc":"dexaraniiznx","iterations":"100"}' -p dexaraniiznx@active
