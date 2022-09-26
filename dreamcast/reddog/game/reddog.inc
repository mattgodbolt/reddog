; RedDog.inc
; Includes for RedDog assembler routines
#include "Localdefs.h"
#if COUNT_GEOMETRY
#undef COUNT_GEOMETRY
COUNT_GEOMETRY:	.ASSIGNA	1
#else
#undef COUNT_GEOMETRY
COUNT_GEOMETRY:	.ASSIGNA	0
#endif

; Declares a vector floating point
; eg: DEC_XYZW vert,0,1,2,3 declares vertX, vertY, vertZ, vertW, vertXY, vertZW and vert
				.MACRO	DEC_XYZW NAME,A,B,C,D
\NAME'X:		.FREG	FR\A
\NAME'Y:		.FREG	FR\B
\NAME'Z:		.FREG	FR\C
\NAME'W:		.FREG	FR\D
\NAME'XY:		.FREG	DR\A
\NAME'ZW:		.FREG	DR\C
\NAME:			.FREG	FV\A
				.ENDM


; Pushes a register
				.MACRO	PUSH REGISTER
				MOV.L	\REGISTER, @-R15
				.ENDM
; Pop a register
				.MACRO	POP REGISTER
				MOV.L	@R15+, \REGISTER
				.ENDM

; Pushes an FP register
				.MACRO	FPUSH REGISTER
				FMOV.S	\REGISTER, @-R15
				.ENDM
; Pop an FP register
				.MACRO	FPOP REGISTER
				FMOV.S	@R15+, \REGISTER
				.ENDM

; Constant shift left
				.MACRO	SHLLC	REG,AMOUNT
SHIFT_COUNT:	.ASSIGNA	\AMOUNT

				.AWHILE		\&SHIFT_COUNT GE 16
				SHLL16		\REG
SHIFT_COUNT:	.ASSIGNA	\&SHIFT_COUNT - 16				
				.AENDW

				.AWHILE		\&SHIFT_COUNT GE 8
				SHLL8		\REG
SHIFT_COUNT:	.ASSIGNA	\&SHIFT_COUNT - 8				
				.AENDW

				.AWHILE		\&SHIFT_COUNT GE 2
				SHLL2		\REG
SHIFT_COUNT:	.ASSIGNA	\&SHIFT_COUNT - 2				
				.AENDW
				
				.AWHILE		\&SHIFT_COUNT GT 0
				SHLL		\REG
SHIFT_COUNT:	.ASSIGNA	\&SHIFT_COUNT - 1
				.AENDW

				.ENDM

; Constant shift right
				.MACRO	SHLRC	REG,AMOUNT
SHIFT_COUNT:	.ASSIGNA	\AMOUNT

				.AWHILE		\&SHIFT_COUNT GE 16
				SHLR16		\REG
SHIFT_COUNT:	.ASSIGNA	\&SHIFT_COUNT - 16				
				.AENDW

				.AWHILE		\&SHIFT_COUNT GE 8
				SHLR8		\REG
SHIFT_COUNT:	.ASSIGNA	\&SHIFT_COUNT - 8				
				.AENDW

				.AWHILE		\&SHIFT_COUNT GE 2
				SHLR2		\REG
SHIFT_COUNT:	.ASSIGNA	\&SHIFT_COUNT - 2				
				.AENDW
				
				.AWHILE		\&SHIFT_COUNT GT 0
				SHLR		\REG
SHIFT_COUNT:	.ASSIGNA	\&SHIFT_COUNT - 1
				.AENDW

				.ENDM

; Test for zero
				.MACRO CMPZ REG
				TST			\REG, \REG
				.ENDM

vCacheSize:		.ASSIGNA	512
vCacheMask:		.EQU	(511 * 16)
