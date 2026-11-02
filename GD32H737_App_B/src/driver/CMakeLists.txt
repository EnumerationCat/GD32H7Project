foreach(t ${FIRMWARE_TARGETS})
    target_compile_definitions(${t} PRIVATE GD32H7XX GD32H7XXV GD32H737)

    target_include_directories(${t} PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/public"
    )

    target_sources(${t} PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/sdk_init.c"
        "${CMAKE_CURRENT_LIST_DIR}/startup_gd32h737vmt6.c"
        "${CMAKE_CURRENT_LIST_DIR}/system_gd32h7xx.c"
    )
endforeach()
