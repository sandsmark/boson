print "BoScriptAI: - [INFO] Loading Python script..."

from sys import exit
from utils import boprint

# This is just to be able to test the syntax whith you
# Python interpreter on the console
try:
  import BoScript
except ImportError:
  boprint("error", "Couldn't import BoScript, something bad is going on, watch your back !")


cycle = 0
player = -1


def init(id):
  global player
  boprint("debug", "Init called")
  player = id
  oldAIInit()

def advance():
  global cycle
  cycle = cycle + 1
  #if (cycle % 10) == 0:
  #  boprint("debug", "advance method called, cycle: %s" % cycle)
  #advanceBigCircle(cycle)
  #advanceA10Game(cycle)
  if cycle == 10:
    printUnitInfo()
  if cycle == 5:
    spawnSomeUnits()
  oldAIAdvance()

def advanceA10Game(cycle):
  if cycle == 100:
    boprint("info", "let's attack with some units")
    units = [25, 26, 32, 33, 82, 88, 89 ]
    for unit in units:
      BoScript.moveUnitWithAttacking(player, unit, 46 * 48, 46 * 48)
  elif cycle == 140:
    boprint("info", "stop two units")
    BoScript.stopUnit(player, 25)
    BoScript.stopUnit(player, 26)
  elif cycle == 160:
    boprint("info", "move unit")
    BoScript.moveUnit(player, 88, 20 * 48, 46 * 48)
  elif cycle == 180:
    boprint("info", "attack unit")
    BoScript.attack(player, 32, 1)

def advanceMaxPayne(cycle):
  x = cycle % 150
  if x == 9:
    BoScript.moveCamera(5, -5, 0)
    BoScript.setCameraRotation(45)
    BoScript.setCameraZ(4)
    BoScript.setCameraRadius(0)
    BoScript.commitCameraChanges(1)
  elif x == 10:
    BoScript.moveCamera(25, -25, 0)
    BoScript.commitCameraChanges(50)
  elif x == 60:
    BoScript.moveCamera(45, -45, 0)
    BoScript.setCameraZ(10)
    BoScript.setCameraRadius(10)
    BoScript.commitCameraChanges(50)

def advanceBigCircle(cycle):
  if cycle == 100:
    BoScript.setCameraMoveMode(2)
    BoScript.moveCamera(25, -25, 0)
    BoScript.setCameraRotation(0)
    BoScript.setCameraZ(20)
    BoScript.setCameraRadius(20)
    BoScript.commitCameraChanges(90)
  elif cycle == 200:
    BoScript.setCameraRotation(720)
    BoScript.commitCameraChanges(4000)

def advanceSimpleRotation(cycle):
  if cycle == 100:
    BoScript.setCameraRotation(20)
    BoScript.setCameraZ(10)
    BoScript.setCameraRadius(0)
    BoScript.commitCameraChanges(1)
  elif cycle == 150:
    BoScript.setCameraRotation(380)
    BoScript.commitCameraChanges(100)

def advanceA10Video(cycle):
  if cycle == 100:
    boprint("info", "Let's move the camera!")
    BoScript.setCameraMoveMode(2)
    BoScript.setCameraRotation(-140)
    BoScript.moveCamera(25, -33, 0)
    BoScript.setCameraZ(10)
    BoScript.setCameraRadius(8)
    BoScript.commitCameraChanges(100)
  elif cycle == 250:
    BoScript.moveCamera(27.6, -34.85, 0)
    BoScript.setCameraRotation(400)
    BoScript.commitCameraChanges(100)
  elif cycle == 350:
    BoScript.moveCamera(28.11, -35.22, 0)
    BoScript.commitCameraChanges(20)
  elif cycle == 370:
    BoScript.moveCamera(32, -38, 0)
    BoScript.setCameraZ(15)
    BoScript.setCameraRadius(4)
    BoScript.commitCameraChanges(100)
  elif cycle == 470:
    BoScript.moveCamera(35.9, -40.77, 0)
    BoScript.setCameraZ(20)
    BoScript.setCameraRotation(-60)
    BoScript.commitCameraChanges(150)
  elif cycle == 620:
    BoScript.moveCamera(37.2, -41.7, 0)
    BoScript.setCameraZ(22)
    BoScript.setCameraRotation(270)
    BoScript.commitCameraChanges(100)
  elif cycle == 720:
    boprint("info", "Camera should be stopped now. What now?")

def myfunc():
  boprint("info", "hi! this is myfunc")

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
  boprint("info", "oldAIInit() called for player: %s" % player)
  global aidelay
  aidelay = int(BoScript.aiDelay() * 20)
  boprint("debug", "aidelay set to %s" % aidelay)

def oldAIAdvance():
  global aidelay, aicycle, aiunit, aitarget
  # AI will do something every aidelay advance calls
  #   aicycle = aicycle + 1
  #   if not aicycle == aidelay:
  #     return
  # boprint("debug", "oldAIAdvance() executing")
  # reset aicycle
  global player
  boprint("info", "oldAIAdvance() called for player: %s" % player)
  aicycle = 0
  # check if target is still alive
  if aitarget == -1 or BoScript.isUnitAlive(aitarget) == 0:
    # boprint("warning", "Target not set")
    aitarget = oldAIFindTarget()
    if aitarget == -1:
      boprint("info", "No enemies left. Disabling self")
      aidelay = 0
      return
  # boprint("info", "Target is %s" % aitarget)
  # find attacker
  attacker = -1
  units = BoScript.allPlayerUnits(player)
  while attacker == -1:
    aiunit = aiunit + 1
    if aiunit >= len(units):
      aiunit = -1
      # boprint("info", "No attacker found, returning")
      return
    u = units[aiunit]
    if BoScript.isUnitMobile(u) and BoScript.canUnitShoot(u):
      attacker = u
      #boprint("info", "attacker set to %s" % attacker)
  # boprint("info", "Sending %s unit with id %s to attack" % (aiunit, attacker))
  targetpos = BoScript.unitPosition(aitarget)
  BoScript.moveUnitWithAttacking(player, attacker, targetpos[0], targetpos[1])


def oldAIFindTarget():
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
  boprint("info", "spawning some units for player: %s" % player)
  for x in range(4):
    BoScript.spawnUnit(player, 10035, 5, 5 + x * 2)

