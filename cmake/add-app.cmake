set(ADD_APP_ASSET_DIR ${CMAKE_CURRENT_LIST_DIR})

function(add_app)
    # Argument parsing

    cmake_parse_arguments(
        ARG
        ""
        "APP_NAME;DEVICE_NAME"
        "SOURCES"
        ${ARGN}
    )

    if(NOT ARG_APP_NAME)
        message(FATAL_ERROR "You must provide an app name")
    endif(NOT ARG_APP_NAME)

    if(NOT ARG_DEVICE_NAME)
        message(FATAL_ERROR "You must provide a device name")
    endif(NOT ARG_DEVICE_NAME)

    # Actual work

    add_executable(${ARG_APP_NAME}
        ${ARG_SOURCES}
        )

    target_link_libraries(${ARG_APP_NAME} ${ARG_DEVICE_NAME}Lib)

    target_compile_options(${ARG_APP_NAME}
        PRIVATE
            -nostdlib
            -ffreestanding
        )

    target_link_options(${ARG_APP_NAME}
        PRIVATE
            -T ${CMAKE_CURRENT_SOURCE_DIR}/../../../common/LinkerScript.lds
            -nostdlib
            -ffreestanding
            -Wl,--whole-archive
        )

    add_custom_command(
        OUTPUT ${ARG_APP_NAME}.bin
        COMMAND ${CMAKE_OBJCOPY} --output-format=binary
            $<TARGET_FILE:${ARG_APP_NAME}> ${ARG_APP_NAME}.bin
        DEPENDS ${ARG_APP_NAME}
        COMMENT "Generating ${ARG_APP_NAME}.bin"
        )

    if(${ARG_DEVICE_NAME} STREQUAL "STM32F446")
        configure_file(${ADD_APP_ASSET_DIR}/Flash.jlink.in Flash.jlink)

        add_custom_target(flash_${ARG_APP_NAME}
            COMMAND JLinkExe -device STM32F446RE -if JTAG -speed 4000 -jtagconf
                -1,-1 -autoconnect 1
                -CommanderScript Flash.jlink
            DEPENDS ${ARG_APP_NAME}.bin
            )
    else()
        message(FATAL_ERROR "Unknown device ${ARG_DEVICE_NAME}")
    endif()
endfunction()
