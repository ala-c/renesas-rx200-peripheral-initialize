#include "typedefine.h"
#include "xtal.h"
#include <machine.h>

//追記テスト
void XtalInit()
{
	/*
	このコードは　20MHzのメインクロック水晶、32kHzのサブ水晶を利用し
	PLL回路を利用してメインクロックを2分周10逓倍の100MHｚで動作させ
	CPU処理メインクロック(ICLK)をPLL2分周の50MHｚ（最大）
	各種ペリフェラルの処理用クロック(PCLKB)を25MHz(最大は32MHｚ)
	アナログ変換機能S12AD専用クロック(PCLKD)を50MHｚ（最大）
	FlashIFに供給されるFlashIFクロック（FCLK）を25MHｚ(最大は32MHｚ、書き込み時は４MHｚ制限あり)
	外部バスに供給される外部バスクロック（BCLK）を25MHz (max)
	リアルタイムクロックに供給されるRTC専用サブクロック（RTCSCLK）を32.768kHz
	IWDTに供給されるIWDT専用低速クロック（IWDTCLK）を125kHz
	にします。
	*/
	
	//for reset oscillator config protecton.
	volatile unsigned char * cp = (unsigned char *)0x00080200;
	
	
	/* ---- Disable write protection ---- ライトプロテクションの解除 これをやらないと特定のレジスタへの書き込みが出来ない*/
    SYSTEM.PRCR.WORD = 0xA507;              /* Enable writing to registers */
                                            /* related to the clock generation circuit. */
                                            /* Enable writing to registers */
                                            /* related to the low power consumption function. */
                                            /* Enable writing to address 0008 0200h. */
    /* ---- Write to address 0008 0200h ---- */
    *cp = 0x00;	//コレは必ずやらないとダメとのこと（ルネサスマニュアル9.8.1使用上の注意より）

    /* ---- Turn off the HOCO power supply ---- */
    SYSTEM.HOCOPCR.BYTE = 0x01;             /* Highspeed On Chip oscillator power supply is turned off. */

	
    /* ---- Stop the sub-clock ---- サブクロックは SOSCCRとRCR3で制御されます*/
    SYSTEM.SOSCCR.BYTE = 0x01;              /* Sub-clock oscillator is stopped. */
    while(SYSTEM.SOSCCR.BYTE != 0x01){      /* Confirm that the written value can be read correctly. */
    }
    RTC.RCR3.BYTE = 0x0C;                   /* Sub-clock oscillator is stopped.標準CL用ドライブ能力に設定 */
    while(RTC.RCR3.BYTE != 0x0C){           /* Confirm that the written value can be read correctly. */
    }

　   /* ---- Set the main clock oscillator drive capability メインクロック発生設定---- */
    SYSTEM.MOFCR.BIT.MODRV = 7;             /* 111b -- 16MHz to 20MHzの水晶 */
                                            /* 000b -- other xtal  */
											
	SYSTEM.MOFCR.BIT.MODRV2 = 2;  			/* 01b -- 1MHz to 8MHz */
											/* 10b -- 8.1MHz to 15.9MHz */
											/* 11b -- 16MHz to 20MHz */
											
    /* ---- Set wait time until the main clock oscillator stabilizes ---- */
    SYSTEM.MOSCWTCR.BYTE = 0x0D;            /* Wait time is 131072 cycles ビットによってウェイト回数が変わりますが、特にこの設定で問題なし */
		
	/* ---- Operate the main clock oscillator ---- */
    SYSTEM.MOSCCR.BYTE = 0x00;              /* Main clock oscillator is operating. */
    while(SYSTEM.MOSCCR.BYTE != 0x00){      /* Confirm that the written value can be read correctly. */
    }

	
	//PLL処理
	/* ---- Set the PLL division ratio and multiplication factor ---- */
    SYSTEM.PLLCR.BIT.PLIDIV = 1;		/* PLL input division ratio is divide-by-2. */
	SYSTEM.PLLCR.BIT.STC = 9;           /* Frequency multiplication factor is multiply-by-10. */

	/* ---- Set wait time until the PLL clock oscillator stabilizes ---- */
    SYSTEM.PLLWTCR.BYTE = 0x0C;             /* Wait time is 524288 cycles */
	
	/* ---- Operate the PLL clock oscillator ---- */
    SYSTEM.PLLCR2.BYTE = 0x00;              /* PLL is operating. */
    /* ---- Wait processing for the clock oscillation stabilization ---- */
    cmt0_wait( 10500L/FOR_CMT0_TIME+1 );    /* Wait until the main clock and PLL clock */
                                            /* oscillation stabilize (10.5 ms). */

	//サブクロック処理	
	/* ---- Set the sub-clock oscillator drive ability ---- */
    RTC.RCR3.BYTE = 0x0C;                   /* Drive ability for standard CL 他の設定は低消費電力モードで使うが、クロックが安定しない*/
    while(RTC.RCR3.BYTE != 0x0C){           /* Confirm that the written value can be read correctly. */
    }
	
	/* ---- Set wait time until the sub-clock oscillator stabilizes ---- */
    SYSTEM.SOSCWTCR.BYTE = 0x00;            /* Wait for tSUBOSCWT0 + 2 cycles */

	/* ---- Wait processing for the clock oscillation stabilization ---- */
    cmt0_wait( 153L/FOR_CMT0_TIME+1 );      /* Wait for 5 cycles of the sub-clock (153 us). */
											
    /* ---- Operate the sub-clock oscillator ---- */
    SYSTEM.SOSCCR.BYTE = 0x00;              /* Sub-clock oscillator is operating. */
    while (SYSTEM.SOSCCR.BYTE != 0x00) {    /* Confirm that the written value can be read correctly. */
    }
	
	/* ---- Wait processing for the clock oscillation stabilization ---- */
    cmt0_wait( 2600000L/FOR_CMT0_TIME+1 );  /* Wait until the sub-clock oscillation stabilizes (2.6 s) */

	
	/* ---- Set the operating power control mode ---- */
    SYSTEM.OPCCR.BYTE = 0x00;               /* High-speed operating mode */
    /* ---- Transition to the operation power control mode completed? ---- */
    while(SYSTEM.OPCCR.BIT.OPCMTSF == 1){
    }

	//システムクロック設定　--　ペリフェラル用のクロック設定
	SYSTEM.SCKCR.BIT.FCK = 2;		// FlashIF clock (FCLK) divide by 4 -> 25MHz
	SYSTEM.SCKCR.BIT.ICK = 1; 		//System clock (ICLK), divide by 2 -> 50MHz 
	
	
	
	
	
	
	
	