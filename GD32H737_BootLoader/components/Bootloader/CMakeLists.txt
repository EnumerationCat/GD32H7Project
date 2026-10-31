# Bootloader：链接在 0x08000000 的独立镜像，只服务 bootloader 目标
target_include_directories(bootloader PRIVATE
	"${CMAKE_CURRENT_LIST_DIR}/"
	"${CMAKE_CURRENT_LIST_DIR}/Shell"
	"${CMAKE_CURRENT_LIST_DIR}/Zmodern"
	"${CMAKE_CURRENT_LIST_DIR}/Zmodern/rz"
	"${CMAKE_CURRENT_LIST_DIR}/Zmodern/file"
)

target_sources(bootloader PRIVATE
	"${CMAKE_CURRENT_LIST_DIR}/bootloader.c"
	"${CMAKE_CURRENT_LIST_DIR}/ring_buffer.c"
	"${CMAKE_CURRENT_LIST_DIR}/Shell/shell.c"
	"${CMAKE_CURRENT_LIST_DIR}/Shell/command.c"
	"${CMAKE_CURRENT_LIST_DIR}/Shell/help_cmd.c"
	"${CMAKE_CURRENT_LIST_DIR}/Shell/boot_cmd.c"
	"${CMAKE_CURRENT_LIST_DIR}/Shell/parser.c"
	"${CMAKE_CURRENT_LIST_DIR}/Zmodern/zport.c"
	"${CMAKE_CURRENT_LIST_DIR}/Zmodern/rz/lrz.c"
	"${CMAKE_CURRENT_LIST_DIR}/Zmodern/rz/zm.c"
	"${CMAKE_CURRENT_LIST_DIR}/Zmodern/rz/zreadline.c"
	"${CMAKE_CURRENT_LIST_DIR}/Zmodern/rz/rbsb.c"
	"${CMAKE_CURRENT_LIST_DIR}/Zmodern/rz/crctab.c"
	"${CMAKE_CURRENT_LIST_DIR}/Zmodern/rz/mrz.c"
	"${CMAKE_CURRENT_LIST_DIR}/Zmodern/file/file.c"
)
