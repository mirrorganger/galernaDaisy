# This assumes libdaisy and daisy exists both as submodules in the top directory.

set(LIBDAISY_DIR ${CMAKE_SOURCE_DIR}/libDaisy)
set(DAISYSP_DIR  ${CMAKE_SOURCE_DIR}/DaisySP)

set(FLASH_ADDRESS 0x08000000)

set(DAISYSP_LIB DaisySP)
set(LINKER_SCRIPT ${LIBDAISY_DIR}/core/${LINKER_SCRIPT_FILE})

set(OCD_DIR /usr/local/share/openocd/scripts)
set(PGM_DEVICE interface/stlink.cfg)
set(CHIPSET stm32h7x)

FUNCTION(find_files_matching_patterns output directory filter_masks)
	set( file_list )	
	foreach( filter_mask ${filter_masks} )
		file(GLOB_RECURSE found_files ${directory}/${filter_mask})
		list(APPEND file_list ${found_files})
		endforeach()
	set(${output} ${file_list} PARENT_SCOPE)
ENDFUNCTION()

FUNCTION(__add_dependencies TARGET TARGET_DEPENDS TARGET_DEPENDS_NO_TARGET)
	foreach( dependency ${TARGET_DEPENDS} ) 
		if (NOT TARGET ${dependency})
			message(FATAL_ERROR "The dependency target \"${dependency}\" of target \"${TARGET}\" does not exist, please check and reorder the add_subdirectory() base on dependency")
		endif() 
		target_link_libraries(${TARGET} PUBLIC ${dependency})
	endforeach()
	foreach( dependency ${TARGET_DEPENDS_NO_TARGET} ) 
		target_link_libraries(${TARGET} PUBLIC ${dependency})
	endforeach()
ENDFUNCTION()

FUNCTION(add_daisy_library)
	set(options UNIT_TEST)
	set(oneValueArgs NAME PATH COMPONENT)
	set(multiValueArgs DEPENDS EXTERNAL_DEPENDS)
	
	cmake_parse_arguments(LIBRARY "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

	if (NOT "${LIBRARY_PATH}" STREQUAL "")
		set(LIBRARY_PATH "${LIBRARY_PATH}/")
	endif()

	set(HEADER_FILES_FILTER_MASK *.h *.hpp)
	set(SOURCE_FILES_FILTER_MASK *.cpp *.cc)

	find_files_matching_patterns(publicIncludes ${LIBRARY_PATH}include "${HEADER_FILES_FILTER_MASK}")
	find_files_matching_patterns(privateIncludes ${LIBRARY_PATH}src "${HEADER_FILES_FILTER_MASK}")
	find_files_matching_patterns(src ${LIBRARY_PATH}src "${SOURCE_FILES_FILTER_MASK}")
	
	add_library(${LIBRARY_NAME} ${publicIncludes} ${src} ${privateIncludes})

    target_link_libraries(${LIBRARY_NAME}
        PRIVATE
        daisy
        ${DAISYSP_LIB}
        c
        m
        nosys
    )

	if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${LIBRARY_PATH}include")
		message(STATUS "${LIBRARY_PATH}")
		target_include_directories(${LIBRARY_NAME} PUBLIC ${LIBRARY_PATH}include)
	endif()
	if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${LIBRARY_PATH}src")
		target_include_directories(${LIBRARY_NAME} PRIVATE ${LIBRARY_PATH}src)
	endif()

	__add_dependencies(${LIBRARY_NAME} "${LIBRARY_DEPENDS}" "${LIBRARY_EXTERNAL_DEPENDS}")
	# add_dependencies(ALL_COMPILE ${LIBRARY_NAME})
ENDFUNCTION()



FUNCTION(add_daisy_firmware)
	set(options)
	set(oneValueArgs NAME)
	set(multiValueArgs DEPENDS EXTERNAL_DEPENDS)
	cmake_parse_arguments(FIRMWARE "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
	
	set(SOURCE_FILES_FILTER_MASK *.cpp *.cc *.h *.hpp)

	find_files_matching_patterns(FIRMWARE_SOURCES src "${SOURCE_FILES_FILTER_MASK}")

	add_executable(${FIRMWARE_NAME} "${FIRMWARE_SOURCES}")
	set_property(TARGET ${EXECUTABLE_NAME} PROPERTY FOLDER ${EXECUTABLE_NAME})
	
    target_link_libraries(${FIRMWARE_NAME}
        PRIVATE
        daisy
        ${DAISYSP_LIB}
        c
        m
        nosys
    )
	__add_dependencies(${FIRMWARE_NAME} "${FIRMWARE_DEPENDS}" "${FIRMWARE_EXTERNAL_DEPENDS}")

    set_target_properties(${FIRMWARE_NAME} PROPERTIES
        CXX_STANDARD 14
        CXX_STANDARD_REQUIRED YES
        SUFFIX ".elf"
    )

    target_link_options(${FIRMWARE_NAME} PUBLIC
        -T ${LINKER_SCRIPT}
        -Wl,-Map=${FIRMWARE_NAME}.map,--cref
        -Wl,--check-sections
        -Wl,--unresolved-symbols=report-all
        -Wl,--warn-common
        -Wl,--warn-section-align
        -Wl,--print-memory-usage
    )

	install(TARGETS ${FIRMWARE_NAME} DESTINATION ${CMAKE_SOURCE_DIR}/products)

    add_custom_command(TARGET ${FIRMWARE_NAME} POST_BUILD
        COMMAND ${CMAKE_OBJCOPY}
        ARGS -O ihex
        -S ${FIRMWARE_NAME}.elf
        ${FIRMWARE_NAME}.hex
        BYPRODUCTS
        ${FIRMWARE_NAME}.hex
        COMMENT "Generating HEX image"
        VERBATIM)

    add_custom_command(TARGET ${FIRMWARE_NAME} POST_BUILD
        COMMAND ${CMAKE_OBJCOPY}
        ARGS -O binary
        -S ${FIRMWARE_NAME}.elf
        ${FIRMWARE_NAME}.bin
        BYPRODUCTS
        ${FIRMWARE_NAME}.bin
        COMMENT "Generating binary image"
    VERBATIM)

    add_custom_command(
        TARGET ${FIRMWARE_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND}
        ARGS -E copy ${FIRMWARE_NAME}.bin ${FIRMWARE_NAME}.elf ${CMAKE_SOURCE_DIR}/products
        COMMENT "Copying binary to products folder..."
    )

    add_custom_target(upload-dfu-${FIRMWARE_NAME} DEPENDS ${FIRMWARE_NAME}
        COMMAND dfu-util -a 0 -s ${FLASH_ADDRESS}:leave -D ${FIRMWARE_NAME}.bin -d ,0483:df11
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/products
        COMMENT "Uploading to board...")

    add_custom_target(upload-openocd-${FIRMWARE_NAME} DEPENDS ${FIRMWARE_NAME}
        COMMAND openocd -s "${OCD_DIR}" -f "${PGM_DEVICE}" -f "target/${CHIPSET}.cfg" -c "program ${FIRMWARE_NAME}.elf verify reset exit"
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/products
        COMMENT "Uploading to board...")

ENDFUNCTION()


# $(OCD) -s $(OCD_DIR) $(OCDFLAGS) \
# -c "program ./build/$(TARGET).elf verify reset exit"



# OCD=openocd
# OCD_DIR ?= /usr/local/share/openocd/scripts
# PGM_DEVICE ?= interface/stlink.cfg
# OCDFLAGS = -f $(PGM_DEVICE) -f target/$(CHIPSET).cfg