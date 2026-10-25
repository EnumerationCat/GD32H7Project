# Bootloader：链接在 0x08000000 的独立镜像，只服务 BOOT 目标
target_include_directories(BOOT PRIVATE
	"${CMAKE_CURRENT_LIST_DIR}/"
)

target_sources(BOOT PRIVATE
	"${CMAKE_CURRENT_LIST_DIR}/bootloader.c"
)
