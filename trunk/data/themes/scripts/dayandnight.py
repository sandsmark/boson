from math import *

from utils import *
from bomath import *

# This is just to be able to test the syntax whith you
# Python interpreter on the console
try:
  import BoScript
except ImportError:
  boprint("error", "Couldn't import BoScript, something bad is going on, watch your back !")


cycle = 0
enable = 1


def init(cycle = 0):
  # Init lighting
  advanceDay(cycle)


def setEnabled(e):
  if e != 0 and e != 1:
    print "Error: dayandnight.setEnabled(): e must be 0 or 1!"
    return
  enable = e


def advance():
  global cycle
  cycle = cycle + 1
  advanceDay(cycle)


### "Day And Night" demo
### Simulates day and night
def advanceDay(cycle):
  # Make sure day and night cycle is enabled
  global enable
  if enable == 0:
    return

  # How many cycles does one day last?
  duration = 2400  # that's 2 minutes
  # Light is updated every updatetime advance calls
  updatetime = 5

  if (cycle % updatetime) != 0:
    return

  # Is it day or night?  (this is basically time)
  # 0 = 1 = midnight (time is 0:00); 0.5 = midday (time is 12:00)
  # We add (duration * 0.25) to cycle because we want to start at morning
  dayfactor = ((cycle + duration * 0.25) % duration) / float(duration)
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
  baseambientday = 0.55  # Ambient color at day
  baseambientnight = 0.35  # Ambient color at night
  basediffuseday = 0.8  # Diffuse color at day
  basediffusenight = 0.15  # Diffuse color at night

  # How much sun lights the land. Note that this is 1 if sun is at >=50 degrees,
  #  not at 90 degrees.
  if sunangle >= 50:
    sunfactor = 1
  elif sunangle <= 0:
    sunfactor = 0
  else:
    # Sun angle is between 50 and 0 (exclusive)
    sunfactor = sin(radians(sunangle / 50 * 90))

  # Calculate base ambient intensity
  # If the sun is shining bright, ambient intensity is quite big (in real world,
  #  it's because of radiosity). Increase it
  # Note that we don't want it to get too high either, because we'd have no
  #  shading then
  baseambient = baseambientnight + sunfactor * (baseambientday - baseambientnight)

  # Calculate base diffuse intensity
  # Diffuse color is based on the angle of the sun. Note that here, we set
  #  diffuse color to max when sun angle is 60 degrees (max here), not 90.
  basediffuse = basediffusenight + sunfactor * (basediffuseday - basediffusenight)

  # Set light colors
  ambient = baseambient, baseambient, baseambient, 1.0
  diffuse = basediffuse, basediffuse, basediffuse, 1.0
  BoScript.setLightAmbient(0, ambient)
  BoScript.setLightDiffuse(0, diffuse)
  BoScript.setLightSpecular(0, diffuse)  # not used yet, same as diffuse

  #print "Time: ", int(dayfactor * 24), ":", int(dayfactor * 1440 - 60 * int(dayfactor * 24)), " (dayfactor: ", dayfactor, "), lightfactor: ", lightfactor, ":  sun pos: (", int(lightpos2d[0]), "; ", int(-lightpos2d[1]), "; ", int(height), "), ambient: ", baseambient, ", diffuse: ", basediffuse

