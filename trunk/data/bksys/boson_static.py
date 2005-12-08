# Copyright Andreas Beckermann 2005
# slightly based on libxml.py by Thomas Nagy: Copyright Thomas Nagy 2005
# BSD license (see COPYING)

def exists(env):
	return true

def generate(env):
	do_static = False
	use_plugins = True

	# ... TODO: make configurable

	if do_static:
		use_plugins = False


	from SCons.Options import Options
	cachefile=env['CACHEDIR']+'boson_static.cache.py'
	opts = Options(cachefile)
	opts.AddOptions(
		('BOSON_STATIC_ISCONFIGURED', ''),
		('BOSON_DO_STATIC', ''),
		('BOSON_USE_PLUGINS', ''),
	)
	opts.Update(env)

	import os
	if not env['HELP'] and (env['_CONFIGURE_'] or not env.has_key('BOSON_STATIC_ISCONFIGURED')):
		if not os.path.exists(env['_BUILDDIR_']): os.mkdir(env['_BUILDDIR_'])
		dest = open(env.join(env['_BUILDDIR_'], 'config-boson_static.h'), 'w')

		dest.write('#define BOSON_LINK_STATIC ')
		if do_static:
			dest.write('1\n')
			env['BOSON_DO_STATIC']=True
		else:
			env['BOSON_DO_STATIC']=False
			dest.write('0\n')

		dest.write('#define USE_BO_PLUGINS ')
		if use_plugins:
			dest.write('1\n')
			env['BOSON_USE_PLUGINS']=True
		else:
			env['BOSON_USE_PLUGINS']=False
			dest.write('0\n')

		dest.close()

		env['_CONFIG_H_'].append('boson_static')

		env['BOSON_STATIC_ISCONFIGURED']=1
		opts.Save(cachefile, env)



