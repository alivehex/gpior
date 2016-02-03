#include "cmd.h"
#include "cmdline.h"
#include "types.h"
#include "dbgu/dbgu.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define LINE_BUFFER_SIZE	64
#define PREV_BUFFER_NUM		4

#define BS			0x08
#define ENTER		0x0d
#define ESC			0x1b
#define DEL			0x7f
#define AR_M0		0x1b
#define AR_M1 		0x5b

#define AR_UP		0x41
#define AR_DOWN		0x42
#define AR_RIGHT	0x43
#define AR_LEFT		0x44

static uint8_t CommandBuffer[LINE_BUFFER_SIZE];
static uint8_t PrevCommandBuffer[PREV_BUFFER_NUM][LINE_BUFFER_SIZE];
static uint8_t *CommandBufferPointer = NULL;
static int PrevCmdBufIndex = 0;

static uint8_t serialReadByte(void);
static void serialWriteByte(int c);
static int isCtrlCode(uint8_t c);
static void clearPrevChar(void);
static int clearLine(void);

const tCmdLineEntry g_sCmdTable[] = {
	//{"echo", cmdEcho, "Echo back what is given: echo Hello"},
	{NULL, NULL, NULL,},
};


#define START_MSG0 "\r\n------------------------------------------------\r\n"
#define START_MSG1 "       MCU Mini System Version 1.0\r\n"
#define START_MSG2 "       alivehex@gmail.com"
#define START_MSG3 "\r\n------------------------------------------------\r\n"
#define START_MSG4 "-> input ? or help to see all commands <-"


void command_loop() {
	uint8_t c;
	
	printf(START_MSG0);
	printf(START_MSG1);
	printf(START_MSG2);
	printf(START_MSG3);
	printf(START_MSG4);
	
	while (1) {
		CommandBufferPointer = CommandBuffer;		
		printf("\r\n$");
		
		while (1) {
			c = serialReadByte();
			
			if (!isCtrlCode(c)) {
				*CommandBufferPointer ++ = c;
				serialWriteByte(c);
			} else {
				if ( (c == BS) || (c == DEL) ) {
					if (CommandBufferPointer > CommandBuffer) {
						clearPrevChar();
						CommandBufferPointer --;
					}
				} else if (c == ENTER) {
					*CommandBufferPointer = '\0';
					PrevCmdBufIndex = (PrevCmdBufIndex + 1) % PREV_BUFFER_NUM;
					strcpy((char *)PrevCommandBuffer[PrevCmdBufIndex], (const char *)CommandBuffer);
					// Start with a new line
					printf("\r\n");
					int ret = CmdLineProcess((char *)CommandBuffer);
					if (ret == CMDLINE_BAD_CMD) {
						printf("%s: command not found", CommandBuffer);
					} else if (ret == CMDLINE_TOO_MANY_ARGS) {
						printf("two many arguments");
					}
					break;
				} else if (c == AR_M0) {
					serialReadByte();
					c = serialReadByte();
					
					if (strlen((const char *)PrevCommandBuffer[PrevCmdBufIndex]) > 0) {	
						clearLine();
						strcpy((char *)CommandBuffer, (const char *)PrevCommandBuffer[PrevCmdBufIndex]);
							
						printf("%s", (const char *)CommandBuffer);
						CommandBufferPointer = &CommandBuffer[strlen((const char *)CommandBuffer)];
					}
					if (c == AR_UP) {
						if (PrevCmdBufIndex == 0) {
							PrevCmdBufIndex = (PREV_BUFFER_NUM - 1);
						} else {
							PrevCmdBufIndex --;
						}					
					} else if (c == AR_DOWN) {
						PrevCmdBufIndex = (PrevCmdBufIndex + 1) % PREV_BUFFER_NUM;
					} else if (c == AR_LEFT) {
						if (CommandBufferPointer > CommandBuffer) {
							printf("\b");
							CommandBufferPointer --;
						}
					} else if (c == AR_RIGHT) {
						if (CommandBufferPointer < (CommandBuffer + strlen((const char *)CommandBuffer))) {
							printf("%s", CommandBufferPointer);
							CommandBufferPointer ++;
						}
					} else {
						PrevCmdBufIndex = (PrevCmdBufIndex + 1) % PREV_BUFFER_NUM;
					}
				} else {
					// dump all the other codes
				}
			}
		}
	}
}

static int isCtrlCode(uint8_t c) {
	if ( (c == BS) || (c == ENTER) || (c == ESC) || (c == DEL) || (c == AR_M0)) 
		return 1;
	else
		return 0;
}

static void clearPrevChar(void) {
	printf("%c", BS);
	printf("%c", ' ');
	printf("%c", BS);
}

static int clearLine(void) {
	int i;
	
	// nothing to be clear
	if (CommandBufferPointer == CommandBuffer)
		return 0;
	
	int delta = CommandBufferPointer - CommandBuffer;
	
	for(i = 0; i < delta; i ++) {
		printf("%c", BS);
	}
	for(i = 0; i < delta; i ++) {
		printf("%c", ' ');
	}
	for(i = 0; i < delta; i ++) {
		printf("%c", BS);
	}
	return delta;
}

static uint8_t serialReadByte(void) {
	return DBGU_GetChar();
}

static void serialWriteByte(int c) {
	DBGU_PutChar(c);
}


