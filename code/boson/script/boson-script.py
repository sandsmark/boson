print "Loading python script..."
import BoScript;

cycle = 0;

def init2():
  print "Init called";
  method1();

def method1():
  print "script method 1 called";

def method2(arg):
  print "script method 2 called";
  print arg;

def method3(arg):
  print "script method 3 called";
  print arg;

def advance():
  global cycle;
  cycle = cycle + 1;
  if (cycle % 10) == 0:
    print "advance method called, cycle: ", cycle;
  advanceBigCircle(cycle);

def advanceMaxPayne(cycle):
  x = cycle % 150;
  if x == 9:
    BoScript.moveCamera(5, -5, 0);
    BoScript.setCameraRotation(45);
    BoScript.setCameraZ(4);
    BoScript.setCameraRadius(0);
    BoScript.commitCameraChanges(1);
  elif x == 10:
    BoScript.moveCamera(25, -25, 0);
    BoScript.commitCameraChanges(50);
  elif x == 60:
    BoScript.moveCamera(45, -45, 0);
    BoScript.setCameraZ(10);
    BoScript.setCameraRadius(10);
    BoScript.commitCameraChanges(50);

def advanceBigCircle(cycle):
  if cycle == 100:
    BoScript.setCameraMoveMode(2);
    BoScript.moveCamera(25, -25, 0);
    BoScript.setCameraRotation(0);
    BoScript.setCameraZ(20);
    BoScript.setCameraRadius(20);
    BoScript.commitCameraChanges(90);
  elif cycle == 200:
    BoScript.setCameraRotation(720);
    BoScript.commitCameraChanges(4000);

def advanceSimpleRotation(cycle):
  if cycle == 100:
    BoScript.setCameraRotation(20);
    BoScript.setCameraZ(10);
    BoScript.setCameraRadius(0);
    BoScript.commitCameraChanges(1);
  elif cycle == 150:
    BoScript.setCameraRotation(380);
    BoScript.commitCameraChanges(100);

def advanceA10Video(cycle):
  if cycle == 100:
    print "Let's move the camera!";
    BoScript.setCameraMoveMode(2);
    BoScript.setCameraRotation(-140);
    BoScript.moveCamera(25, -33, 0);
    BoScript.setCameraZ(10);
    BoScript.setCameraRadius(8);
    BoScript.commitCameraChanges(100);
  elif cycle == 250:
    BoScript.moveCamera(27.6, -34.85, 0);
    BoScript.setCameraRotation(400);
    BoScript.commitCameraChanges(100);
  elif cycle == 350:
    BoScript.moveCamera(28.11, -35.22, 0);
    BoScript.commitCameraChanges(20);
  elif cycle == 370:
    BoScript.moveCamera(32, -38, 0);
    BoScript.setCameraZ(15);
    BoScript.setCameraRadius(4);
    BoScript.commitCameraChanges(100);
  elif cycle == 470:
    BoScript.moveCamera(35.9, -40.77, 0);
    BoScript.setCameraZ(20);
    BoScript.setCameraRotation(-60);
    BoScript.commitCameraChanges(150);
  elif cycle == 620:
    BoScript.moveCamera(37.2, -41.7, 0);
    BoScript.setCameraZ(22);
    BoScript.setCameraRotation(270);
    BoScript.commitCameraChanges(100);
  elif cycle == 720:
    print "Camera should be stopped now. What now?";

def myfunc():
  print "hi! this is myfunc";
