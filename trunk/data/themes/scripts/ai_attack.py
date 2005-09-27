# AB: "ai_attack" searches for the enemy (explore) and attacks the enemy.

from sys import exit
from utils import *
from random import randint

# This is just to be able to test the syntax whith your
# Python interpreter on the console
try:
  import BoScript
except ImportError:
  boprint("error", "Couldn't import BoScript, something bad is going on, watch your back !")

try:
  import ai
except ImportError:
  boprint("error", "Couldn't import ai. Won't work.")


module = "ai_attack for player %d" % ai.player
aidelay = 0
aicycle = 0
aiunit = -1
aitarget = -1

def init():
  boprint("debug", "%s called" % module)
  global expl
  global aidelay
  aidelay = int(BoScript.aiDelay() * 20)
  boprint("debug", "%s: aidelay set to %d" % (module, aidelay))
  expl = -1

def advance():
  global aidelay, aicycle, aiunit, aitarget

  # AI will do something every aidelay advance calls
  aicycle = aicycle + 1
  if not aicycle == aidelay:
    return

  boprint("debug", "%s: advance()" % module)
  #reset aicycle
  aicycle = 0

  # check if target is still alive
  if aitarget == -1 or BoScript.isUnitAlive(aitarget) == 0:
    boprint("info", "Target not set")
    aitarget = findTarget()
    if aitarget == -1:
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


def findTarget():
  boprint("debug", "%s: findTarget()" % module)
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


def explore():
  global expl
  boprint("debug", "exploring")
  units = BoScript.allPlayerUnits(ai.player)
  unit = -1
  if expl == -1 :
    attacker = -1
    while attacker == -1:
      unit = unit + 1
      if unit >= len(units):
        unit = -1
        boprint("info", "No units found, returning")
        return
      u = units[unit]
      if BoScript.isUnitMobile(u):
        if BoScript.canUnitShoot(u):
          attacker = u
          expl = u
          pos = BoScript.unitPosition(expl)
          if pos[0] == -1:
            attacker = -1
            expl = -1
  pos = BoScript.unitPosition(expl)
  boprint("debug", "explore, expl %s, pos %s" % (expl,pos))
  # randint's arguments must be integers, so convert pos to integer
  x = int(pos[0])
  y = int(pos[1])
  BoScript.moveUnit(expl,randint(x-50, x+50), randint(y-50,y+50))
  boprint("debug", "exploring: %d to %d,%d" %(expl, x, y))
  if not BoScript.isUnitAlive(expl):
    expl = -1



# vim: et sw=2
