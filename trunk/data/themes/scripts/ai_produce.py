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
  boprint("debug","ai_produce.init()")
  produce()

def unitProduced(ownerid, pos, type, factorid):
  # Don't place other players' units
  if(ownerid != ai.player):
    return
  boprint("debug","unit with id %s produced" % type)
  placeUnit(factorid, type)

def placeUnit(factory, unitType):
  if(BoScript.isUnitTypeMobile(ai.player, int(unitType)) == False):
    boprint("debug", "Ok , building")
    pos = BoScript.unitPosition(int(factory))
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
    BoScript.placeProduction(ai.player, int(factory), tmpx, tmpy)


def unitPlaced(unitid, ownerid, pos, type):
  if(ownerid != ai.player):
    return
  boprint("debug","unit with id %s and type  %s placed " % (unitid,type))


def produce():
  units = BoScript.allPlayerUnits(ai.player)
  for u in units:
    # AB: unitWork(u) == 0 means that the unit is idle
    if BoScript.canUnitProduce(u) and BoScript.unitWork(u) == 0:
      boprint("debug", "start production algorithm for unit %d" % u)
      prod = BoScript.productionTypes(u)
      canProduceFacilities = 0
      canProduceMobiles = 0
      for p in prod:
        isMobile = BoScript.isUnitTypeMobile(ai.player, p)
        if isMobile:
          canProduceMobiles = 1
        else:
          canProduceFacilities = 1
      if canProduceFacilities:
        produceFacilities(u)
      if canProduceMobiles:
        produceMobiles(u)

def place():
  units = BoScript.allPlayerUnits(ai.player)
  for u in units:
    # AB; unitWork(u) == 9 means "WorkPlugin", which is e.g. produce
    if BoScript.canUnitProduce(u) and BoScript.unitWork(u) == 9 and BoScript.hasUnitCompletedProduction(u):
      boprint("debug", "start placement algorithm for unit %d" % u)
      placeUnit(u, BoScript.completedProductionType(u))

