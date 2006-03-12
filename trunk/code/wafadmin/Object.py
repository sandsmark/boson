#! /usr/bin/env python
# encoding: utf-8
# Thomas Nagy, 2005 (ita)

# genobj is an abstract class for declaring targets:
#   * creates tasks (consisting of a task, an environment, a list of source and list of target)
#   * sets environment on the tasks (which are copies most of the time)
#   * modifies environments as needed
# 
# genobj cannot be used as it is, so you must either create a subclass or use a customobj
#
# subclassing
#   * makes it possible to share environment copies for several objects at once (efficiency)
#   * be careful to call Object.genobj.__init__(...) in the init of your subclass
#
# using customobj
#   * makes it possible to declare new kind of targets quickly (give a pattern ? and the action name)
#   * is not really flexible

import os, shutil, types
import Action, Params, Environment, Runner, Task

def trace(msg):
	Params.trace(msg, 'Object')
def debug(msg):
	Params.debug(msg, 'Object')
def error(msg):
	Params.error(msg, 'Object')

g_register={}
g_allobjs=[]

def store(name, node):
	global g_register
	if name in g_register: error("attempt to store %s which is already in"%name)
	g_register[name]=node

def get(name):
	global g_register
	return g_register[name]

# call flush for every group of object to process
def flush():
	trace("delayed operation called")
	while len(Params.g_outstanding_objs)>0:
		trace("posting object")

		obj=Params.g_outstanding_objs.pop()
		obj.post()
		Params.g_posted_objs.append(obj)

		trace("object posted")

class genobj:
	def __init__(self, type, actname):
		self.m_type  = type
		self.m_posted = 0
		self.m_current_path = Params.g_curdirnode # emulate chdir when reading scripts

		# TODO if we are building something, we need to make sure the folder is scanned
		#if not Params.g_curdirnode in Params...

		# default name of the action to use for the tasks
		self.m_actname = actname

		# we use a simple list for the tasks
		self.m_tasks = []

		# targets / sources
		self.source = ''
		self.target = ''

		# things that the obj depends on
		self.m_deps = []

		# no default environment - in case if
		self.env = None

		# register ourselves - used at install time
		g_allobjs.append(self)

		# allow delayed operations on objects created (declarative style)
		# an object is then posted when another one is added
		# of course, you may want to post the object yourself first :)
		#flush()
		Params.g_outstanding_objs.append(self)

		if not type in self.get_valid_types():
			error("BUG genobj::init : invalid type given")

	def get_valid_types(self):
		return ['program', 'shlib', 'staticlib', 'other']

	# runs the code to create the tasks
	def post(self):
		if self.m_posted:
			error("OBJECT ALREADY POSTED")
			return
		self.apply()
		self.m_posted=1

	# probably important ?
	def to_real_file(self, file):
		# append the builddir to the file name, calls object to .. :)
		pass

	# the following function is used to modify the environments of the tasks
	def setForAll(self, var, val):
		for task in self.m_tasks:
			task.setvar(var, val)
	# the following function is used to modify the environments of the tasks
	def prependAll(self, var, val):
		for task in self.m_tasks:
			task.prependvar(var, val)
	# the following function is used to modify the environments of the tasks
	def appendAll(self, var, val):
		for task in self.m_tasks:
			task.appendvar(var, val)

	# important, creates the default tasks FIXME remove
	def createTasks(self):
		if not self.env:
			self.env = Params.g_default_env
		l_task=Task.Task(self.m_actname, self.env)
		self.m_tasks.append(l_task)

	def create_task(self, type, env, priority=5):
		task = Task.Task(type, env, priority)
		self.m_tasks.append(task)
		return task

	# creates the tasks, override this method
	def apply(self):
		# subclass me
		trace("nothing to do")

	def get_mirror_node(self, node, filename):
		tree=Params.g_build.m_tree
		return tree.mirror_file(node, filename)

	def file_in(self, filename):
		return [ self.get_mirror_node(self.m_current_path, filename) ]

	# ==== dependency part ====
	def set_name(self, name):
		global g_register
		if name in g_register: error("attempt to store %s which is already in"%name)
		g_register[name]=self

	def add_dep(self, obj):
		#if self.m_posted:
		#	error("OBJECT IS ALREADY POSTED DAMMIT!")
		#	return
		self.m_deps.append(obj)

	def depends(self, name):
		global g_register
		self.m_deps.append( g_register[name] )

	#def canbuild(self):
	#	for obj in self.m_deps:
	#		if not obj.up2date(): return False
	#	return True

	# an object is to be posted, even if only for install
	def install(self):
		# subclass me
		pass

	def install_results(self, var, subdir, task):

		trace('install results called')

		destpath = task.m_env[var]
		destdir  = task.m_env['DESTDIR']

		if destdir:
			destpath = destdir+destpath
		if subdir:
			destpath = os.path.join(destpath, subdir)

		try:
			os.stat(destpath)
		except:
			Runner.exec_command('mkdir -p %s' % destpath)

		for node in task.m_outputs:
			print "* installing %s to %s" % (node.bldpath(), destpath)
			shutil.copy2(node.abspath(), destpath)
		
class customobj:
	def __init__(genbj, val):
		genobj.__init__(self, val, 'CUSTOM')

		self.m_prefixes=[]
		self.m_suffixes=[]

		self.instdir=None
		self.instfiles=[]

		self.m_env = None
	# ?
	def apply(self):
		genobj.apply()
	# ?
	def handledeps(self):
		genobj.handledeps()

	def install(self):
		for node in self.instfiles:
			shutil.copy2(node.abspath(), self.instdir)

# ['CXX', ..] -> [env['CXX'], ..]
def list_to_env_list(env, vars_list):
	def get_env_value(var):
		# TODO add a cache here ?
		#return env[var]
		try:
			v = env[var]
			if type(v) is types.ListType:
				return " ".join(v)
			else:
				return v
		except:
			debug("variable %s does not exist in env !" % var)
			return ''
	return map(get_env_value, vars_list)

def sign_env_vars(env, vars_list):
	#lst = list_to_env_list(env, vars_list)
	#val = reduce( lambda a,b : Params.h_string(b)+Params.h_string(a), lst )
	#return val
	lst = list_to_env_list(env, vars_list)
	return Params.h_list(lst)

def reset():
	global g_register
	g_register={}
	g_allobjs=[]

