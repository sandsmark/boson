print "BoScriptAI: - [INFO] Loading Python script for localplayer..."

from sys import exit
from math import *
from utils import boprint
import sys

# This is just to be able to test the syntax whith you
# Python interpreter on the console
try:
  import BoScript
except ImportError:
  boprint("error", "Couldn't import BoScript, something bad is going on, watch your back !")

# Python didn't have radians() before 2.3, so in this case we have to add this
#  function ourselves
if sys.version_info[1] < 3:
  def radians(x):
    return x * (3.14159265359/180)


cycle = 0
player = -1

enabledayandnight = 1


def init(id):
  global player
  boprint("debug", "Init called")
  player = id
  # Init lighting
  advanceDay(0)

def advance():
  global cycle
  cycle = cycle + 1
  advanceDay(cycle)


### "Day And Night" demo
### Simulates day and night

def advanceDay(cycle):
  # Make sure day and night cycle is enabled
  global enabledayandnight
  if enabledayandnight == 0:
    return

  # How many cycles does one day last?
  duration = 2400  # that's 2 minutes
  # Light is updated every updatetime advance calls
  updatetime = 5

  if (cycle % updatetime) != 0:
    return

  # Is it day or night?  (this is basically time)
  # 0 = 1 = midnight (time is 0:00); 0.5 = midday (time is 12:00)
  # We add (duration / 2) to cycle because we want to start on midday
  dayfactor = ((cycle + duration / 2) % duration) / float(duration)
  setTime(dayfactor)

# Sets time of day and modifies lighting
def setTime(dayfactor):
  # How dark/light it is?
  # 0 = dark = night; 1 = light = day
  if dayfactor <= 0.5:
    # day is coming; clamp [0; 0.5]  (night - day) to [0; 1]  (dark - light)
    lightfactor = dayfactor * 2
  else:
    # night is coming; clamp [0.5; 1]  (day - night) to [1; 0]  (light - dark)
    lightfactor = 1 - (dayfactor - 0.5) * 2


  ### Light position:
  # Calculate position of the light on xy plane
  # It depends on dayfactor (time)
  # We add -0.2 to make it a bit more interesting
  lightpos2d = pointByRotation(((dayfactor - 0.2) * 360.0) % 360, 5000)

  # Calculate height of the light. Maximal height is 8660 (= 60 degrees)
  # Angle between ground and light will be 60 to -5
  sunangle = lightfactor * 65 - 5
  sunangletan = tan(radians(sunangle))
  height = 5000 * sunangletan

  # Compose final position for the light and set it
  pos = lightpos2d[0], -lightpos2d[1], height, 1
  BoScript.setLightPos(0, pos)


  ### Light colors:
  # Base intensities for ambient and diffuse colors
  baseambient = 0.4  # Ambient color at night (will be 0.65 at midday)
  basediffuse = 1.3  # Diffuse color at day (will be 0 at night)

  # How much sun lights the land. Note that this is 1 if sun is at >=50 degrees,
  #  not at 90 degrees.
  if sunangle >= 50:
    sunfactor = 1
  elif sunangle <= 0:
    sunfactor = 0
  else:
    # Sun angle is between 50 and 0 (exclusive)
    sunfactor = sin(radians(sunangle / 50 * 90))
  # If the sun is shining bright, ambient intensity is quite big (in real world,
  #  it's because of radiosity). Increase it
  # Note that we don't want it to get too high either, because we'd have no
  #  shading then
  baseambient += sunfactor * 0.25

  # Diffuse color is based on the angle of the sun. Note that here, we set
  #  diffuse color to max when sun angle is 60 degrees (max here), not 90.
  basediffuse = sunfactor * basediffuse

  # Set light colors
  ambient = baseambient, baseambient, baseambient, 1.0
  diffuse = basediffuse, basediffuse, basediffuse, 1.0
  BoScript.setLightAmbient(0, ambient)
  BoScript.setLightDiffuse(0, diffuse)
  BoScript.setLightSpecular(0, diffuse)  # not used yet, same as diffuse

  #print "Time: ", int(dayfactor * 24), ":", int(dayfactor * 1440 - 60 * int(dayfactor * 24)), " (dayfactor: ", dayfactor, "), lightfactor: ", lightfactor, ":  sun pos: (", int(lightpos2d[0]), "; ", int(-lightpos2d[1]), "; ", int(height), "), ambient: ", baseambient, ", diffuse: ", basediffuse

