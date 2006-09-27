from sys import exit
from utils import *
from random import randint

aidelay = 0
cycle = 0
player = -1

# This is just to be able to test the syntax whith you
# Python interpreter on the console
try:
  import BoScript
except ImportError:
  boprint("error", "Couldn't import BoScript, something bad is going on, watch your back !")

try:
  import ai_produce, ai_attack
except ImportError:
  boprint("error", "Couldn't import ai_produce, ai_attack. Won't work.")


def unitDestroyed(unitid, ownerid, pos):
  boprint("debug","unit with id %s destroyed" % unitid)


def init(id):
  #boprint_setDebugLevel("debug")
  boprint("debug", "AI Init called")

  resetAIDelay()
  boprint("debug", "aidelay set to %d" % aidelay)

  setPlayerId(id)
  BoScript.addEventHandler("UnitWithTypeProduced", "ai.unitProduced", "plab")
  BoScript.addEventHandler("ProducedUnitWithTypePlaced", "ai.unitPlaced", "upla")
  BoScript.addEventHandler("UnitWithTypeDestroyed", "ai.unitDestroyed", "upl")

  ai_attack.init()

def setPlayerId(id):
  global player
  player = id

def resetAIDelay():
  global aidelay
  # AB: aiDelay() is in seconds - aiDelay * 20 is advance calls.
  aidelay = int(BoScript.aiDelay() * 20)


def advance():
  global cycle
  global player
  global aidelay

  # AI does something once aidelay reaches 0 only.
  if aidelay >= 1:
    aidelay = aidelay - 1
    return
  resetAIDelay()
  if aidelay < 0:
    boprint("debug", "aidelay < 0. AI disabled.")
    return

  cycle = cycle + 1
  if (cycle % 2) == 0:
    boprint("debug", "produced method called, cycle: %s" % cycle)
    ai_produce.produce()
  if (cycle % 5) == 0:
    # AB: this is only a fallback - the unit should be placed by the event
    ai_produce.place()
  if (cycle % 2) == 0:
    boprint("debug", "mine method called, cycle: %s" % cycle)
    mine()
#  if (cycle % 20) == 0:
     #spawnSomeUnits()
  ai_attack.advance()


def spawnSomeUnits():
  global player
  boprint("debug", "spawning some units for player: %s" % player)
  for x in range(4):
    BoScript.spawnUnit(10035, 5, 5 + x * 2)

def mine():
  global player
  units = BoScript.allPlayerUnits(player)
  for u in units:
    if BoScript.canUnitMineOil(u) and BoScript.unitAdvanceWork(u)==0:
      boprint("debug", "id %s" % u)
      pos=BoScript.unitPosition(u)
      oil=BoScript.nearestOilLocations(int(pos[0]),int(pos[1]),1,150)
      boprint("debug", "Mine oil  %s" % oil)
      if len(oil) > 0:
        BoScript.mineUnit(u, oil[0][0], oil[0][1])
        boprint("debug", "Mine oil done")
    elif BoScript.canUnitMineMinerals(u) and BoScript.unitAdvanceWork(u)==0 :
      boprint("debug", "id %s" % u)
      pos=BoScript.unitPosition(u)
      oil=BoScript.nearestMineralLocations(int(pos[0]),int(pos[1]),1,150)
      boprint("debug", "Mine mineral %s" % oil)
      if len(oil) > 0:
        BoScript.mineUnit(u, oil[0][0], oil[0][1])
        boprint("debug", "Mine  minerals done")


def unitProduced(ownerid, pos, type, factorid):
  ai_produce.unitProduced(ownerid, pow, type, factorid)

def unitPlaced(unitid, ownerid, pos, type):
  ai_produce.unitPlaced(unitid, ownerid, pos, type)



# vim: et sw=2
