cmake_minimum_required( VERSION 3.7 )

find_program( CCACHE_PROGRAM ccache )
if( CCACHE_PROGRAM )
	message( STATUS "Using ccache." )
	set( CMAKE_C_COMPILER_LAUNCHER ${CCACHE} )
	set( CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE} )
endif()

project( main CXX )

set( CMAKE_CXX_STANDARD 17 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )

set( BLEND2D_STATIC TRUE )
add_subdirectory( src/blend2d "${CMAKE_SOURCE_DIR}/_build_blend2d_${CMAKE_BUILD_TYPE}" )


find_package( SDL2 REQUIRED )

set( COMPILE_OPTS_FILE ${CMAKE_CURRENT_SOURCE_DIR}/main_compile_opts.hpp )

add_executable( main
	main.cpp

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
	src/san_ui_console.hpp
	src/san_ui_ctrl.hpp
	src/san_ui_ctrl_button.hpp
	src/san_ui_ctrl_checkbox.hpp
	src/san_ui_ctrl_text.hpp
	src/san_ui_ctrl_link.hpp

	src/san_image_view.hpp

	src/san_stack_blur_naive.hpp
	src/san_stack_blur_naive_calc.hpp

	src/san_stack_blur_simd.hpp
	src/san_stack_blur_simd_calc_common.hpp
	src/san_stack_blur_simd_calc_sse2.hpp
	src/san_stack_blur_simd_calc_sse41.hpp
	src/san_stack_blur_simd_fastest.hpp
	src/san_stack_blur_simd_fastest_2.hpp

	#san_recursive_blur_calc_rgba.hpp

	src/san_image_list.hpp

	${COMPILE_OPTS_FILE} )

target_precompile_headers( main PUBLIC
	"$<$<COMPILE_LANGUAGE:CXX>:<src/agg/agg_color_rgba.h$<ANGLE-R>>"
	"$<$<COMPILE_LANGUAGE:CXX>:<src/agg/agg_blur.h$<ANGLE-R>>"
	"$<$<COMPILE_LANGUAGE:CXX>:<src/agg/agg_array.h$<ANGLE-R>>"
	"$<$<COMPILE_LANGUAGE:CXX>:<src/agg/agg_basics.h$<ANGLE-R>>"
	"$<$<COMPILE_LANGUAGE:CXX>:<src/agg/agg_config.h$<ANGLE-R>>"
	"$<$<COMPILE_LANGUAGE:CXX>:<src/agg/agg_pixfmt_transposer.h$<ANGLE-R>>" )

set( CMAKE_CXX_FLAGS_DEBUG "-O1 ${CMAKE_CXX_FLAGS_DEBUG}" )

message( STATUS " -------------------------------------------" )
message( STATUS "       CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}" )
message( STATUS "  CMAKE_CXX_FLAGS_DEBUG: ${CMAKE_CXX_FLAGS_DEBUG}" )
message( STATUS "CMAKE_CXX_FLAGS_RELEASE: ${CMAKE_CXX_FLAGS_RELEASE}" )
message( STATUS " -------------------------------------------" )

if( MSVC )
	target_compile_options( main PRIVATE /W4 )
else()
	target_compile_options( main PRIVATE -march=native -Wall -fno-exceptions -fno-rtti ) # -msse2 -Wextra -Wpedantic
endif()


get_target_property( COMPILE_OPTS main COMPILE_OPTIONS )

if( CMAKE_BUILD_TYPE STREQUAL "Debug" )
	set( COMPILE_OPTS "${CMAKE_CXX_FLAGS_DEBUG} ${COMPILE_OPTS}" )
else()
	set( COMPILE_OPTS "${CMAKE_CXX_FLAGS_RELEASE} ${COMPILE_OPTS}" )
endif()

add_custom_command(
	OUTPUT ${COMPILE_OPTS_FILE}
	COMMAND ${CMAKE_COMMAND} -E echo "static const char * g_compiler = \"${CMAKE_CXX_COMPILER}\";" > ${COMPILE_OPTS_FILE}
	COMMAND ${CMAKE_COMMAND} -E echo "static const char * g_build_type = \"${CMAKE_BUILD_TYPE}\";" >> ${COMPILE_OPTS_FILE}
	COMMAND ${CMAKE_COMMAND} -E echo "static const char * g_compile_options = \"${COMPILE_OPTS}\";" >> ${COMPILE_OPTS_FILE}
	VERBATIM
)

add_custom_target         ( opts DEPENDS ${COMPILE_OPTS_FILE} )
add_dependencies          ( main opts )
target_link_options       ( main PRIVATE -mconsole -s ) # -s -static
target_include_directories( main PRIVATE ${CMAKE_SOURCE_DIR} )
target_include_directories( main PRIVATE ${CMAKE_SOURCE_DIR}/src )
target_link_libraries     ( main PRIVATE SDL2::SDL2 blend2d::blend2d )
set_target_properties     ( main PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR} )
set_target_properties     ( main PROPERTIES OUTPUT_NAME "${CMAKE_PROJECT_NAME}-${CMAKE_CXX_COMPILER_ID}-${CMAKE_BUILD_TYPE}" )

# Remove COMPILE_OPTS_FILE...
add_custom_command( TARGET main PRE_BUILD
	COMMAND ${CMAKE_COMMAND} -E remove ${COMPILE_OPTS_FILE}
)
