def exists(env):
	return true

def generate(env):
	def Check_boson_prefix(context):
		context.Message('Checking for boson prefix... ')

		# AB: BOSON_PREFIX is always exactly PREFIX.
		#     we use a different name mostly for historic reasons.
		#     (autotools suck)
		prefix = env['PREFIX']
		context.Result(prefix)


		return prefix

        import os

	if not env['HELP'] and env['_CONFIGURE_']:
		conf = env.Configure(custom_tests = { 'Check_boson_prefix' : Check_boson_prefix} )

		boson_prefix = conf.Check_boson_prefix()
		if not boson_prefix:
			print 'cannot determine boson prefix.'
			env.Exit(1)

		if not os.path.exists(env['_BUILDDIR_']): os.mkdir(env['_BUILDDIR_'])
		dest = open(env.join(env['_BUILDDIR_'], 'config-boson_prefix.h'), 'w')
		dest.write('/* boson prefix (always equal to PREFIX) */\n')
		dest.write('#define BOSON_PREFIX \"' + boson_prefix + '\"\n')
		dest.close()
		env['_CONFIG_H_'].append('boson_prefix')
		env = conf.Finish()

