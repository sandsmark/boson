from math import *
from random import *
from utils import *

# This is just to be able to test the syntax whith you
# Python interpreter on the console
try:
  import BoScript
except ImportError:
  boprint("error", "Couldn't import BoScript, something bad is going on, watch your back !")



cycle = 0
enable = 1

rand = Random(96435)
wbase = 0.0, 0.0, 0.0
wrandom = 0.0, 0.0, 0.0
wrandomscale = 0.4
wchangespeed = 0.6

def init(startcycle = 0):
  # Init lighting
  global cycle
  cycle = startcycle
  advance()


def setEnabled(e):
  global enable
  if e != 0 and e != 1:
    print "Error: wind.setEnabled(): e must be 0 or 1!"
    return
  enable = e

def setBaseWind(x, y, z):
  global wbase
  wbase = x, y, z

def setRandomScale(scale):
  global wrandomscale
  wrandomscale = scale

def setChangeSpeed(speed):
  global wchangespeed
  if speed < 0.0 or speed > 1.0:
    print "Error: wind.setChangeSpeed(): speed must be between 0.0 and 1.0!"
    return
  wchangespeed = speed



def advance():
  global cycle, rand, wbase, wrandom, wrandomscale, wchangespeed
  cycle = cycle + 1

  if (cycle % 40) != 0:
    return

  # The bigger newf is, the faster the wind changes
  newf = wchangespeed
  oldf = 1 - newf
  windx = wrandom[0] * oldf + (rand.random() - 0.5) * newf
  windy = wrandom[1] * oldf + (rand.random() - 0.5) * newf
  windz = wrandom[2] * oldf + (rand.random() - 0.5) * newf
  windz *= 0.5

  # Normalize wind vector
  l = sqrt(windx*windx + windy*windy + windz*windz)
  windx /= l
  windy /= l
  windz /= l
  wrandom = windx, windy, windz

  newwind = wbase[0] + wrandom[0]*wrandomscale, wbase[1] + wrandom[1]*wrandomscale, wbase[2] + wrandom[2]*wrandomscale
  BoScript.setWind(newwind)

