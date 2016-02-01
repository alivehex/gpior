/* ----------------------------------------------------------------------------
 *         ATMEL Microcontroller Software Support 
 * ----------------------------------------------------------------------------
 * Copyright (c) 2008, Atmel Corporation
 *
 * All rights reserved.
 */

#ifndef TRACE_H
#define TRACE_H

//------------------------------------------------------------------------------
//         Headers
//------------------------------------------------------------------------------

#include <stdio.h>

//------------------------------------------------------------------------------
//         Global Definitions
//------------------------------------------------------------------------------
#define TRACE_LEVEL_DEBUG      5
#define TRACE_LEVEL_INFO       4
#define TRACE_LEVEL_WARNING    3
#define TRACE_LEVEL_ERROR      2
#define TRACE_LEVEL_FATAL      1
#define TRACE_LEVEL_NO_TRACE   0

#define TRACE_NAME_ENABLE	   1
#define TRACE_NAME_DISABLE	   0

// By default, all traces are output except the debug one.
#if !defined(TRACE_LEVEL)    
#define TRACE_LEVEL TRACE_LEVEL_NO_TRACE
#endif

// By default, trace level is static (not dynamic)
#if !defined(DYN_TRACES)
#define DYN_TRACES 0
#endif

#if defined(NOTRACE)
#error "Error: NOTRACE has to be not defined !"
#endif

#undef NOTRACE
#if (TRACE_LEVEL == TRACE_LEVEL_NO_TRACE)
#define NOTRACE
#endif

#ifndef TRACE_TITLE
#define TRACE_TITLE	"-"
#endif

//------------------------------------------------------------------------------
//         Global Macros
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
/// Outputs a formatted string using <printf> if the log level is high
/// enough. Can be disabled by defining TRACE_LEVEL=0 during compilation.
/// \param format  Formatted string to output.
/// \param ...  Additional parameters depending on formatted string.
//------------------------------------------------------------------------------
#if defined(NOTRACE)

// Empty macro
#define TRACE(...)			  { }
#define TRACE_DEBUG(...)      { }
#define TRACE_INFO(...)       { }
#define TRACE_WARNING(...)    { }               
#define TRACE_ERROR(...)      { }
#define TRACE_FATAL(...)      { while(1); }

#define TRACE_DEBUG_WP(...)   { }
#define TRACE_INFO_WP(...)    { }
#define TRACE_WARNING_WP(...) { }
#define TRACE_ERROR_WP(...)   { }
#define TRACE_FATAL_WP(...)   { while(1); }

#elif (DYN_TRACES == 1)

// Trace output depends on traceLevel value
#define TRACE_DEBUG(...)      { if (traceLevel >= TRACE_LEVEL_DEBUG)   { printf("\r\n");printf(TRACE_TITLE);printf("-D- " __VA_ARGS__); } }
#define TRACE_INFO(...)       { if (traceLevel >= TRACE_LEVEL_INFO)    { printf("\r\n");printf(TRACE_TITLE);printf("-I- " __VA_ARGS__); } }
#define TRACE_WARNING(...)    { if (traceLevel >= TRACE_LEVEL_WARNING) { printf("\r\n");printf(TRACE_TITLE);printf("-W- " __VA_ARGS__); } }
#define TRACE_ERROR(...)      { if (traceLevel >= TRACE_LEVEL_ERROR)   { printf("\r\n");printf(TRACE_TITLE);printf("-E- " __VA_ARGS__); } }
#define TRACE_FATAL(...)      { if (traceLevel >= TRACE_LEVEL_FATAL)   { printf("\r\n");printf(TRACE_TITLE);printf("-F- " __VA_ARGS__); while(1); } }

#define TRACE_DEBUG_WP(...)   { if (traceLevel >= TRACE_LEVEL_DEBUG)   { printf(__VA_ARGS__); } }
#define TRACE_INFO_WP(...)    { if (traceLevel >= TRACE_LEVEL_INFO)    { printf(__VA_ARGS__); } }
#define TRACE_WARNING_WP(...) { if (traceLevel >= TRACE_LEVEL_WARNING) { printf(__VA_ARGS__); } }
#define TRACE_ERROR_WP(...)   { if (traceLevel >= TRACE_LEVEL_ERROR)   { printf(__VA_ARGS__); } }
#define TRACE_FATAL_WP(...)   { if (traceLevel >= TRACE_LEVEL_FATAL)   { printf(__VA_ARGS__); while(1); } }

