!IFDEF DEMO
Q:\SamplerDisk\track3\RedDog\RDDemo.bin:	dummy
	cd ..\Game
	nmake /f Game.mak _RELEASE= RDDemo.elf
	<<DoIt.bat
	c:\dreamcast\bin\ELF2BIN -s 8c010000 RDDemo.elf Q:\SamplerDisk\track5\RedDog\RDDemo.bin
	touch Q:\SamplerDisk\track5\RedDog\RDDemo.bin
<<KEEP
!ELSE
d:\dreamcast\wads\1st_read.bin:	dummy
	cd ..\Game
	nmake /f Game.mak 1st_read.bin
	copy 1st_read.bin d:\dreamcast\wads\1st_read.bin
!ENDIF

dummy:
