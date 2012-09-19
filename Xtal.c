#include "typedefine.h"
#include "xtal.h"
#include <machine.h>


void XtalInit()
{
	/*
	���̃R�[�h�́@20MHz�̃��C���N���b�N�����A32kHz�̃T�u�����𗘗p��
	PLL��H�𗘗p���ă��C���N���b�N��2����10���{��100MH���œ��삳��
	CPU�������C���N���b�N(ICLK)��PLL2������50MH���i�ő�j
	�e��y���t�F�����̏����p�N���b�N(PCLKB)��25MHz(�ő��32MH��)
	�A�i���O�ϊ��@�\S12AD��p�N���b�N(PCLKD)��50MH���i�ő�j
	FlashIF�ɋ��������FlashIF�N���b�N�iFCLK�j��25MH��(�ő��32MH���A�������ݎ��͂SMH����������)
	�O���o�X�ɋ��������O���o�X�N���b�N�iBCLK�j��25MHz (max)
	���A���^�C���N���b�N�ɋ��������RTC��p�T�u�N���b�N�iRTCSCLK�j��32.768kHz
	IWDT�ɋ��������IWDT��p�ᑬ�N���b�N�iIWDTCLK�j��125kHz
	�ɂ��܂��B
	*/
	
	//for reset oscillator config protecton.
	volatile unsigned char * cp = (unsigned char *)0x00080200;
	
	
	/* ---- Disable write protection ---- ���C�g�v���e�N�V�����̉��� ��������Ȃ��Ɠ���̃��W�X�^�ւ̏������݂��o���Ȃ�*/
    SYSTEM.PRCR.WORD = 0xA507;              /* Enable writing to registers */
                                            /* related to the clock generation circuit. */
                                            /* Enable writing to registers */
                                            /* related to the low power consumption function. */
                                            /* Enable writing to address 0008 0200h. */
    /* ---- Write to address 0008 0200h ---- */
    *cp = 0x00;	//�R���͕K�����Ȃ��ƃ_���Ƃ̂��Ɓi���l�T�X�}�j���A��9.8.1�g�p��̒��ӂ��j

    /* ---- Turn off the HOCO power supply ---- */
    SYSTEM.HOCOPCR.BYTE = 0x01;             /* Highspeed On Chip oscillator power supply is turned off. */

	
    /* ---- Stop the sub-clock ---- �T�u�N���b�N�� SOSCCR��RCR3�Ő��䂳��܂�*/
    SYSTEM.SOSCCR.BYTE = 0x01;              /* Sub-clock oscillator is stopped. */
    while(SYSTEM.SOSCCR.BYTE != 0x01){      /* Confirm that the written value can be read correctly. */
    }
    RTC.RCR3.BYTE = 0x0C;                   /* Sub-clock oscillator is stopped.�W��CL�p�h���C�u�\�͂ɐݒ� */
    while(RTC.RCR3.BYTE != 0x0C){           /* Confirm that the written value can be read correctly. */
    }

�@   /* ---- Set the main clock oscillator drive capability ���C���N���b�N�����ݒ�---- */
    SYSTEM.MOFCR.BIT.MODRV = 7;             /* 111b -- 16MHz to 20MHz�̐��� */
                                            /* 000b -- other xtal  */
											
	SYSTEM.MOFCR.BIT.MODRV2 = 2;  			/* 01b -- 1MHz to 8MHz */
											/* 10b -- 8.1MHz to 15.9MHz */
											/* 11b -- 16MHz to 20MHz */
											
    /* ---- Set wait time until the main clock oscillator stabilizes ---- */
    SYSTEM.MOSCWTCR.BYTE = 0x0D;            /* Wait time is 131072 cycles �r�b�g�ɂ���ăE�F�C�g�񐔂��ς��܂����A���ɂ��̐ݒ�Ŗ��Ȃ� */
		
	/* ---- Operate the main clock oscillator ---- */
    SYSTEM.MOSCCR.BYTE = 0x00;              /* Main clock oscillator is operating. */
    while(SYSTEM.MOSCCR.BYTE != 0x00){      /* Confirm that the written value can be read correctly. */
    }

	
	//PLL����
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

	//�T�u�N���b�N����	
	/* ---- Set the sub-clock oscillator drive ability ---- */
    RTC.RCR3.BYTE = 0x0C;                   /* Drive ability for standard CL ���̐ݒ�͒����d�̓��[�h�Ŏg�����A�N���b�N�����肵�Ȃ�*/
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

	//�V�X�e���N���b�N�ݒ�@--�@�y���t�F�����p�̃N���b�N�ݒ�
	SYSTEM.SCKCR.BIT.FCK = 2;		// FlashIF clock (FCLK) divide by 4 -> 25MHz
	SYSTEM.SCKCR.BIT.ICK = 1; 		//System clock (ICLK), divide by 2 -> 50MHz 
	
	
	
	
	
	
	
	