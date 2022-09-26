chip	LC868700
world	external
; *-----------------------------------------------------*
; *	External header program Ver 1.00		*
; *					05/20-'98	*
; *-----------------------------------------------------*

public	fm_wrt_ex_exit,fm_vrf_ex_exit
public	fm_prd_ex_exit,timer_ex_exit,_game_start,_game_end
other_side_symbol	fm_wrt_in,fm_vrf_in
other_side_symbol	fm_prd_in,timer_in,game_end
 
; *-----------------------------------------------------*
; *	Vector table(?)					*
; *-----------------------------------------------------*
cseg
org	0000h
_game_start:
;reset:
		jmp	reset		; main program jump
org	0003h
;int_03:
		jmp	int_03
org	000bh
;int_0b:
		jmp	int_0b
org	0013h
;int_13:
		jmp	int_13
org	001bh
;int_1b:
		jmp	int_1b
org	0023h
;int_23:
		jmp	int_23
org	002bh
;int_2b:
		jmp	int_2b
org	0033h
;int_33:
		jmp	int_33
org	003bh
;int_3b:
		jmp	int_3b
org	0043h
;int_43:
		jmp	int_43
org	004bh
;int_4b:
		jmp	int_4b
; *-----------------------------------------------------*
; *	interrupt programs				*
; *-----------------------------------------------------*
int_03:
		reti
int_0b:
		reti
int_13:
		reti
int_23:
		reti
int_2b:
		reti
int_33:
		reti
int_3b:
		reti
; *-----------------------------------------------------*
int_43:
		reti
int_4b:
		clr1	p3int,1		;interrupt flag clear
		reti

org	0100h
; *-----------------------------------------------------*
; *	flash memory write external program		*
; *-----------------------------------------------------*
fm_wrt_ex:
		change	fm_wrt_in
	fm_wrt_ex_exit:
		ret
org	0110h
; *-----------------------------------------------------*
; *	flash memory verify external program		*
; *-----------------------------------------------------*
fm_vrf_ex:
		change	fm_vrf_in
	fm_vrf_ex_exit:
		ret

org	0120h
; *-----------------------------------------------------*
; *	flash memory page read external program		*
; *-----------------------------------------------------*
fm_prd_ex:
		change	fm_prd_in
	fm_prd_ex_exit:
		ret

org	0130h
; *-----------------------------------------------------*
; *	flash memory => timer call external program	*
; *-----------------------------------------------------*
int_1b:
timer_ex:
		push	ie
		clr1	ie,7		;interrupt prohibition
		change	timer_in
	timer_ex_exit:
		pop	ie
		reti

org	01f0h
_game_end:
		change	game_end
org	0200h
reset:
end
