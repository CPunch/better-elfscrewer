# better-elfscrewer.c

Small program to patch ELF32/ELF64 binaries to break simple gdb debugging and other misc. analysis tools. Based on [this article](https://dustri.org/b/screwing-elf-header-for-fun-and-profit.html).

## Usage

```sh
$ ./better-elfscrewer [ELF FILE]
```

## Compiling

```sh
gcc better-elfscrewer.c -o better-elfscrewer
```