print "BoScriptAI: - [INFO] Loading Python script for AI..."

from sys import exit
from utils import boprint

# This is just to be able to test the syntax whith you
# Python interpreter on the console
try:
  import BoScript
except ImportError:
  boprint("error", "Couldn't import BoScript, something bad is going on, watch your back !")


cycle = 0
player = -1


def init(id):
  global player
  boprint("debug", "Init called")
  player = id
  oldAIInit()

def advance():
  global cycle
  cycle = cycle + 1
  #if (cycle % 10) == 0:
  #  boprint("debug", "advance method called, cycle: %s" % cycle)
  #if cycle == 10:
  #  printUnitInfo()
  #if cycle == 5:
  #  spawnSomeUnits()
  oldAIAdvance()


def myfunc():
  boprint("info", "hi! this is myfunc")

def printUnitInfo():
  units = BoScript.unitsInRect(0, 0, 1000, 1000)
  myunits = 0
  boprint("info", "There are %s units on the map" % len(units))
  for unit in units:
    if BoScript.unitOwner(unit) == player:
      myunits = myunits + 1
  boprint("info", "I have %s of them" % myunits)


# AI stuff
aidelay = 0
aicycle = 0
aiunit = -1
aitarget = -1

def oldAIInit():
  global player
  boprint("info", "oldAIInit() called for player: %s" % player)
  global aidelay
  aidelay = int(BoScript.aiDelay() * 20)
  boprint("debug", "aidelay set to %s" % aidelay)

def oldAIAdvance():
  global aidelay, aicycle, aiunit, aitarget
  # AI will do something every aidelay advance calls
  aicycle = aicycle + 1
  if not aicycle == aidelay:
    return
  boprint("debug", "oldAIAdvance() executing")
  reset aicycle
  global player
  boprint("info", "oldAIAdvance() called for player: %s" % player)
  aicycle = 0
  # check if target is still alive
  if aitarget == -1 or BoScript.isUnitAlive(aitarget) == 0:
    # boprint("warning", "Target not set")
    aitarget = oldAIFindTarget()
    if aitarget == -1:
      boprint("info", "No enemies left. Disabling self")
      aidelay = 0
      return
  # boprint("info", "Target is %s" % aitarget)
  # find attacker
  attacker = -1
  units = BoScript.allPlayerUnits(player)
  while attacker == -1:
    aiunit = aiunit + 1
    if aiunit >= len(units):
      aiunit = -1
      # boprint("info", "No attacker found, returning")
      return
    u = units[aiunit]
    if BoScript.isUnitMobile(u) and BoScript.canUnitShoot(u):
      attacker = u
      #boprint("info", "attacker set to %s" % attacker)
  # boprint("info", "Sending %s unit with id %s to attack" % (aiunit, attacker))
  targetpos = BoScript.unitPosition(aitarget)
  BoScript.moveUnitWithAttacking(player, attacker, targetpos[0], targetpos[1])


def oldAIFindTarget():
  players = BoScript.allPlayers()
  target = -1
  # iterate through all players
  for p in players:
    if not BoScript.areEnemies(p, player):
      continue
    units = BoScript.allPlayerUnits(p)
    # iterate through all units of player
    for u in units:
      if not BoScript.isUnitAlive(u):
        continue
      # FIXME: command center id is hardcoded
      if BoScript.unitType(u) == 5:
        return u
      if target == -1:
        target = u
    # if cmdcenter wasn't found, return any other unit
    return target
  # nothing was found
  return -1

def spawnSomeUnits():
  global player
  boprint("info", "spawning some units for player: %s" % player)
  for x in range(4):
    BoScript.spawnUnit(player, 10035, 5, 5 + x * 2)

