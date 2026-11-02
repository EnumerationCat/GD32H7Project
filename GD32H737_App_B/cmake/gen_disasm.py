#!/usr/bin/env python3
"""生成便于阅读的 .s 反汇编清单：向量表 + 反汇编，地址统一为 0x%08x。"""
import argparse
import re
import subprocess


def run(cmd):
    return subprocess.run(cmd, capture_output=True, check=True).stdout


def decode(data):
    for enc in ("utf-8", "gbk"):
        try:
            return data.decode(enc)
        except UnicodeDecodeError:
            pass
    return data.decode("utf-8", errors="replace")


def parse_symtab(nm, elf):
    names = {}
    for line in decode(run([nm, "-n", elf])).splitlines():
        m = re.match(r"^([0-9a-fA-F]+)\s+([A-Za-z])\s+(.+)$", line)
        if m:
            names.setdefault(int(m.group(1), 16), []).append(m.group(3).strip())
    return names


def pick_name(names, addr):
    candidates = names[addr]
    return "Default_Handler" if "Default_Handler" in candidates else candidates[0]


def vector_block(objdump, nm, elf):
    text = decode(run([objdump, "-s", "-j", ".vectors", elf]))
    start, words = None, []
    for line in text.splitlines():
        m = re.match(r"^\s*([0-9a-fA-F]+)\s+(.*)$", line)
        if not m:
            continue
        data = re.split(r"\s{2,}", m.group(2))[0]
        hexwords = re.findall(r"(?<![0-9a-fA-F])[0-9a-fA-F]{8}(?![0-9a-fA-F])", data)
        if not hexwords:
            continue
        if start is None:
            start = int(m.group(1), 16)
        words.extend(int.from_bytes(bytes.fromhex(h), "little") for h in hexwords)

    if start is None:
        return []

    names = parse_symtab(nm, elf)
    lines = [
        "@ ==================== Vector Table ====================",
        f"@ Section .vectors @ 0x{start:08x}, {len(words) * 4} bytes ({len(words)} entries)",
        "",
    ]
    for i, w in enumerate(words):
        if i == 0:
            label = "initial SP"
        elif w == 0:
            label = "reserved"
        else:
            target = w & ~1
            label = pick_name(names, target) if target in names else ""
        lines.append(f"0x{start + i * 4:08x}:  {w:08x}  .word  0x{w:08x}  @ [{i}] {label}".rstrip())
    lines.append("")
    return lines


def format_disassembly(text):
    out = []
    for line in text.splitlines():
        line = re.sub(r"^\s*([0-9a-fA-F]+):(?=\t)",
                      lambda m: f"   0x{int(m.group(1), 16):08x}:", line)
        line = re.sub(r"^([0-9a-fA-F]{8,16}) <",
                      lambda m: f"0x{int(m.group(1), 16):08x} <", line)
        line = re.sub(r"(?<=[\s(])([0-9a-fA-F]{7,8}) <",
                      lambda m: f"0x{int(m.group(1), 16):08x} <", line)
        out.append(line)
    return out


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--objdump", required=True)
    parser.add_argument("--nm", required=True)
    parser.add_argument("--elf", required=True)
    parser.add_argument("--out", required=True)
    args = parser.parse_args()

    lines = vector_block(args.objdump, args.nm, args.elf)
    lines += format_disassembly(decode(run([args.objdump, "-d", "-S", args.elf])))

    with open(args.out, "w", encoding="utf-8", newline="\n") as f:
        f.write("\n".join(lines) + "\n")


if __name__ == "__main__":
    main()
