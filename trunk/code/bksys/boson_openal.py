# Copyright Andreas Beckermann 2005
# slightly based on libxml.py by Thomas Nagy: Copyright Thomas Nagy 2005
# BSD license (see COPYING)

def exists(env):
	return true

def generate(env):
	lib_bosound = 'bosound/libbosonsound.a bosound/libbosonsoundcommon.a'
	if env['LIB_OPENAL']:
		lib_bosound = lib_bosound + 'bosound/openal/libbosonsounopenal.a' + ' openal'
	env['LIB_BOSOUND'] = lib_bosound

	import os
	if env['HELP']:
		return
	if not env['HELP'] and (env['_CONFIGURE_']):
		if not os.path.exists(env['_BUILDDIR_']): os.mkdir(env['_BUILDDIR_'])
		dest = open(env.join(env['_BUILDDIR_'], 'config-boson_openal.h'), 'w')

		if env['LIB_OPENAL']:
			dest.write('#define BOSON_USE_OPENAL 1\n')

		dest.close()
		env['_CONFIG_H_'].append('boson_openal')

