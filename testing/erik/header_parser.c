#include <stdio.h>

int main() {
  FILE * file = fopen("test", "r");
  unsigned char magic[4];
  fread((void *)magic, 1, 4, file);

  printf("%x %x %x %x \n", magic[0], magic[1], magic[2], magic[3]);
}