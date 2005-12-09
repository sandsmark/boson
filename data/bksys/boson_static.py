# Copyright Andreas Beckermann 2005
# slightly based on libxml.py by Thomas Nagy: Copyright Thomas Nagy 2005
# BSD license (see COPYING)

def exists(env):
	return true

def generate(env):
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

	if env['HELP']:
		env.pprint('BOLD', '* semi-static      ', 'semi-static=yes: link to most (but not all) libraries statically. Implies use-plugins=no')
		env.pprint('BOLD', '* use-plugins      ', 'use-plugins=no: don\'t use debug plugins (they are used by default). Can be safely disabled without losing features.')

	if not env['HELP'] and (env['_CONFIGURE_'] or not env.has_key('BOSON_STATIC_ISCONFIGURED')):
		if not os.path.exists(env['_BUILDDIR_']): os.mkdir(env['_BUILDDIR_'])
		dest = open(env.join(env['_BUILDDIR_'], 'config-boson_static.h'), 'w')

		do_static = False
		use_plugins = True
		if env.has_key('ARGS'):
			if env['ARGS'].get('semi-static', 'no') != 'no':
				do_static = True
			if env['ARGS'].get('use-plugins', 'yes') == 'no':
				use_plugins = False

			if do_static:
				if not 'qtdir' in env['ARGS']:
					env.pprint('RED', 'You must use qtdir=<path> when using semi-static, to specify the path to the static Qt directory!')
				if not 'kdedir' in env['ARGS']:
					env.pprint('RED', 'You must use kdedir=<path> when using semi-static, to specify the path to the static KDE directory!')


		if do_static:
			use_plugins = False



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



