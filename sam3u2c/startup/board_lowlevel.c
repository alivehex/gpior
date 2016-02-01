/* ----------------------------------------------------------------------------
 *         ATMEL Microcontroller Software Support
 * ----------------------------------------------------------------------------
 * Copyright (c) 2008, Atmel Corporation
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the disclaimer below.
 *
 * Atmel's name may not be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * DISCLAIMER: THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * ----------------------------------------------------------------------------
 */

//------------------------------------------------------------------------------
/// \unit
///
/// !Purpose
///
/// Provides the low-level initialization function that gets called on chip
/// startup.
///
/// !Usage
///
/// LowLevelInit() is called in #board_cstartup_xxx.c#.
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//         Headers
//------------------------------------------------------------------------------

#include "board.h"
#include "board_memories.h"
#include "board_lowlevel.h"
#include <pio/pio.h>

//------------------------------------------------------------------------------
//         Local definitions
//------------------------------------------------------------------------------
// Settings at 48/48MHz
#define AT91C_CKGR_MUL_SHIFT         16
#define AT91C_CKGR_OUT_SHIFT         14
#define AT91C_CKGR_PLLCOUNT_SHIFT     8
#define AT91C_CKGR_DIV_SHIFT          0

#define BOARD_OSCOUNT         (AT91C_CKGR_MOSCXTST & (0x3F << 8))
//#define BOARD_OSCOUNT         (AT91C_CKGR_MOSCXTST & (0xFF << 8))

#define BOARD_PLLR ((1 << 29) | (0x7 << AT91C_CKGR_MUL_SHIFT) \
        | (0x0 << AT91C_CKGR_OUT_SHIFT) |(0x3f << AT91C_CKGR_PLLCOUNT_SHIFT) \
        | (0x1 << AT91C_CKGR_DIV_SHIFT))
//#define BOARD_MCKR ( AT91C_PMC_PRES_CLK_2 | AT91C_PMC_CSS_PLLA_CLK)
#define BOARD_MCKR (AT91C_PMC_PRES_CLK | AT91C_PMC_CSS_PLLA_CLK)

// Define clock timeout
#define CLOCK_TIMEOUT           0xFFFFFFFF

