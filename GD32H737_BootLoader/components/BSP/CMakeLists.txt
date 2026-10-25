target_compile_definitions(${PROJECT_NAME} PRIVATE
    
)

target_include_directories(${PROJECT_NAME} PRIVATE
	"${CMAKE_CURRENT_LIST_DIR}/sys"
	"${CMAKE_CURRENT_LIST_DIR}/mpu"	
	"${CMAKE_CURRENT_LIST_DIR}/watchdog"
	"${CMAKE_CURRENT_LIST_DIR}/led"
	"${CMAKE_CURRENT_LIST_DIR}/usart"
	"${CMAKE_CURRENT_LIST_DIR}/timer"

	"${CMAKE_CURRENT_LIST_DIR}/qspi"
	"${CMAKE_CURRENT_LIST_DIR}/norflash"


	"${CMAKE_CURRENT_LIST_DIR}/lcd"

	



    
)


target_sources(${PROJECT_NAME} PRIVATE
	"${CMAKE_CURRENT_LIST_DIR}/sys/sys.c"
	"${CMAKE_CURRENT_LIST_DIR}/mpu/mpu.c"
	"${CMAKE_CURRENT_LIST_DIR}/watchdog/iwdg.c"
	"${CMAKE_CURRENT_LIST_DIR}/watchdog/wwdg.c"
	"${CMAKE_CURRENT_LIST_DIR}/led/led.c"
	"${CMAKE_CURRENT_LIST_DIR}/usart/usart.c"
	"${CMAKE_CURRENT_LIST_DIR}/timer/timer.c"
	"${CMAKE_CURRENT_LIST_DIR}/lcd/lcd.c"
	"${CMAKE_CURRENT_LIST_DIR}/lcd/lcd_ex.c"
	"${CMAKE_CURRENT_LIST_DIR}/qspi/qspi.c"
	"${CMAKE_CURRENT_LIST_DIR}/norflash/norflash.c"
	"${CMAKE_CURRENT_LIST_DIR}/lcd/ctiic.c"
	"${CMAKE_CURRENT_LIST_DIR}/lcd/gt9xxx.c"
	"${CMAKE_CURRENT_LIST_DIR}/lcd/touch.c"


 
)