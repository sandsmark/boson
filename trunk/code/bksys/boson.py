from SCons.Script.SConscript import SConsEnvironment

def exists(env):
	return true

def generate(env):
	if env['HELP']:
		return

	import kde3
	class boobject(env.kobject):
		def __init__(self, val, senv=None):
			self.botype = val
			if val == 'boplugin':
				# give 'shlib' to kobject and genobj
				val = 'shlib'
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
			env.kobject.execute(self)

	SConsEnvironment.boobject = boobject

