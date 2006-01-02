from SCons.Script.SConscript import SConsEnvironment

def exists(env):
	return true

def generate(env):
	if env['HELP']:
		return

	import generic
	import SCons

	class BoDummyLib:
		def __init__(self, lib, static):
			self.lib = lib
			self.automatic = False

			# only used when automatic == False
			self.static = static

	def createDummyLibs(self, libs, force_static = False, force_shared = False):
		if (force_static and force_shared):
			print "Can have either force_shared or force_static, not both"
			force_static = False
		libs = self.make_list(libs)
		list = []
		for lib in libs:
			l = BoDummyLib(lib, force_static)
			if force_static or force_shared:
				l.automatic = False
			else:
				l.automatic = True
			list.append(l)
		return list

	class BoLib:
		def __init__(self, lib):
			self.lib = lib

			self.dirname = None
			self.filename = None
			self.libname = None
			self.linker_flag = ''

	class BoSharedLib(BoLib):
		def __init__(self, lib, local = True):
			BoLib.__init__(self, lib)
			self.linker_flag = '-Bdynamic'
			if local:
				import re
				file = slashify(lib)

				if sys.platform == 'darwin':
					reg = re.compile("(.*)/lib(.*).(dep|la|so|dylib)$")
				else:
					reg = re.compile("(.*)/lib(.*).(dep|la|so)$")
				result = reg.match(file)
				if not result:
					if sys.platform == 'darwin':
						reg = re.compile("(.*)/lib(.*)\.(\d+)\.(dep|la|so|dylib)")
					else:
						reg = re.compile("(.*)/lib(.*)\.(dep|la|so)\.(\d+)")
					result = reg.match(file)
					if not result:
						print "Unknown la file given "+file
						return
					dir  = result.group(1)
					link = result.group(2)
				else:
					dir  = result.group(1)
					link = result.group(2)

				f = SCons.Node.FS.default_fs.File(file)
				self.libname = link
				self.dirname = f.dir.path
				self.filename = f.name
			else:
				self.libname = lib
				self.dirname = None
				self.filename = None

	class BoStaticLib(BoLib):
		def __init__(self, lib, local = True):
			BoLib.__init__(self, lib)
			self.linker_flag = '-Bstatic'
			if local:
				import re
				file = generic.slashify(lib)
				reg = re.compile("(.*)/lib(.*)\.(dep|la|a)$")
				result = reg.match(file)
				if not result:
					print "Unknown archive file given "+file
					return

				self.libname = result.group(2)

				f = SCons.Node.FS.default_fs.File(file)
				self.dirname = f.dir.path
				self.filename = f.name
			else:
				print "WARNING: global static libs not yet handled propertly"
				self.libname = lib


	import kde3
	class boobject(env.kobject):
		def __init__(self, val, senv=None):
			self.botype = val
			if val == 'boplugin':
				# give 'shlib' to kobject and genobj
				val = 'shlib'
			self.bo_libs = []
			env.kobject.__init__(self, val, senv)
		def execute(self):
			if self.botype == 'boplugin':
				# AB: WARNING this is probably not dependable!
				# the following line makes sure that
				# kobject.execute() does not override the
				# instdir.
				# we cannot use e.g. shlib here, as kobject sets
				# a instdir then. however we cannot set it to
				# boplugin either, as kobject doesn't execute
				# other required tasks then
				# -> module works atm.
				#    however I am not sure if it "currently"
				#    works only, i.e. by accident.
				#    a clean solution is yet to be found.
				self.type = 'module'

				self.instdir = env['BOSON_PLUGIN_DIR']


			self._libflags = ''
			linker_flag = ''
			for lib in self.bo_libs:
				if lib.linker_flag != linker_flag:
					self._libflags += ' -Wl,' + lib.linker_flag
					linker_flag = lib.linker_flag
				if lib.dirname:
					self._libflags += ' -L' + lib.dirname
				if lib.libname:
					self._libflags += ' -l' + lib.libname
			if linker_flag != '-Bdynamic':
				self._libflags += ' -Wl,-Bdynamic'

			if not self.env:
				self.env = self.orenv.Copy()
			if len(self._libflags)>0:
				self.env.Prepend(_LIBFLAGS = self.orenv.make_list(self._libflags))


			env.kobject.execute(self)

		def boAddLibObjects(self, libs):
			for lib in libs:
				if lib.automatic == True:
					self.boAddLibs(lib.lib)
				else:
					if lib.static == True:
						self.boAddLibs(lib.lib, True)
					else:
						self.boAddLibs(lib.lib, False, True)

		def boAddLibs(self, libs, force_static = False, force_shared = False):
			list = self.orenv.make_list(libs)
			if self.orenv['WINDOWS']:
				lext = ['.la','.dep']
			else:
				lext = ['.so', '.la', '.dylib','.dll','.dep']
			sext = ['.a']

			for lib in list:
				sal = SCons.Util.splitext(lib)
				if len(sal) > 1:
					if not force_shared and (sal[-1] in lext) and (sys.platform == 'darwin'):
						l = self.fixpath(sal[0]+'.dylib')[0]
						self.bo_libs.append(BoSharedLib(l, True))
					elif not force_shared and (sal[1] in lext) and (sys.platform != 'darwin'):
						# rh: don't know if DEPFILE should be supported on darwin too
						if self.orenv['DEPFILE']:
							l = self.fixpath(sal[0]+'.dep')[0]
							self.bo_libs.append(BoSharedLib(l, True))
						elif self.orenv['LIBTOOL']:
							l = self.fixpath(sal[0]+'.la')[0]
							self.bo_libs.append(BoSharedLib(l, True))
					elif not force_shared and sal[1] in sext:
						l = self.fixpath(sal[0]+'.a')[0]
						self.bo_libs.append(BoStaticLib(l, True))
					else:
						l = lib
						if force_static:
							self.bo_libs.append(BoStaticLib(l, False))
						else:
							self.bo_libs.append(BoSharedLib(l, False))


	import os
	# TODO: somehow make sure that dir actually points to the source
	# directory (not to build dir)
	def installDirectory(env, destination, dir):
		abspath = os.path.abspath(dir)
		for i in os.listdir(abspath):
			target = destination + '/' + dir
			env.Alias('install', target)
			if os.path.isfile(abspath + '/' + i):
				env.Install(target, dir + '/' + i)
			elif os.path.isdir(abspath + '/' + i):
				if i == 'CVS' or i == '.svn':
					continue
				installDirectory(env, destination, dir + '/' + i)


	SConsEnvironment.installDirectory = installDirectory
	SConsEnvironment.boobject = boobject
	SConsEnvironment.BoDummyLib = BoDummyLib
	SConsEnvironment.createDummyLibs = createDummyLibs

