from math import *

from utils import *
from bomath import *
import traceback
import time

# This is just to be able to test the syntax whith you
# Python interpreter on the console
try:
  import BoScript
except ImportError:
  boprint("error", "Couldn't import BoScript, something bad is going on, watch your back !")



# The sun class to calculate horizontal coordinates (altitude/azimuth) of the Sun
class Sun:
  def __init__(self):
    self.alt = 0
    self.az = 0
    self.lon = 24.85
    self.lat = 48.633
    self.tzoffset = time.timezone / 3600.0
    if time.daylight != 0:
      self.tzoffset = time.altzone / 3600.0

    self.update(time.time())


  def equationOfTime(self, dayofyear):
    B = 2*pi * (dayofyear - 81) / 364
    return 9.87*sin(2*B) - 7.53*cos(B) - 1.5*sin(B)

  # Sets the observer location on earth
  def setObserverLocation(self, lon, lat):
    self.lon = lon
    self.lat = lat

  # Sets how many hours in front of UTC the time is
  def setTimezoneOffset(self, offset):
    self.tzoffset = offset


  # Updates altitude/azimuth of the sun
  # Alogrithm taken from:
  #  http://en.wikipedia.org/wiki/Horizontal_coordinate_system#The_position_of_the_Sun
  def update(self, curtime):
    try:
        # It doesn't matter what you add here (e.g. 1)
        # sometimes a TypeError Exception is thrown because curtime
        # not being an integer
        mytime = time.localtime(int(curtime))
    except TypeError:
        print("ERROR: FIXME: Is this a python bug ?")
        traceback.print_exc()
        return
    yearstart_ = time.struct_time((mytime.tm_year, 1, 1, 0, 0, 0, 0, 0, 0))
    yearstart = time.mktime(yearstart_)
    day = (curtime - yearstart) / (24*60*60.0) + 1

    # Calculate declination
    decl = -23.45 * cos((2*pi/365) * (day + 10))

    # Calculate hour angle
    T = mytime.tm_hour + mytime.tm_min/60.0 + mytime.tm_sec/3600.0
    T += self.lon/15.0 + self.tzoffset
    T += self.equationOfTime(day) / 60.0
    ha = (12 - T) * 15.0

    # Convert some variables to radians
    lat_rad = radians(self.lat)
    decl_rad = radians(decl)
    ha_rad = radians(ha)
    # Calculate altitude and azimuth
    self.alt = asin(sin(lat_rad) * sin(decl_rad) + cos(lat_rad) * cos(decl_rad) * cos(ha_rad))
    self.az = acos((cos(lat_rad) * sin(decl_rad) - sin(lat_rad) * cos(decl_rad) * cos(ha_rad)) / cos(self.alt))
    if ha < 0:
      self.az = 2*pi - self.az



cycle = 0
enable = 1
# How many cycles does one day last?
duration = 9600  # that's 8 minutes, as we have 20 advance calls per second

sun = Sun()
# Start at 2005/06/13 08:00
curtime_ = time.struct_time((2005, 6, 13,   8, 0, 0,   0, 1, -1))
curtime = time.mktime(curtime_)
sun.update(curtime)


def init(startcycle = 0):
  # Init lighting
  global cycle
  cycle = startcycle
  advanceDay(cycle)


def setEnabled(e):
  global enable
  if e != 0 and e != 1:
    print "Error: dayandnight.setEnabled(): e must be 0 or 1!"
    return
  enable = e

def setDayDuration(d):
  global duration
  duration = d


def advance():
  global cycle
  cycle = cycle + 1
  advanceDay(cycle)

def setTime(hour, min, sec = 0):
  global curtime
  mytime_ = time.struct_time((2005, 6, 13,   hour, min, sec,   0, 1, -1))
  curtime = time.mktime(mytime_)
  sun.update(curtime)
  updateLighting()




### "Day And Night" demo
### Simulates day and night
def advanceDay(cycle):
  # Make sure day and night cycle is enabled
  global enable, duration, sun, curtime
  if enable == 0:
    return

  # Light is updated every updatetime advance calls
  updatetime = 2

  if (cycle % updatetime) != 0:
    return

  ### Advance time
  # 24 hours is duration seconds here, so time is (86400.0 / duration) times
  #  faster here. And we need to multiply this by updatetime
  curtime += (86400.0 / duration) * updatetime

  ### Update sun
  sun.update(curtime)

  updateLighting()

def updateLighting():
  global sun
  ### Light position:
  # Calculate position of the light on xy plane
  lightpos2d = pointByRotation(degrees(sun.az), 5000)
  # Calculate height of the light.
  height = 5000 * tan(sun.alt)
  # Compose final position for the light and set it
  pos = lightpos2d[0], -lightpos2d[1], height, 1
  BoScript.setLightPos(0, pos)



  # Sunfactor should be an approximate of how much of Sun's light will reach
  #  the ground (some of it will be dispersed in the atmosphere)
  sunfactor = 0.0
  if sun.alt >= 90:
    sunfactor = 1.0
  elif sun.alt > 0.0:
    adist = 1.0 / sin(sun.alt)
    #sunfactor = 1.0 / sqrt(adist)
    sunfactor = 1.0 / adist


  ### Light colors:
  # Base intensities for ambient and diffuse colors
  baseambientday = 0.4  # Ambient color at day
  baseambientnight = 0.3  # Ambient color at night
  basediffuseday = 0.9  # Diffuse color at day
  basediffusenight = 0.0  # Diffuse color at night


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

  #print time.strftime("%Y/%m/%d %H:%M:%S", time.localtime(curtime)), " : sunpos: (%.1f; %.1f);  sf: %5.3f;  pos: (%5d; %5d; %5d)" % (degrees(sun.alt), degrees(sun.az),  sunfactor,  pos[0], -pos[1], pos[2])