//------------------------------------------------------------------------------
//         Local variables
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//         Local functions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//         Exported functions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
/// Performs the low-level initialization of the chip. This includes EFC, master
/// clock, IRQ & watchdog configuration.
//------------------------------------------------------------------------------
void LowLevelInit(void)
{
    unsigned int timeout = 0;

    /* Set 2 WS for Embedded Flash Access
     ************************************/
	AT91C_BASE_EFC0->EFC_FMR = AT91C_EFC_FWS_2WS;
	AT91C_BASE_EFC1->EFC_FMR = AT91C_EFC_FWS_2WS;

    /* Watchdog initialization
     *************************/
    AT91C_BASE_WDTC->WDTC_WDMR = AT91C_WDTC_WDDIS;

    /* Select external slow clock
     ****************************/
    if ((AT91C_BASE_SUPC->SUPC_SR & AT91C_SUPC_OSCSEL) != AT91C_SUPC_OSCSEL) {
        AT91C_BASE_SUPC->SUPC_CR = AT91C_SUPC_XTALSEL | (0xA5 << 24);
        timeout = 0;
        while (!(AT91C_BASE_SUPC->SUPC_SR & AT91C_SUPC_OSCSEL) && (timeout++ < CLOCK_TIMEOUT));
    }

    /* Initialize main oscillator
     ****************************/
    AT91C_BASE_PMC->PMC_MOR = (0x37 << 16) | BOARD_OSCOUNT |
                              AT91C_CKGR_MOSCXTEN | AT91C_CKGR_MOSCSEL;
    timeout = 0;
	
#if (0)
	// debug failed without these nops
	__asm {
		nop;
		nop;
		nop;
	};
#endif
	
    while (!(AT91C_BASE_PMC->PMC_SR & AT91C_PMC_MOSCXTS) && (timeout++ < CLOCK_TIMEOUT));

    /* Switch to main oscillator */
    AT91C_BASE_PMC->PMC_MCKR = (AT91C_BASE_PMC->PMC_MCKR & ~AT91C_PMC_CSS) | AT91C_PMC_CSS_MAIN_CLK;
    timeout = 0;
    while (!(AT91C_BASE_PMC->PMC_SR & AT91C_PMC_MCKRDY) && (timeout++ < CLOCK_TIMEOUT));

    /* Initialize PLLA */
    //AT91C_BASE_PMC->PMC_PLLAR = BOARD_PLLR;
	
	// PLLA freq = 24Mhz * (3 + 1) / 1 = 96Mhz
    AT91C_BASE_PMC->PMC_PLLAR = ((1 << 29) | (3 << AT91C_CKGR_MUL_SHIFT) |
       (0x3f << AT91C_CKGR_PLLCOUNT_SHIFT) | (0x1 << AT91C_CKGR_DIV_SHIFT));
	timeout = 0;
    while (!(AT91C_BASE_PMC->PMC_SR & AT91C_PMC_LOCKA) && (timeout++ < CLOCK_TIMEOUT));

    /* Initialize UTMI for USB usage */
    //AT91C_BASE_CKGR->CKGR_UCKR |= (AT91C_CKGR_UPLLCOUNT & (3 << 20)) | AT91C_CKGR_UPLLEN;
	AT91C_BASE_CKGR->CKGR_UCKR |= (AT91C_CKGR_UPLLCOUNT & (0xf << 20)) | AT91C_CKGR_UPLLEN;
    timeout = 0;
    while (!(AT91C_BASE_PMC->PMC_SR & AT91C_PMC_LOCKU) && (timeout++ < CLOCK_TIMEOUT));

    /* Switch to fast clock
     **********************/
	// MCLK = PLLA = 96Mhz, processor clock = PLLA / 1 = 96Mhz
    AT91C_BASE_PMC->PMC_MCKR = (BOARD_MCKR & ~AT91C_PMC_CSS) | AT91C_PMC_CSS_MAIN_CLK;
    timeout = 0;
    while (!(AT91C_BASE_PMC->PMC_SR & AT91C_PMC_MCKRDY) && (timeout++ < CLOCK_TIMEOUT));

    AT91C_BASE_PMC->PMC_MCKR = BOARD_MCKR;
    timeout = 0;
    while (!(AT91C_BASE_PMC->PMC_SR & AT91C_PMC_MCKRDY) && (timeout++ < CLOCK_TIMEOUT));

    /* Enable clock for UART
     ************************/
    //AT91C_BASE_PMC->PMC_PCER = (1 << AT91C_ID_DBGU);
	//AT91C_BASE_PMC->PMC_PCER = (1 << AT91C_ID_PIOA);

#ifdef psram
    unsigned int tmp;
    const Pin pinPsram[] = {BOARD_PSRAM_PINS};
    AT91PS_HSMC4_CS pSMC = AT91C_BASE_HSMC4_CS0;

    // Open EBI clock
    AT91C_BASE_PMC->PMC_PCER = (1<< AT91C_ID_HSMC4);

    // Configure I/O
    PIO_Configure(pinPsram, PIO_LISTSIZE(pinPsram));

    // Setup the PSRAM (HSMC4_EBI.CS0, 0x60000000 ~ 0x60FFFFFF)
    pSMC->HSMC4_SETUP = 0
                        | ((1 <<  0) & AT91C_HSMC4_NWE_SETUP)
                        | ((1 <<  8) & AT91C_HSMC4_NCS_WR_SETUP)
                        | ((1 << 16) & AT91C_HSMC4_NRD_SETUP)
                        | ((1 << 24) & AT91C_HSMC4_NCS_RD_SETUP)
                        ;
    pSMC->HSMC4_PULSE = 0
                        | ((5 <<  0) & AT91C_HSMC4_NWE_PULSE)
                        | ((5 <<  8) & AT91C_HSMC4_NCS_WR_PULSE)
                        | ((5 << 16) & AT91C_HSMC4_NRD_PULSE)
                        | ((5 << 24) & AT91C_HSMC4_NCS_RD_PULSE)
                        ;
    pSMC->HSMC4_CYCLE = 0
                        | ((6 <<  0) & AT91C_HSMC4_NWE_CYCLE)
                        | ((6 << 16) & AT91C_HSMC4_NRD_CYCLE)
                        ;
    tmp = pSMC->HSMC4_TIMINGS
        & (AT91C_HSMC4_OCMSEN | AT91C_HSMC4_RBNSEL | AT91C_HSMC4_NFSEL);
    pSMC->HSMC4_TIMINGS = tmp
                        | ((0 <<  0) & AT91C_HSMC4_TCLR) // CLE to REN
                        | ((0 <<  4) & AT91C_HSMC4_TADL) // ALE to Data
                        | ((0 <<  8) & AT91C_HSMC4_TAR)  // ALE to REN
                        | ((0 << 16) & AT91C_HSMC4_TRR)  // Ready to REN
                        | ((0 << 24) & AT91C_HSMC4_TWB)  // WEN to REN
                        ;
    tmp = pSMC->HSMC4_MODE & ~(AT91C_HSMC4_DBW);
    pSMC->HSMC4_MODE = tmp
                        | (AT91C_HSMC4_READ_MODE)
                        | (AT91C_HSMC4_WRITE_MODE)
                        | (AT91C_HSMC4_DBW_WIDTH_SIXTEEN_BITS)
                        ;
#endif // #ifndef psram

    /* Optimize CPU setting for speed */
    SetDefaultMaster(1);
	
	//SetFlashWaitState(10);
}

