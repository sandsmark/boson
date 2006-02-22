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
		ufo_major_version = '0'
		ufo_minor_version = '8'
		ufo_micro_version = '0'
		ufo_version = ufo_major_version + '.' + ufo_minor_version + '.' + ufo_micro_version

		conf = env.Configure()

		if not os.path.exists(env['_BUILDDIR_']): os.mkdir(env['_BUILDDIR_'])
		dest = open(env.join(env['_BUILDDIR_'], 'config-libufo.h'), 'w')

		define_header(dest, 'HAVE_SSTREAM', conf.CheckCXXHeader('sstream'))
		define_header(dest, 'HAVE_STRSTREAM', conf.CheckCXXHeader('strstream'))
		define_header(dest, 'HAVE_STDINT_H', conf.CheckCHeader('stdint.h'))
		define_header(dest, 'HAVE_EXECINFO_H', conf.CheckCHeader('execinfo.h'))
		define_header(dest, 'HAVE_PWD_H', conf.CheckCHeader('pwd.h'))
		define_header(dest, 'HAVE_DLFCN_H', conf.CheckCHeader('dlfcn.h'))

		dest.close()
		env['_CONFIG_H_'].append('libufo')
		env = conf.Finish()

