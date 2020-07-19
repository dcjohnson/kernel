.section ".text.boot"
.global _start

_start:
	@ This is the code that is used to park the other core's
	@ execution. Remove it when running in a real raspberry pi.
	@ Of course. The init section must still be executed.
	@@@@@@@@@
	@ START @
	@@@@@@@@@
	@ Get the register ID
	mrc p15, 0, r0, c0, c0, 5

	@ And the value with the number 3. This allows us to ignore the rest of the bits of the register
	@ and ensure that the only core that executes the code is core 0. The other cores will result in a
	@ non-zero value in r0. 
	and r0, r0, #3

	@ Compare r0 with 0.
	cmp r0, #0
	@ If equal jump to init
	beq init

	@ Else hang
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

	@ load the address of kernel_main
	ldr r3, =kernel_main
	@ Call kernel_main
	blx r3

halt:
	wfe
	b halt
