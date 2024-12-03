/*******************************************************************************
* @version    V1.15
* @date       Mon Jun 24 20:42:47 2024
* @copyright  YHFT Compiler Group, School of Computer Science ,NUDT. 
********************************************************************************/ 
	.file	"compilation_test.c"
.text;
	; ;APP 
	.section ._gsm,"aw",%nobits
	; ;NO_APP 
	.section	.global,"ax",@progbits
	.global	test
	.type	test, @function
test:
		SMVAAG.M1		AR8, R61
		SNOP		1
		SSTW		R61, *+AR9
		SNOP		3
		SMVAAA.M1		AR9, AR8
		SNOP		1
		SMOVI24		-48, R61
		SADDA.M1		R61,AR9,AR9
		SNOP		1
		SSTW		R63, *+AR9[1]
		SNOP		3
		SMOVI24		10, R10
		SBR		scalar_malloc
		SMOVI.M1		.L2, R63
		SNOP		5
.L2:
		SSTW		R10, *-AR8[1]
		SNOP		3
		SLDW		*-AR8[1], R50
		SNOP		6
		SMVAGA.M1		R50, AR14
		SNOP		1
		SMVAAA.M1		AR14, AR13
		SNOP		1
		SLDB		*+AR13, R50
		SNOP		6
		SSTB		R50, *-AR8[9]
		SNOP		3
		SLDB		*-AR8[9], R50
		SNOP		6
		SADD.M1		1,R50,R50
		SSTB		R50, *-AR8[9]
		SNOP		3
		SLDW		*-AR8[1], R10
		SNOP		6
		SBR		scalar_free
		SMOVI.M1		.L3, R63
		SNOP		5
.L3:
		SNOP		1
		SLDW		*+AR9[1], R63
		SNOP		6
		SMVAAA.M1		AR8, AR9
		SNOP		1
		SLDW		*+AR9, R61
		SNOP		6
		SMVAGA.M1		R61, AR8
		SNOP		1
		SBR		R63
		SNOP		6
	.size	test, .-test
	.ident	"GCC: (GNU) 8.3.0"
