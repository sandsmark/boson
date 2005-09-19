from sys import exit
from utils import *
from random import randint

cycle = 0
player = -1

# This is just to be able to test the syntax whith you
# Python interpreter on the console
try:
  import BoScript
except ImportError:
  boprint("error", "Couldn't import BoScript, something bad is going on, watch your back !")

try:
  import ai_produce, ai_old
except ImportError:
  boprint("error", "Couldn't import ai_produce, ai_old. Won't work.")


def unitDestroyed(unitid, ownerid, pos):
  boprint("debug","unit with id %s destroyed" % unitid)


def init(id):
  global expl
  #boprint_setDebugLevel("debug")
  boprint("debug", "AI Init called")
  setPlayerId(id)
  BoScript.addEventHandler("UnitWithTypeProduced", "ai.unitProduced", "plab")
  BoScript.addEventHandler("ProducedUnitWithTypePlaced", "ai.unitPlaced", "upla")
  BoScript.addEventHandler("UnitWithTypeDestroyed", "ai.unitDestroyed", "upl")

  expl = -1

  ai_old.oldAIInit()
  ai_produce.init()

def setPlayerId(id):
  global player
  player = id


def advance():
  global cycle
  global player
  cycle = cycle + 1
  if (cycle % 100) == 0:
    boprint("debug", "produced method called, cycle: %s" % cycle)
    ai_produce.produce()
  if (cycle % 100) == 0:
    ai_produce.place()
  if (cycle % 100) == 0:
    boprint("debug", "mine method called, cycle: %s" % cycle)
    mine()
  if (cycle % 200) == 0:
    explore()
#  if (cycle % 20) == 0:
     #spawnSomeUnits()
  #boprint("debug", "hi! advance")
  ai_old.oldAIAdvance()


def explore():
  global player,expl
  units = BoScript.allPlayerUnits(player)
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
  BoScript.moveUnit(player, expl,randint(x-50, x+50), randint(y-50,y+50))
  if not BoScript.isUnitAlive(expl):
    expl = -1


def spawnSomeUnits():
  global player
  boprint("debug", "spawning some units for player: %s" % player)
  for x in range(4):
    BoScript.spawnUnit(player, 10035, 5, 5 + x * 2)

def mine():
  global player
  units = BoScript.allPlayerUnits(player)
  for u in units:
    if BoScript.canUnitMineOil(u) and BoScript.unitWork(u)==0:
      boprint("debug", "id %s" % u)
      pos=BoScript.unitPosition(u)
      oil=BoScript.nearestOilLocations(player,int(pos[0]),int(pos[1]),1,150)
      boprint("debug", "Mine oil  %s" % oil)
      if len(oil) > 0:
        BoScript.mineUnit(player, u, oil[0][0], oil[0][1])
        boprint("debug", "Mine oil done")
    elif BoScript.canUnitMineMinerals(u) and BoScript.unitWork(u)==0 :
      boprint("debug", "id %s" % u)
      pos=BoScript.unitPosition(u)
      oil=BoScript.nearestMineralLocations(player,int(pos[0]),int(pos[1]),1,150)
      boprint("debug", "Mine mineral %s" % oil)
      if len(oil) > 0:
        BoScript.mineUnit(player, u, oil[0][0], oil[0][1])
        boprint("debug", "Mine  minerals done")


def unitProduced(ownerid, pos, type, factorid):
  ai_produce.unitProduced(ownerid, pow, type, factorid)

def unitPlaced(unitid, ownerid, pos, type):
  ai_produce.unitPlaced(unitid, ownerid, pos, type)



# vim: et sw=2
