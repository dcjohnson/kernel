.section ".text.boot"
.global _start

_start:
	@ This is the code that is used to park the other core's
	@ execution. Remove it when running in a real raspberry pi.
	@ Of course. The init section must still be executed.
	@@@@@@@@@
	@ START @
	@@@@@@@@@
	mrc p15, 0, r0, c0, c0, 5
	and r0, r0, #3
	cmp r0, #0
	@ if the process then execute kernel_main. All others hang.
	beq init 
	b halt
	@@@@@@@
	@ END @
	@@@@@@@

init:	
	mov sp, #0x8000

	ldr r4, =__bss_start
	ldr r9, =__bss_end
	mov r5, #0
	mov r6, #0
	mov r7, #0
	mov r8, #0
	b 2f

1:
	stmia r4!, {r5 - r8}

2:
	cmp r4, r9
	blo 1b

	ldr r3, =kernel_main
	blx r3

halt:
	wfe
	b halt
