project(boglc)

include_directories(
  ${CMAKE_SOURCE_DIR}/bodebug
  ${QT_INCLUDE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
)

set(boglc_SRCS
	boglc.cpp
)

find_package(Qt5 REQUIRED COMPONENTS GUI)
include_directories(${QT_INCLUDE_DIR})

add_executable(boglc ${boglc_SRCS})
target_link_libraries(boglc
	${QT_LIBRARIES}
)


