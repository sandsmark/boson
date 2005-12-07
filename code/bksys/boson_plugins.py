def exists(env):
	return true

def generate(env):
	if env['HELP']:
		return

	env['BOSON_PLUGIN_DIR'] = env['KDEMODULE'] + '/plugins/boson'

