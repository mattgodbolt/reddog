; Set up the chip and the interrupt functions
chip				LC868700
world				external

; Constants
LCD					equ		180h

; Code starts at 200h
org	0200h
GameInit:
	; Set the register banks to [0-3]
	clr1	psw, 4
	clr1	psw, 3

	; Generate interrupts on keypresses (cancels hold mode too)
	set1	p3int, 2
	; Acknowldege any previous keypresses
	clr1	p3int, 1
	; Enable P3 interrupts
	set1	p3int, 0

	; Enable interrupts from port 3 globally
	set1	i23cr, 4
	; Interrupts on I3 rising
	set1	i23cr, 7
	; No interrupts on I3 falling
	clr1	i23cr, 6
urk
	; Clear the screen
	call	ClearScreen

	; Put into halt mode
	set1	pcon, 0

here:
	jmp		here

ClearBank:
	mov		#080h, 02h
	mov		#124, B
clearloop:
	st		@r2
	inc		02h
	dbnz	B, clearloop
	ret

ClearScreen:
	;xxx need system clock in rc oscillation mode says docs
	;xxx also docs say don't use 0c-0f of each row
	push	acc
	push	B
	push	02h
	mov		#000h, acc

	mov		#025h, 02h
	mov		#0, @r2
	call	ClearBank
	mov		#025h, 02h
	mov		#1, @r2
	call	ClearBank
	pop		02h
	pop		B
	pop		acc
	ret

end
