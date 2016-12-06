#!/usr/bin/env python3

import subprocess, shlex, timeit
from signer import sign_binary

num_iters = 20
for size in range(1, int(1e8), int(1e5)):
  code = """static int array [""" + str(size) + """] = {5};
    int main() {
      return 0;
    }
  """

  with open("hashing_test_file.c", "w+") as file:
    file.write(code)

  command = "gcc -o hashing_test_file hashing_test_file.c"
  subprocess.check_call(shlex.split(command))

  kwargs = {
    'password': 'crypto'
  }
  sign_binary('hashing_test_file', 'cert.pem', 'privatekey.pem', **kwargs)

  command = "./hashing_test_file"
  time = timeit.timeit("subprocess.check_call({})".format(shlex.split(command)), number=num_iters, setup="import subprocess")
  print("{}, {}".format(size, time/num_iters))
