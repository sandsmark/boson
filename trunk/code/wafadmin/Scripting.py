#! /usr/bin/env python
# encoding: utf-8
# Thomas Nagy, 2005 (ita)

import os, os.path, types, sys, cPickle
import Build, Params

def trace(msg):
	Params.trace(msg, 'Scripting')
def debug(msg):
	Params.debug(msg, 'Scripting')
def error(msg):
	Params.error(msg, 'Scripting')

# each script calls add_subdir
def add_subdir(dir):
	if Params.g_inroot:
		#node = Params.g_curdirnode.find_node(dir.split('/'))
		node = Params.g_build.m_tree.ensure_node_from_lst(Params.g_curdirnode, dir.split('/'))
		Params.g_subdirs.append( [node, Params.g_curdirnode] )

		if not node:
			error("grave error in add_subdir, subdir not found for "+str(dir))
			#print Params.g_curdirnode
			Params.g_curdirnode.debug()
			sys.exit(1)
		return

	restore = Params.g_curdirnode
	#if dir is None:
	#	error("error in subdir( "+dir)
	#	return

	#Params.g_curdirnode = Params.g_curdirnode.find_node(dir.split('/'))
	Params.g_curdirnode = Params.g_build.m_tree.ensure_node_from_lst(Params.g_curdirnode, dir.split('/'))
	if Params.g_curdirnode is None:
		error("subdir not found ("+dir+"), restore is "+str(restore))
		sys.exit(1)

	Params.g_subdirs.append(  [Params.g_curdirnode, restore]    )

def Main():
	# configure the project
	if Params.g_commands['configure']:
		from Utils import add_option
		import Environment, Configure, Tools
		from Environment import create_env
		from Configure import sub_config, create_config
		file = open('sconfigure', 'r')
		exec file
		sys.exit(0)

	# compile the project and/or install the files
	bld = Build.Build()
	bld.load()
	#bld.m_tree.dump()

	Params.g_inroot=1
	file=open('sconstruct', 'r')
	exec file
	Params.g_inroot=0

	while len(Params.g_subdirs)>0:
		# read scripts, saving the context everytime (Params.g_curdirnode)

		# cheap queue
		lst=Params.g_subdirs[0]
		Params.g_subdirs=Params.g_subdirs[1:]

		new=lst[0]
		old=lst[1]

		# take the new node position
		Params.g_curdirnode=new

		# idea : add an exception right here
		# but does this slow down the process ? to investigate (TODO ita)
		file=open('%s/sconscript'%new.abspath(), 'r')
		exec file

		# restore the old node position
		Params.g_curdirnode=old

	#bld.m_tree.dump()

	# for now
	bld.compile()

	try:
		if Params.g_commands['install']:
			bld.install()
	finally:
		bld.cleanup()
		bld.store()

# dist target - should be portable
def Dist(appname, version):
	import shutil, tarfile

	# Our temporary folder where to put our files
	TMPFOLDER=appname+'-'+version

	# Remove an old package directory
	if os.path.exists(TMPFOLDER): shutil.rmtree(TMPFOLDER)

	# Copy everything into the new folder
	shutil.copytree('.', TMPFOLDER)

	# Enter into it and remove unnecessary files
	os.chdir(TMPFOLDER)
	for (root, dirs, filenames) in os.walk('.'):
		clean_dirs = []
		for d in dirs:
			if d in ['CVS', 'cache', '_build_', '{arch}']:
				shutil.rmtree(os.path.join(root,d))
			elif d.startswith('.'):
				shutil.rmtree(os.path.join(root,d))
			else:
				clean_dirs += d
		dirs = clean_dirs
					
		to_remove = False
		for f in list(filenames):
			if f.startswith('.'): to_remove = True
			elif f.endswith('~'): to_remove = True
			elif f.endswith('.pyc'): to_remove = True
			elif f.endswith('.bak'): to_remove = True
			elif f.endswith('.orig'): to_remove = True
			elif f in ['config.log']: to_remove = True
			elif f.endswith('.tar.bz2'): to_remove = True
			elif f.endswith('.zip'): to_remove = True
			
			if to_remove:
				os.remove(os.path.join(root, f))
				to_remove = False

	# go back to the root directory
	os.chdir('..')
	
	tar = tarfile.open(TMPFOLDER+'.tar.bz2','w:bz2')
	tar.add(TMPFOLDER)
	tar.close()
	print 'Your archive is ready -> '+TMPFOLDER+'.tar.bz2'

	if os.path.exists(TMPFOLDER): shutil.rmtree(TMPFOLDER)

	sys.exit(0)

# distclean target - should be portable too
def DistClean():
	import os, shutil
	try: os.remove('.dblite')
	except: pass

	try: shutil.rmtree('_build_')
	except: pass

	try: shutil.rmtree('cache')
	except: pass

	for (root, dirs, filenames) in os.walk('.'):
		to_remove = False
		for f in list(filenames):
			if f.endswith('~'): to_remove = True
			elif f.endswith('.pyc'): to_remove = True
			
			if to_remove:
				#print "removing ",os.path.join(root, f)
				os.remove(os.path.join(root, f))
				to_remove = False
	sys.exit(0)

