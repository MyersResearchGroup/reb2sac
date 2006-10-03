/**********************************************************************


Copyright (C) 1999  Hiroyuki Kuwahara   All rights reserved

FILE NAME:      			log.c

DATE:           			6/2/99	

PROGRAMMER NAME:			Hiroyuki Kuwahara	hkuwahara@yahoo.com

HISTORY:

  September 6, 2000 -- ADD ERROR ID handling


**********************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include "log.h"


static FILE *log_file_handle = NULL;

/**********************************************************************

DATE:           			6/2/99	

PROGRAMMER NAME:			Hiroyuki Kuwahara	

FUNCTION NAME:  			StartLog	

PARAMETERS:     	

RETURN TYPE:    			RET_VAL	



DESCRIPTION:			


**********************************************************************/
RET_VAL StartLog(LPCSTR fn )		 		
{
	
#if defined(TAKE_LOG)
	char current_time[TIME_SIZE];
	char file_name[FILE_NAME_SIZE];
	size_t length = 0;
	time_t timer;
	struct tm *lct = localtime(&timer);

	memset( file_name, 0, FILE_NAME_SIZE );
	length = strlen(fn);
	if( ( length + strlen(LOG_SUFFIX) ) >= FILE_NAME_SIZE ) {
		length  = FILE_NAME_SIZE - strlen(LOG_SUFFIX) - 1;
	}

	memcpy( file_name, fn, length );
	strcpy( file_name + length, LOG_SUFFIX ); 

	if( (log_file_handle = fopen( file_name, "w") ) == NULL ) {
		fprintf( stderr, "File open error\n"); 				
		return E_FILE_OPEN | E_LOG_FILE;
	}
/*	
	memset( current_time, 0, TIME_SIZE );
	timer = time(NULL);
	strftime( current_time, TIME_SIZE, "DATE: %x. TIME: %X\n\n", lct ); 
	fputs( current_time, log_file_handle );
*/
#endif
	return SUCCESS;
}

/**********************************************************************

DATE:           			6/2/99	

PROGRAMMER NAME:			Hiroyuki Kuwahara	

FUNCTION NAME:  			TakeLog	

PARAMETERS:     	
		LPCSTR			file
		LPCSTR			func
		LPCSTR			status

RETURN TYPE:    			void	



DESCRIPTION:			


**********************************************************************/
void TakeLog( LPCSTR file, LPCSTR func, LPCSTR status )
{
#if defined(TAKE_LOG)
	fprintf( log_file_handle, "File: %s   Function: %s  Status: %s\n", 
		            file, func, status ); 
	fflush( log_file_handle );
#endif

#if 0
	printf( "File: %s   Function: %s  Status: %s\n", 
		            file, func, status ); 
#endif

}


/**********************************************************************

DATE:           			6/2/99	

PROGRAMMER NAME:			Hiroyuki Kuwahara	

FUNCTION NAME:  			EndLog	

PARAMETERS:     	
		LPCSTR			file
		LPCSTR			func
		LPCSTR			status

RETURN TYPE:    			void	



DESCRIPTION:			


**********************************************************************/
void EndLog() 
{
#if defined(TAKE_LOG)
	fclose(log_file_handle);
#endif
}

