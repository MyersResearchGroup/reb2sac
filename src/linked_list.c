/*******************************************************************************


  This is a linked list API  


*******************************************************************************/
#if defined(DEBUG)
#undef DEBUG
#endif

#include "linked_list.h"


LINKED_LIST *CreateLinkedList()
{
	LINKED_LIST *list = NULL;
	
	START_FUNCTION("CreateLinkedList");

	list = (LINKED_LIST*)CALLOC( 1, sizeof(LINKED_LIST) );
	if( list == NULL ) {
		END_FUNCTION("CreateLinkedList", FAILING );
		return NULL;
	}
	END_FUNCTION("CreateLinkedList", SUCCESS );

	return list;
}

LINKED_LIST *CloneLinkedList( LINKED_LIST *list )
{
	LINKED_LIST *new_list = NULL;
	CADDR_T element = NULL;
	LINKED_LIST_ELEMENT *current = NULL;	
	
	START_FUNCTION("CloneLinkedList");

	new_list = (LINKED_LIST*)CALLOC( 1, sizeof(LINKED_LIST) );
	if( new_list == NULL ) {
		END_FUNCTION("CloneLinkedList", FAILING );
		return NULL;
	}
	
	/*save current*/
	current = list->current;
	
	ResetCurrentElement( list );
	while( ( element = GetNextFromLinkedList(list) ) != NULL ) {
		if( IS_FAILED( AddElementInLinkedList( element, new_list ) ) ) {
			END_FUNCTION("CloneLinkedList", FAILING );
			return NULL;
		}
	}
	list->current = current;
	new_list->current = current;

	END_FUNCTION("CloneLinkedList", SUCCESS );

	return new_list;
}


RET_VAL AddElementInLinkedList( CADDR_T element, LINKED_LIST *list )
{
		
	RET_VAL ret = SUCCESS;
	LINKED_LIST_ELEMENT *list_element = NULL;	

	START_FUNCTION("AddElementInLinkedList");

	list_element = (LINKED_LIST_ELEMENT*)CALLOC( 1, sizeof(LINKED_LIST_ELEMENT) );
	if( list_element == NULL ) {
		return ErrorReport( FAILING, "AddElementInLinkedList", 
					"Cannot allocate Linked list elements" ); 
	}
	list_element->element = element;

	if( list->head == NULL ) {
		list->head = list_element;
		list->tail = list_element;
	}
	else {
		list->tail->next = list_element;
		list_element->previous = list->tail;
		list->tail = list_element;
	}
	list->size++;
	END_FUNCTION("CreateLinkedList", ret );
	return ret;
}
			
RET_VAL InsertHeadInLinkedList( CADDR_T element, LINKED_LIST *list )
{
	RET_VAL ret = SUCCESS;

	LINKED_LIST_ELEMENT *list_element = NULL;	

	START_FUNCTION("InsertHeadInLinkedList");

	list_element = (LINKED_LIST_ELEMENT*)CALLOC( 1, sizeof(LINKED_LIST_ELEMENT) );
	if( list_element == NULL ) {
		return ErrorReport( FAILING, "InsertHeadInLinkedList", 
					"Cannot allocate Linked list elements" ); 
	}
	list_element->element = element;

	if( list->head == NULL ) {
		list->head = list_element;
		list->tail = list_element;
		list->current = list_element;
	}
	else {
		list->head->previous = list_element;
		list_element->next = list->head;
		list->head = list_element;
	}

	list->size++;
	END_FUNCTION("InsertHeadInLinkedList", ret );
	return ret;

}
			
