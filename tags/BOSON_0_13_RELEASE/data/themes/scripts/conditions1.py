print "BoScriptAI: - [INFO] Loading Python script for localplayer..."

from sys import exit
from utils import boprint

# This is just to be able to test the syntax whith you
# Python interpreter on the console
try:
  import BoScript
except ImportError:
  boprint("error", "Couldn't import BoScript, something bad is going on, watch your back !")


def playerHasLotsOfMinerals(player):
  if BoScript.minerals(player) > 3000:
    return 1
  else
    return 0

def playerHasLotsOfOil(player):
  if BoScript.oil(player) > 3000:
    return 1
  else
    return 0

def playerHasManyUnits(player):
  if BoScript.allPlayerUnitsCount(player) > 15:
    return 1
  else
    return 0

def isUnitAlive(unit):
  if BoScript.isUnitAlive(unit):
    return 1
  else
    return 0

def isUnitDead(unit):
  # doesn't check if unit actually exists
  if BoScript.isUnitAlive(unit):
    return 0
  else
    return 1

