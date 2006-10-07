# TODO: delete old species installation

# species_source_path: path to the "species" directory, i.e.
#                      <path_to_data>/themes/species
#                      -> NOT including the species name
# species_install_dir: the path to the install "species" directory, i.e.
#                      <path_to_kde3_data>/boson/themes/species
#                      -> NOT including the species name
# species:             the name of the actual species ("human", "heutral", ...)
macro(BOSON_INSTALL_SPECIES species_source_path species_install_dir species)
	set(full_species_path ${species_source_path}/${species})
	set(full_species_install_path ${species_install_dir}/${species})

	install_files(${full_species_install_path} FILES
		${full_species_path}/index.species
		${full_species_path}/index.technologies
	)


	# AB: non-playable players won't have this
	if(EXISTS ${full_species_path}/actions.boson)
		install_files(${full_species_install_path} FILES
			${full_species_path}/actions.boson
		)
	endif(EXISTS ${full_species_path}/actions.boson)

	# AB: non-playable players won't have this
	if(EXISTS ${full_species_path}/pixmaps)
		BOSON_INSTALL_SPECIES_PIXMAPS(${full_species_path}/pixmaps ${full_species_install_path}/pixmaps)
	endif(EXISTS ${full_species_path}/pixmaps)

	BOSON_INSTALL_SPECIES_SOUNDS(${full_species_path}/sounds ${full_species_install_path}/sounds)
	BOSON_INSTALL_SPECIES_OBJECTS(${full_species_path}/objects ${full_species_install_path}/objects)
	BOSON_INSTALL_SPECIES_UNITS(${full_species_path}/units ${full_species_install_path}/units)
endmacro(BOSON_INSTALL_SPECIES)


macro(BOSON_INSTALL_SPECIES_PIXMAPS pixmaps_source pixmaps_dest)
	file(GLOB jpg ${pixmaps_source}/*.jpg)
	file(GLOB png ${pixmaps_source}/*.png)
	foreach(file ${jpg} ${png})
		install_files(${pixmaps_dest} FILES
			${file}
		)
	endforeach(file)
endmacro(BOSON_INSTALL_SPECIES_PIXMAPS pixmaps_source pixmaps_dest)

macro(BOSON_INSTALL_SPECIES_SOUNDS sounds_source sounds_dest)
	file(GLOB sound_files ${sounds_source}/*.ogg)
	foreach(file ${sound_files})
		install_files(${sounds_dest} FILES
			${file}
		)
	endforeach(file)
endmacro(BOSON_INSTALL_SPECIES_SOUNDS sounds_source sounds_dest)


macro(BOSON_INSTALL_SPECIES_OBJECTS objects_source objects_dest)
	install_files(${objects_dest} FILES
		${objects_source}/objects.boson
	)

	file(GLOB object_models_3ds ${objects_source}/*.3ds)
	file(GLOB object_models_ac ${objects_source}/*.ac)
	foreach(file ${object_models_3ds} ${object_models_ac})
		install_files(${objects_dest} FILES
			${file}
		)
	endforeach(file)
endmacro(BOSON_INSTALL_SPECIES_OBJECTS objects_source objects_dest)


# AB: TODO: it'd be much better to install the whole directory (except of the
# .svn/CVS directory)
macro(BOSON_INSTALL_SPECIES_UNITS units_source units_dest)
	BOSON_CHECK_SPECIES_UNITS(${units_source})
	file(GLOB units ${units_source}/*/index.unit)

	foreach(abs_unit ${units})
		file(RELATIVE_PATH unit ${units_source} ${abs_unit})
		string(REGEX REPLACE index\\.unit$ "" unit ${unit})

		set(unit_path ${units_source}/${unit})
		install_files(${units_dest}/${unit} FILES
			${unit_path}/index.unit
		)
		file(GLOB unit_files ${unit_path}/unit.*)
		file(GLOB png ${unit_path}/*.png)
		foreach(file ${png} ${unit_files})
			install_files(${units_dest}/${unit} FILES
				${file}
			)
		endforeach(file)
	endforeach(abs_unit)
endmacro(BOSON_INSTALL_SPECIES_UNITS units_source units_dest)

macro(BOSON_CHECK_SPECIES_UNITS units_source)
	file(GLOB units ${units_source}/*/index.unit)
	set(ids "")
	foreach(abs_unit ${units})
		file(READ ${abs_unit} index)
		string(REGEX MATCH "\nId *= *[0-9]+" id "${index}")
		string(REGEX MATCH "[0-9]+" id "${id}")
		if(NOT "${id}" MATCHES [0-9]+)
			set(id "")
		endif(NOT "${id}" MATCHES [0-9]+)
		if("${id}" MATCHES ^$)
			message(FATAL_ERROR "The unit file ${abs_unit} has no valid 'Id=<number>' entry!")
		endif("${id}" MATCHES ^$)

		set(ids "${id};${ids}")
	endforeach(abs_unit)

	foreach(id ${ids})
		set(seen 0)
		foreach(id2 ${ids})
			if(${id} STREQUAL ${id2})
				if(seen)
					message(SEND_ERROR "The Id ${id} is used more than once in ${units_source}")
				else(seen)
					set(seen 1)
				endif(seen)
			endif(${id} STREQUAL ${id2})
		endforeach(id2 ${ids})
	endforeach(id ${ids})
endmacro(BOSON_CHECK_SPECIES_UNITS units_source)