RET_VAL RemoveElementFromLinkedListByIndex( UINT32 index, LINKED_LIST *list )
{
	RET_VAL ret = SUCCESS;
	UINT32 i = 0;
	LINKED_LIST_ELEMENT *list_element = NULL;

	START_FUNCTION("RemoveElementFromLinkedListByIndex");

	if(  index >= list->size ) {
		return ErrorReport( WARNING, "RemoveElementFromLinkedListByIndex", "bad index" );
	}
		
	/*case when head == tail*/
	if( list->size == 1 ) {
		FREE( list->head );
		/*reinitialize the list*/		
		BZERO( list, sizeof(LINKED_LIST) );
		END_FUNCTION("RemoveElementFromLinkedListByIndex", ret );
		return ret;
	}

	
	list_element = list->head;
	for( i = 1; i < index; i++ ) {
		list_element = list_element->next;				
	}
	
	if( list_element == list->head ) {
		if( list->current == list->head ) {
			list->current = list->head->next;
		}
		list->head = list->head->next;
		list->head->previous = NULL;
	}
	else if( list_element == list->tail ) {
		if( list->current == list->tail ) {
			list->current = list->head;
		}
		list->tail = list->tail->previous;
		list->tail->next = NULL;
	}
	else {
		if( list->current == list_element ) {
			list->current = list->current->next;
		}
		list_element->next->previous = list_element->previous;
		list_element->previous->next = list_element->next;
	}

	list->size--;
	FREE( list_element );
	
	END_FUNCTION("RemoveElementFromLinkedListByIndex", ret );
	return ret;
}


RET_VAL RemoveElementFromLinkedList( CADDR_T element, LINKED_LIST *list )
{
	RET_VAL ret = SUCCESS;
	UINT32 i = 0;
	LINKED_LIST_ELEMENT *list_element = NULL;
    CADDR_T current = NULL;

    START_FUNCTION("RemoveElementFromLinkedList");
    if( list->current != NULL ) {
        if( list->current->element == element ) {
            ret = RemoveCurrentFromLinkedList( list );
            END_FUNCTION("RemoveElementFromLinkedList", ret );
                return ret;
        }
    }
    list_element = list->current;        

    list->current = NULL;
    while( ( current = GetNextFromLinkedList( list ) ) != NULL ) {
        if( current == element ) {
            ret = RemoveCurrentFromLinkedList( list );
            list->current = list_element;
            END_FUNCTION("RemoveElementFromLinkedList", ret );
	        return ret;
        }
    }
    list->current = list_element;
    END_FUNCTION("RemoveElementFromLinkedList", ret );
	return ret;
}




RET_VAL RemoveCurrentFromLinkedList( LINKED_LIST *list )
{
	RET_VAL ret = SUCCESS;
	LINKED_LIST_ELEMENT *list_element = NULL;

	START_FUNCTION("RemoveCurrentFromLinkedList");

		
	if( list->current == NULL ) {
		END_FUNCTION("RemoveCurrentFromLinkedList", ret );
		return ret;
	}

	
	/*case when head == tail*/
	if( list->size == 1 ) {
		FREE( list->head );
		/*reinitialize the list*/		
		BZERO( list, sizeof(LINKED_LIST) );
		END_FUNCTION("RemoveCurrentFromLinkedList", ret );
		return ret;
	}

	
	list_element = list->current;
	
	if( list_element == list->head ) {
		if( list->current == list->head ) {
			list->current = NULL;
		}
		list->head = list->head->next;
		list->head->previous = NULL;
	}
	else if( list_element == list->tail ) {
		if( list->current == list->tail ) {
			list->current = list->tail->previous;
		}
		list->tail = list->tail->previous;
		list->tail->next = NULL;
	}
	else {
		if( list->current == list_element ) {
			list->current = list->current->previous;
		}
		list_element->next->previous = list_element->previous;
		list_element->previous->next = list_element->next;
	}

	list->size--;
	FREE( list_element );
	
	END_FUNCTION("RemoveCurrentFromLinkedList", ret );
	return ret;
}



