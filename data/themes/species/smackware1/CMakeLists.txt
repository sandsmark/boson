include(../install_species.cmake)

file(RELATIVE_PATH current_species ${BOSON_SPECIES_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
boson_install_species(${BOSON_SPECIES_DIR} ${BOSON_SPECIES_INSTALL_DIR} ${current_species})

