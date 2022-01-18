/**
    better-elfscrewer.c - based on this article, https://dustri.org/b/screwing-elf-header-for-fun-and-profit.html however this has support for both ELF32 and
    ELF64 files

    MIT License

    Copyright (c) 2020 Seth Stubbs

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
**/

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <elf.h>

typedef enum {
    VERIFY_FAILED,
    VERIFY_32, /* for 32bit ELF binaries */
    VERIFY_64, /* for 64bit ELF binaries */
} VERIFYRESULT;

VERIFYRESULT verifyELF(int file) {
    unsigned char magic[EI_NIDENT];

    /* read ELF header */
    if (read(file, magic, EI_NIDENT) != EI_NIDENT) {
        printf("Failed to read ELF header!\n");
        return VERIFY_FAILED;
    }

    /* check ELF magic */
    if (memcmp(magic, ELFMAG, SELFMAG)) {
        printf("Not an ELF file!\n");
        return VERIFY_FAILED;
    }

    /* read ELF class type */
    switch(magic[EI_CLASS]) {
        case ELFCLASS32: return VERIFY_32;
        case ELFCLASS64: return VERIFY_64;
        default: printf("Invalid EI_CLASS!\n"); return VERIFY_FAILED;
    }
}

void patchELF32(int file) {
    Elf32_Ehdr *header;

    /* map header */
    if ((header = (Elf32_Ehdr *)mmap(NULL, sizeof(Elf32_Ehdr), PROT_READ | PROT_WRITE, MAP_SHARED, file, 0)) == MAP_FAILED) {
        printf("Failed to map ELF32 header!\n");
        exit(EXIT_FAILURE);
    }

    /* patch values */
    header->e_shoff = 0xFFFF;
    header->e_shnum = 0xFFFF;
    header->e_shstrndx = 0xFFFF;

    /* sync file changes */
    if (msync(header, sizeof(Elf32_Ehdr), MS_SYNC) == -1) {
        printf("msync() failed!\n");
        exit(EXIT_FAILURE);
    }

    if (munmap(header, sizeof(Elf32_Ehdr)) == -1) {
        printf("munmap() failed!\n");
        exit(EXIT_FAILURE);
    }
}

void patchELF64(int file) {
    Elf64_Ehdr *header;

    /* map header */
    if ((header = (Elf64_Ehdr *)mmap(NULL, sizeof(Elf64_Ehdr), PROT_READ | PROT_WRITE, MAP_SHARED, file, 0)) == MAP_FAILED) {
        printf("Failed to map ELF32 header!\n");
        exit(EXIT_FAILURE);
    }

    /* patch values */
    header->e_shoff = 0xFFFF;
    header->e_shnum = 0xFFFF;
    header->e_shstrndx = 0xFFFF;

    /* sync file changes */
    if (msync(header, sizeof(Elf64_Ehdr), MS_SYNC) == -1) {
        printf("msync() failed!\n");
        exit(EXIT_FAILURE);
    }

    if (munmap(header, sizeof(Elf64_Ehdr)) == -1) {
        printf("munmap() failed!\n");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char **argv) {
    VERIFYRESULT elfType;
    int file;

    /* check args */
    if (argc != 2) {
        printf("USAGE: %s [ELF FILE]\n", argv[0]);
        return EXIT_FAILURE;
    }

    /* try to open file */
    if ((file = open(argv[1], O_RDWR)) == -1) {
        printf("Failed to open '%s'!\n", argv[1]);
        return EXIT_FAILURE;
    }

    if ((elfType = verifyELF(file)) == VERIFY_FAILED) {
        /* verifyElf() already printed to stdout the error */
        return EXIT_FAILURE;
    }

    switch(elfType) {
        case VERIFY_32: patchELF32(file); break;
        case VERIFY_64: patchELF64(file); break;
        default: break; /* not possible */
    }

    printf("'%s' ELF header patched!\n", argv[1]);
    return 0;
}