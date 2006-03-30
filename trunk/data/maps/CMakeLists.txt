set(maps_install_dir ${KDE3_DATA_INSTALL_DIR}/boson/maps)

install_files(${maps_install_dir} .*\\.bpf)

# AB: install_files(<dir> <regexp>) cannot handle subdirs.
#     install_files(<dir> FILES file1 file2 ...) can handle subdirs, but then we
#     would have to list every single file.
#     any solutions?

# TODO: subdirectories
message(STATUS "TODO: install maps/<foo>/*.bpf")

