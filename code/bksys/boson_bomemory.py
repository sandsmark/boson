def exists(env):
	return true

def generate(env):
	if env['HELP']:
		env.pprint('BOLD', '* use-bomemory     ', 'use-bomemory=yes: Compile and use bomemory. Highly experimental and debugging only. Increases memory usage and decreases performance.')
		return

	import os

	from SCons.Options import Options
	cachefile=env['CACHEDIR']+'boson_bomemory.cache.py'
	opts = Options(cachefile)
	opts.AddOptions(
		('BOSON_BOMEMORY_ISCONFIGURED', ''),
		('LIB_BOMEMORY', ''),
	)
	opts.Update(env)

	if not env['HELP'] and (env['_CONFIGURE_'] or not env.has_key('BOSON_BOMEMORY_ISCONFIGURED')):
		bomemory = False
		if env.has_key('ARGS') and (env['ARGS'].get('bomemory', 'no') == 'yes'):
			bomemory = True
		env['LIB_BOMEMORY'] = ''
		if bomemory:
			env['LIB_BOMEMORY'] = '#build/bomemory/libbomemory.a'

		if not os.path.exists(env['_BUILDDIR_']): os.mkdir(env['_BUILDDIR_'])
		dest = open(env.join(env['_BUILDDIR_'], 'config-boson_bomemory.h'), 'w')

		dest.write('#define BOSON_USE_BOMEMORY ')
		if bomemory:
			dest.write('1\n')
		else:
			dest.write('0\n')

		dest.close()
		env['_CONFIG_H_'].append('boson_bomemory')

