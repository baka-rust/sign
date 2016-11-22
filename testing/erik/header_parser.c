<<<<<<< HEAD
#include <elf.h>
=======
#include <linux/elf.h>
>>>>>>> 770b4655df148d9c3a6e303fccc75272f9e51cf8
#include <stdio.h>

int main() {
  FILE *file = fopen("test", "r");
<<<<<<< HEAD
  elfhdr hdr;
  fread(&hdr, 1, sizeof(elfhdr), file);
=======
  struct elf64_hdr hdr;
  fread(&hdr, 1, sizeof(struct elf64_hdr), file);
>>>>>>> 770b4655df148d9c3a6e303fccc75272f9e51cf8

  printf("%x %x %x %x\n", hdr.e_ident[EI_MAG0], hdr.e_ident[EI_MAG1],
         hdr.e_ident[EI_MAG2], hdr.e_ident[EI_MAG3]);

<<<<<<< HEAD
  printf("Entry point: %lx\n", hdr.e_entry);

  elf_phdr phdr;
  fseek(file, hdr.e_phoff, SEEK_CUR);
  fread(&phdr, 1, sizeof(elf_phdr), file);
=======
  printf("Entry point: %lx\n", hdr.e_phoff);

  struct elf64_phdr phdr;
  fseek(file, hdr.e_phoff, SEEK_CUR);
  fread(&phdr, 1, sizeof(struct elf64_phdr), file);
>>>>>>> 770b4655df148d9c3a6e303fccc75272f9e51cf8

  printf("%lx\n", phdr.p_offset);
}