# GD32H737 A/B Bootloader + LVGL App

A bare-metal **dual-slot (A/B) bootloader** for the **GD32H737VMT6** (Cortex-M7), paired with a **FreeRTOS + LVGL** application. Firmware is packaged in U-Boot **uImage** format, validated by CRC32, and updated over the UART with **ZMODEM**. The bootloader shell and serial log are mirrored to the on-board LCD.

## Features

- A/B application slots with automatic fallback if the active slot is invalid
- U-Boot uImage support (64-byte header, header/data CRC32, load/entry checks)
- In-system update over UART using the `rz` ZMODEM receiver (no programmer needed)
- Interactive bootloader shell (`help`, `rz`, `boot`, `reboot`, ...)
- Boot log and progress mirrored on the LCD
- Same UART used by bootloader and app: **USART0, PA9 (TX), PA10 (RX), 115200 8-N-1**
- CMake + `arm-none-eabi-gcc` build; automatic `.bin` / `.hex` / `.map` / `.s` and `u-boot_app.img` generation

## Memory Map

| Region | Address range | Size | Purpose |
| --- | --- | --- | --- |
| Bootloader | `0x08000000` - `0x0800EFFF` | 60 KiB | Bootloader code |
| Boot config | `0x0800F000` - `0x0800FFFF` | 4 KiB | Active-slot marker (`magic`, `active_slot`, `~active_slot`) |
| App slot A | `0x08010000` - `0x081FFFFF` | 1984 KiB | uImage header at `0x08010000`, payload/vector table at `0x08010400` |
| App slot B | `0x08200000` - `0x083BFFFF` | 1792 KiB | Same layout as slot A |

- AXI SRAM `0x24000000` (512 KiB) is used for stack/heap and most data.
- TCM: ITCM `0x00000000` (256 KiB), DTCM `0x20000000` (256 KiB).
- The vector table is 1 KiB aligned; the App is linked at `0x08010400`.

## Repository Layout

```
src/app/                  App entry point (FreeRTOS tasks, LVGL UI, UART shell)
src/driver/               Shared drivers
components/Bootloader/    Bootloader, shell, flash, ZMODEM receiver
components/BSP/           Board support (LCD, UART, ...)
components/Middlewares/   FreeRTOS, LVGL, FatFs, GUI applications
third_party/              GD32H7xx SDK and CMSIS
cmake/                    Toolchain, disassembly and uImage scripts
tools/                    Bundled mkimage (Windows)
```

## Prerequisites

- `arm-none-eabi-gcc` with newlib in `PATH` (xPack GCC ARM or STM32CubeCLT)
- CMake >= 3.20 and Ninja
- Python 3
- OpenOCD with GD32H7 support and a CMSIS-DAP/DAP-Link probe
- A serial terminal with ZMODEM support (Tera Term, XShell, SecureCRT, or `lrzsz` on Linux/macOS)

## Build

```bash
cmake -GNinja -Bbuild -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE
cmake --build build
```

Artifacts under `build/`:

| File | Description |
| --- | --- |
| `bootloader.elf` / `.hex` / `.bin` | Bootloader (flash at `0x08000000`) |
| `app.elf` / `.hex` / `.bin` | Raw application (linked at `0x08010400`) |
| `u-boot_app.img` | uImage used for flashing or ZMODEM update |

> `u-boot_app.img` is the **only** image the bootloader accepts. Do not flash the raw `app.bin` to `0x08010000`.

## Flash the Board

Using OpenOCD (adjust the path and probe config to your setup):

```bash
# Bootloader
openocd -f interface/cmsis-dap.cfg -f target/gd32h7xx.cfg \
  -c "program ./build/bootloader.elf verify reset exit"

# App into slot A (uImage at 0x08010000)
openocd -f interface/cmsis-dap.cfg -f target/gd32h7xx.cfg \
  -c "reset halt" \
  -c "flash write_image erase ./build/u-boot_app.img 0x08010000 bin" \
  -c "verify_image ./build/u-boot_app.img 0x08010000 bin" \
  -c "reset run" \
  -c "exit"
```

VS Code users can run the tasks in `.vscode/tasks.json` (`Boot Flash`, `App Flash`, `Flash All (Boot + App)`). Update the hard-coded OpenOCD path there if needed.

**First run note:** the bootloader checks the TCM shared-RAM option bytes. If they are not 256 KiB ITCM / 256 KiB DTCM, it reconfigures them and performs one automatic reset. This is expected and happens only once.

## Boot Flow

1. At power-on the bootloader validates both slots (magic, CRC32, load/entry range).
2. It prints the slot status and selects the active slot (falling back to the other slot if needed).
3. If a valid image exists, it waits **5 seconds**: press any key to enter the bootloader shell, otherwise it jumps to the app.
4. If no valid image exists, it enters the shell automatically.

## Bootloader Shell

Connect to USART0 at `115200 8-N-1`, then press a key during the 5-second window. Prompt: `boot_shell >`.

| Command | Description |
| --- | --- |
| `help [command]` | List commands or show usage |
| `rz` | Receive a uImage over ZMODEM into the inactive slot, verify it, and mark it for next boot |
| `boot` / `jump` / `exit` | Jump to the active slot |
| `boot a` / `boot b` | Jump to slot A or B |
| `boot 0xADDR` | Jump to a uImage or vector table at an address inside a slot |
| `reboot` | Reset the MCU |

## Update the App over UART (ZMODEM)

1. Build the app and locate `build/u-boot_app.img`.
2. Reset the board and press a key within 5 seconds to enter the bootloader shell.
3. Type `rz`. The bootloader waits for a ZMODEM sender.
4. Start a ZMODEM upload **in binary mode**:

   ```bash
   # Linux / macOS (lrzsz)
   sz -b build/u-boot_app.img
   ```

   In Tera Term use **File > Transfer > ZMODEM > Send...**; in XShell/SecureCRT use the ZMODEM upload command.
5. The bootloader receives into its 384 KiB RAM buffer, validates the uImage header and CRC, writes the **inactive** slot, and marks it active.
6. Run `boot` (or reset) to start the new firmware. If the update fails, the previous slot is left untouched.

> Transfer size limit: 384 KiB per image (RAM receive buffer). At 115200 baud, a full transfer may take a few minutes.

## App Shell

After the bootloader jumps to the app, the LVGL UI starts and a shell is available on the same UART (prompt `app_shell>`). Supported commands: `help`, `clear`, `reboot`. The shell output is also shown on the LCD.

## Troubleshooting

| Symptom | Fix |
| --- | --- |
| `no valid image, entering shell` | No valid uImage in either slot; flash or `rz` the app |
| `invalid uImage (magic/CRC/load ...)` | Wrong file or a header-less `.bin`; use `u-boot_app.img` |
| ZMODEM transfer hangs or fails | Disable flow control, ensure binary mode (`sz -b`), and avoid sending a file larger than 384 KiB |
| Bootloader resets once on first power-up | Expected: TCM option bytes were being reconfigured to 256K/256K |
| OpenOCD task fails | Update the OpenOCD path in `.vscode/tasks.json` and check the CMSIS-DAP connection |
| LCD stays blank in bootloader | Check the LCD wiring and backlight; bootloader does not run the RTOS |

## Debugging

Open the folder in VS Code and use the `dplinkDebug` configuration (Cortex-Debug + OpenOCD, SVD file included). The `Wake DP (pyOCD)` task brings the debug port out of low-power mode before connecting.
