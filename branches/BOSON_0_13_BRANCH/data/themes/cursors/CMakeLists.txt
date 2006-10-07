set(cursors_install_dir ${KDE3_DATA_INSTALL_DIR}/boson/themes/cursors)

set(cursors
	alternative
	modern1
	futuristic
	nato-standard
)

file(GLOB index_files ${CMAKE_CURRENT_SOURCE_DIR}/*/index.cursor)
foreach(index_file ${index_files})
	file(RELATIVE_PATH index_file ${CMAKE_CURRENT_SOURCE_DIR} ${index_file})
	string(REGEX REPLACE \\/.* "" type ${index_file})

	install_files(${cursors_install_dir}/${type} FILES ${index_file})

	file(GLOB mode_index_files ${CMAKE_CURRENT_SOURCE_DIR}/${type}/*/index.cursor)

	foreach(mode_index_file ${mode_index_files})
		file(RELATIVE_PATH mode_index_file ${CMAKE_CURRENT_SOURCE_DIR}/${type} ${mode_index_file})
		string(REGEX REPLACE \\/.* "" mode ${mode_index_file})

		install_files(${cursors_install_dir}/${type}/${mode} FILES ${type}/${mode_index_file})


		file(GLOB jpeg_files ${CMAKE_CURRENT_SOURCE_DIR}/${type}/${mode}/*.jpg)
		file(GLOB png_files ${CMAKE_CURRENT_SOURCE_DIR}/${type}/${mode}/*.png)
		foreach(image ${jpeg_files} ${png_files})
			install_files(${cursors_install_dir}/${type}/${mode} FILES ${image})
		endforeach(image)
	endforeach(mode_index_file)

endforeach(index_file)