# Same as Bo3dTools::pointByRotation()
def pointByRotation(angle, radius):
  # Some quick tests
  if angle == 0:
    return 0, -radius;
  elif angle == 90:
    return radius, 0;
  elif angle == 180:
    return 0, radius;
  elif angle == 270:
    return -radius, 0;

  tmpx = tan(radians(angle))
  tmpy = 1.0
  length = sqrt(tmpx * tmpx + tmpy * tmpy)
  tmpx = tmpx / length * radius;
  tmpy = tmpy / length * radius;

  if angle < 90:
    return tmpx, -tmpy;
  elif angle < 180:
    return -tmpx, tmpy;
  elif angle < 270:
    return -tmpx, tmpy;
  else:
    return tmpx, -tmpy;



### Camera movement demos

# Tests free camera mode and changes up vector so that camera appears to be
#  tilted
def advanceFreeCamTest(cycle):
  if (cycle % 20) == 0:
    print "cycle: ", cycle

  if cycle == 10:
    print "checkpoint 1"
    #BoScript.setCameraFree(1)
  if cycle == 100:
    print "checkpoint 2"
    BoScript.setCameraLookAt(15, -15, 0)
    BoScript.setCameraRotation(45)
    BoScript.setCameraZ(7)
    BoScript.setCameraRadius(5)
    BoScript.commitCameraChanges(1)
  elif cycle == 120:
    print "checkpoint 3"
    BoScript.setCameraFreeMode(1)
  elif cycle == 140:
    print "checkpoint 4"
    # get current camera pos
    camerapos = BoScript.cameraPos()
    # camera will go 6 units down (will be 1 unit from the ground)
    BoScript.setCameraPos(camerapos[0], camerapos[1], camerapos[2] - 6)
    # lookat will be 2 units up from the new camera pos (camera will be looking upwards)
    BoScript.setCameraLookAt(15, -15, camerapos[2] - 5)
    # and camera will be slightly rotated
    BoScript.setCameraUp(-0.5, 0, 1.0)
    BoScript.commitCameraChanges(200)
  elif cycle == 360:
    print "checkpoint 5"
    BoScript.setCameraFreeMode(0)


# Demo gameplay on a10 map
def advanceA10Game(cycle):
  if cycle == 100:
    boprint("info", "let's attack with some units")
    units = [25, 26, 32, 33, 82, 88, 89 ]
    for unit in units:
      BoScript.moveUnitWithAttacking(player, unit, 46, 46)
  elif cycle == 140:
    boprint("info", "stop two units")
    BoScript.stopUnit(player, 25)
    BoScript.stopUnit(player, 26)
  elif cycle == 160:
    boprint("info", "move unit")
    BoScript.moveUnit(player, 88, 20, 46)
  elif cycle == 180:
    boprint("info", "attack unit")
    BoScript.attack(player, 32, 1)

# Camera movement similar to the one in Max Payne's intro
def advanceMaxPayne(cycle):
  x = cycle % 150
  if x == 9:
    BoScript.setCameraLookAt(5, -5, 0)
    BoScript.setCameraRotation(45)
    BoScript.setCameraZ(4)
    BoScript.setCameraRadius(0)
    BoScript.commitCameraChanges(1)
  elif x == 10:
    BoScript.setCameraLookAt(25, -25, 0)
    BoScript.commitCameraChanges(50)
  elif x == 60:
    BoScript.setCameraLookAt(45, -45, 0)
    BoScript.setCameraZ(10)
    BoScript.setCameraRadius(10)
    BoScript.commitCameraChanges(50)

# Camera makes a big circle (rotates) on the map
def advanceBigCircle(cycle):
  if cycle == 100:
    BoScript.setCameraMoveMode(2)
    BoScript.setCameraLookAt(25, -25, 0)
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
    BoScript.setCameraLookAt(25, -33, 0)
    BoScript.setCameraZ(10)
    BoScript.setCameraRadius(8)
    BoScript.commitCameraChanges(100)
  elif cycle == 250:
    BoScript.setCameraLookAt(27.6, -34.85, 0)
    BoScript.setCameraRotation(400)
    BoScript.commitCameraChanges(100)
  elif cycle == 350:
    BoScript.setCameraLookAt(28.11, -35.22, 0)
    BoScript.commitCameraChanges(20)
  elif cycle == 370:
    BoScript.setCameraLookAt(32, -38, 0)
    BoScript.setCameraZ(15)
    BoScript.setCameraRadius(4)
    BoScript.commitCameraChanges(100)
  elif cycle == 470:
    BoScript.setCameraLookAt(35.9, -40.77, 0)
    BoScript.setCameraZ(20)
    BoScript.setCameraRotation(-60)
    BoScript.commitCameraChanges(150)
  elif cycle == 620:
    BoScript.setCameraLookAt(37.2, -41.7, 0)
    BoScript.setCameraZ(22)
    BoScript.setCameraRotation(270)
    BoScript.commitCameraChanges(100)
  elif cycle == 720:
    boprint("info", "Camera should be stopped now. What now?")
