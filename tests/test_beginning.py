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
        "initmap", {"acc":CAROL,"height":"20","width":"20"}, permission=(CAROL, Permission.ACTIVE))

    HOST.push_action(
        "spawnplayer", {"acc":ALICE,"planet_id":"44"}, permission=(ALICE, Permission.ACTIVE))
    
    HOST.push_action(
        "getstats", {"planet_id":"44"}, permission=(ALICE, Permission.ACTIVE))


    # Adding a building task with type 0 >> Building
    # autoupdate -> 0 TRUE  (queue the deferred update TX)
    # autoupdate -> 1 FALSE (no update TX)
    HOST.push_action(
        "addtask", {"acc":ALICE,"id":"44","type":"0","task_id":"6","quantity":"0","autoupdate":"0"}, permission=(ALICE, Permission.ACTIVE))

    # Need to add more METAL for further building task
    # "mode":"2"        =>  ADD
    # "type":"0"        =>  Gift type is RESOURCE
    # "gift_obj_id":"0" =>  Gifted resource is METAL
    # "count":"500"     =>  Add 500 units of metal
    HOST.push_action(
        "adminmodify", {"acc":ALICE,"id":"44","mode":"2","type":"0","gift_obj_id":"0","count":"500"}, permission=(ALICE, Permission.ACTIVE))

    # Now we need an ASSEMBLY building to queue the ship construction
    # "mode":"2"        =>  ADD
    # "type":"1"        =>  Gift type is BUILDING
    # "gift_obj_id":"1" =>  Gifted structure is ASSEMBLY
    # "count":"1"       =>  We only need 1 level to start production
    HOST.push_action(
        "adminmodify", {"acc":ALICE,"id":"44","mode":"2","type":"1","gift_obj_id":"1","count":"1"}, permission=(ALICE, Permission.ACTIVE))

    # Adding a building task with type 1 >> Assembling
    HOST.push_action(
        "addtask", {"acc":ALICE,"id":"44","type":"1","task_id":"1","quantity":"2","autoupdate":"1"}, permission=(ALICE, Permission.ACTIVE))

    HOST.push_action(
        "getstats", {"planet_id":"44"}, permission=(CAROL, Permission.ACTIVE))

    COMMENT('''
    Clearing out and ending ......
    ''')
    HOST.push_action(
        "erasemap", {"acc":CAROL}, permission=(CAROL, Permission.ACTIVE))

    stop()


if __name__ == "__main__":
    test()
    print("END")
