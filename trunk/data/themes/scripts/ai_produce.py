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


def init():
  global newProd

  newProd = {0:0}

def unitProduced(ownerid, pos, type, factorid):
  # Don't place other players' units
  if(ownerid != ai.player):
    return
  boprint("debug","unit with id %s produced" % type)
  if(BoScript.isUnitTypeMobile(int(ownerid),int(type)) == False):
    boprint("debug", "Ok , building")
    pos = BoScript.unitPosition(int(factorid))
    x = int(pos[0])
    y = int(pos[1])
    i = 0
    tmpx = randint(x-i, x+i)
    tmpy = randint(y-i, y+i)
    if(tmpx < 0):
      tmpx = x
    if(tmpy < 0):
      tmpy = y
    while (BoScript.cellOccupied( tmpx,tmpy) == True):
      tmpx = randint(x-i, x+i)
      tmpy = randint(y-i, y+i)
      if(tmpx < 0):
        tmpx = x
      if(tmpy < 0):
        tmpy = y
      i = i + 1
    boprint("debug","placed tmpx %s,tmpy %s " % (tmpx,tmpy))
    BoScript.placeProduction(int(ownerid), int(factorid), tmpx, tmpy)


def unitPlaced(unitid,ownerid, pos, type):
  boprint("debug","unit with id %s and type  %s placed " % (unitid,type))


def produce():
  units = BoScript.allPlayerUnits(ai.player)
  boprint("debug", "production init")
  for u in units:
    if u not in newProd:
      newProd[u] = 0
      boprint("debug", "unit %s" % u)
    if BoScript.isUnitMobile(u) == 0 and BoScript.canUnitProduce(u):
      boprint("debug", "production")
      prod=BoScript.productionTypes(u)
      if newProd[u] < len (prod):
        p=prod[newProd[u]]
        boprint("debug", "production set to %s type %s " % (len(prod),BoScript.unitType(u)))
        boprint("debug", "production types %s, type %s, count %s" % (prod,p,newProd[u]))
        newProd[u] = newProd[u] + 1
        if BoScript.canUnitTypeMineMinerals(ai.player,p)==0 and BoScript.canUnitTypeMineOil(ai.player,p)==0:
          BoScript.produceUnit(ai.player,u,p)
        elif BoScript.playerUnitsOfTypeCount(ai.player,p)<3 and BoScript.canUnitTypeMineMinerals(ai.player,p):
          BoScript.produceUnit(ai.player,u,p)
        elif BoScript.playerUnitsOfTypeCount(ai.player,p)<3 and BoScript.canUnitTypeMineOil(ai.player,p):
          BoScript.produceUnit(ai.player,u,p)
      else:
        boprint("debug", "newProd %s " % newProd[u])
        newProd[u] = 0


# vim: et sw=2
