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
