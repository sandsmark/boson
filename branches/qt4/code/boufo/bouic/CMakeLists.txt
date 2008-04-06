project(bouic)

include_directories(
  ${CMAKE_SOURCE_DIR}/bodebug
  ${QT_INCLUDE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
)

set(bouic_SRCS
	bouic.cpp
)

boson_add_executable(bouic ${bouic_SRCS})
boson_target_link_libraries(bouic
	${QT_QT3SUPPORT_LIBRARY}
	${QT_QTXML_LIBRARY}
	${QT_QTCORE_LIBRARY}
)


