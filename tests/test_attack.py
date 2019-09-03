import sys
from eosfactory.eosf import *

verbosity([Verbosity.INFO, Verbosity.OUT, Verbosity.DEBUG])

CONTRACT_WORKSPACE = sys.path[0] + "/../"

# Actors of the test:
MASTER = MasterAccount()
HOST = Account()
ALICE = Account()
BOB = Account()
CAROL = Account()

def test():
    SCENARIO('''
    Attack of a fleet.
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
    create_account("BOB", MASTER)
    create_account("CAROL", MASTER)

    COMMENT('''
    Test map init:
    ''')
    HOST.push_action(
        "initmap", {"acc":CAROL}, permission=(CAROL, Permission.ACTIVE))

    COMMENT('''
    Initing a debug planet 1 (id=3, x=1, y=2)
    ''')
    HOST.push_action(
        "adminit", {"acc":CAROL,"planet_id":"3","x":"1","y":"2","battleships":"3","cargoships":"0","resource_metal":"100","resource_gas":"500"}, permission=(CAROL, Permission.ACTIVE))

    COMMENT('''
    Initing a debug planet 2 (id=4, x=2, y=3)
    ''')
    HOST.push_action(
        "adminit", {"acc":CAROL,"planet_id":"4","x":"2","y":"3","battleships":"15","cargoships":"0","resource_metal":"25","resource_gas":"200"}, permission=(CAROL, Permission.ACTIVE))



    COMMENT('''
    -------------------
    ''')
    COMMENT('''
    Test attacking ( 1 battleship sent):  
    ''')

    ## Orders
    #  0 - idle
    #  1 - travel home
    #  2 - attack the destination
    #  3 - transport resources
    HOST.push_action(
        "fleetorder", {"acc":ALICE,"planet_id":"3","destination_id":"4","order_type":"2","battleship_count":"1","cargoship_count":"0","colonizer_count":"0","cargo_metal":"0","cargo_crystal":"0","cargo_gas":"0"}, permission=(ALICE, Permission.ACTIVE))

    HOST.push_action("updateplanet", {"acc":ALICE,"id":"3"}, permission=(ALICE, Permission.ACTIVE))

    COMMENT('''
    -------------------
    ''')
    COMMENT('''
    Test attacking ( 2 battleships sent):  
    ''')
    HOST.push_action(
        "fleetorder", {"acc":ALICE,"planet_id":"3","destination_id":"4","order_type":"2","battleship_count":"2","cargoship_count":"0","colonizer_count":"0","cargo_metal":"0","cargo_crystal":"0","cargo_gas":"0"}, permission=(ALICE, Permission.ACTIVE))

    HOST.push_action("updateplanet", {"acc":CAROL,"id":"3"}, permission=(ALICE, Permission.ACTIVE))

    HOST.push_action("updateplanet", {"acc":BOB,"id":"3"}, permission=(BOB, Permission.ACTIVE))

    COMMENT('''
    -------------------
    ''')
    COMMENT('''
    Printing planet stats:  
    ''')
    HOST.push_action(
        "getstats", {"planet_id":"3"}, permission=(ALICE, Permission.ACTIVE))

    HOST.push_action(
        "getstats", {"planet_id":"4"}, permission=(ALICE, Permission.ACTIVE))

    stop()


if __name__ == "__main__":
    test()
    print("END")