def produceFacilities(factory):
  boprint("debug", "produceFacilities()")
  prod = BoScript.productionTypes(factory)

  # at first we make sure that we always have at least 1000 more than we need.
  boprint("debug", "  rule 1: power!")
  powerGenerated = BoScript.powerGeneratedAfterConstructions(ai.player)
  powerConsumed = BoScript.powerConsumedAfterConstructions(ai.player)
  if powerGenerated < powerConsumed + 1000:
    boprint("debug", "  rule 1: not fullfilled - powerGenerated=%d, powerConsumed=%d" % (powerGenerated, powerConsumed))
    # try to build a powerplant (ID=2)
    for p in prod:
      if p == 2: # powerplant
        BoScript.produceUnit(ai.player, factory, p)
        return
    boprint("debug", "  rule 1: cannot fullfill rule")

  # buld mineral/oil refineries
  boprint("debug", "  rule 2: refineries!")
  haveMineralRefinery = BoScript.playerUnitsOfTypeCount(ai.player, 13)
  haveOilRefinery = BoScript.playerUnitsOfTypeCount(ai.player, 8)
  if haveMineralRefinery < 1 or haveOilRefinery < 1:
    boprint("debug", "  rule 2: not fullfilled")
    for p in prod:
      if p == 13 and haveMineralRefinery < 1:
        BoScript.produceUnit(ai.player, factory, p)
        return
      if p == 8 and haveOilRefinery < 1:
        BoScript.produceUnit(ai.player, factory, p)
        return
    boprint("debug", "  rule 2: cannot fullfill rule")

  # build weapon factory (ID=3) if necessary
  boprint("debug", "  rule 2: weaponsfactory!")
  haveWeaponsFactory = BoScript.playerUnitsOfTypeCount(ai.player, 3)
  if haveWeaponsFactory < 1:
    boprint("debug", "  rule 3: not fullfilled")
    for p in prod:
      if p == 3:
        BoScript.produceUnit(ai.player, factory, p)
        return
    boprint("debug", "  rule 3: cannot fullfill rule")

  # build at least a minimum amount of defense facilities (turrets, samsites)
  # TODO
  boprint("debug", "  rule 4: defenses! (TODO)")
  haveDefenses = 0 # TODO
  defensesMinimum = 0 # TODO
  if haveDefenses < defensesMinimum:
    boprint("debug", "  rule 4: not fullfilled")
    wantMoreTurrets = 0 # TODO
    wantMoreSamsites = 0 # TODO
    wantMoreAirTurrets = 0 # TODO
    for p in prod:
      if p == 10 and wantMoreTurrets > 0: # turret
        BoScript.produceUnit(ai.player, factory, p)
        return
      if p == 6 and wantMoreSamsites > 0: # samsite
        BoScript.produceUnit(ai.player, factory, p)
        return
      if p == 18 and wantMoreAirTurrets > 0: # airturret
        BoScript.produceUnit(ai.player, factory, p)
        return
    boprint("debug", "  rule 4: cannot fullfill rule")


  # build secondary production facilities and comsat
  boprint("debug", "  rule 5: secondary productions!")
  haveComsat = BoScript.playerUnitsOfTypeCount(ai.player, 14)
  haveHelipad = BoScript.playerUnitsOfTypeCount(ai.player, 1)
  haveTechcenter = BoScript.playerUnitsOfTypeCount(ai.player, 12)
  if haveComsat < 1 or haveHelipad < 1 or haveTechcenter < 1:
    boprint("debug", "  rule 5: not fullfilled")
    for p in prod:
      if p == 14 and haveComsat < 1:
        BoScript.produceUnit(ai.player, factory, p)
        return
      if p == 1 and haveHelipad < 1:
        BoScript.produceUnit(ai.player, factory, p)
        return
      if p == 12 and haveTechcenter < 1:
        BoScript.produceUnit(ai.player, factory, p)
        return
    boprint("debug", "  rule 5: cannot fullfill rule")


  # now make sure that we have some power reserves
  boprint("debug", "  rule 6: power reserves!")
  if powerGenerated < powerConsumed * 1.25:
    boprint("debug", "  rule 6: not fullfilled")
    for p in prod:
      if p == 2: # powerplant
        BoScript.produceUnit(ai.player, factory, p)
        return
    boprint("debug", "  rule 6: cannot fullfill rule")


  # finally build additional defenses
  defensesMaximum = 0 # TODO
  boprint("debug", "  rule 7: additional defenses! (TODO)")
  if haveDefenses < defensesMaximum:
    boprint("debug", "  rule 7: not fullfilled")
    wantMoreTurrets = 0 # TODO
    wantMoreSamsites = 0 # TODO
    wantMoreAirTurrets = 0 # TODO
    for p in prod:
      if p == 10 and wantMoreTurrets > 0: # turret
        BoScript.produceUnit(ai.player, factory, p)
        return
      if p == 6 and wantMoreSamsites > 0: # samsite
        BoScript.produceUnit(ai.player, factory, p)
        return
      if p == 18 and wantMoreAirTurrets > 0: # airturret
        BoScript.produceUnit(ai.player, factory, p)
        return
    boprint("debug", "  rule 7: cannot fullfill rule")


  boprint("debug", "  no rules left. nothing to produce.")

