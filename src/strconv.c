/**********************************************************************


Copyright (C) 1999  Hiroyuki Kuwahara   All rights reserved

FILE NAME:      			strconv.cpp

DATE:           			6/2/99	

PROGRAMMER NAME:			Hiroyuki Kuwahara	hkuwahara@yahoo.com


**********************************************************************/
#include "strconv.h"
#include "log.h"

/**********************************************************************

DATE:           			6/2/99	

PROGRAMMER NAME:			Hiroyuki Kuwahara	

FUNCTION NAME:  			StrToINT16	

PARAMETERS:     	
		INT16 *			num
		LPCSTR 			str

RETURN TYPE:    			RET_VAL	



DESCRIPTION:			


**********************************************************************/
RET_VAL StrToINT16( INT16 *num, LPCSTR str )
{
	LPSTR endp = NULL;
	INT32 l = 0;
	
	START_FUNCTION("StrToINT16");

	if( StrToINT32( &l, str) != SUCCESS ) {
		return ErrorReport( FAILING, "StrToINT16", "%s", str ); 

	}
	if( ( l > (INT32)INT16_MAX ) || ( l < (INT32)INT16_MIN ) ) {
		return ErrorReport( E_CONVERT_STRING | E_OVERFLOW, "StrToINT16", "%s", str); 

	}

	*num = (INT16)l;
	END_FUNCTION("StrToINT16", SUCCESS);
	return SUCCESS; 
}
	
/**********************************************************************

DATE:           			6/2/99	

PROGRAMMER NAME:			Hiroyuki Kuwahara	

FUNCTION NAME:  			StrToINT32	

PARAMETERS:     	
		INT32 *			num
		LPCSTR 			str

RETURN TYPE:    			RET_VAL	



DESCRIPTION:			


**********************************************************************/
RET_VAL StrToINT32( INT32 *num, LPCSTR str )
{
	LPSTR endp = NULL;

	START_FUNCTION("StrToINT32");
	*num = strtol( str, &endp, 0 );
	if( *endp != '\0' ) {
		return ErrorReport( E_CONVERT_STRING | E_WRONGDATA, "StrToINT32", "%s", str); 
	}
	END_FUNCTION("StrToINT32", SUCCESS );
	return SUCCESS;
}

/**********************************************************************

DATE:           			6/2/99	

PROGRAMMER NAME:			Hiroyuki Kuwahara	

FUNCTION NAME:  			StrToFloat	

PARAMETERS:     	
		double *			num
		LPCSTR  			str

RETURN TYPE:    			RET_VAL	



DESCRIPTION:			


**********************************************************************/
RET_VAL StrToFloat( double *num, LPCSTR str )
{
	LPSTR endp = NULL;

	START_FUNCTION("StrToFloat");

	*num = strtod( str, &endp );
	if( *endp != '\0' ) {
		return ErrorReport( E_CONVERT_STRING | E_WRONGDATA, "StrToFLOAT", "%s", str); 
	}
	END_FUNCTION("StrToFloat", SUCCESS );
	return SUCCESS;
}

/**********************************************************************

DATE:           			6/2/99	

PROGRAMMER NAME:			Hiroyuki Kuwahara	

FUNCTION NAME:  			StrToUINT16	

PARAMETERS:     	
		UINT16 *			num
		LPCSTR  			str

RETURN TYPE:    			RET_VAL	



DESCRIPTION:			


**********************************************************************/
RET_VAL StrToUINT16( UINT16 *num, LPCSTR str )
{
	LPSTR endp = NULL;
	UINT32 l = 0;

	START_FUNCTION("StrToUINT16");

	if( StrToUINT32( &l, str) != SUCCESS ) {
		return ErrorReport( FAILING, "StrToUINT16", "%s", str); 

	}
	if( ( l > (UINT32)INT16_MAX ) || ( l < (UINT32)INT16_MIN ) ) {
		return ErrorReport( E_CONVERT_STRING | E_OVERFLOW, "StrToUINT16", "%s", str); 

	}

	*num = (UINT16)l;
	END_FUNCTION("StrToUINT16", SUCCESS );
	return SUCCESS;
}

/**********************************************************************

DATE:           			6/2/99	

PROGRAMMER NAME:			Hiroyuki Kuwahara	

FUNCTION NAME:  			StrToUINT32	

PARAMETERS:     	
		UINT32 *			num
		LPCSTR  			str

RETURN TYPE:    			RET_VAL	



DESCRIPTION:			


**********************************************************************/
RET_VAL StrToUINT32( UINT32 *num, LPCSTR str )
{
	LPSTR endp = NULL;

	START_FUNCTION("StrToUINT32");

	*num = strtoul( str, &endp, 0 );
	if( *endp != '\0' ) {
		return ErrorReport( E_CONVERT_STRING | E_WRONGDATA, "StrToUINT32", "%s", str); 
	}
	END_FUNCTION("StrToUINT32", SUCCESS );
	return SUCCESS;
}





