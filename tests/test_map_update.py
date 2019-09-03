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
    Map update test
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
        "initmap", {"acc":CAROL,"height":"4","width":"4","offsetx":"0","offsety":"0","init_planets":"1"}, permission=(CAROL, Permission.ACTIVE))

    #HOST.push_action(
    #    "cleargame", {"owner":CAROL}, permission=(CAROL, Permission.ACTIVE))

    HOST.push_action(
        "initmap", {"acc":CAROL,"height":"3","width":"3","offsetx":"0","offsety":"0","init_planets":"1"}, permission=(CAROL, Permission.ACTIVE))

    COMMENT('''
    Testing player spawn:
    ''')

    HOST.push_action(
        "spawnplayer", {"acc":ALICE,"planet_id":"5"}, permission=(ALICE, Permission.ACTIVE))
        
    COMMENT('''
    Pushing the first update cycle:
    ''')

    #HOST.push_action(
    #    "setstate", {"owner":CAROL}, permission=(CAROL, Permission.ACTIVE))

    HOST.push_action(
        "setcyclic", {"acc":ALICE,"allowed":"1"}, permission=(ALICE, Permission.ACTIVE))

    HOST.push_action(
        "updatemap", {"min_cycles":"0","acc":HOST,"delay":"360","repeat":"0"}, permission=(HOST, Permission.ACTIVE))

    COMMENT('''
    Appending one more map fragment:
    ''')

    COMMENT('''
    Clearing out and ending ......
    ''')

    #HOST.push_action(
    #    "cleargame", {"owner":ALICE}, permission=(ALICE, Permission.ACTIVE))



    stop()


if __name__ == "__main__":
    test()
    print("END")
