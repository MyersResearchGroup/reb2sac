/***************************************************************************
 *   Copyright (C) 2004 by Hiroyuki Kuwahara                               *
 *   kuwahara@cs.utah.edu                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "nary_order_decider.h"

static RET_VAL _DecideDistinctOrder( ABSTRACTION_METHOD *method, CRITICAL_CONCENTRATION_INFO *info, double *criticalCons, int len );
static int _CompareConcentrationElements( double *e1, double *e2 );

NARY_ORDER_DECIDER *GetNaryOrderDeciderInstance( ABSTRACTION_METHOD_MANAGER *manager ) {
    static NARY_ORDER_DECIDER decider;
    char *deciderName = NULL;
    char buf[256];
    COMPILER_RECORD_T *record = NULL;
    REB2SAC_PROPERTIES *properties = NULL;
    
    START_FUNCTION("GetNaryOrderDeciderInstance");
    
    record = manager->GetCompilerRecord( manager );
    properties = record->properties;
    
    if( ( deciderName = properties->GetProperty( properties, REB2SAC_NARY_ORDER_DECIDER_KEY ) ) == NULL ) {
        deciderName = REB2SAC_DEFAULT_NARY_ORDER_DECIDER;
    }
    
    switch( deciderName[0] ) {
        case 'd':
            if( strcmp( deciderName, "distinct" ) == 0 ) {
                decider.Decide = _DecideDistinctOrder;
                END_FUNCTION("GetNaryOrderDeciderInstance", SUCCESS );
                return &decider;                
            }
            else {
                END_FUNCTION("GetNaryOrderDeciderInstance", FAILING );    
                return NULL;
            }
        break;
        
        default:
            END_FUNCTION("GetNaryOrderDeciderInstance", FAILING );    
        return NULL;                            
    }
    
    END_FUNCTION("GetNaryOrderDeciderInstance", FAILING );    
    return NULL;
}



static RET_VAL _DecideDistinctOrder( ABSTRACTION_METHOD *method, CRITICAL_CONCENTRATION_INFO *info, double *criticalCons, int len ) {
    RET_VAL ret = SUCCESS;
    int i = 0;
    int j = 0;
    int newLen = 0;
    SPECIES *species = NULL;
    CRITICAL_CONCENTRATION_ELEMENT *elements = NULL;
    
    START_FUNCTION("_DecideDistinctOrder");
        
    species = info->species;
    
    qsort( criticalCons, len, sizeof(double), (int(*)(const void *, const void *))_CompareConcentrationElements );
#ifdef DEBUG
    printf( "pre-decided concentration levels of %s is:", GetCharArrayOfString( GetSpeciesNodeName( species ) ) );
    for( i = 0; i < len; i++ ) {
        printf("  %f", criticalCons[i] ); 
    }    
    printf( NEW_LINE ); 
#endif

    newLen = len;
    for( i = 1; i < len; i++ ) {
        if( IS_REAL_EQUAL( criticalCons[i -1], criticalCons[i] ) ) {
            newLen--;
        } 
    }
    
    if( ( elements = (CRITICAL_CONCENTRATION_ELEMENT*)MALLOC( sizeof(CRITICAL_CONCENTRATION_ELEMENT) * newLen ) ) == NULL ) {
        return ErrorReport( FAILING, "_DecideDistinctOrder", "failed to create critical concentrations for %s", GetCharArrayOfString( GetSpeciesNodeName( species ) ) ); 
    }

    elements[0].concentration = criticalCons[0];
    for( i = 1, j = 1; i < len; i++ ) {
        TRACE_3( "%ith and %ith diff is %f" NEW_LINE, i - 1, i, fabs( criticalCons[i -1]- criticalCons[i] ) );
        if( !IS_REAL_EQUAL( criticalCons[i -1], criticalCons[i] ) ) {
            TRACE_2( "%ith %f is distinct" NEW_LINE, i, criticalCons[i] );
            elements[j].concentration = criticalCons[i];
            j++;
        } 
    }
#ifdef DEBUG
    printf( "concentration levels of %s is:", GetCharArrayOfString( GetSpeciesNodeName( species ) ) );
    for( i = 0; i < newLen; i++ ) {
        printf("  %f", elements[i].concentration ); 
    }    
    printf( NEW_LINE ); 
#endif

    info->elements = elements;
    info->len = newLen;

    END_FUNCTION("_DecideDistinctOrder", ret );
    return ret;
}



static int _CompareConcentrationElements( double *e1, double *e2 ) {
    double c1 = 0.0;
    double c2 = 0.0;
    
    c1 = *e1;
    c2 = *e2;
    
    if( c1 < c2 ) {
        return -1;
    }
    else if( c1 > c2 ) {
        return 1;
    }
    else {
        return 0;
    }
}


