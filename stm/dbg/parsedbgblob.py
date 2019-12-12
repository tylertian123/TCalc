import zlib
import struct
import sys

with open(sys.argv[1], "rb") as f:
    s1 = struct.unpack("<I", f.read(4))[0]

    b1 = f.read(s1)
    b2 = f.read()

# decompress
b1 = zlib.decompress(b1)
b2 = zlib.decompress(b2)

for i in range(0, len(b2), 10):
    print("{:08x}-{:08x} = {}".format(*struct.unpack(">IIh", b2[i:i+10])))
