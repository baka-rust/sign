#!/usr/bin/env python3

'''
  Usage: ./signer.py <path_to_binary> <path_to_cert> <path_to_private_key>
'''

import subprocess, shlex
from sys import argv

if len(argv) != 4:
  print("Usage: ./signer.py <path_to_binary> <path_to_cert> <path_to_private_key>")
  raise RuntimeError("Wrong number of arguments")

binary_name = argv[1]
cert_name = argv[2]
private_key_name = argv[3]

command = "openssl cms -sign -signer {} -inkey {} -binary -in {} -outform der -noattr".format(cert_name, private_key_name, binary_name)
signature = subprocess.Popen(shlex.split(command), stdout=subprocess.PIPE).communicate()[0]

num_bytes = len(signature)

if num_bytes == 0:
  raise RuntimeError("Signing failed")

with open(binary_name, "ab") as f:
  f.write(signature)
  f.write(num_bytes.to_bytes(4, byteorder='little'))
  f.write("~~ BINARY SIGNATURE ~~".encode("utf-8"))