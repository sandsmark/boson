from sys import exit
from utils import *
from random import randint

# This is just to be able to test the syntax whith you
# Python interpreter on the console
try:
  import BoScript
except ImportError:
  boprint("error", "Couldn't import BoScript, something bad is going on, watch your back !")

try:
  import ai
except ImportError:
  boprint("error", "Couldn't import ai. Won't work.")


aidelay = 0
aicycle = 0
aiunit = -1
aitarget = -1

def oldAIInit():
  boprint("debug", "oldAIInit() called for player: %s" % ai.player)
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
  #reset aicycle
  boprint("debug", "oldAIAdvance() called for player: %s" % ai.player)
  aicycle = 0
  # check if target is still alive
  if aitarget == -1 or BoScript.isUnitAlive(aitarget) == 0:
    boprint("info", "Target not set")
    aitarget = oldAIFindTarget()
    if aitarget == -1:
      boprint("info", "No enemies left. Disabling self")
      aidelay = 0
      return
  boprint("info", "Target is %s" % aitarget)
  # find attacker
  attacker = -1
  units = BoScript.allPlayerUnits(ai.player)
  while attacker == -1:
    aiunit = aiunit + 1
    if aiunit >= len(units):
      aiunit = -1
      boprint("info", "No attacker found, returning")
      return
    u = units[aiunit]
    if BoScript.isUnitMobile(u):
      if BoScript.canUnitShoot(u) and u != ai.expl:
        attacker = u
        boprint("debug", "attacker set to %s" % attacker)
        boprint("debug", "Sending %s unit with id %s to attack" % (aiunit, attacker))
  targetpos = BoScript.unitPosition(aitarget)
  BoScript.moveUnitWithAttacking(attacker, targetpos[0], targetpos[1])


def oldAIFindTarget():
  boprint("debug", "Target")
  players = BoScript.allPlayers()
  target = -1
  # iterate through all players
  for p in players:
    if not BoScript.isEnemy(p):
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

# vim: et sw=2