RET_VAL RemoveHeadFromLinkedList( LINKED_LIST *list )
{
	RET_VAL ret = SUCCESS;

	LINKED_LIST_ELEMENT *list_element = NULL;

	START_FUNCTION("RemoveHeadFromLinkedList");

	if( list->size == 0 ) {
		END_FUNCTION("RemoveHeadFromLinkedList", ret );
		return ret;
	}
	
	/*case when head == tail*/
	if( list->size == 1 ) {
		FREE( list->head );
		/*reinitialize the list*/		
		BZERO( list, sizeof(LINKED_LIST) );
		END_FUNCTION("RemoveHeadFromLinkedList", ret );
		return ret;
	}

	list_element = list->head;
	
	if( list->current == list->head ) {
		list->current = NULL;
	}
	list->head = list->head->next;
	list->head->previous = NULL;

	list->size--;
	FREE( list_element );
	
	END_FUNCTION("RemoveHeadFromLinkedList", ret );

	return ret;
}



RET_VAL RemoveTailFromLinkedList( LINKED_LIST *list )
{
	RET_VAL ret = SUCCESS;

	LINKED_LIST_ELEMENT *list_element = NULL;

	START_FUNCTION("RemoveTailFromLinkedList");

	if( list->size == 0 ) {
		END_FUNCTION("RemoveTailFromLinkedList", ret );
		return ret;
	}
			
	/*case when head == tail*/
	if( list->size == 1 ) {
		FREE( list->head );
		/*reinitialize the list*/		
		BZERO( list, sizeof(LINKED_LIST) );
		END_FUNCTION("RemoveTailFromLinkedList", ret );
		return ret;
	}

	list_element = list->tail;
	
	if( list->current == list->tail ) {
		list->current = list->tail->previous;
	}
	list->tail = list->tail->previous;
	list->tail->next = NULL;

	list->size--;
	FREE( list_element );
	
	END_FUNCTION("RemoveTailFromLinkedList", ret );

	return ret;
}

RET_VAL ResetCurrentElement( LINKED_LIST *list )
{
	RET_VAL ret = SUCCESS;
	
	START_FUNCTION("ResetCurrentElement");
	list->current = NULL;
	END_FUNCTION("ResetCurrentElement", ret );
	return ret;
}


CADDR_T GetNextFromLinkedList( LINKED_LIST *list )
{

	START_FUNCTION("GetNextFromLinkedList");
	if( list->current == NULL ) {
		list->current = list->head;
		END_FUNCTION("GetNextFromLinkedList", SUCCESS );
		return (list->current == NULL) ? NULL : list->current->element;
	}
	
	list->current = list->current->next;
	
	if( list->current == NULL ) {
		END_FUNCTION("GetNextFromLinkedList", SUCCESS );
		return NULL;
	}
	
	END_FUNCTION("GetNextFromLinkedList", SUCCESS );
	return list->current->element;
}

DLLSCOPE CADDR_T  STDCALL GetCurrentFromLinkedList( LINKED_LIST *list ) {
    LINKED_LIST_ELEMENT *current = list->current;

	START_FUNCTION("GetCurrentFromLinkedList");
	END_FUNCTION("GetCurrentFromLinkedList", SUCCESS );

	return (current == NULL) ? NULL : current->element;
}



CADDR_T GetHeadFromLinkedList( LINKED_LIST *list )
{
	START_FUNCTION("GetHeadFromLinkedList");
	if( list->head == NULL ) {
		END_FUNCTION("GetHeadFromLinkedList", SUCCESS );
		return NULL;
	}
			
	END_FUNCTION("GetHeadFromLinkedList", SUCCESS );
	return list->head->element;
}


CADDR_T GetTailFromLinkedList( LINKED_LIST *list )
{
	START_FUNCTION("GetTailFromLinkedList");
	if( list->tail == NULL ) {
		END_FUNCTION("GetTailFromLinkedList", SUCCESS );
		return NULL;
	}
			
	END_FUNCTION("GetTailFromLinkedList", SUCCESS );
	return list->tail->element;
}


