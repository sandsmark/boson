from utils import boprint

try:
  import BoScript
except ImportError:
  boprint("error", "Couldn't import BoScript, something bad is going on, watch your back !")


class Camera:
  """
  A wrapper for BoScript which handles everything about the camera.
  """

  #
  #----------------------  public  ----------------------
  #
  def __init__(self):
    # all these variables are private - use them read only!
    pass


  def setPos(self, x, y, z):
    BoScript.setCameraPos(x, y, z)

  def setLookAt(self, x, y, z):
    BoScript.setCameraLookAt(x, y, z)

  def setUp(self, x, y, z):
    BoScript.setCameraUp(x, y, z)

  def setDistance(self, dist):
    BoScript.setCameraDistance(dist)

  def setRotation(self, r):
    BoScript.setCameraRotation(r)

  def setXRotation(self, r):
    BoScript.setCameraXRotation(r)


  def addPos(self, x, y, z,  time):
    BoScript.addCameraPosPoint(x, y, z,  time)

  def addLookAt(self, x, y, z,  time):
    BoScript.addCameraLookAtPoint(x, y, z,  time)

  def addUp(self, x, y, z,  time):
    BoScript.addCameraUpPoint(x, y, z,  time)


  def setImmediateMode(self):
    BoScript.setCameraMoveMode(1)

  def setSegmentMode(self):
    BoScript.setCameraMoveMode(10)
    BoScript.setCameraInterpolationMode(1)

  def setCubicMode(self):
    BoScript.setCameraMoveMode(11)
    BoScript.setCameraInterpolationMode(10)


  def setFreeMode(self, on):
    BoScript.setCameraFreeMode(on)

  def setLimits(self, on):
    BoScript.setCameraLimits(on)


  def commit(self, time):
    BoScript.commitCameraChanges(time)
