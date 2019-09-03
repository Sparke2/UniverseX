import sys
from eosfactory.eosf import *

verbosity([Verbosity.INFO, Verbosity.OUT, Verbosity.DEBUG])

CONTRACT_WORKSPACE = sys.path[0] + "/../"

# Actors of the test:
MASTER = MasterAccount()
HOST = Account()
ALICE = Account()
CAROL = Account()

def test():
    SCENARIO('''
    Begin the game (test)
    ''')
    reset()
    create_master_account("MASTER")

    COMMENT('''
    Build and deploy the contract:
    ''')
    create_account("HOST", MASTER)
    smart = Contract(HOST, CONTRACT_WORKSPACE)
    smart.build(force=False)
    smart.deploy()

    COMMENT('''
    Create test accounts:
    ''')
    create_account("ALICE", MASTER)
    create_account("CAROL", MASTER)

    COMMENT('''
    Test map init:
    ''')
    HOST.push_action(
        "initmap", {"acc":CAROL,"height":"4","width":"4","offset_x":"1000","offset_y":"1000","init_planets":"0"}, permission=(CAROL, Permission.ACTIVE))

    HOST.push_action(
        "spawnplayer", {"acc":CAROL,"planet_id":"6"}, permission=(CAROL, Permission.ACTIVE))

    HOST.push_action(
        "adminit", {"acc":ALICE,"planet_id":"0","x":"0","y":"0","battleships":"5","cargoships":"0","resource_metal":"0","resource_gas":"500"}, permission=(ALICE, Permission.ACTIVE))

    
    HOST.push_action(
        "getstats", {"planet_id":"0"}, permission=(ALICE, Permission.ACTIVE))


    ## Orders
    #  0 - idle
    #  1 - travel home
    #  2 - attack the destination
    #  3 - transport resources
    HOST.push_action(
        "fleetorder", {"acc":ALICE,"planet_id":"0","destination_id":"6","order_type":"2","battleship_count":"1","cargoship_count":"0","colonizer_count":"0","cargo_metal":"0","cargo_crystal":"0","cargo_gas":"0"}, permission=(ALICE, Permission.ACTIVE))

    HOST.push_action(
        "fleetorder", {"acc":ALICE,"planet_id":"0","destination_id":"6","order_type":"2","battleship_count":"3","cargoship_count":"0","colonizer_count":"0","cargo_metal":"0","cargo_crystal":"0","cargo_gas":"0"}, permission=(ALICE, Permission.ACTIVE))

    HOST.push_action(
        "getstats", {"planet_id":"0"}, permission=(CAROL, Permission.ACTIVE))

    COMMENT('''
    Clearing out and ending ......
    ''')
    HOST.push_action(
        "erasemap", {"acc":CAROL}, permission=(CAROL, Permission.ACTIVE))

    stop()


if __name__ == "__main__":
    test()
    print("END")