CADDR_T GetElementByIndex( UINT32 index, LINKED_LIST *list )
{

	UINT32 i = 0;
	LINKED_LIST_ELEMENT *list_element = NULL;

	START_FUNCTION("GetElementByIndex");

	if( index >= list->size ) {
		END_FUNCTION("GetElementByIndex", WARNING );
		return NULL;
	}
	
	list_element = list->head;
	
	for( i = 0; i < index; i++ ) {
		list_element = list_element->next;
	}
	END_FUNCTION("GetElementByIndex", SUCCESS );
	return list_element->element;
}


UINT32  GetLinkedListSize( LINKED_LIST *list )
{
	START_FUNCTION("GetLinkedListSize");
	END_FUNCTION("GetLinkedListSize", SUCCESS );
	return list->size;
}

int FindElementInLinkedList( CADDR_T element, COMPARE_FUNC_TYPE func, LINKED_LIST *list )
{
    int i = 0;
    LINKED_LIST_ELEMENT *list_element = NULL;
    CADDR_T target = NULL;
    
    START_FUNCTION("FindElementInLinkedList");

    for( list_element = list->head; list_element != NULL; list_element = list_element->next ) {
        target = list_element->element;
        if( func( element, target ) == 0 ) {
            END_FUNCTION("FindElementInLinkedList", SUCCESS );
            return i;             
        }
        i++;             
    }
    END_FUNCTION("FindElementInLinkedList", SUCCESS );
	return -1;
}

DLLSCOPE RET_VAL STDCALL SetAsCurrentInLinkedList( CADDR_T newCurrent, LINKED_LIST *list )
{
    LINKED_LIST_ELEMENT *list_element = NULL;
    CADDR_T target = NULL;
    
    START_FUNCTION("SetAsCurrentInLinkedList");

    if( newCurrent == NULL ) {
        list->current = NULL;
        END_FUNCTION("SetAsCurrentInLinkedList", SUCCESS );
        return SUCCESS;             
    }

    for( list_element = list->head; list_element != NULL; list_element = list_element->next ) {
        target = list_element->element;
        if( target == newCurrent ) {
            list->current = list_element;
            END_FUNCTION("SetAsCurrentInLinkedList", SUCCESS );
            return SUCCESS;             
        }
    }
    return ErrorReport( FAILING, "SetAsCurrentInLinkedList", "Cannot seet the input new current element" ); 
}


RET_VAL UpdateElementByIndex( UINT32 index, CADDR_T element, LINKED_LIST *list )
{
	RET_VAL ret = SUCCESS;
	UINT32 i = 0;
	LINKED_LIST_ELEMENT *list_element = NULL;

	START_FUNCTION("UpdateElementByIndex");

	if(  index >= list->size ) {
		return ErrorReport( WARNING, "UpdateElementByIndex", "bad index" );
	}
		
	
	list_element = list->head;
	for( i = 1; i < index; i++ ) {
		list_element = list_element->next;				
	}
	list_element->element = element;
	
	END_FUNCTION("UpdateElementByIndex", ret );
	return ret;
}


RET_VAL UpdateCurrentElement( CADDR_T element, LINKED_LIST *list )
{
	RET_VAL ret = SUCCESS;

	START_FUNCTION("UpdateCurrentElement");

	if( list->current == NULL ) {
		return ErrorReport( FAILING, "UpdateCurrentElement", "can't update current element" );
	}

	list->current->element = element;
	END_FUNCTION("UpdateCurrentElement", ret );
	return ret;
}



RET_VAL DeleteLinkedList( LINKED_LIST **list )
{
	RET_VAL ret = SUCCESS;
	LINKED_LIST_ELEMENT *list_element = NULL;
	LINKED_LIST_ELEMENT *target = NULL;
	
	START_FUNCTION("DeleteLinkedList");

	if( *list == NULL ) {
		END_FUNCTION("DeleteLinkedList", ret );
		return ret;
	}
	
	list_element = (*list)->head;
	while( list_element != NULL ) {
		target = list_element;
		list_element = list_element->next;
		FREE(target);
	}
	FREE(*list);
	END_FUNCTION("DeleteLinkedList", ret );
	return ret;
}



