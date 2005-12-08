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

		bouic = env['BOUIC']

		compile = bouic + ' --input %s --output %s --addinclude klocale.h' % (source[0].path, base)
		compile_moc = '$QT_MOC -o %s %s' % (base + '.moc', base + '.h')
		if env.Execute(compile): return ret
		ret = env.Execute(compile_moc)
		return ret

	def boui_scan(node, env, path): # AB: arguments might be broken (scons doc said it should take 4, but it takes only 3)
		# this function adds "bouic" as requirement to .boui files
		return [env['BOUIC']]


	import SCons.Defaults
	import SCons.Tool
	import SCons.Util
	import SCons.Node
	Builder = SCons.Builder.Builder
	Scanner = SCons.Scanner.Scanner


	env['BUILDERS']['Bouic']=Builder(action=bouic_processing, emitter=bouicEmitter, suffix='.h', src_suffix='.boui')

	bouiscan = Scanner(function = boui_scan, skeys = ['.boui'])
	env.Append(SCANNERS = bouiscan)

	print "TODO: Review this code (bouic scons code)!"
	# AB: review the following code and adapt according to
	#     http://www.scons.org/doc/HTML/scons-user.html#AEN1795
	#     !
	#     -> just prefix a path with '#' to make it point to the
	#        top-directory

	import os
	abspath = os.path.abspath('.')
	if env['_BUILDDIR_']:
		abspath += '/' + env['_BUILDDIR_']
	abspath += '/' + 'boson/boufo/bouic/bouic'
	env['BOUIC'] = abspath


