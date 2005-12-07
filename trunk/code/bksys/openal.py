# Copyright Andreas Beckermann 2005
# slightly based on libxml.py by Thomas Nagy: Copyright Thomas Nagy 2005
# BSD license (see COPYING)

"""
Find and load the OpenAL necessary compilation and link flags
"""

def exists(env):
	return true

def generate(env):
	from SCons.Options import Options
	cachefile=env['CACHEDIR']+'openal.cache.py'
	opts = Options(cachefile)
	opts.AddOptions(
		('OPENAL_ISCONFIGURED', ''),
		('LIB_OPENAL', ''),
		('OPENAL_INCLUDES', ''),
	)
	opts.Update(env)

	import os
	if env['HELP']:
		return
	if not env['HELP'] and (env['_CONFIGURE_'] or not env.has_key('OPENAL_ISCONFIGURED')):
		conf = env.Configure()
		have_al_lib = False
		have_al_header = False
		if conf.CheckCHeader('AL/al.h'):
			have_al_header = True
			if conf.CheckLib('libopenal'):
				have_al_lib = True
			else:
				have_al_lib = False
		else:
			have_al_header = False
		conf.Finish()

		have_al = (have_al_lib and have_al_header)
		if have_al:
			env['LIB_OPENAL'] = 'openal'
		else:
			if have_al_lib:
				env.pprint('YELLOW', 'OpenAL headers not found (only library found).')
			elif have_al_headers:
				env.pprint('YELLOW', 'OpenAL library not found.')
			else:
				env.pprint('YELLOW', 'OpenAL not found (headers and library missing).')
			# AB: note that missing OpenAL does NOT result in a
			#     configure abort!

		if not os.path.exists(env['_BUILDDIR_']): os.mkdir(env['_BUILDDIR_'])
		dest = open(env.join(env['_BUILDDIR_'], 'config-openal.h'), 'w')
		dest.write('#define HAVE_OPENAL ')
		if have_al:
			dest.write('true\n')
		else:
			dest.write('false\n')
		dest.close()
		env['_CONFIG_H_'].append('openal')

		env['OPENAL_ISCONFIGURED']=1
		opts.Save(cachefile, env)