#else // without defined DYN_TRACE

// Trace compilation depends on TRACE_LEVEL value

// trace function name output enable, cost more RAM space and CPU time
#ifndef TRACE_NAME
#define TRACE_NAME TRACE_NAME_DISABLE
#endif

#if (TRACE_NAME)
extern unsigned char __TRACE_FUNN__[128] ;
extern unsigned char trace_funn;
#include <string.h>

#define TRACE_FUNN_OUTPUT	{ \
	if(trace_funn) { \
		if(strcmp((const char *)__TRACE_FUNN__, (const char *)__FUNCTION__) != 0) { \
			printf("\r\n[ CURRENT < %s > ", __FUNCTION__); \
			printf("PREV < %s > ]", (const char *)__TRACE_FUNN__);\
			strcpy((char *)__TRACE_FUNN__, (const char *)__FUNCTION__); \
		} \
	} \
}
#else // without TRACE_NAME
#define TRACE_FUNN_OUTPUT {}
#endif // ifdef TRACE_FUNN
	
#if (TRACE_LEVEL >= TRACE_LEVEL_DEBUG)
#define TRACE_DEBUG(...)      { \
	TRACE_FUNN_OUTPUT \
	printf("\r\n");printf(TRACE_TITLE);printf("-D-" __VA_ARGS__); }
#define TRACE_DEBUG_WP(...)   { \
	TRACE_FUNN_OUTPUT \
	printf(__VA_ARGS__); }
#else
#define TRACE_DEBUG(...)      { }
#define TRACE_DEBUG_WP(...)   { }
#endif

#if (TRACE_LEVEL >= TRACE_LEVEL_INFO)
#define TRACE_INFO(...)       { \
	TRACE_FUNN_OUTPUT \
	printf("\r\n");printf(TRACE_TITLE);printf("-I-" __VA_ARGS__); }
#define TRACE_INFO_WP(...)    { \
	TRACE_FUNN_OUTPUT \
	printf(__VA_ARGS__); }
#else
#define TRACE_INFO(...)       { }
#define TRACE_INFO_WP(...)    { }
#endif

#if (TRACE_LEVEL >= TRACE_LEVEL_WARNING)
#define TRACE_WARNING(...)    { \
	TRACE_FUNN_OUTPUT \
	printf("\r\n");printf(TRACE_TITLE);printf("-W-" __VA_ARGS__); }
#define TRACE_WARNING_WP(...) { printf(__VA_ARGS__); }
#else
#define TRACE_WARNING(...)    { }
#define TRACE_WARNING_WP(...) { }
#endif

#if (TRACE_LEVEL >= TRACE_LEVEL_ERROR)
#define TRACE_ERROR(...)      { \
	TRACE_FUNN_OUTPUT \
	printf("\r\n");printf(TRACE_TITLE);printf("-E-" __VA_ARGS__); }
#define TRACE_ERROR_WP(...)   { printf(__VA_ARGS__); }
#else
#define TRACE_ERROR(...)      { }
#define TRACE_ERROR_WP(...)   { }
#endif

#if (TRACE_LEVEL >= TRACE_LEVEL_FATAL)
#define TRACE_FATAL(...)      { \
	TRACE_FUNN_OUTPUT \
	printf("\r\n");printf(TRACE_TITLE);printf("-F-" __VA_ARGS__); while(1); }
#define TRACE_FATAL_WP(...)   { printf(__VA_ARGS__); while(1); }
#else
#define TRACE_FATAL(...)      { while(1); }
#define TRACE_FATAL_WP(...)   { while(1); }
#endif

#endif // if defined(DYN_TRACE)


//------------------------------------------------------------------------------
//         Exported variables
//------------------------------------------------------------------------------
// Depending on DYN_TRACES, traceLevel is a modifable runtime variable
// or a define
#if !defined(NOTRACE) && (DYN_TRACES == 1)
    extern unsigned int traceLevel;
#endif

#endif //#ifndef TRACE_H

