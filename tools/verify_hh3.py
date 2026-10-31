"""Inspect the finished ClassPad ELF without executing it or needing packages."""
from pathlib import Path
import hashlib
import json
import struct
import sys


REGIONS = {
    "ram": (0x8C052800, 337920),
    "eram": (0x8CFF0000, 0xEF00),
    "vbr": (0x8CFFEF00, 0x1100),
    "ilram": (0xE5200000, 4096),
    "xyram": (0xE500E000, 16384),
}


def require(condition, message):
    if not condition:
        raise ValueError(message)


def inspect(path):
    data = path.read_bytes()
    require(len(data) >= 52, "Truncated ELF header")
    header = struct.unpack_from(">16sHHIIIIIHHHHHH", data)
    ident, kind, machine, version, entry, phoff, shoff, flags = header[:8]
    ehsize, phsize, phcount, shsize, shcount, string_index = header[8:]
    require(ident[:7] == b"\x7fELF\x01\x02\x01", "Expected ELF32, big endian, version 1")
    require(kind == 2 and machine == 42 and version == 1,
            "Expected a static SuperH executable")
    require(ehsize == 52 and phsize == 32 and phcount > 0,
            "Invalid ELF/program header sizes")
    require(phoff + phcount * phsize <= len(data), "Truncated program headers")
    loads = []
    for index in range(phcount):
        fields = struct.unpack_from(">IIIIIIII", data, phoff + index * phsize)
        ptype, offset, address, physical, filesz, memsz, permissions, alignment = fields
        require(ptype not in (2, 3), "Dynamic loading or an interpreter is not supported")
        if ptype != 1 or not memsz:
            continue
        require(filesz <= memsz, "A segment's file data exceeds its memory size")
        require(not filesz or offset + filesz <= len(data), "Truncated load segment")
        require(alignment <= 1 or (alignment & (alignment - 1) == 0 and
                offset % alignment == address % alignment), "Invalid segment alignment")
        region = next((name for name, (start, size) in REGIONS.items()
                       if start <= address and address + memsz <= start + size), None)
        require(region is not None, "Load segment outside known ClassPad gint regions")
        for previous in loads:
            require(address + memsz <= previous["address"] or
                    address >= previous["address"] + previous["memory_bytes"],
                    "Overlapping load segments")
        loads.append({"region": region, "address": address, "physical_address": physical,
                      "memory_bytes": memsz, "file_bytes": filesz,
                      "executable": bool(permissions & 1)})
    require(any(segment["executable"] and segment["address"] <= entry <
                segment["address"] + segment["memory_bytes"] for segment in loads),
            "Entry point is outside an executable load segment")

    require(shsize == 40 and shcount and string_index < shcount,
            "Section headers required for metadata inspection")
    require(shoff + shcount * shsize <= len(data), "Truncated section headers")
    sections = [struct.unpack_from(">IIIIIIIIII", data, shoff + i * shsize)
                for i in range(shcount)]
    strings_section = sections[string_index]
    string_start, string_size = strings_section[4:6]
    require(string_start + string_size <= len(data), "Truncated section names")
    strings = data[string_start:string_start + string_size]
    metadata = {}
    for section in sections:
        name_offset = section[0]
        require(name_offset < len(strings), "Invalid section name")
        name = strings[name_offset:].split(b"\0", 1)[0]
        if name != b".note.hollyhock":
            continue
        start, size = section[4:6]
        require(start + size <= len(data), "Truncated Hollyhock metadata")
        notes = data[start:start + size]
        cursor = 0
        while cursor + 12 <= len(notes):
            namesz, descsz, _ = struct.unpack_from(">III", notes, cursor)
            cursor += 12
            if namesz == descsz == 0:
                break
            name_end = cursor + namesz
            desc_start = (name_end + 3) & ~3
            desc_end = desc_start + descsz
            require(desc_end <= len(notes), "Truncated Hollyhock note")
            key = notes[cursor:name_end].rstrip(b"\0").decode("utf-8")
            value = notes[desc_start:desc_end].rstrip(b"\0").decode("utf-8")
            metadata[key] = value
            cursor = (desc_end + 3) & ~3
    for key in ("NAME", "DESCRIPTION", "AUTHOR", "VERSION"):
        require(metadata.get(key), "Missing Hollyhock metadata: " + key)
    require(metadata["NAME"] in ("Minimine", "Casio Sandbox"), "Unexpected application name")

    regions = {}
    for name, (start, capacity) in REGIONS.items():
        end = max([segment["address"] + segment["memory_bytes"]
                   for segment in loads if segment["region"] == name] + [start])
        regions[name] = {"start": hex(start), "used_bytes": end - start,
                         "capacity_bytes": capacity, "free_bytes": capacity - (end - start)}
    root = Path(__file__).resolve().parent.parent
    source_hashes = {}
    for source in sorted((root / "src").glob("*")):
        if source.is_file():
            source_hashes[source.relative_to(root).as_posix()] = hashlib.sha256(
                source.read_bytes()).hexdigest()
    source_hashes["CMakeLists.txt"] = hashlib.sha256((root / "CMakeLists.txt").read_bytes()).hexdigest()
    pin = root / "tools/toolchain-image.txt"
    return {"artifact": path.name, "file_bytes": len(data),
            "sha256": hashlib.sha256(data).hexdigest(), "format": "ELF32 big-endian SuperH EXEC",
            "entry_point": hex(entry), "machine_flags": hex(flags),
            "metadata": metadata, "regions": regions, "load_segments": loads,
            "toolchain_image": pin.read_text().strip(), "source_sha256": source_hashes,
            "hardware_tested": False}


if __name__ == "__main__":
    artifact = Path(sys.argv[1] if len(sys.argv) > 1 else "build-cp/CasioSandbox.hh3")
    try:
        report = inspect(artifact)
    except (OSError, ValueError, struct.error) as error:
        print("HH3 verification failed:", error, file=sys.stderr)
        sys.exit(1)
    output = artifact.with_name("build-info.json")
    output.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print("HH3 verification passed:", artifact)
    print("File:", report["file_bytes"], "bytes; SHA-256:", report["sha256"])
    for region, usage in report["regions"].items():
        if usage["used_bytes"]:
            print(region + ":", usage["used_bytes"], "/", usage["capacity_bytes"], "bytes")
    print("Report:", output)
