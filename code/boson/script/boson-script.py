print "Loading python script..."
import BoScript;

cycle = 0;

def init():
  print "Init called";
  method1();
  oldAIInit();

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
  #if (cycle % 10) == 0:
  #  print "advance method called, cycle: ", cycle;
  #advanceBigCircle(cycle);
  #advanceA10Game(cycle);
  if cycle == 10:
    printUnitInfo();
  oldAIAdvance();

def advanceA10Game(cycle):
  if cycle == 100:
    print "let's attack with some units";
    units = [25, 26, 32, 33, 82, 88, 89 ];
    for unit in units:
      BoScript.moveUnitWithAttacking(unit, 46 * 48, 46 * 48);
  elif cycle == 140:
    print "stop two units";
    BoScript.stopUnit(25);
    BoScript.stopUnit(26);
  elif cycle == 160:
    print "move unit";
    BoScript.moveUnit(88, 20 * 48, 46 * 48);
  elif cycle == 180:
    print "attack unit";
    BoScript.attack(32, 1);

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

def printUnitInfo():
  units = BoScript.unitsInRect(0, 0, 1000, 1000)
  myunits = 0
  print "There are ", len(units), " units on the map"
  for unit in units:
    if BoScript.isMyUnit(unit) == 1:
      myunits = myunits + 1;
  print "I have ", myunits, " of them";


# AI stuff
aidelay = 0;
aicycle = 0;
aiunit = -1;
aitarget = -1;

def oldAIInit():
  print "oldAIInit() called";
  global aidelay;
  aidelay = int(BoScript.aiDelay() * 20);
  print "  aidelay set to ", aidelay;

def oldAIAdvance():
  global aidelay, aicycle, aiunit, aitarget;
  # AI will do something every aidelay advance calls
  aicycle = aicycle + 1;
  if not aicycle == aidelay:
    return;
  print "oldAIAdvance() executing";
  # reset aicycle
  aicycle = 0;
  # check if target is still alive
  if aitarget == -1 or BoScript.isUnitAlive(aitarget) == 0:
    print "  Target not set";
    aitarget = oldAIFindTarget();
    if aitarget == -1:
      print "No enemies left. Disabling self";
      aidelay = 0;
      return;
  print "  Target is ", aitarget;
  # find attacker
  attacker = -1;
  units = BoScript.allMyUnits();
  while attacker == -1:
    aiunit = aiunit + 1;
    if aiunit >= len(units):
      aiunit = -1;
      print "No attacker found, returning";
      return;
    u = units[aiunit];
    if BoScript.isUnitMobile(u) and BoScript.canUnitShoot(u):
      attacker = u;
      print "  attacker set to ", attacker;
  print "  Sending ", aiunit, ". unit with id ", attacker, " to attack";
  targetpos = BoScript.unitPosition(aitarget);
  BoScript.moveUnitWithAttacking(attacker, targetpos[0], targetpos[1]);


def oldAIFindTarget():
  players = BoScript.allPlayers();
  target = -1;
  # iterate through all players
  for p in players:
    if not BoScript.areEnemies(p, BoScript.playerId()):
      continue;
    units = BoScript.allPlayerUnits(p);
    # iterate through all units of player
    for u in units:
      if not BoScript.isUnitAlive(u):
        continue;
      # FIXME: command center id is hardcoded
      if BoScript.unitType(u) == 5:
        return u;
      if target == -1:
        target = u;
    # if cmdcenter wasn't found, return any other unit
    return target;
  # nothing was found
  return -1;


