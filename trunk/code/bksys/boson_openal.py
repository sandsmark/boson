# Copyright Andreas Beckermann 2005
# slightly based on libxml.py by Thomas Nagy: Copyright Thomas Nagy 2005
# BSD license (see COPYING)

def exists(env):
	return true

def generate(env):
	if env['HELP']:
		return

	import os

	# local libs
	lib_bosound = ['bosound/libbosonsound.a', 'bosound/libbosonsoundcommon.a']
	if env['LIB_OPENAL']:
		lib_bosound.append('bosound/openal/libbosonsoundopenal.a')

	tmp = lib_bosound
	lib_bosound = []
	abspath = os.path.abspath('.')
	if env['_BUILDDIR_']:
		abspath += '/' + env['_BUILDDIR_']
	for dir in tmp:
		lib_bosound.append(abspath + '/' + dir)

	# global libs
	if env['LIB_OPENAL']:
		lib_bosound.append('openal')

	env['LIB_BOSOUND'] = lib_bosound

	if not env['HELP'] and (env['_CONFIGURE_']):
		if not os.path.exists(env['_BUILDDIR_']): os.mkdir(env['_BUILDDIR_'])
		dest = open(env.join(env['_BUILDDIR_'], 'config-boson_openal.h'), 'w')

		if env['LIB_OPENAL']:
			dest.write('#define BOSON_USE_OPENAL 1\n')

		dest.close()
		env['_CONFIG_H_'].append('boson_openal')

