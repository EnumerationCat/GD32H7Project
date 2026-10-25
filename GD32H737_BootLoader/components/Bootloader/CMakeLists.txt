# Bootloader：链接在 0x08000000 的独立镜像，只服务 bootloader 目标
target_include_directories(bootloader PRIVATE
	"${CMAKE_CURRENT_LIST_DIR}/"
)

target_sources(bootloader PRIVATE
	"${CMAKE_CURRENT_LIST_DIR}/bootloader.c"
)
