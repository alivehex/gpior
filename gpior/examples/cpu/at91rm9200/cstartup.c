#include "at91rm9200.h"
#include "types.h"


#define SDRC_TR_VALUE		
#define SDRC_CR_VALUE
#define SDRC_SRR_VALUE
#define OSC_FREQ_HZ			18432000
#define PLLC_TIMEOUT		5000

typedef struct _PLLC_CFG {
	uint32 PLLA_DIV;
	uint32 PLLA_MUL;
	uint32 PLLB_DIV;
	uint32 PLLB_MUL;
	uint32 MCK_SOURCE;
	uint32 MCK_SRC_DIV;
	uint32 CPU_DIV;
} PLLC_CFG_T;

/************************************* 
MCK source 
	AT91C_PMC_CSS_SLOW_CLK
	AT91C_PMC_CSS_MAIN_CLK
	AT91C_PMC_CSS_PLLA_CLK
	AT91C_PMC_CSS_PLLB_CLK
MCK source div
	AT91C_PMC_PRES_CLK
	AT91C_PMC_PRES_CLK_2
	AT91C_PMC_PRES_CLK_4
	AT91C_PMC_PRES_CLK_8
	AT91C_PMC_PRES_CLK_16
	AT91C_PMC_PRES_CLK_32
	AT91C_PMC_PRES_CLK_64
CPU_DIV
	AT91C_PMC_MDIV_1
	AT91C_PMC_MDIV_2
	AT91C_PMC_MDIV_3
	AT91C_PMC_MDIV_4
*************************************/	

/*
 * @ EXT-RAM init
 */
void EXTRAM_init(void) {
	AT91PS_EBI ebi = AT91C_BASE_EBI;
	AT91PS_SDRC sdram = AT91C_BASE_SDRC;
	AT91PS_PIO pioc = AT91C_BASE_PIOC;
	unsigned int *ptr = (unsigned int *)0x20000000;
	int i;

	/* NOR Flash CS0, SDRAM CS1 */
	ebi->EBI_CSA = (AT91C_EBI_CS0A_SMC | AT91C_EBI_CS1A_SDRAMC);
	/* data bus pull-up */
	ebi->EBI_CFGR = AT91C_EBI_DBPUC;
	
	/* SDRAM init */
	pioc->PIO_PDR = 0xFFFF0000;
	/* pre-charge */
	sdram->SDRC_MR = AT91C_SDRC_MODE_PRCGALL_CMD;
	*ptr = 0;
	/* reflash */
	sdram->SDRC_MR = AT91C_SDRC_MODE_RFSH_CMD;
	for(i = 0; i < 8; i ++) {
	 	*ptr = 0;
	}
	/* load */
	sdram->SDRC_MR = AT91C_SDRC_MODE_LMR_CMD;
	*(ptr + 0x80) = 0;
	/* reflash timer count */
	sdram->SDRC_TR = 0x000002E0;
	*ptr = 0;
	/**/
	sdram->SDRC_CR  = 0x2188C155;
	sdram->SDRC_SRR = 0x00001955;
	sdram->SDRC_IER = 0;
}

/*
 * @ PLLC init
 */
void PLLC_init(PLLC_CFG_T *pllc_cfg) {
	AT91PS_PMC	pmc = AT91C_BASE_PMC;
	AT91PS_CKGR ckgr = AT91C_BASE_CKGR;
	unsigned int cache;
	volatile unsigned int timeout;

	/* enable CPU and USB host clock */
	pmc->PMC_SCER = AT91C_PMC_PCK | AT91C_PMC_UHP;
	/* enable USB host, PIOA, PIOB, PIOC */
	pmc->PMC_PCER =	(AT91C_ID_PIOA | AT91C_ID_PIOB | AT91C_ID_PIOC | AT91C_ID_UHP);
	/* main oscillator */
	ckgr->CKGR_MOR = (AT91C_CKGR_MOSCEN | 0xFF00);
	timeout = PLLC_TIMEOUT;
	while((!(pmc->PMC_SR & 1)) && (timeout --));
	/* PLLA and PLLB */
	cache = pllc_cfg->PLLA_DIV | ((pllc_cfg->PLLA_MUL - 1) << 16) | (0x3F << 8);
	if((OSC_FREQ_HZ / pllc_cfg->PLLA_DIV * pllc_cfg->PLLA_MUL) > 160000000)
		cache |= (1 << 15); 
	ckgr->CKGR_PLLAR = cache;
	timeout = PLLC_TIMEOUT;
	while((!(pmc->PMC_SR & (1 << 1))) && (timeout --));
	cache = pllc_cfg->PLLB_DIV | ((pllc_cfg->PLLB_MUL - 1) << 16) | (0x3F << 8);
	if((OSC_FREQ_HZ / pllc_cfg->PLLB_DIV * pllc_cfg->PLLB_MUL) > 48000000)
		cache |= (1 << 28);
	ckgr->CKGR_PLLBR = cache;
	timeout = PLLC_TIMEOUT; 
	while((!(pmc->PMC_SR & (1 << 2))) && (timeout --));
	/* MCK */
	pmc->PMC_MCKR = (pllc_cfg->MCK_SOURCE | pllc_cfg->MCK_SRC_DIV | pllc_cfg->CPU_DIV);
	timeout = PLLC_TIMEOUT;
	while((!(pmc->PMC_SR & (1 << 3))) && (timeout --));
}

/*
 * @ REMAP
 */
void REMAP(void) {
 	
}

/*
 * @ C-CODE startup function
 * @ Called from assembly code
 */
void _cstartup(int config_extram) {
	PLLC_CFG_T pllc_cfg;
	
	pllc_cfg.PLLA_DIV = 4;
	pllc_cfg.PLLA_MUL = 39;
	pllc_cfg.PLLB_DIV = 14;
	pllc_cfg.PLLB_MUL = 73;
	pllc_cfg.MCK_SOURCE = AT91C_PMC_CSS_PLLA_CLK;
	pllc_cfg.MCK_SRC_DIV = AT91C_PMC_PRES_CLK;
	pllc_cfg.CPU_DIV = AT91C_PMC_MDIV_2;
	PLLC_init(&pllc_cfg);
	if(config_extram)
 		EXTRAM_init();
	REMAP();
}
