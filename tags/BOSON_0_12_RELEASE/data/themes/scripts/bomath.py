from math import *
import sys


# Python didn't have radians() before 2.3, so in this case we have to add this
#  function ourselves
if sys.version_info[0] <= 2 and sys.version_info[1] < 3:
  def radians(x):
    return x * (3.14159265359/180)


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

