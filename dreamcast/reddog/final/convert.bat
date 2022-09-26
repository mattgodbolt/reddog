@echo ** Creating S-file **
cnvs ..\Game\RedDog.elf RedDog.sfl
@echo ** Creating BIN-file **
s2bin RedDog.sfl 1st_read.bin
