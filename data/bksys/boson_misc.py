def exists(env):
	return true

def generate(env):
	def define_header(file, name, have):
		file.write('#ifndef ' + name + '\n')
		if have:
			file.write('#define ' + name + ' 1\n')
		else:
			file.write('/* #undef ' + name + ' */\n')
		file.write('#endif\n')

        import os

	if not env['HELP'] and env['_CONFIGURE_']:
		conf = env.Configure()

		if not os.path.exists(env['_BUILDDIR_']): os.mkdir(env['_BUILDDIR_'])
		dest = open(env.join(env['_BUILDDIR_'], 'config-boson_misc.h'), 'w')

		define_header(dest, 'HAVE_SYS_TIME_H', conf.CheckCHeader('sys/time.h'))
		define_header(dest, 'HAVE_SYS_STAT_H', conf.CheckCHeader('sys/stat.h'))
		define_header(dest, 'HAVE_LIMITS_H', conf.CheckCHeader('limits.h'))

		dest.close()
		env['_CONFIG_H_'].append('boson_misc')
		env = conf.Finish()

