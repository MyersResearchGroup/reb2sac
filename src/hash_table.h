/**********************************************************************


FILE NAME:      			hash_table.h

DATE:           			9/12/2002	

DESIGNER NAME:  			Hiroyuki Kuwahara	hkuwahara@cs.utah.com

Interface for hash_table API

**********************************************************************/
#if !defined(HAVE_HASH_TABLE)
#define HAVE_HASH_TABLE
	
#include "common.h"
#include "linked_list.h"




#if defined(__cplusplus)
	extern "C" {
#endif

		typedef struct {
			LINKED_LIST **buckets;
			UINT32 bucket_size;
			UINT32 entry_count;
		} HASH_TABLE;
		
		typedef struct {
			CADDR_T key;
			UINT32 key_size;
			CADDR_T value;
		} HASH_ENTRY;


		DLLSCOPE HASH_TABLE * STDCALL CreateHashTable( UINT32 bucket_size );
		DLLSCOPE RET_VAL  STDCALL PutInHashTable( CADDR_T key, UINT32 key_size, CADDR_T value, HASH_TABLE *table );
		DLLSCOPE CADDR_T  STDCALL GetValueFromHashTable( CADDR_T key, UINT32 key_size, HASH_TABLE *table );
		DLLSCOPE RET_VAL  STDCALL RemoveFromHashTable( CADDR_T key, UINT32 key_size, HASH_TABLE *table );
		DLLSCOPE LINKED_LIST * STDCALL GenerateKeyList( HASH_TABLE *table );
		DLLSCOPE LINKED_LIST * STDCALL GenerateValueList( HASH_TABLE *table );
		DLLSCOPE UINT32  STDCALL GetHashEntryCount( HASH_TABLE *table );
		DLLSCOPE BOOL  STDCALL ExistInHashTable( CADDR_T key, UINT32 key_size, HASH_TABLE *table );	
		DLLSCOPE RET_VAL  STDCALL DeleteHashTable( HASH_TABLE **table );


#	if defined(__cplusplus)
	}
#	endif


#endif