def produceMobiles(factory):
  boprint("debug", "produceMobiles()")
  prod = BoScript.productionTypes(factory)

  # build at least one harvester per kind
  boprint("debug", "  rule 1: harvesters!")
  haveMineralHarvesters = BoScript.playerUnitsOfTypeCount(ai.player, 10003)
  haveOilHarvesters = BoScript.playerUnitsOfTypeCount(ai.player, 10002)
  if haveMineralHarvesters < 1 or haveOilHarvesters < 1:
    boprint("debug", "  rule 1: not fullfilled")
    for p in prod:
      if p == 10002 and haveOilHarvesters < 1:
        BoScript.produceUnit(ai.player, factory, p)
        return
      if p == 10003 and haveMineralHarvesters < 1:
        BoScript.produceUnit(ai.player, factory, p)
        return
    boprint("debug", "  rule 1: cannot fullfill rule")


  # have at least 2 harvester per kind if minerals/oil is low
  boprint("debug", "  rule 2: harvesters for bad times!")
  minerals = BoScript.minerals(ai.player)
  oil = BoScript.minerals(ai.player)
  needMineralHarvesters = 0
  needOilHarvesters = 0
  if haveMineralHarvesters < 2 and minerals < 1000:
    needMineralHarvesters = 1
  if haveOilHarvesters < 2 and oil < 1000:
    needOilHarvesters = 1
  if needMineralHarvesters > 0 or needOilHarvesters > 0:
    boprint("debug", "  rule 2: not fullfilled")
    for p in prod:
      if p == 10002 and needOilHarvesters > 0:
        BoScript.produceUnit(ai.player, factory, p)
        return
      if p == 10003 and needMineralHarvesters > 0:
        BoScript.produceUnit(ai.player, factory, p)
        return
    boprint("debug", "  rule 2: cannot fullfill rule")

  # have at least 3 tanks
  boprint("debug", "  rule 3: battle units!")
  haveTanks = countUnitsOfType([10010, 1008, 10018])
  if haveTanks < 3:
    boprint("debug", "  rule 3: not fullfilled")
    if tryProduce(factory, 10010): # tiger
      return
    if tryProduce(factory, 10008): # leopard
      return
    if tryProduce(factory, 10018): # wolf
      return
    boprint("debug", "  rule 3: cannot fullfill rule")


  # if have refinery: have >= 3 harvesters
  boprint("debug", "  rule 4: more harvesters!")
  haveMineralRefinery = BoScript.playerUnitsOfTypeCount(ai.player, 13)
  haveOilRefinery = BoScript.playerUnitsOfTypeCount(ai.player, 8)
  if haveMineralRefinery > 0 and haveMineralHarvesters < 3:
    needMineralHarvesters = 1
  if haveOilRefinery > 0 and haveOilHarvesters < 3:
    needOilHarvesters = 1
  if needMineralHarvesters > 0 or needOilHarvesters > 0:
    boprint("debug", "  rule 4: not fullfilled")
    for p in prod:
      if p == 10002 and needOilHarvesters > 0:
        BoScript.produceUnit(ai.player, factory, p)
        return
      if p == 10003 and needMineralHarvesters > 0:
        BoScript.produceUnit(ai.player, factory, p)
        return
    boprint("debug", "  rule 4: cannot fullfill rule")


  # have at least 2 harvesters per refinery
  # (AB: note: already fullfilled by rule "have >= 3 harvesters", if we have
  # only one refinery)
  boprint("debug", "  rule 5: harvesters for everyone!")
  if haveMineralHarvesters < 2 * haveMineralRefinery:
    needMineralHarvesters = 1
  if haveOilHarvesters < 2 * haveMineralRefinery:
    needOilHarvesters = 1
  if needMineralHarvesters > 0 or needOilHarvesters > 0:
    boprint("debug", "  rule 5: not fullfilled")
    for p in prod:
      if p == 10002 and needOilHarvesters > 0:
        BoScript.produceUnit(ai.player, factory, p)
        return
      if p == 10003 and needMineralHarvesters > 0:
        BoScript.produceUnit(ai.player, factory, p)
        return
    boprint("debug", "  rule 5: cannot fullfill rule")


  # just build something to fight with
  boprint("debug", "  rule 6: fight!")
  # build all kinds of mobile units, except of harvesters
  allowedList = []
  for p in prod:
    if BoScript.isUnitTypeMobile(ai.player, p):
      if p != 10002 and p != 10003:
        allowedList = allowedList + p
  for p in allowedList:
    BoScript.produceUnit(ai.player, factory, p)

  boprint("debug", "  rule 6: cannot fullfill rule")


def countUnitsOfType(list):
  count = 0
  for u in list:
    count += BoScript.playerUnitsOfTypeCount(ai.player, u)
  return count

# produce "type" in "factory".
# returns 1 (true) if successful, otherwise 0 (false).
def tryProduce(factory, type):
  prod = BoScript.productionTypes(factory)
  for p in prod:
    if p == type:
      BoScript.produceUnit(ai.player, factory, p)
      return 1
  return 0

# vim: et sw=2
