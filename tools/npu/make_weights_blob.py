"""Convert stedgeai's network_atonbuf.AXISRAM5.raw into the firmware header
network_weights_blob.h (weights copied to npuRAM5 at runtime in debug builds)."""

from pathlib import Path

SRC = Path(__file__).parent / "st_ai_output" / "network_atonbuf.AXISRAM5.raw"
DST = (Path(__file__).parents[2] / "NECCS_N647_App" / "Appli" / "Core" / "Src"
       / "npu" / "network_weights_blob.h")

data = SRC.read_bytes()
lines = []
for i in range(0, len(data), 16):
    chunk = ",".join(f"0x{b:02x}" for b in data[i:i + 16])
    lines.append(f"  {chunk},")

DST.write_text(
    "/* Auto-generated from network_atonbuf.AXISRAM5.raw - int8 sound classifier weights. */\n"
    "#ifndef NETWORK_WEIGHTS_BLOB_H\n"
    "#define NETWORK_WEIGHTS_BLOB_H\n"
    "#include <stdint.h>\n"
    "#define NETWORK_WEIGHTS_BLOB_ADDR 0x342E0000UL\n"
    f"#define NETWORK_WEIGHTS_BLOB_SIZE {len(data)}U\n"
    f"static const uint8_t g_network_weights_blob[NETWORK_WEIGHTS_BLOB_SIZE] = {{\n"
    + "\n".join(lines)
    + "\n};\n#endif /* NETWORK_WEIGHTS_BLOB_H */\n",
    newline="\n",
)
print(f"wrote {DST} ({len(data)} bytes)")
