from string import upper

debugLevel = "info"

def boprint(type, text):
  """Print something on the console

  boprint(text, type)
  type    debug, info, warning, error
  text    The text to print out

  type == error will not exit the script or boson !
  """

  if type <= debugLevel:
    if debugLevel == "debug":
      print "BoScriptAI: [" + upper(type)+ "] " + `text`
    elif debugLevel == "info" and type != "debug":
      print "BoScriptAI: [" + upper(type)+ "] " + `text`
    elif debugLevel == "warning":
      if type != "info" or type != "debug":
        print "BoScriptAI: [" + upper(type)+ "] " + `text`
    elif debugLevel == "error":
      if type != "info" or type != "debug" or type != "warning":
        print "BoScriptAI: [" + upper(type)+ "] " + `text`

