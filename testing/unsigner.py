#!/usr/bin/env python3
from sys import argv

for file in argv[1:]:
  with open(file, "rb+") as file:
    file.seek(-22, 2)

    magic = file.read()
    if magic.decode('utf-8') == "~~ BINARY SIGNATURE ~~":
      file.seek(-26, 2)
      size = int.from_bytes(file.read(4), byteorder="little", signed=False)

      file.seek(-(size + 4), 1)
      file.truncate()
