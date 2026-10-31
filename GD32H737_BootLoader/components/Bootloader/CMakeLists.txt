# Bootloader：链接在 0x08000000 的独立镜像，只服务 bootloader 目标
target_include_directories(bootloader PRIVATE
	"${CMAKE_CURRENT_LIST_DIR}/"
	"${CMAKE_CURRENT_LIST_DIR}/Shell"
)

target_sources(bootloader PRIVATE
	"${CMAKE_CURRENT_LIST_DIR}/bootloader.c"
	"${CMAKE_CURRENT_LIST_DIR}/Shell/shell.c"
	"${CMAKE_CURRENT_LIST_DIR}/Shell/command.c"
	"${CMAKE_CURRENT_LIST_DIR}/Shell/help_cmd.c"

)
