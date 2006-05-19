# AB: "ai_attack" searches for the enemy (explore) and attacks the enemy.

from sys import exit
from utils import *
from random import randint, shuffle

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

try:
  import unit
except ImportError:
  boprint("error", "Couldn't import unit. Won't work.")

class AIExplorer:
  def __init__(self, player):
    self.mPlayer = player
    self.mExplorer = 0

  def explore(self):
    boprint("debug", "exploring")
    if self.mExplorer and not self.mExplorer.isAlive():
      self.mExplorer = 0
    if not self.mExplorer:
      self.mExplorer = self.findExplorerUnit()
      if not self.mExplorer:
        boprint("debug", "no explorer unit found. not exploring.")
        return
    pos = self.mExplorer.position()
    boprint("debug", "explore, explore unit: %s current position: %s" % (self.mExplorer.id(), pos))

    exploreAt = self.findExploreLocation(self.mExplorer)
    if exploreAt[0] == -1:
      boprint("debug", "not exploring")
      return

    boprint("debug", "exploring with unit %s at: %s" % (self.mExplorer.id(), exploreAt))
    self.mExplorer.move(exploreAt[0], exploreAt[1])


  def isIdExploring(self, unitId):
    if not self.mExplorer:
      return False
    if unitId == self.mExplorer.id():
      return True
    return False

  def isUnitExploring(self, unit):
    return self.isIdExploring(unit.id())


  def findExplorerUnit(self):
    units = BoScript.allPlayerUnits(self.mPlayer)
    for u in units:
      if BoScript.isUnitMobile(u):
        if BoScript.canUnitMineMinerals(u) or BoScript.canUnitMineOil(u):
          continue
        pos = BoScript.unitPosition(u)
        if pos[0] != -1:
          return unit.Unit(self.mPlayer, u)
    return 0


  def findExploreLocation(self, unit):
    pos = unit.position()
    boprint("debug", "%d, %s, %d" %(unit.id(), unit.position(), unit.sightRange()))

    # we use integers
    posX = int(pos[0])
    posY = int(pos[1])

    w = BoScript.mapWidth()
    h = BoScript.mapHeight()

    if unit.sightRange() < 2:
      boprint("debug", "sightrange of unit %d too small (%d)" % (unit.id(), unit.sightRange()))
      return (-1, -1)

    sightRange2 = unit.sightRange() * 2
    range = 0
    maxRange = sightRange2 * 5
    searchCells = []
    # search in a distance up to maxRange cells away for fogged cells
    # only every sightRange2-th cell is checked.
    while range <= maxRange and len(searchCells) == 0:
      range = range + sightRange2
      minX = max(posX - range/2, 0)
      minY = max(posY - range/2, 0)
      maxX = min(posX + range/2, w - 1)
      maxY = min(posY + range/2, h - 1)

      # AB: we search in the rect
      #     (posX-range/2, posY-range/2, posX+range/2, posY + range/2)
      #     for fogged cells.
      #     all cells that are found are added to searchCells.
      #     every sightRange2-th cell is tested for being visibility
      #     -> this is important so that we won't test 100*100 cells or so which
      #        would be extremely slow!
      x = minX
      while x < maxX:
        y = minY
        while y < maxY:
          if BoScript.isCellFogged(self.mPlayer, x, y):
            searchCells = searchCells + [(x, y)]
          y = y + sightRange2
        x = x + sightRange2

    if len(searchCells) == 0:
      boprint("debug", "nothing to explore found within a range of %s around unit %s" % (maxRange, unit.id()))
      return (-1, -1)

    shuffle(searchCells)
    # TODO: check if unit can actually go there (or rather at least nearby)
    cell = searchCells[0]
    boprint("debug", "found location to explore for unit %d: %s" % (unit.id(), cell))
    return cell



module = "ai_attack"
aiunit = -1
aitarget = -1
explorerObject = 0


def init():
  global module
  global explorerObject

  module = "ai_attack for player %d" % ai.player
  boprint("debug", "%s called" % module)
  explorerObject = AIExplorer(ai.player)

def advance():
  global explorerObject
  boprint("debug", "%s: advance()" % module)
  if not explorerObject:
    boprint("error", "%s: advanceExplore(): explorerObject not yet created" % module)
    return

  if ai.cycle % 2 == 0:
    advanceExplore()
  if ai.cycle % 1 == 0: # always true
    advanceAttack()


def advanceExplore():
  global explorerObject
  boprint("debug", "%s: advanceExplore()" % module)
  explorerObject.explore()

def advanceAttack():
  global aiunit, aitarget
  boprint("debug", "%s: advanceAttack()" % module)
  # check if target is still alive
  if aitarget == -1 or not BoScript.isUnitAlive(aitarget):
    boprint("debug", "searching for new target...")
    aitarget = findTarget()
    if aitarget == -1:
      boprint("debug", "... no target found. nothing to do.")
      return
  boprint("debug", "... target is now %s" % aitarget)

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
      if BoScript.canUnitShoot(u) and not explorerObject.isIdExploring(u):
        attacker = u
  targetpos = BoScript.unitPosition(aitarget)
  boprint("debug", "ordering unit %s to attack targetpos %s containing target %s" % (attacker, targetpos, aitarget))
  BoScript.moveUnitWithAttacking(attacker, targetpos[0], targetpos[1])


def findTarget():
  boprint("debug", "%s: findTarget()" % module)
  target = -1
  units = BoScript.allEnemyUnitsVisibleFor(ai.player)
  for u in units:
    # FIXME: command center id is hardcoded
    if BoScript.unitType(u) == 5:
      return u
    if target == -1:
      target = u
  # if cmdcenter wasn't found, return any other unit
  return target



# vim: et sw=2