//------------------------------------------------------------------------------
/// Enable or disable default master access
/// \param enalbe 1 enable defaultMaster settings, 0 disable it.
//------------------------------------------------------------------------------
void SetDefaultMaster(unsigned char enable)
{
    AT91PS_HMATRIX2 pMatrix = AT91C_BASE_MATRIX;

    // Set default master
    if (enable == 1) {

        // Set default master: SRAM0 -> Cortex-M3 System
        pMatrix->HMATRIX2_SCFG0 |= AT91C_MATRIX_FIXED_DEFMSTR_SCFG0_ARMS |
                                   AT91C_MATRIX_DEFMSTR_TYPE_FIXED_DEFMSTR;

        // Set default master: Internal flash0 -> Cortex-M3 Instruction/Data
        pMatrix->HMATRIX2_SCFG3 |= AT91C_MATRIX_FIXED_DEFMSTR_SCFG3_ARMC |
                                   AT91C_MATRIX_DEFMSTR_TYPE_FIXED_DEFMSTR;
    } else {

        // Clear default master: SRAM0 -> Cortex-M3 System
        pMatrix->HMATRIX2_SCFG0 &= (~AT91C_MATRIX_DEFMSTR_TYPE);


        // Clear default master: Internal flash0 -> Cortex-M3 Instruction/Data
        pMatrix->HMATRIX2_SCFG3 &= (~AT91C_MATRIX_DEFMSTR_TYPE);
    }
}

//------------------------------------------------------------------------------
/// Set flash wait state
/// \param ws    Value of flash wait state
//------------------------------------------------------------------------------
void SetFlashWaitState(unsigned char ws)
{
    // Set Wait State for Embedded Flash Access
	AT91C_BASE_EFC0->EFC_FMR = ((ws << 8) & AT91C_EFC_FWS);
	AT91C_BASE_EFC1->EFC_FMR = ((ws << 8) & AT91C_EFC_FWS);
}

