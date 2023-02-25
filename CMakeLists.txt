cmake_minimum_required(VERSION 3.13)

project(maxmod
	LANGUAGES C ASM
)

include(GNUInstallDirs)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
	set(CMAKE_INSTALL_PREFIX "${NDS_ROOT}" CACHE PATH "" FORCE)
endif()

if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "armv4t")
	set(PLATFORM_SUFFIX "7")
	set(ARM7 TRUE)
elseif("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "armv5te")
	set(PLATFORM_SUFFIX "9")
	set(ARM9 TRUE)
else()
	message(FATAL_ERROR "Invalid processor, must be armv4t or armv5te")
endif()

# Define static library target
add_library(mm STATIC)
set_target_properties(mm PROPERTIES RELEASE_POSTFIX    "${PLATFORM_SUFFIX}")
set_target_properties(mm PROPERTIES MINSIZEREL_POSTFIX "${PLATFORM_SUFFIX}")

# Add compiler flags
target_compile_options(mm PRIVATE
	-Wall -Werror
)

target_compile_definitions(mm PRIVATE
	SYS_CALICO
	SYS_NDS
)

# Add include directories
target_include_directories(mm PRIVATE asm_include include)

if(ARM7)
	target_compile_definitions(mm PRIVATE SYS_NDS7)
	target_sources(mm PRIVATE
		source/mm_main.s
		source/mm_effect.s
		source/mm_mas.s
		source/mm_mas_arm.s

		source_ds7/mm_main7.s
		source_ds7/mm_mixer_super.s

		source_calico/mm_comms7.c
	)

	# Install the headers
	install(
		DIRECTORY ${PROJECT_SOURCE_DIR}/include/
		DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
		FILES_MATCHING
			PATTERN maxmod7.h
			PATTERN mm_types.h
	)
endif()

# Install the library
install(
	TARGETS mm
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
