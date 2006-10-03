/**********************************************************************


Copyright (C) 1999  Hiroyuki Kuwahara   All rights reserved

FILE NAME:      			log.h

DATE:           			6/2/99	

DESIGNER NAME:  			Hiroyuki Kuwahara	hkuwahara@yahoo.com

HISTORY:
	September 6, 2000 -- ADD ERROR IDs for CS5460 University of Utah



**********************************************************************/
#if !defined(HAVE_LOG)
#	define HAVE_LOG

#include "dll_scope.h"	
#	include "type.h"

#	if defined(__cplusplus)
		extern "C" {
#	endif

/***************************************************
	1 at the second LSB indicates errors

	|0000000000000000| |00000000| |000000| |10|
	------------------ ---------- -------- ----
			|				|			|	|
			|				|			|	---	means error
			|				|			|
			|				|			|------ error ID1
			|				|			
			|				|-------------------error ID2
			|
			|-----------------------------------reserved for future use
***************************************************/

/*	error ID1
	mostly specify the cause of error 
*/
#	define E_ID1_MASK			0X000000FE

#	define E_FILE_OPEN			0X00000006
#	define E_OVERFLOW			0X0000000A
#	define E_UNDERFLOW			0X0000000E
#	define E_WRONGDATA 			0X00000012
#	define E_NODATA				0X00000016

#	define E_ALLOCATION			0X0000001A
#	define E_UNEXPECTED_VALUE	0X0000001E 
#	define E_FORK				0X00000022 
#	define E_NULL_INPUT			0X00000026 
#	define E_PIPE				0X0000002A 
/*
#	define FAILING			0X0000002E 
#	define FAILING			0X00000032 
*/
/*	error ID2
	mostly specify the cause of error more
*/

#	define E_ID2_MASK			0X0000FF00

#	define E_LOG_FILE			0X00000100
#	define E_CONVERT_STRING		0X00000200 
#	define E_NOT_ENOUGH_SPACE	0X00000300 
#	define E_ZERO				0X00000400 
#	define E_NOT_HEX			0X00000500 

#	define E_END_OF_FILE		0X00000600 
/*
#	define FAILING				0X00000700 
*/

#	if defined(TAKE_LOG)

#		define RETURN_VAL(r) (( r&FAILING ) ? "FAILING" : "SUCCESS") 
#		define START_FUNCTION(fname) TakeLog( __FILE__, fname, "IN") 
#		define END_FUNCTION(fname,status) (TakeLog( __FILE__, fname, RETURN_VAL(status))) 

#	else
#		define START_FUNCTION(fname) 
#		define END_FUNCTION(fname,status) 
#		define RETURN_VAL(r) ""
#	endif	

	
#	define TIME_SIZE 64
/* not used.  vprintf() vfprintf() are used instead */
#	define ERRORREPORT_SIZE 1024  
#	define LOG_SUFFIX ".log"

DLLSCOPE RET_VAL STDCALL StartLog( LPCSTR );		 		
DLLSCOPE void STDCALL TakeLog( LPCSTR, LPCSTR, LPCSTR );
DLLSCOPE RET_VAL STDCALL ErrorReport( RET_VAL, LPCSTR, LPCSTR, ... );
DLLSCOPE void STDCALL SetReport( LPSTR, RET_VAL, LPCSTR );
DLLSCOPE LPSTR STDCALL GetErrorID1( RET_VAL );
DLLSCOPE LPSTR STDCALL GetErrorID2( RET_VAL );

DLLSCOPE void STDCALL EndLog();


DLLSCOPE void STDCALL TakeReport( LPCSTR format, ... );
DLLSCOPE void STDCALL PrintLine(LPCSTR format, ... );

/* not used.  vprintf() vfprintf() are used instead */
#define REPORT_MESSAGE_SIZE 2048
#define TRACE_MESSAGE_SIZE 2048

#if defined(TAKE_LOG)
#define TAKE_REPORT_0(format) TakeReport(format)
#define TAKE_REPORT_1(format,a1) TakeReport(format,a1)
#define TAKE_REPORT_2(format,a1,a2) TakeReport(format,a1,a2)
#define TAKE_REPORT_3(format,a1,a2,a3) TakeReport(format,a1,a2,a3)
#define TAKE_REPORT_4(format,a1,a2,a3,a4) TakeReport(format,a1,a2,a3,a4)
#define TAKE_REPORT_5(format,a1,a2,a3,a4,a5) TakeReport(format,a1,a2,a3,a4,a5)
#define TAKE_REPORT_6(format,a1,a2,a3,a4,a5,a6) TakeReport(format,a1,a2,a3,a4,a5,a6)
#define TAKE_REPORT_7(format,a1,a2,a3,a4,a5,a6,a7) TakeReport(format,a1,a2,a3,a4,a5,a6,a7)
#define TAKE_REPORT_8(format,a1,a2,a3,a4,a5,a6,a7,a8) TakeReport(format,a1,a2,a3,a4,a5,a6,a7,a8)
#define TAKE_REPORT_9(format,a1,a2,a3,a4,a5,a6,a7,a8,a9) TakeReport(format,a1,a2,a3,a4,a5,a6,a7,a8,a9)
#else
#define TAKE_REPORT_0(format) 
#define TAKE_REPORT_1(format,a1) 
#define TAKE_REPORT_2(format,a1,a2) 
#define TAKE_REPORT_3(format,a1,a2,a3) 
#define TAKE_REPORT_4(format,a1,a2,a3,a4) 
#define TAKE_REPORT_5(format,a1,a2,a3,a4,a5)
#define TAKE_REPORT_6(format,a1,a2,a3,a4,a5,a6) 
#define TAKE_REPORT_7(format,a1,a2,a3,a4,a5,a6,a7)
#define TAKE_REPORT_8(format,a1,a2,a3,a4,a5,a6,a7,a8) 
#define TAKE_REPORT_9(format,a1,a2,a3,a4,a5,a6,a7,a8,a9)
#endif



#if defined(DEBUG)
#define TRACE_0(format) PrintLine(format)
#define TRACE_1(format,a1) PrintLine(format,a1)
#define TRACE_2(format,a1,a2) PrintLine(format,a1,a2)
#define TRACE_3(format,a1,a2,a3) PrintLine(format,a1,a2,a3)
#define TRACE_4(format,a1,a2,a3,a4) PrintLine(format,a1,a2,a3,a4)
#define TRACE_5(format,a1,a2,a3,a4,a5) PrintLine(format,a1,a2,a3,a4,a5)
#define TRACE_6(format,a1,a2,a3,a4,a5,a6) PrintLine(format,a1,a2,a3,a4,a5,a6)
#define TRACE_7(format,a1,a2,a3,a4,a5,a6,a7) PrintLine(format,a1,a2,a3,a4,a5,a6,a7)
#define TRACE_8(format,a1,a2,a3,a4,a5,a6,a7,a8) PrintLine(format,a1,a2,a3,a4,a5,a6,a7,a8)
#define TRACE_9(format,a1,a2,a3,a4,a5,a6,a7,a8,a9) PrintLine(format,a1,a2,a3,a4,a5,a6,a7,a8,a9)
#else
#define TRACE_0(format) 
#define TRACE_1(format,a1) 
#define TRACE_2(format,a1,a2) 
#define TRACE_3(format,a1,a2,a3) 
#define TRACE_4(format,a1,a2,a3,a4) 
#define TRACE_5(format,a1,a2,a3,a4,a5)
#define TRACE_6(format,a1,a2,a3,a4,a5,a6) 
#define TRACE_7(format,a1,a2,a3,a4,a5,a6,a7)
#define TRACE_8(format,a1,a2,a3,a4,a5,a6,a7,a8) 
#define TRACE_9(format,a1,a2,a3,a4,a5,a6,a7,a8,a9)
#endif



#	if defined(__cplusplus)
		}
#	endif


#endif
