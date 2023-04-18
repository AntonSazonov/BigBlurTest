cmake_minimum_required( VERSION 3.7 )

find_program( CCACHE_PROGRAM ccache )
if( CCACHE_PROGRAM )
	message( STATUS "Using ccache." )
	set( CMAKE_C_COMPILER_LAUNCHER ${CCACHE} )
	set( CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE} )
endif()

set( BBT_PROJECT_NAME "BigBlurTest" )

project( ${BBT_PROJECT_NAME} CXX )

set( CMAKE_CXX_STANDARD 17 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )

set( BLEND2D_STATIC TRUE )
add_subdirectory( src/blend2d )


find_package( SDL2 REQUIRED )

add_executable( ${BBT_PROJECT_NAME}
	BigBlurTest.cpp

	src/sdl_window_rgba.hpp
	src/stb/stb_image.h
	#src/stb/stb_image_resize.h
	src/stb_impl.hpp
	src/stb_impl.cpp
	src/san_parallel_for.hpp

	src/agg/agg_array.h
	src/agg/agg_basics.h
	src/agg/agg_blur.h
	src/agg/agg_color_rgba.h
	src/agg/agg_config.h
	src/agg/agg_pixfmt_transposer.h

	src/san_agg_image_adaptor.hpp

	src/san_ui.hpp
	#src/san_ui_console.hpp
	src/san_ui_ctrl.hpp
	src/san_ui_ctrl_button.hpp
	src/san_ui_ctrl_checkbox.hpp
	src/san_ui_ctrl_text.hpp
	src/san_ui_ctrl_link.hpp

	src/san_image_view.hpp
	src/san_image_list.hpp

	src/san_blur_gaussian_naive.hpp

	src/san_stack_blur_luts.hpp
	src/san_stack_blur_naive.hpp
	src/san_stack_blur_naive_calc.hpp

	src/san_stack_blur_simd.hpp
	src/san_stack_blur_simd_calc.hpp
	src/san_stack_blur_simd_fastest.hpp
	src/san_stack_blur_simd_fastest_2.hpp

	)

target_precompile_headers( ${BBT_PROJECT_NAME} PUBLIC
	"$<$<COMPILE_LANGUAGE:CXX>:<src/agg/agg_color_rgba.h$<ANGLE-R>>"
	"$<$<COMPILE_LANGUAGE:CXX>:<src/agg/agg_blur.h$<ANGLE-R>>"
	"$<$<COMPILE_LANGUAGE:CXX>:<src/agg/agg_array.h$<ANGLE-R>>"
	"$<$<COMPILE_LANGUAGE:CXX>:<src/agg/agg_basics.h$<ANGLE-R>>"
	"$<$<COMPILE_LANGUAGE:CXX>:<src/agg/agg_config.h$<ANGLE-R>>"
	"$<$<COMPILE_LANGUAGE:CXX>:<src/agg/agg_pixfmt_transposer.h$<ANGLE-R>>" )

set( CMAKE_CXX_FLAGS_DEBUG "-O1 ${CMAKE_CXX_FLAGS_DEBUG}" )

if( CMAKE_CXX_COMPILER_ID STREQUAL "Clang" )
	#target_compile_options( ${BBT_PROJECT_NAME} PRIVATE -march=native -Ofast -ffast-math -funroll-loops -Wall -fno-exceptions -fno-rtti ) # -msse2 -Wextra -Wpedantic
	target_compile_options( ${BBT_PROJECT_NAME} PRIVATE -march=native -Wall -fno-exceptions -fno-rtti )
	target_link_options   ( ${BBT_PROJECT_NAME} PRIVATE -mconsole -s )

elseif( CMAKE_CXX_COMPILER_ID STREQUAL "GNU" )
	target_compile_options( ${BBT_PROJECT_NAME} PRIVATE -march=native -Wall -fno-exceptions -fno-rtti -Wno-class-memaccess )
	target_link_options   ( ${BBT_PROJECT_NAME} PRIVATE -mconsole -s )

elseif( CMAKE_CXX_COMPILER_ID STREQUAL "Intel" )
	# ...

elseif( CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" )
	target_compile_options( ${BBT_PROJECT_NAME} PRIVATE /W4 )
	target_link_options   ( ${BBT_PROJECT_NAME} PRIVATE /subsystem:console )
endif()

get_target_property( COMPILE_OPTS ${BBT_PROJECT_NAME} COMPILE_OPTIONS )

if( CMAKE_BUILD_TYPE STREQUAL "Debug" )
	set( COMPILE_OPTS "${CMAKE_CXX_FLAGS_DEBUG} ${COMPILE_OPTS}" )
else()
	set( COMPILE_OPTS "${CMAKE_CXX_FLAGS_RELEASE} ${COMPILE_OPTS}" )
endif()

string( REPLACE ";" " " COMPILE_OPTS_STR "${COMPILE_OPTS}" )

message( STATUS " -------------------------------------------" )
message( STATUS " Compiler ID: ${CMAKE_CXX_COMPILER_ID}" )
message( STATUS " Build  type: ${CMAKE_BUILD_TYPE}" )
message( STATUS " Build flags: ${COMPILE_OPTS_STR}" )
message( STATUS " -------------------------------------------" )

set( SAN_CMAKE_CONFIG_FILE "src/san_cmake_config.hpp" )
configure_file( ${CMAKE_SOURCE_DIR}/${SAN_CMAKE_CONFIG_FILE}.in ${CMAKE_SOURCE_DIR}/${SAN_CMAKE_CONFIG_FILE} )

target_include_directories( ${BBT_PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR} )
target_include_directories( ${BBT_PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/src )
target_link_libraries     ( ${BBT_PROJECT_NAME} PRIVATE SDL2::SDL2 blend2d::blend2d )
set_target_properties     ( ${BBT_PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR} )
set_target_properties     ( ${BBT_PROJECT_NAME} PROPERTIES OUTPUT_NAME "${CMAKE_PROJECT_NAME}-${CMAKE_CXX_COMPILER_ID}-${CMAKE_BUILD_TYPE}" )
