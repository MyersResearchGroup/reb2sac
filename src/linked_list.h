/**********************************************************************


FILE NAME:      			linked_list.h

DATE:           			9/12/2002	

DESIGNER NAME:  			Hiroyuki Kuwahara	kuwahara@cs.utah.com

Interface for linked list API

**********************************************************************/
#if !defined(HAVE_LINKED_LIST)
#define HAVE_LINKED_LIST
	
#include "common.h"


#if defined(__cplusplus)
	extern "C" {
#endif

		struct _LINKED_LIST_ELEMENT {
			CADDR_T element;
			struct _LINKED_LIST_ELEMENT *next;
			struct _LINKED_LIST_ELEMENT *previous;		
		};
		
		typedef struct _LINKED_LIST_ELEMENT LINKED_LIST_ELEMENT;


		typedef struct {
			LINKED_LIST_ELEMENT *head;
			LINKED_LIST_ELEMENT *tail;
			LINKED_LIST_ELEMENT *current;
			UINT32 size;
		} LINKED_LIST;
	
        typedef STDCALL int (*COMPARE_FUNC_TYPE)( CADDR_T a, CADDR_T b );
    
		DLLSCOPE LINKED_LIST * STDCALL CreateLinkedList();
		DLLSCOPE LINKED_LIST * STDCALL CloneLinkedList( LINKED_LIST *list );
		DLLSCOPE RET_VAL  STDCALL AddElementInLinkedList( CADDR_T element, LINKED_LIST *list );
		DLLSCOPE RET_VAL  STDCALL InsertHeadInLinkedList( CADDR_T element, LINKED_LIST *list );
		DLLSCOPE RET_VAL  STDCALL RemoveElementFromLinkedListByIndex( UINT32 index, LINKED_LIST *list );
		DLLSCOPE RET_VAL  STDCALL RemoveElementFromLinkedList( CADDR_T element, LINKED_LIST *list );
		DLLSCOPE RET_VAL  STDCALL RemoveCurrentFromLinkedList( LINKED_LIST *list );
		DLLSCOPE RET_VAL  STDCALL RemoveHeadFromLinkedList( LINKED_LIST *list );
		DLLSCOPE RET_VAL  STDCALL RemoveTailFromLinkedList( LINKED_LIST *list );
		DLLSCOPE RET_VAL  STDCALL ResetCurrentElement( LINKED_LIST *list );
		DLLSCOPE CADDR_T  STDCALL GetNextFromLinkedList( LINKED_LIST *list );
		DLLSCOPE CADDR_T  STDCALL GetCurrentFromLinkedList( LINKED_LIST *list );
		DLLSCOPE CADDR_T  STDCALL GetHeadFromLinkedList( LINKED_LIST *list );
		DLLSCOPE CADDR_T  STDCALL GetTailFromLinkedList( LINKED_LIST *list );
		DLLSCOPE CADDR_T  STDCALL GetElementByIndex( UINT32 index, LINKED_LIST *list );
		DLLSCOPE RET_VAL  STDCALL UpdateElementByIndex( UINT32 index, CADDR_T element, LINKED_LIST *list );  
		DLLSCOPE RET_VAL  STDCALL UpdateCurrentElement( CADDR_T element, LINKED_LIST *list );
		DLLSCOPE RET_VAL  STDCALL SetAsCurrentInLinkedList( CADDR_T newCurrent, LINKED_LIST *list );
		DLLSCOPE RET_VAL  STDCALL DeleteLinkedList( LINKED_LIST **list );
		DLLSCOPE UINT32   STDCALL GetLinkedListSize( LINKED_LIST *list );
        DLLSCOPE int  STDCALL FindElementInLinkedList( CADDR_T element, COMPARE_FUNC_TYPE func, LINKED_LIST *list );

#	if defined(__cplusplus)
	}
#	endif


#endif
