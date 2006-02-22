def exists(env):
	return true

def generate(env):
	if env['HELP']:
		env.pprint('BOLD', '*** Boson options ***')
		env.pprint('BOLD', '---------------------')

