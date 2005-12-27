# Copyright Andreas Beckermann 2005
# slightly based on libxml.py by Thomas Nagy: Copyright Thomas Nagy 2005
# BSD license (see COPYING)

"""
Find Xrandr library and header
"""

def exists(env):
	return true

def generate(env):
	from SCons.Options import Options
	cachefile=env['CACHEDIR']+'libxrandr.cache.py'
	opts = Options(cachefile)
	opts.AddOptions(
		('LIBXRANDR_ISCONFIGURED', ''),
		('LIB_XRANDR', ''),
	)
	opts.Update(env)

	import os
	if env['HELP']:
		return
	if not env['HELP'] and (env['_CONFIGURE_'] or not env.has_key('LIBXRANDR_ISCONFIGURED')):
		conf = env.Configure()

		have_lib = False
		have_header = False
		header = ['X11/Xlib.h', 'X11/extensions/Xrandr.h']
		lib = 'Xrandr'
		import SCons.Conftest
		import SCons.SConf
		if conf.CheckHeader(header):
			have_header = True
		else:
			have_header = False

		conf.env.Append(LIBPATH = env['LIBPATH_X11'])
		if conf.CheckLib(lib):
			have_lib = True
		else:
			have_lib = False

		conf.Finish()

		have_xrandr = (have_lib and have_header)
		if have_xrandr:
			env['LIB_XRANDR'] = lib
		else:
			env['LIB_XRANDR'] = ''
			if have_lib:
				env.pprint('YELLOW', 'Xrandr headers not found (only library found).')
			elif have_header:
				env.pprint('YELLOW', 'Xrandr library not found.')
			else:
				env.pprint('YELLOW', 'Xrandr not found (headers and library missing).')
			# AB: note that missing Xrandr does NOT result in a
			#     configure abort!

		if not os.path.exists(env['_BUILDDIR_']): os.mkdir(env['_BUILDDIR_'])
		dest = open(env.join(env['_BUILDDIR_'], 'config-libxrandr.h'), 'w')
		if have_xrandr:
			dest.write('#define HAVE_XRANDR 1\n')
		dest.close()
		env['_CONFIG_H_'].append('libxrandr')

		env['LIBXRANDR_ISCONFIGURED']=1
		opts.Save(cachefile, env)


