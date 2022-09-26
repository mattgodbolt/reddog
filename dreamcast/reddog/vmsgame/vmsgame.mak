
ASM		= P:\KATANA\utl\Dev\VMSTool\Dev\m86k
LNK		= P:\KATANA\utl\Dev\VMSTool\Dev\l86k
HEX		= P:\KATANA\utl\Dev\VMSTool\Dev\e2h86k

.ASM.OBJ:
	$(ASM) -N $*;

VMSGame.hex: VMSGame.eva
	$(HEX) /I VMSGame.eva
	del VMSGame.hex
	ren VMSGame.h00 VMSGame.hex

VMSGame.eva: RedDog.obj GHead.obj
	$(LNK) -p -l RedDog.obj GDummy.obj GHead.obj,VMSGame.eva,,

