/* Memory Map for GD32H737VMT6 */
ENTRY(Reset_Handler)

__STACK_SIZE = 0x4000;
__HEAP_SIZE  = 0x10000;

MEMORY
{
    /* 前 64K 保留给 bootloader，App 从 0x08010000 开始 */
    FLASH       (rx)    : ORIGIN = 0x08010000, LENGTH = 3776K
    /* 共享 RAM 512K 由选项字节全部分配为 256K ITCM + 256K DTCM，无 AXI 共享段 */
    AXISRAM     (xrw)   : ORIGIN = 0x24000000, LENGTH = 512K
    ITCMRAM     (xrw)   : ORIGIN = 0x00000000, LENGTH = 256K
    DTCMRAM     (xrw)   : ORIGIN = 0x20000000, LENGTH = 256K
    SRAM0       (xrw)   : ORIGIN = 0x30000000, LENGTH = 16K
    SRAM1       (xrw)   : ORIGIN = 0x30004000, LENGTH = 16K
    BACKUPRAM   (xrw)   : ORIGIN = 0x38800000, LENGTH = 4K
}

SECTIONS
{
    .vectors :
    {
        . = ALIGN(4);
        KEEP(*(.vectors))
        . = ALIGN(4);
    } >FLASH

    .text :
    {
        . = ALIGN(4);
        *(.text)
        *(.text*)
        *(.glue_7)
        *(.glue_7t)
        *(.eh_frame)
        KEEP (*(.init))
        KEEP (*(.fini))
        . = ALIGN(4);
        _etext = .;
    } >FLASH

    .rodata :
    {
        . = ALIGN(4);
        *(.rodata)
        *(.rodata*)
        . = ALIGN(4);
    } >FLASH

    .ARM.extab : { . = ALIGN(4); *(.ARM.extab* .gnu.linkonce.armextab.*) . = ALIGN(4); } >FLASH
    .ARM : { . = ALIGN(4); __exidx_start = .; *(.ARM.exidx*) __exidx_end = .; . = ALIGN(4); } >FLASH

    .preinit_array : { PROVIDE_HIDDEN (__preinit_array_start = .); KEEP (*(.preinit_array*)); PROVIDE_HIDDEN (__preinit_array_end = .); } >FLASH
    .init_array : { PROVIDE_HIDDEN (__init_array_start = .); KEEP (*(SORT(.init_array.*))) KEEP (*(.init_array*)); PROVIDE_HIDDEN (__init_array_end = .); } >FLASH
    .fini_array : { PROVIDE_HIDDEN (__fini_array_start = .); KEEP (*(SORT(.fini_array.*))) KEEP (*(.fini_array*)); PROVIDE_HIDDEN (__fini_array_end = .); } >FLASH

    _sidata = LOADADDR(.data);
    .data :
    {
        . = ALIGN(4);
        _sdata = .;
        *(.data)
        *(.data*)
        _edata = .;
    } >AXISRAM AT> FLASH

    .bss :
    {
        _sbss = .;
        __bss_start__ = _sbss;
        *(.bss)
        *(.bss*)
        *(COMMON)
        _ebss = .;
        __bss_end__ = _ebss;
    } >AXISRAM

    /* 标准栈配置：100%兼容LD，无内存溢出 */
    .stack (NOLOAD) :
    {
        . = ALIGN(8);
        __StackLimit = .;
        . += __STACK_SIZE;
        . = ALIGN(8);
        __StackTop = .;
        PROVIDE(__stack = __StackTop);
        PROVIDE(__INITIAL_SP = __StackTop);
    } > AXISRAM

    /* 标准堆配置 */
    .heap (NOLOAD) :
    {
        . = ALIGN(8);
        PROVIDE(end = .);
        PROVIDE(_end = .);
        . += __HEAP_SIZE;
        . = ALIGN(8);
        __HeapLimit = .;
    } > AXISRAM

    .copy.table :
    {
        . = ALIGN(4);
        __copy_table_start__ = .;
        LONG(LOADADDR(.data))
        LONG(ADDR(.data))
        LONG(SIZEOF(.data) / 4)
        __copy_table_end__ = .;
    } >FLASH

    .zero.table :
    {
        . = ALIGN(4);
        __zero_table_start__ = .;
        LONG(ADDR(.bss))
        LONG(SIZEOF(.bss) / 4)
        __zero_table_end__ = .;
    } >FLASH

    .dtcm_data (NOLOAD) :     { . = ALIGN(4); *(.dtcm_data) *(.dtcm_data*) . = ALIGN(4); } >DTCMRAM
    .itcm_code :              { . = ALIGN(4); *(.itcm_code) *(.itcm_code*) . = ALIGN(4); } >ITCMRAM AT> FLASH
    .sram0_data (NOLOAD) :     { . = ALIGN(4); *(.sram0_data) *(.sram0_data*) . = ALIGN(4); } >SRAM0
    .sram1_data (NOLOAD) :     { . = ALIGN(4); *(.sram1_data) *(.sram1_data*) . = ALIGN(4); } >SRAM1
    .backup_data (NOLOAD) :    { . = ALIGN(4); *(.backup_data) *(.backup_data*) . = ALIGN(4); } >BACKUPRAM

    /DISCARD/ : { libc.a ( * ) libm.a ( * ) libgcc.a ( * ) }
    .ARM.attributes 0 : { *(.ARM.attributes) }
}