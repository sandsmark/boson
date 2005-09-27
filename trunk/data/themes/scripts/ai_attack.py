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


module = "ai_attack"
aiunit = -1
aitarget = -1
expl = -1

def init():
  global expl
  global module

  module = "ai_attack for player %d" % ai.player
  boprint("debug", "%s called" % module)
  expl = -1

def advance():
  boprint("debug", "%s: advance()" % module)
  if ai.cycle % 2 == 0:
    advanceExplore()
#  if ai.cycle % 1 == 0: # always true
#    advanceAttack()

def advanceExplore():
  boprint("debug", "%s: advanceExplore()" % module)
  explore()

def advanceAttack():
  global aiunit, aitarget
  boprint("debug", "%s: advanceAttack()" % module)
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
  if expl != -1 and not BoScript.isUnitAlive(expl):
    expl = -1
  if expl == -1:
    expl = findExplorerUnit()
    if expl == -1:
      boprint("debug", "no explorer unit found. not exploring.")
      return
  pos = BoScript.unitPosition(expl)
  boprint("debug", "explore, explore unit: %s current position: %s" % (expl, pos))

  exploreAt = findExploreLocation(expl)
  if exploreAt[0] == -1:
    boprint("debug", "not exploring")
    return

  boprint("debug", "exploring with unit %s at: %s" % (expl, exploreAt))
  BoScript.moveUnit(expl, exploreAt[0], exploreAt[1])


def findExplorerUnit():
  units = BoScript.allPlayerUnits(ai.player)
  for u in units:
    if BoScript.isUnitMobile(u):
      if BoScript.canUnitMineMinerals(u) or BoScript.canUnitMineOil(u):
        continue
      pos = BoScript.unitPosition(u)
      if pos[0] != -1:
        return u
  return -1

def findExploreLocation(unit):
  pos = BoScript.unitPosition(unit)

  # randint's arguments must be integers, so convert pos to integer
  posX = int(pos[0])
  posY = int(pos[1])

  x = randint(posX - 50, posX + 50)
  y = randint(posY - 50, posY + 50)

  return (x, y)


# vim: et sw=2