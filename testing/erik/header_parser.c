#include <linux/elf.h>
#include <stdio.h>

int main() {
  FILE *file = fopen("test", "r");
  struct elf64_hdr hdr;
  fread(&hdr, 1, sizeof(struct elf64_hdr), file);

  printf("%x %x %x %x\n", hdr.e_ident[EI_MAG0], hdr.e_ident[EI_MAG1],
         hdr.e_ident[EI_MAG2], hdr.e_ident[EI_MAG3]);

  printf("Entry point: %lx\n", hdr.e_phoff);

  struct elf64_phdr phdr;
  fseek(file, hdr.e_phoff, SEEK_CUR);
  fread(&phdr, 1, sizeof(struct elf64_phdr), file);

  printf("%lx\n", phdr.p_offset);
}