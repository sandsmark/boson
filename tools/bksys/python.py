# Copyright Andreas Beckermann 2005
# slightly based on libxml.py by Thomas Nagy: Copyright Thomas Nagy 2005
# BSD license (see COPYING)

"""
Find and load the python necessary compilation and link flags
"""

def exists(env):
	return true

def generate(env):
	def findPythonLib(env, major, minor):
		libname = 'python%d.%d' % (major, minor)
		conf = env.Configure()
		found = conf.CheckLib(libname)
		env = conf.Finish()
		if found:
			return libname
		return ''


	def findPythonHeaders(env, major, minor):
		python_dir = ''
		incdirs = []
		baseincdirs = ['/usr/include', '/usr/local/include']
		if python_dir:
			baseincdirs += [python_dir + '/include']
		pythonversiondir = '/python%d.%d' % (major, minor)
		for dir in baseincdirs:
			incdirs += [dir]
			incdirs += [dir + pythonversiondir]
		conf = env.Configure()
		for dir in incdirs:
			found = conf.CheckCHeader(dir + '/Python.h')
			if found:
				env = conf.Finish()
				return dir
		env = conf.Finish()
		return ''

	def requirePythonLib(env, major, minor):
		i = 9
		while i >= minor:
			have_headers = 0
			have_lib = findPythonLib(env, major, i)
			if have_lib:
				have_headers = findPythonHeaders(env, major, i)

			if  have_lib and have_headers:
				return (major, i, have_headers, have_lib)
			else:
				if have_lib:
					print "python %d.%d libs found, but headers missing" % (major, i)
				elif have_headers:
					print "python %d.%d headers found, but libs missing" % (major, i)
			i = i - 1;
		return ''



	from SCons.Options import Options
	cachefile=env['CACHEDIR']+'python.cache.py'
	opts = Options(cachefile)
	opts.AddOptions(
		('PYTHON_ISCONFIGURED', ''),
		('LIB_PYTHON', ''),
		('PYTHON_INCLUDES', ''),
	)
	opts.Update(env)

	import os
	if env['HELP']:
		return
	if not env['HELP'] and (env['_CONFIGURE_'] or not env.has_key('PYTHON_ISCONFIGURED')):
		needMajor = 2
		needMinor = 0
		found = requirePythonLib(env, needMajor, needMinor)
		if found:
			(foundMajor, foundMinor, headerDir, lib) = found
			have_python = 1
		else:
			have_python = 0

		if not have_python:
			env.pprint('RED', 'python >= %d.%d was not found (mandatory).' % (needMajor, needMinor))
			env.Exit(1)


		if not os.path.exists(env['_BUILDDIR_']): os.mkdir(env['_BUILDDIR_'])
		dest = open(env.join(env['_BUILDDIR_'], 'config-python.h'), 'w')

		dest.write('#define PYTHON_MAJOR %d\n' % foundMajor)
		dest.write('#define PYTHON_MINOR %d\n' % foundMinor)

		dest.close()

		env['_CONFIG_H_'].append('python')

		env['PYTHON_INCLUDES'] = headerDir
		env['LIB_PYTHON'] = lib

		env['PYTHON_ISCONFIGURED']=1
		opts.Save(cachefile, env)



