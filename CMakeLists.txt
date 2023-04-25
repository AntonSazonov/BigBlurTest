cmake_minimum_required( VERSION 3.7 )

find_program( CCACHE_PROGRAM ccache )
if( CCACHE_PROGRAM )
	message( STATUS "Using ccache." )
	set( CMAKE_C_COMPILER_LAUNCHER ${CCACHE} )
	set( CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE} )
endif()

set( BBT_PROJECT_NAME "BigBlurTest" )

project( ${BBT_PROJECT_NAME} CXX )

if( NOT CMAKE_SYSTEM_NAME STREQUAL "Windows" )
	message( STATUS " Platform: ${CMAKE_SYSTEM_NAME}" )
	message( FATAL_ERROR " Only Windows platform supported at the moment." )
endif()

set( CMAKE_CXX_STANDARD 17 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )

set( BLEND2D_STATIC TRUE )
add_subdirectory( src/blend2d )

set( BUILD_TESTING OFF )
add_subdirectory( src/cpu_features )

add_executable( ${BBT_PROJECT_NAME}
	BigBlurTest.cpp

	src/stb_impl.hpp
	src/stb_impl.cpp
	src/stb/stb_image.h
	src/stb/stb_image_resize.h

	src/agg/agg_array.h
	src/agg/agg_basics.h
	src/agg/agg_blur.h
	src/agg/agg_color_rgba.h
	src/agg/agg_config.h
	src/agg/agg_pixfmt_transposer.h

	src/platform/san_platform.hpp
	src/platform/san_window_base.hpp
	src/platform/san_window_win32.hpp

	src/san_parallel_for.hpp
	src/san_surface.hpp
	src/san_image_list.hpp
	src/san_impls_list.hpp
	src/san_adaptor_agg_image.hpp

	src/ui/san_ui.hpp
	#src/ui/san_ui_console.hpp
	src/ui/san_ui_ctrl.hpp
	src/ui/san_ui_ctrl_button.hpp
	src/ui/san_ui_ctrl_checkbox.hpp
	src/ui/san_ui_ctrl_text.hpp
	src/ui/san_ui_ctrl_link.hpp
	src/ui/san_ui_ctrl_slider.hpp

	src/san_adaptor_straight_line.hpp
	src/san_blur_gaussian_naive.hpp
	src/san_blur_stack_luts.hpp
	src/san_blur_stack_naive.hpp
	src/san_blur_stack_naive_calc.hpp
	src/san_blur_stack_simd_naive.hpp
	src/san_blur_stack_simd_calc.hpp
	src/san_blur_stack_simd_optimized_1.hpp
	src/san_blur_stack_simd_optimized_2.hpp )

target_precompile_headers( ${BBT_PROJECT_NAME} PUBLIC
	"$<$<COMPILE_LANGUAGE:CXX>:<src/san_pch.hpp$<ANGLE-R>>" )

# -march=native -Ofast -ffast-math -funroll-loops -fno-exceptions -fno-rtti ) # -Wextra -Wpedantic
set( GNU_AND_CLANG_COMMON_COMP_OPTS -mavx2 -Wall -fno-exceptions -fno-rtti )
set( GNU_AND_CLANG_COMMON_LINK_OPTS -mconsole -s ) # -static

if( CMAKE_CXX_COMPILER_ID STREQUAL "Clang" )
	set( CMAKE_CXX_FLAGS_DEBUG "-O1 ${CMAKE_CXX_FLAGS_DEBUG}" )
	target_compile_options( ${BBT_PROJECT_NAME} PRIVATE ${GNU_AND_CLANG_COMMON_COMP_OPTS} )
	target_link_options   ( ${BBT_PROJECT_NAME} PRIVATE ${GNU_AND_CLANG_COMMON_LINK_OPTS} )

elseif( CMAKE_CXX_COMPILER_ID STREQUAL "GNU" )
	set( CMAKE_CXX_FLAGS_DEBUG "-O1 ${CMAKE_CXX_FLAGS_DEBUG}" )
	target_compile_options( ${BBT_PROJECT_NAME} PRIVATE ${GNU_AND_CLANG_COMMON_COMP_OPTS} ) # -Wno-class-memaccess
	target_link_options   ( ${BBT_PROJECT_NAME} PRIVATE ${GNU_AND_CLANG_COMMON_LINK_OPTS} )

#elseif( CMAKE_CXX_COMPILER_ID STREQUAL "Intel" )
# not tested

elseif( CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" )
	set( CMAKE_CXX_FLAGS_DEBUG "/O1 ${CMAKE_CXX_FLAGS_DEBUG}" )
	target_compile_options( ${BBT_PROJECT_NAME} PRIVATE /W3 /arch:AVX2 ) # /W4 /Wall /favor:blend
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
target_link_libraries     ( ${BBT_PROJECT_NAME} PRIVATE blend2d::blend2d cpu_features )
set_target_properties     ( ${BBT_PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR} )
set_target_properties     ( ${BBT_PROJECT_NAME} PROPERTIES OUTPUT_NAME "${CMAKE_PROJECT_NAME}-${CMAKE_CXX_COMPILER_ID}-${CMAKE_BUILD_TYPE}" )
