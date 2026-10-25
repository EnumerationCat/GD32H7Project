#!/usr/bin/env python3
"""给 App 裸 bin 加 U-Boot uImage 头，入口点由 ELF 中的 Reset_Handler 推导。"""

import argparse
import subprocess


def find_symbol(nm, elf, symbol):
    out = subprocess.check_output([nm, elf], text=True)
    for line in out.splitlines():
        parts = line.split()
        if len(parts) >= 3 and parts[2] == symbol:
            return int(parts[0], 16)
    raise SystemExit(f"{symbol} not found in {elf}")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--mkimage", required=True)
    parser.add_argument("--nm", required=True)
    parser.add_argument("--elf", required=True)
    parser.add_argument("--bin", required=True)
    parser.add_argument("--load", required=True, help="uImage 加载地址（App 链接地址）")
    parser.add_argument("--name", default="GD32H737-APP")
    parser.add_argument("--out", required=True)
    args = parser.parse_args()

    load = int(args.load, 0)
    entry = find_symbol(args.nm, args.elf, "Reset_Handler") | 1

    subprocess.check_call([
        args.mkimage,
        "-A", "arm",
        "-O", "linux",
        "-T", "kernel",
        "-C", "none",
        "-a", f"0x{load:08x}",
        "-e", f"0x{entry:08x}",
        "-n", args.name,
        "-d", args.bin,
        args.out,
    ])


if __name__ == "__main__":
    main()
