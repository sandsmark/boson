# Copyright Andreas Beckermann 2005
# BSD license (see COPYING)

def exists(env):
	return true

def generate(env):
	from SCons.Options import Options
	if env['HELP']:
		return

	if not env['BOSON_DO_STATIC']:
		# AB: linking dynamically, most dependencies are resolved
		#     automatically. only Xmu may be required (boson depends
		#     on it)
		env['LIB_KDECORE_DEPENDENCIES_OBJECTS'] = env.createDummyLibs('qt-mt Xmu')
		return

	# TODO: figure out how to calculate the dependencies (e.g. by somehow reading the .la files)

	libs = []
	libs += env.createDummyLibs('kdefx DCOP qt-mt Xmu', env['BOSON_DO_STATIC'])
	libs += env.createDummyLibs('SM ICE Xcursor Xrender Xrandr Xext X11', env['BOSON_DO_STATIC'])
	libs += env.createDummyLibs('idn art_lgpl_2', env['BOSON_DO_STATIC'])

	# NOT static:
	libs += env.createDummyLibs('pthread dl', False)

	env['LIB_KDECORE_DEPENDENCIES_OBJECTS'] = libs


