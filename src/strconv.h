/**********************************************************************


Copyright (C) 1999  Hiroyuki Kuwahara   All rights reserved

FILE NAME:      			strconv.h

DATE:           			6/2/99	

DESIGNER NAME:  			Hiroyuki Kuwahara	hkuwahara@yahoo.com


**********************************************************************/
#if !defined(HAVE_STRCONV)
#	define 	HAVE_STRCONV

#	include <stdlib.h>
#	include <string.h>
#	include "type.h"

#	if defined(__cplusplus)
		extern "C" {
#	endif

	RET_VAL StrToINT16( INT16*, LPCSTR );
	RET_VAL StrToINT32( INT32*, LPCSTR );
	RET_VAL StrToFloat( double*, LPCSTR );

	RET_VAL StrToUINT16( UINT16*, LPCSTR );
	RET_VAL StrToUINT32( UINT32*, LPCSTR );
	
#	if defined(__cplusplus)
		}
#	endif



#endif