/**********************************************************************

DATE:           			6/2/99	

PROGRAMMER NAME:			Hiroyuki Kuwahara	

FUNCTION NAME:  			ErrorReport	

PARAMETERS:     	
		RET_VAL			ret
		LPCSTR 			func
		LPCSTR 			format

RETURN TYPE:    			RET_VAL	



DESCRIPTION:			


**********************************************************************/
RET_VAL ErrorReport( RET_VAL ret, LPCSTR func, LPCSTR format, ... )
{
/*
#ifndef DEBUG 
#	ifndef TAKE_LOG 
		return ret;
#	endif
#endif
*/
#if 0
	char buf[ERRORREPORT_SIZE];
	char temp[ERRORREPORT_SIZE];
#endif
	va_list args;


	va_start( args, format );
	
#if 0
        memset( temp, 0, ERRORREPORT_SIZE);

	vsprintf( temp, format, args );
	temp[ERRORREPORT_SIZE-1] = '\0';
#endif

#if 0
	SetReport( buf, ret, temp );
        fprintf( stderr, "%s\n", buf );
#endif
        fprintf( stderr, "%s: %s: \t",  GetErrorID1(ret), GetErrorID2(ret) );
        vfprintf( stderr, format, args );
        fprintf( stderr, NEW_LINE );
                
#if defined(TAKE_LOG)
        fprintf( log_file_handle, "\t%s: %s: \t",  GetErrorID1(ret), GetErrorID2(ret) );
        vfprintf( log_file_handle, format, args );
        fprintf( log_file_handle, NEW_LINE );
#if 0
        fprintf( log_file_handle, "\t%s\n", buf );
	fflush( log_file_handle );
#endif
#endif
	va_end(args);
	END_FUNCTION( func, ret );
	return ret;
}


/**********************************************************************

DATE:           			6/2/99	

PROGRAMMER NAME:			Hiroyuki Kuwahara	

FUNCTION NAME:  			SetReport	

PARAMETERS:     	
		LPSTR  			buf
		RET_VAL			ret
		LPCSTR 			temp

RETURN TYPE:    			void	



DESCRIPTION:			


**********************************************************************/
void SetReport( LPSTR buf, RET_VAL ret, LPCSTR temp )
{

	memset( buf, 0, ERRORREPORT_SIZE);
	sprintf( buf, "%s:  %s:  \t%s", GetErrorID1(ret), GetErrorID2(ret), temp );
	buf[ERRORREPORT_SIZE-1] = '\0';
}


/**********************************************************************

DATE:           			6/2/99	

PROGRAMMER NAME:			Hiroyuki Kuwahara	

FUNCTION NAME:  			GetErrorID1	

PARAMETERS:     	
		RET_VAL			ret

RETURN TYPE:    			LPSTR	



DESCRIPTION:			


**********************************************************************/
LPSTR GetErrorID1( RET_VAL ret )
{
	
	switch( ret & E_ID1_MASK ) {
		case E_FILE_OPEN:
		return "File open error";	

		case E_OVERFLOW:	
		return "Over flow";

		case E_UNDERFLOW:
		return "Under flow";

		case E_WRONGDATA:
		return "Wrong data type";

		case E_NODATA:
		return "No data";

		case E_ALLOCATION:
		return "Allocation Failed";

		case E_UNEXPECTED_VALUE:
		return "Unexpected value";

		case E_FORK: 
		return "Fork failed";
		
		case E_NULL_INPUT: 
		return "Null input";

		case E_PIPE: 
		return "Pipe error";
		

		default:
		return " ";	
	}
}

/**********************************************************************

DATE:           			6/2/99	

PROGRAMMER NAME:			Hiroyuki Kuwahara	

FUNCTION NAME:  			GetErrorID2	

PARAMETERS:     	
		RET_VAL			ret

RETURN TYPE:    			LPSTR	



DESCRIPTION:			


**********************************************************************/
LPSTR GetErrorID2( RET_VAL ret )
{
	switch( ret & E_ID2_MASK ) {
		case E_LOG_FILE:
		return "Log File";	

		case E_CONVERT_STRING:	
		return "While converting string";

		case E_NOT_ENOUGH_SPACE:
		return "Not Enough Space";

		case E_ZERO:
		return "Zero is input";

		case E_NOT_HEX:
		return "Not Hex Value";
		default:
		return " ";	
	}
}





void TakeReport( LPCSTR format, ... ) {
    va_list args;
    
    
    va_start( args, format );
    
    fprintf( log_file_handle, "\t" );
    vfprintf( log_file_handle, format, args );
    fprintf( log_file_handle, NEW_LINE );    
    fflush( log_file_handle );
    
    va_end(args);
}

void PrintLine(LPCSTR  format, ... ) {
    va_list args;
        
    va_start( args, format );
    
    vprintf( format, args );
    printf( NEW_LINE ); 
    fflush( stdout );   
    
    va_end(args);
}




