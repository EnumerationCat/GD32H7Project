#!/usr/bin/env python3
"""给 App 裸 bin 加 U-Boot uImage 头，入口点由 ELF 中的 Reset_Handler 推导。

uImage 头固定 64 字节；数据从 load 地址开始，头尾之间用 0xFF 填充。
Cortex-M7 要求向量表按"异常数向上取整到 2 的幂"对齐（186 项 -> 1024 字节），
所以 App 的 load 地址取 0x08010400，而不是紧接头部的 0x08010040/0x08010080。
"""

import argparse
import os
import subprocess
import tempfile

IH_HDR_SIZE = 64


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
    parser.add_argument("--base", required=True, help="镜像烧录起始地址")
    parser.add_argument("--load", required=True, help="uImage 加载地址（App 链接地址）")
    parser.add_argument("--name", default="GD32H737-APP")
    parser.add_argument("--out", required=True)
    args = parser.parse_args()

    base = int(args.base, 0)
    load = int(args.load, 0)
    pad = load - base - IH_HDR_SIZE
    if pad < 0:
        raise SystemExit(f"load 0x{load:08x} overlaps the uImage header")

    entry = find_symbol(args.nm, args.elf, "Reset_Handler") | 1

    with open(args.bin, "rb") as f:
        data = b"\xff" * pad + f.read()

    fd, payload = tempfile.mkstemp(suffix=".bin")
    try:
        with os.fdopen(fd, "wb") as f:
            f.write(data)
        subprocess.check_call([
            args.mkimage,
            "-A", "arm",
            "-O", "linux",
            "-T", "kernel",
            "-C", "none",
            "-a", f"0x{load:08x}",
            "-e", f"0x{entry:08x}",
            "-n", args.name,
            "-d", payload,
            args.out,
        ])
    finally:
        os.unlink(payload)


if __name__ == "__main__":
    main()
