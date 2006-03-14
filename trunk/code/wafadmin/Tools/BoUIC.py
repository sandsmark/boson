#! /usr/bin/env python

import os, shutil, sys
import Action, Common, Object, Task, Params, Runner, Utils, Scan

def trace(msg):
	Params.trace(msg, 'BoUIC')
def debug(msg):
	Params.debug(msg, 'BoUIC')
def error(msg):
	Params.error(msg, 'BoUIC')




uic_vardeps = ['UIC', 'QTPLUGINS']
bouic_vardeps = ['bouic']
def bouic_build(task):
	base = task.m_outputs[1].m_name
	base = base[:len(base)-4]

	inc_kde  ='#include <klocale.h>\n#include <kdialog.h>\n'
	inc_moc  ='#include "%s.moc"\n' % base

	boui_path   = task.m_inputs[0].bldpath()
	h_path    = task.m_outputs[0].bldpath()
	cpp_path  = task.m_outputs[1].bldpath()

	bouic_command = task.m_env['bouic']

	compile  = bouic_command
	compile += ' --input ' + boui_path
	compile += ' --output %s' + base
	compile += ' --adinclude klocale.h'

	ret = Runner.exec_command( compile )

	if ret:
		return ret

	return ret


bouicact = Action.GenAction('bouic', bouic_vardeps)
bouicact.m_function_to_run = bouic_build



def detect(conf):
	return 0


