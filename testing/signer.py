#!/usr/bin/env python3

'''
  Usage: ./signer.py <path_to_binary> <path_to_cert> <path_to_private_key> <private key password>
  If <password> is not supplied, it will be prompted for
'''

import subprocess, shlex

def check_if_shoud_sign(filename):
  with open(filename, "rb") as file:
    elf_magic = file.read(4)
    is_elf = elf_magic == b'\x7fELF'

    if is_elf:
      file.seek(-22, 2)
      our_magic = file.read()
      try:
        is_signed = our_magic.decode('utf-8') == "~~ BINARY SIGNATURE ~~"

        return not is_signed
      except:
        return True
    else:
      return False


def sign_binary(binary_name, cert_name, private_key_name, **kwargs):

  if check_if_shoud_sign(binary_name):
    if 'password' in kwargs:
      command = "openssl cms -sign -signer {} -inkey {} -binary -in {} -outform der -noattr -passin pass:{}".format(cert_name, private_key_name, binary_name, kwargs['password'])
    else:
      command = "openssl cms -sign -signer {} -inkey {} -binary -in {} -outform der -noattr".format(cert_name, private_key_name, binary_name)

    child =  subprocess.Popen(shlex.split(command), stdout=subprocess.PIPE)
    child.wait()

    if child.returncode != 0:
      raise RuntimeError("openssl failed to sign")

    signature = child.communicate()[0]

    num_bytes = len(signature)

    with open(binary_name, "ab") as f:
      f.write(signature)
      f.write(num_bytes.to_bytes(4, byteorder='little'))
      f.write("~~ BINARY SIGNATURE ~~".encode("utf-8"))

    return True

  return False

if __name__ == '__main__':
  from sys import argv

  if len(argv) != 4 and len(argv) != 5:
    print("Usage: ./signer.py <path_to_binary> <path_to_cert> <path_to_private_key> <private_key_password>")
    print("\tIf the private key password is not supplied, the program will prompt the user to enter it")
    raise RuntimeError("Wrong number of arguments")

  if len(argv) == 4:
    signed =  sign_binary(argv[1], argv[2], argv[3])
  else:
    kwargs = {
      'password': argv[4]
    }
    signed = sign_binary(argv[1], argv[2], argv[3], **kwargs)

  if signed:
    print("Signed {}".format(argv[1]))