/***************************************************************************
                          vector.h  -  description
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
#if !defined(HAVE_VECTOR)
#define HAVE_VECTOR

#include "common.h"

#if defined(__cplusplus)
extern "C" {
#endif

#define DEFAULT_CAPACITY 32

typedef struct {
    CADDR_T *elements;
    UINT capacity;
    UINT size;
} VECTOR;

typedef int (*VECTOR_COMPARE_FUNC_TYPE)( CADDR_T a, CADDR_T b );

VECTOR *CreateVector();
VECTOR *CreateVectorWithInitialCapacity( UINT capacity );
RET_VAL AddElementInVector( VECTOR *vector, CADDR_T element );
RET_VAL RemoveElementFromVector( VECTOR *vector, CADDR_T element );
RET_VAL RemoveElementByIndexFromVector( VECTOR *vector, UINT index );
BOOL FindElementInVector( VECTOR *vector, CADDR_T element, VECTOR_COMPARE_FUNC_TYPE func, UINT *index );
CADDR_T GetElementFromVector( VECTOR *vector, UINT index );
RET_VAL SortElementsInVector( VECTOR *vector, VECTOR_COMPARE_FUNC_TYPE func );
RET_VAL DeleteVector( VECTOR **vector );
UINT GetVectorSize( VECTOR *vector );

#if defined(__cplusplus)
}
#endif


#endif
