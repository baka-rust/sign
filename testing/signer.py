#!/usr/bin/env python3

'''
  Usage: ./signer.py <path_to_binary> <path_to_cert> <path_to_private_key> <private key password>
  If <password> is not supplied, it will be prompted for
'''

def check_if_shoud_sign(filename):
  with open(filename, "rb") as file:
    elf_magic = file.read(4)
    is_elf = ord(elf_magic[0]) == 0x7f and elf_magic[1:] == "ELF"

    file.seek(-22, 2)
    our_magic = file.read()
    is_signed = our_magic.decode('utf-8') == "~~ BINARY SIGNATURE ~~"

    return is_elf and not is_signed


import subprocess, shlex
from sys import argv

if len(argv) != 4 and len(argv) != 5:
  print("Usage: ./signer.py <path_to_binary> <path_to_cert> <path_to_private_key>")
  raise RuntimeError("Wrong number of arguments")

binary_name = argv[1]
cert_name = argv[2]
private_key_name = argv[3]

if check_if_shoud_sign(binary_name):
  if len(argv) == 5:
    command = "openssl cms -sign -signer {} -inkey {} -binary -in {} -outform der -noattr -passin pass:{}".format(cert_name, private_key_name, binary_name, argv[4])
  else:
    command = "openssl cms -sign -signer {} -inkey {} -binary -in {} -outform der -noattr".format(cert_name, private_key_name, binary_name)

  child =  subprocess.Popen(shlex.split(command), stdout=subprocess.PIPE, stdin=subprocess.PIPE)
  child.wait()

  if child.returncode != 0:
    raise RuntimeError("Signer failed")

  signature = child.communicate()[0]

  num_bytes = len(signature)

  with open(binary_name, "ab") as f:
    f.write(signature)
    f.write(num_bytes.to_bytes(4, byteorder='little'))
    f.write("~~ BINARY SIGNATURE ~~".encode("utf-8"))
