print "BoScriptAI: - [INFO] Loading Python script for localplayer..."

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

def advance():
  global cycle
  cycle = cycle + 1
  #advanceBigCircle(cycle)
  #advanceA10Game(cycle)


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
