print "BoScriptAI: - [INFO] Loading Python script for AI..."

from sys import exit
from utils import boprint
from random import randint

# This is just to be able to test the syntax whith you
# Python interpreter on the console
try:
  import BoScript
except ImportError:
  boprint("error", "Couldn't import BoScript, something bad is going on, watch your back !")


cycle = 0
player = -1


def init(id):
  global player,newProd,expl
  expl = -1
  newProd = {0:0}
  boprint("debug", "Init called")
  player = id
  boprint("debug", "player %s" % player)
  oldAIInit()

def advance():
  global cycle
  global player
  cycle = cycle + 1
  if (cycle % 100) == 0 and BoScript.minerals(player)>500 and BoScript.oil(player)>500:
    boprint("debug", "produced method called, cycle: %s" % cycle)
    produce()
  if (cycle % 100) == 0:
    boprint("debug", "mine method called, cycle: %s" % cycle)
    mine()
  if (cycle % 200) == 0:
    explore()
#if (cycle % 20) == 0:
	#   spawnSomeUnits()
  boprint("debug", "hi! advance")
  oldAIAdvance()


def myfunc():
  boprint("info", "hi! this is myfunc")

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
  BoScript.moveUnit(player, expl,randint(pos[0]-50,pos[0]+50) ,randint(pos[1]-50,pos[1]+50))
  if not BoScript.isUnitAlive(expl):
    expl = -1

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
  boprint("debug", "oldAIInit() called for player: %s" % player)
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
  global player
  boprint("debug", "oldAIAdvance() called for player: %s" % player)
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
  units = BoScript.allPlayerUnits(player)
  while attacker == -1:
    aiunit = aiunit + 1
    if aiunit >= len(units):
      aiunit = -1
      boprint("info", "No attacker found, returning")
      return
    u = units[aiunit]
    if BoScript.isUnitMobile(u):
      if BoScript.canUnitShoot(u) and u != expl:
        attacker = u
        boprint("debug", "attacker set to %s" % attacker)
        boprint("debug", "Sending %s unit with id %s to attack" % (aiunit, attacker))
  targetpos = BoScript.unitPosition(aitarget)
  BoScript.moveUnitWithAttacking(player, attacker, targetpos[0], targetpos[1])


def oldAIFindTarget():
  boprint("debug", "Target")
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
      oil=BoScript.nearestOilLocations(player,pos[0],pos[1],0,150)
      boprint("debug", "Mine oil  %s" % oil)
      if oil:
        BoScript.moveUnit(player, u, oil[0][0], oil[0][1])
        BoScript.mineUnit(player, u, oil[0][0], oil[0][1])
        boprint("debug", "Mine oil done")
    elif BoScript.canUnitMineMinerals(u) and BoScript.unitWork(u)==0 :
      boprint("debug", "id %s" % u)
      pos=BoScript.unitPosition(u)
      oil=BoScript.nearestMineralLocations(player,pos[0],pos[1],0,150)
      boprint("debug", "Mine mineral %s" % oil)
      if oil:
        BoScript.moveUnit(player, u, oil[0][0], oil[0][1])
        BoScript.mineUnit(player, u, oil[0][0], oil[0][1])
        boprint("debug", "Mine  minerals done")



def produce():
  global player,newProd
  units = BoScript.allPlayerUnits(player)
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
        if BoScript.canUnitTypeMineMinerals(player,p)==0 and BoScript.canUnitTypeMineOil(player,p)==0:
          BoScript.produceUnit(player,u,p)
        elif BoScript.playerUnitsOfTypeCount(player,p)<3 and BoScript.canUnitTypeMineMinerals(player,p):
          BoScript.produceUnit(player,u,p)
        elif BoScript.playerUnitsOfTypeCount(player,p)<3 and BoScript.canUnitTypeMineOil(player,p):
          BoScript.produceUnit(player,u,p)
      else:
        boprint("debug", "newProd %s " % newProd[u])
        newProd[u] = 0


