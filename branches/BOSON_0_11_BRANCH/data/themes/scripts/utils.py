from string import upper

boprint_debugLevel = "info"

def boprint(type, text):
  """Print something on the console

  boprint(text, type)
  type    debug, info, warning, error
  text    The text to print out

  type == error will not exit the script or boson !
  """

  if type <= boprint_debugLevel:
    if boprint_debugLevel == "debug":
      print "BoScript: [" + upper(type)+ "] " + `text`
    elif boprint_debugLevel == "info" and type != "debug":
      print "BoScript: [" + upper(type)+ "] " + `text`
    elif boprint_debugLevel == "warning":
      if type != "info" or type != "debug":
        print "BoScript: [" + upper(type)+ "] " + `text`
    elif boprint_debugLevel == "error":
      if type != "info" or type != "debug" or type != "warning":
        print "BoScript: [" + upper(type)+ "] " + `text`

def boprint_setDebugLevel(level):
  if level != "debug" and level != "info" and level != "warning" and level != "error":
    print "Error: boprint_setDebugLevel(): level must be one of debug, info, warning or error!"
    return
  global boprint_debugLevel
  boprint_debugLevel = level

