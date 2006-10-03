/***************************************************************************
                          vector.c  -  description
                             -------------------
    begin                : Sun Nov 2 2003
    copyright            : (C) 2003 by Hiroyuki Kuwahara
    email                : kuwahara@cs.utah.edu
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "vector.h"

RET_VAL _IncreaseCapacity( VECTOR *vector );
int _CompareElementInVector( CADDR_T a, CADDR_T b );

VECTOR *CreateVector() {               
    VECTOR *vector = NULL;
    
    START_FUNCTION("CreateVector");

    vector = CreateVectorWithInitialCapacity( DEFAULT_CAPACITY );
    if( vector == NULL ) {
        END_FUNCTION("CreateVector", FAILING );
        return NULL;
    }    
    else {
        END_FUNCTION("CreateVector", SUCCESS);
        return vector;
    }        
}

VECTOR *CreateVectorWithInitialCapacity( UINT capacity ) {
    VECTOR *vector = NULL;

    START_FUNCTION("CreateVectorWithInitialCapacity");

    vector = (VECTOR*)CALLOC( 1, sizeof(VECTOR) );
    if( vector == NULL ) {
        END_FUNCTION("CreateVectorWithInitialCapacity", FAILING );
        return NULL;
    }

    vector->capacity = capacity;
    vector->elements = (CADDR_T*)CALLOC( 1, capacity * sizeof(CADDR_T) );
    if( vector->elements == NULL ) {
        FREE( vector );
        END_FUNCTION("CreateVectorWithInitialCapacity", FAILING );
        return NULL;
    }    
                    
    END_FUNCTION("CreateVectorWithInitialCapacity", SUCCESS);
    return vector;
}


RET_VAL AddElementInVector( VECTOR *vector, CADDR_T element ) {
    RET_VAL ret = SUCCESS;
        
    START_FUNCTION("AddElementInVector");

    if( vector->size >= vector->capacity ) {
        if( IS_FAILED( ( ret = _IncreaseCapacity( vector ) ) ) ) {
            END_FUNCTION("AddElementInVector", ret );
            return ret;
        }
    }
    vector->elements[vector->size] = element;
    vector->size++;    

    END_FUNCTION("addElementInVector", ret );
    return ret;
}

RET_VAL RemoveElementFromVector( VECTOR *vector, CADDR_T element ) {
    RET_VAL ret = SUCCESS;
    UINT index = 0;
    
    START_FUNCTION("RemoveElementFromVector");

    if( !FindElementInVector( vector, element, _CompareElementInVector, &index ) ) {
        /*the vector does not contain the element */
        /* just return success */
        END_FUNCTION("RemoveElementFromVector", ret );
        return ret;
    }

    ret = RemoveElementByIndexFromVector( vector, index );
    END_FUNCTION("RemoveElementFromVector", ret );
    return ret;
}

BOOL FindElementInVector( VECTOR *vector, CADDR_T element, VECTOR_COMPARE_FUNC_TYPE func, UINT *index ) {
    UINT i = 0;
    CADDR_T target = NULL;
    
    START_FUNCTION("FindElementInVector");

    for( i = 0; i < vector->size; i++ ) {
        target = vector->elements[i];
        if( func( target, element ) == 0 ) {
            *index = i;
            END_FUNCTION("FindElementInVector", SUCCESS );
            return TRUE;
        }
    }
            
    END_FUNCTION("FindElementInVector", SUCCESS );
    return FALSE;
}


RET_VAL RemoveElementByIndexFromVector( VECTOR *vector, UINT index ) {
    RET_VAL ret = SUCCESS;
    UINT i = 0;
    
    START_FUNCTION("RemoveElementByIndexFromVector");

    if( index >= vector->size ) {
        END_FUNCTION("RemoveElementByIndexFromVector", ret );
        return ret;
    }
    if( index == (vector->size - 1) ) {
        vector->elements[index] = NULL;
        vector->size--;
        END_FUNCTION("RemoveElementByIndexFromVector", ret );
        return ret;
    }    

    for( i = index + 1; i < vector->size; i++ ) {
        vector->elements[i - 1] = vector->elements[i];
    }
    vector->elements[vector->size-1] = NULL;
    vector->size--;
    END_FUNCTION("RemoveElementByIndexFromVector", ret );
    return ret;
}


CADDR_T GetElementFromVector( VECTOR *vector, UINT index ) {

    START_FUNCTION("GetElementFromVector");

    if( index >= vector->size ) {
        END_FUNCTION("GetElementFromVector", SUCCESS );
        return NULL;
    }
      
    END_FUNCTION("GetElementFromVector", SUCCESS );
    return vector->elements[index];
}


RET_VAL SortElementsInVector( VECTOR *vector, VECTOR_COMPARE_FUNC_TYPE func ) {
    RET_VAL ret = SUCCESS;

    START_FUNCTION("SortElementsInVector");

    qsort( vector->elements, vector->size, sizeof( CADDR_T ),
           (int(*)(const void *, const void *))func );

    END_FUNCTION("SortElementsInVector", ret );
    return ret;
}

UINT GetVectorSize( VECTOR *vector ) {
    START_FUNCTION("GetVectorSize");

    END_FUNCTION("GetVectorSize", SUCCESS );
    return vector->size;
}


RET_VAL DeleteVector( VECTOR **vector ) {
    RET_VAL ret = SUCCESS;

    START_FUNCTION("RemoveElementByIndexFromVector");

    FREE( (*vector)->elements );
    FREE( *vector );    
    
    END_FUNCTION("RemoveElementByIndexFromVector", ret );
    return ret;
}


RET_VAL _IncreaseCapacity( VECTOR *vector ) {
    RET_VAL ret = SUCCESS;
    UINT newCapacity = 0;
    CADDR_T *elements = NULL;
    
    START_FUNCTION("_increaseCapacity");

    newCapacity = vector->capacity << 1;
    elements = vector->elements;
    vector->elements = (CADDR_T*)CALLOC( 1, newCapacity * sizeof(CADDR_T) );
    if( vector->elements == NULL ) {
        vector->elements = elements;
        return ErrorReport( FAILING, "_increaseCapacity", "cannot increase capacity to %u", newCapacity );
    }
    memcpy( vector->elements, elements, vector->size * sizeof(CADDR_T) );
    FREE( elements );
    vector->capacity = newCapacity;
    END_FUNCTION("_increaseCapacity", ret );
    return ret;
}

int _CompareElementInVector( CADDR_T a, CADDR_T b ) {
    return (int)b - (int)a;
}


