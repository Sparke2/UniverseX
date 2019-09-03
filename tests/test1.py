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
    Execute simple actions.
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

    HOST.push_action(
        "addplanet", {"acc":CAROL,"x":"1","y":"2","id":"3"}, permission=(CAROL, Permission.ACTIVE))

    COMMENT('''
    Test task placing:
    ''')
    HOST.push_action(
        "placetask", {"acc":CAROL,"id":"3","type":"1","time":"0","quantity":"0"}, permission=(CAROL, Permission.ACTIVE))
    HOST.push_action(
        "placetask", {"acc":CAROL,"id":"3","type":"4","time":"0","quantity":"0"}, permission=(CAROL, Permission.ACTIVE))
    HOST.push_action(
        "placetask", {"acc":CAROL,"id":"3","type":"6","time":"0","quantity":"0"}, permission=(CAROL, Permission.ACTIVE))
    HOST.push_action(
        "placetask", {"acc":CAROL,"id":"3","type":"3","time":"0","quantity":"0"}, permission=(CAROL, Permission.ACTIVE))
    HOST.push_action(
        "placetask", {"acc":ALICE,"id":"3","type":"3","time":"0","quantity":"0"}, permission=(ALICE, Permission.ACTIVE))
    HOST.push_action(
        "placetask", {"acc":ALICE,"id":"3","type":"10","time":"0","quantity":"15"}, permission=(ALICE, Permission.ACTIVE))

    COMMENT('''
    -------------------------
    ''')
    COMMENT('''
    Test UPDATE (1 run):
    ''')
    HOST.push_action("updateplanet", {"acc":CAROL,"id":"3"}, permission=(CAROL, Permission.ACTIVE))

    COMMENT('''
    Test Commanding FLEET:
    ''')
    HOST.push_action(
        "fleetorder", {"acc":ALICE,"planet_id":"3","destination_id":"4","order_type":"2","battleship_count":"0","cargoship_count":"0","colonizer_count":"1","cargo_metal":"5","cargo_crystal":"1","cargo_gas":"0"}, permission=(ALICE, Permission.ACTIVE))

    COMMENT('''
    One more fleet:
    ''')

    HOST.push_action(
        "fleetorder", {"acc":ALICE,"planet_id":"3","destination_id":"4","order_type":"2","battleship_count":"0","cargoship_count":"0","colonizer_count":"2","cargo_metal":"3","cargo_crystal":"1","cargo_gas":"0"}, permission=(ALICE, Permission.ACTIVE))


    HOST.push_action(
        "fleetorder", {"acc":ALICE,"planet_id":"3","destination_id":"4","order_type":"2","battleship_count":"0","cargoship_count":"0","colonizer_count":"3","cargo_metal":"5","cargo_crystal":"1","cargo_gas":"0"}, permission=(ALICE, Permission.ACTIVE))

    
    COMMENT('''
    Test UPDATE (2 run):
    ''')
    HOST.push_action("updateplanet", {"acc":ALICE,"id":"3"}, permission=(CAROL, Permission.ACTIVE))

    COMMENT('''
    Final GetStats:
    ''')
    HOST.push_action(
        "getstats", {"planet_id":"3"}, permission=(ALICE, Permission.ACTIVE))

    COMMENT('''
    Test Erasing map:
    ''')
    HOST.push_action("erasemap", {"acc":CAROL}, permission=(CAROL, Permission.ACTIVE))

    COMMENT('''
    Waiting ... ... ... ending:
    ''')

    stop()


if __name__ == "__main__":
    test()
    print("END")
