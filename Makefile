# Compiler settings
CC=gcc
ICC=icc
CFLAGS=-O2 -Wall
OMPFLAGS=-fopenmp

# Compiling with GNU Compiler
ci: cInsertion.c coordReader.c
	$(CC) $(CFLAGS) cInsertion.c coordReader.c -o ci.exe

fi: fInsertion.c coordReader.c
	$(CC) $(CFLAGS) fInsertion.c coordReader.c -o fi.exe

comp: ompcInsertion.c coordReader.c
	$(CC) $(CFLAGS) $(OMPFLAGS) ompcInsertion.c coordReader.c -o comp.exe

fomp: ompfInsertion.c coordReader.c
	$(CC) $(CFLAGS) $(OMPFLAGS) ompfInsertion.c coordReader.c -o fomp.exe

# Compiling with Intel Compiler
icomp: ompcInsertion.c coordReader.c
	$(ICC) -qopenmp ompcInsertion.c coordReader.c -o icomp.exe

ifomp: ompfInsertion.c coordReader.c
	$(ICC) -qopenmp ompfInsertion.c coordReader.c -o ifomp.exe

# Clean command
clean:
	del *.exe *.o

.PHONY: ci fi comp fomp icomp ifomp clean
