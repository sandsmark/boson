project(boglc)

include_directories(
  ${CMAKE_SOURCE_DIR}/bodebug
  ${QT_INCLUDE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
)

set(boglc_SRCS
	boglc.cpp
)

find_package(Qt3 REQUIRED)
include_directories(${QT_INCLUDE_DIR})

#kde3_automoc(${boglc_SRCS})

add_executable(boglc ${boglc_SRCS})
target_link_libraries(boglc
	${QT_LIBRARIES}
)


