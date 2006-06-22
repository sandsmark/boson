project(bobmfconverter)

add_subdirectory(libgfx)

include_directories(
	${QT_INCLUDE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/mixkit
	${CMAKE_CURRENT_SOURCE_DIR}/libgfx
	${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
	${LIB3DS_INCLUDE_DIR}
)

set(libgfx_SRCS
	libgfx/geom3d.cxx
	libgfx/geom4d.cxx
	libgfx/gltools.cxx
	libgfx/gui.cxx
	libgfx/mat2.cxx
	libgfx/mat3.cxx
	libgfx/mat4.cxx
	libgfx/quat.cxx
	libgfx/raster-jpeg.cxx
	libgfx/raster-png.cxx
	libgfx/raster-pnm.cxx
	libgfx/script.cxx
	libgfx/symmat3.cxx
	libgfx/symmat4.cxx
	libgfx/time.cxx
)

set(libmixkit_SRCS
	mixkit/MxAsp.cxx
	mixkit/MxBlockModel.cxx
	mixkit/MxCmdParser.cxx
	mixkit/MxDualModel.cxx
	mixkit/MxDualSlim.cxx
	mixkit/MxEdgeFilter.cxx
	mixkit/MxFaceTree-2.cxx
	mixkit/MxFaceTree.cxx
	mixkit/MxFeatureFilter.cxx
	mixkit/MxFitFrame-2.cxx
	mixkit/MxFitFrame.cxx
	mixkit/MxFrame.cxx
	mixkit/MxGeom2D.cxx
	mixkit/MxGeom3D.cxx
	mixkit/MxHeap.cxx
	mixkit/MxMat3-jacobi.cxx
	mixkit/MxMat4-jacobi.cxx
	mixkit/MxMatrix.cxx
	mixkit/MxPropSlim.cxx
	mixkit/MxQMetric.cxx
	mixkit/MxQMetric3.cxx
	mixkit/MxQSlim.cxx
	mixkit/MxSMF.cxx
	mixkit/MxStdModel.cxx
	mixkit/MxStdSlim.cxx
	mixkit/MxTriProject.cxx
	mixkit/mixmops.cxx
	mixkit/mixmsg.cxx
#	mixkit/MxCamera.cxx
#	mixkit/MxFitFrame-vis.cxx
#	mixkit/MxGLDebug.cxx
#	mixkit/MxGLUtils.cxx
#	mixkit/MxQVis3.cxx
#	mixkit/MxStdRender.cxx
)

set(bobmfconverter_SRCS
	main.cpp
	bo3dtools.cpp
	frame.cpp
	loader.cpp
	lod.cpp
	material.cpp
	mesh.cpp
	model.cpp
	processor.cpp
	saver.cpp
	texture.cpp

	processors/defaultmaterials.cpp
	processors/frameoptimizer.cpp
	processors/lodcreator.cpp
	processors/materialoptimizer.cpp
	processors/meshoptimizer.cpp
	processors/normalcalculator.cpp
	processors/textureoptimizer.cpp
	processors/transformer.cpp
	processors/unuseddataremover.cpp
	processors/vertexoptimizer.cpp

	loaders/loader-3ds.cpp
	loaders/loader-ac.cpp
	loaders/loader-md2.cpp

	${libgfx_SRCS}
	${libmixkit_SRCS}
)

kde3_automoc(${bobmfconverter_SRCS})

kde3_add_executable(bobmfconverter ${bobmfconverter_SRCS})
boson_target_link_libraries(bobmfconverter
	${QT_AND_KDECORE_LIBS}
	${LIB3DS_LIBRARY}
)

install_targets(/bin
	bobmfconverter
)

