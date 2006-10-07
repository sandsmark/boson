project(bouic)

include_directories(
  ${CMAKE_SOURCE_DIR}/bodebug
  ${QT_INCLUDE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
)

set(bouic_SRCS
	bouic.cpp
)

kde3_automoc(${bouic_SRCS})

boson_add_executable(bouic ${bouic_SRCS})
boson_target_link_libraries(bouic
	${QT_LIBRARIES}
)


