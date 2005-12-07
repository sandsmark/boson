# Copyright Andreas Beckermann 2005
# some parts stolen from kde3.py by Thomas Nagy: Copyright Thomas Nagy 2005

import os, re, types
from SCons.Script.SConscript import SConsEnvironment

def exists(env):
	return true

def generate(env):

	def bouicEmitter(target, source, env):
		adjustixes = SCons.Util.adjustixes
		bs = SCons.Util.splitext(str(source[0].name))[0]
		bs = env.join(str(target[0].get_dir()),bs)
		target.append(bs+'.cpp')
		target.append(bs+'.moc')
		return target, source

	def bouic_processing(target, source, env):
		adjustixes = SCons.Util.adjustixes
		base = SCons.Util.splitext(str(source[0].name))[0]
		base = env.join(str(target[0].get_dir()),base)

		# TODO: FIXME
		bouic = 'build/boson/boufo/bouic/bouic'
		print "TODO: find where uic resides. %s is hardcoded." % bouic

		compile = bouic + ' --input %s --output %s --addinclude klocale.h' % (source[0].path, base)
		compile_moc = '$QT_MOC -o %s %s' % (base + '.moc', base + '.h')
		if env.Execute(compile): return ret
		ret = env.Execute(compile_moc)
		return ret


	import SCons.Defaults
	import SCons.Tool
	import SCons.Util
	import SCons.Node
	Builder = SCons.Builder.Builder


	env['BUILDERS']['Bouic']=Builder(action=bouic_processing, emitter=bouicEmitter, suffix='.h', src_suffix='.boui')



