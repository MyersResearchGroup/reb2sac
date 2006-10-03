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
#include "critical_concentration_finder.h"
#include "law_of_mass_action_util.h"
#include "abstraction_method_properties.h"
#include "strconv.h"

static LINKED_LIST *_CreateListOfMultipleTerms( KINETIC_LAW *kineticLaw  );
static RET_VAL _VisitOpToCreateListOfMultipleTerms( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitIntToCreateListOfMultipleTerms( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitRealToCreateListOfMultipleTerms( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitSpeciesToCreateListOfMultipleTerms( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitSymbolToCreateListOfMultipleTerms( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );

static LINKED_LIST *_CreateListOfSumTerms( KINETIC_LAW *kineticLaw  );
static RET_VAL _VisitOpToCreateListOfSumTerms( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitIntToCreateListOfSumTerms( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitRealToCreateListOfSumTerms( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitSpeciesToCreateListOfSumTerms( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitSymbolToCreateListOfSumTerms( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );


static KINETIC_LAW *_FindDivisor( KINETIC_LAW *kineticLaw );
static RET_VAL _VisitOpToFindDivisor( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitIntToFindDivisor( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitRealToFindDivisor( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitSpeciesToFindDivisor( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitSymbolToFindDivisor( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );

static BOOL _FindSpecies( KINETIC_LAW *kineticLaw, SPECIES *species );
static RET_VAL _VisitOpToFindSpecies( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitIntToFindSpecies( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitRealToFindSpecies( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitSpeciesToFindSpecies( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitSymbolToFindSpecies( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );

static BOOL _FindMultiplicationOfSpecies( KINETIC_LAW *kineticLaw, SPECIES *species, double *result );
static RET_VAL _VisitOpToFindMultiplicationOfSpecies( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitIntToFindMultiplicationOfSpecies( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitRealToFindMultiplicationOfSpecies( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitSpeciesToFindMultiplicationOfSpecies( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitSymbolToFindMultiplicationOfSpecies( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );

static int _GetNumberOfSpecies( KINETIC_LAW *kineticLaw );
static RET_VAL _VisitOpToGetNumberOfSpecies( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitIntToGetNumberOfSpecies( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitRealToGetNumberOfSpecies( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitSpeciesToGetNumberOfSpecies( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitSymbolToGetNumberOfSpecies( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );




static RET_VAL STDCALL _AddListOfCriticalConcentrationLevels( CRITICAL_CONCENTRATION_FINDER *finder, REACTION *reaction, SPECIES *species, LINKED_LIST *list );    
static RET_VAL __AddListOfCriticalConcentrationLevels( double amplifier, KINETIC_LAW *kineticLaw, SPECIES *species, LINKED_LIST *list );
static BOOL _FindCriticalConcentrationLevel( double amplifier, KINETIC_LAW *kineticLaw, SPECIES *species, LINKED_LIST *list );





DLLSCOPE RET_VAL STDCALL InitCriticalConcentrationFinder(  CRITICAL_CONCENTRATION_FINDER *finder, COMPILER_RECORD_T *record ) {
    RET_VAL ret = SUCCESS;
    REB2SAC_PROPERTIES *properties = NULL;
    double amplifier = 0.0;
    char *valueString = NULL;

    START_FUNCTION("InitCriticalConcentrationFinder");

    properties = record->properties;    
    if( ( valueString = properties->GetProperty( properties, REB2SAC_CRITICAL_CONCENTRATION_AMPLIFIER_KEY ) ) == NULL ) {
        amplifier = DEFAULT_REB2SAC_CRITICAL_CONCENTRATION_AMPLIFIER;
    }
    else {
        if( IS_FAILED( ( ret = StrToFloat( &amplifier, valueString ) ) ) ) {
            amplifier = DEFAULT_REB2SAC_CRITICAL_CONCENTRATION_AMPLIFIER;
        }     
    }
    
    finder->amplifier = amplifier;
    finder->AddListOfCriticalConcentrationLevels = _AddListOfCriticalConcentrationLevels;

    END_FUNCTION("InitCriticalConcentrationFinder", SUCCESS );
    return ret;            
}


static RET_VAL STDCALL _AddListOfCriticalConcentrationLevels( CRITICAL_CONCENTRATION_FINDER *finder, REACTION *reaction, SPECIES *species, LINKED_LIST *conList ) {
    RET_VAL ret = SUCCESS;
    double rateRatio = 0.0;    
    double *result = NULL;
    KINETIC_LAW *kineticLaw = NULL;
    KINETIC_LAW *temp = NULL;
    LINKED_LIST *list = NULL;
    
    START_FUNCTION("AddListOfCriticalConcentrationLevels");

    if( IsReactionReversibleInReactionNode( reaction ) ) {
        END_FUNCTION("AddListOfCriticalConcentrationLevels", SUCCESS );
        return ret;            
    }
    
    kineticLaw = GetKineticLawInReactionNode( reaction );    
    if( ( list = _CreateListOfMultipleTerms( kineticLaw ) ) == NULL ) {
        END_FUNCTION("AddListOfCriticalConcentrationLevels", SUCCESS );
        return ret;            
    }
    ResetCurrentElement( list );
    while( ( kineticLaw = (KINETIC_LAW*)GetNextFromLinkedList( list ) ) != NULL ) {
        if( ( temp = _FindDivisor( kineticLaw ) ) != NULL ) {
            TRACE_2( "finding critical concentration levels of %s in %s", 
                GetCharArrayOfString( GetSpeciesNodeName( species ) ), GetCharArrayOfString( GetReactionNodeName( reaction ) ) );
            if( IS_FAILED( ( ret = __AddListOfCriticalConcentrationLevels( finder->amplifier, temp, species, conList ) ) ) ) {
                DeleteLinkedList( &list );
                END_FUNCTION("AddListOfCriticalConcentrationLevels", ret );
                return ret;            
            }         
        }
    }
    DeleteLinkedList( &list );
    END_FUNCTION("AddListOfCriticalConcentrationLevels", SUCCESS );
    return ret;            
}


static RET_VAL __AddListOfCriticalConcentrationLevels( double amplifier, KINETIC_LAW *kineticLaw, SPECIES *species, LINKED_LIST *list ) {
    RET_VAL ret = SUCCESS;
    double value = 0.0;    
    double *criticalCon = NULL;
    KINETIC_LAW *term = NULL;
    LINKED_LIST *terms = NULL;
    STRING *string = NULL;
    
    START_FUNCTION("__AddListOfCriticalConcentrationLevels");
    
    if( ( terms = _CreateListOfSumTerms( kineticLaw ) ) == NULL ) {
        string = ToStringKineticLaw( kineticLaw );
        return ErrorReport( FAILING, "__AddListOfCriticalConcentrationLevels", "could not create a list of sum terms for %s", GetCharArrayOfString( string ) );
    }
    
    ResetCurrentElement( terms );
    while( ( term = (KINETIC_LAW*)GetNextFromLinkedList( terms ) ) != NULL ) {
#if 0                
        string = ToStringKineticLaw( term );
        printf("examinng the critical concentration level of %s in %s" NEW_LINE, 
               GetCharArrayOfString( GetSpeciesNodeName( species ) ), GetCharArrayOfString( string ) );
        FreeString( &string );
#endif
        if( _FindCriticalConcentrationLevel( amplifier, term, species, list ) ) {
        }
        else {
#if 0                
            string = ToStringKineticLaw( term );
            printf("\tnot found from %s" NEW_LINE, GetCharArrayOfString( string ) );
            FreeString( &string );
#endif
        }
    }
    DeleteLinkedList( &terms );
    
    END_FUNCTION("__AddListOfCriticalConcentrationLevels", SUCCESS );
    return ret;
}

static BOOL _FindCriticalConcentrationLevel( double amplifier, KINETIC_LAW *kineticLaw, SPECIES *species, LINKED_LIST *list ) {
    static KINETIC_LAW_VISITOR visitor;
    RET_VAL ret = SUCCESS;
    int num = 0;
    double rateConstant = 1.0;
    double power = 0.0;
    double value = 0.0;
    double *criticalCon = NULL;
    KINETIC_LAW *term = NULL;
    LINKED_LIST *terms = NULL;
    KINETIC_LAW *numer = NULL;
    KINETIC_LAW *denom = NULL;
#ifdef DEBUG
    STRING *string = NULL;
#endif
    
    START_FUNCTION("_FindCriticalConcentrationLevel");
    
#if 0    
    if( FindRateConstant( kineticLaw, &rateConstant ) ) {
        END_FUNCTION("_FindCriticalConcentrationLevel", SUCCESS );
        return FALSE;
    }
    if( IS_REAL_EQUAL( rateConstant, 0.0 ) ) {
        END_FUNCTION("_FindCriticalConcentrationLevel", SUCCESS );
        return FALSE;
    }
    if( _GetNumberOfSpecies( kineticLaw ) != 1 ) {
        END_FUNCTION("_FindCriticalConcentrationLevel", SUCCESS );
        return FALSE;
    }
#endif
    
    if( ( terms = _CreateListOfMultipleTerms( kineticLaw ) ) == NULL ) {
        END_FUNCTION("_FindCriticalConcentrationLevel", FAILING );
        return FALSE;
    }

    ResetCurrentElement( terms );
    while( ( term = (KINETIC_LAW*)GetNextFromLinkedList( terms ) ) != NULL ) {
#if 1        
        if( IsOpKineticLaw( term ) ) {
            if( GetOpTypeFromKineticLaw( term ) == KINETIC_LAW_OP_DIVIDE ) {
                denom = GetOpRightFromKineticLaw( term );
                if( _FindSpecies( denom, species ) ) {
                    if( IS_FAILED( ( ret = __AddListOfCriticalConcentrationLevels( amplifier, denom, species, list ) ) ) ) {
                        END_FUNCTION("_FindCriticalConcentrationLevel", ret );
                        return ret;
                    }                    
                }
                numer = GetOpLeftFromKineticLaw( term );
                if( _FindSpecies( numer, species ) ) {
                    if( IS_FAILED( ( ret = __AddListOfCriticalConcentrationLevels( amplifier, numer, species, list ) ) ) ) {
                        END_FUNCTION("_FindCriticalConcentrationLevel", ret );
                        return ret;
                    }                    
                }                
            }
        }        
#endif
        
        if( _FindMultiplicationOfSpecies( term, species, &power ) ) {
            if( FindRateConstant( kineticLaw, &rateConstant ) ) {
                if( ( num = _GetNumberOfSpecies( kineticLaw ) ) != 1 ) {
#ifdef DEBUG
                    printf("\tnot yet found: number of species is %i in this term" NEW_LINE, num );
#endif                
                    END_FUNCTION("_FindCriticalConcentrationLevel", SUCCESS );
                    return FALSE;
                }
                if( IS_REAL_EQUAL( rateConstant, 0.0 ) ) {
                    END_FUNCTION("_FindCriticalConcentrationLevel", SUCCESS );
                    return FALSE;
                }
                if( IS_REAL_EQUAL( power, 0.0 ) ) {
#ifdef DEBUG
                    printf("\tnot yet found: the power is %f in this term" NEW_LINE, power );
#endif                
                    continue;
                    /*
                    DeleteLinkedList( &terms );
                    END_FUNCTION("_FindCriticalConcentrationLevel", SUCCESS );
                    return FALSE;
                    */
                }
                value = pow( amplifier / ( rateConstant - amplifier * rateConstant), 1.0 / power );
                if( ( criticalCon = (double*)MALLOC( sizeof(double) ) ) == NULL ) {
                    return ErrorReport( FAILING, "_FindCriticalConcentrationLevel", "could not allocate space for critical concentration %f", value );
                }
                *criticalCon = value;
#ifdef DEBUG
                string = ToStringKineticLaw( kineticLaw );
                printf("\t%f is found from %s" NEW_LINE, value, GetCharArrayOfString( string ) );
                FreeString( &string );
#endif
                if( IS_FAILED( ( ret = AddElementInLinkedList( criticalCon, list ) ) ) ) {
                    END_FUNCTION("__AddListOfCriticalConcentrationLevels", ret );
                    return ret;
                }
                
                DeleteLinkedList( &terms );
                END_FUNCTION("_FindCriticalConcentrationLevel", SUCCESS );
                return TRUE;
            }
            else {
#ifdef DEBUG
                printf("\tfailed to find rate constant" NEW_LINE );
#endif
            } 
        }
        else {
#ifdef DEBUG
            printf("\tno species in this term" NEW_LINE );
#endif
        }
    }
    
    DeleteLinkedList( &terms );
    END_FUNCTION("_FindCriticalConcentrationLevel", SUCCESS );
    return FALSE;
}

 
static BOOL _FindSpecies( KINETIC_LAW *kineticLaw, SPECIES *species ) {
    static KINETIC_LAW_VISITOR visitor;
    BOOL flag = FALSE;
    
    START_FUNCTION("_FindSpecies");
    
    if( visitor.VisitOp == NULL ) {
        visitor.VisitOp = _VisitOpToFindSpecies;
        visitor.VisitInt = _VisitIntToFindSpecies;
        visitor.VisitReal = _VisitRealToFindSpecies;
        visitor.VisitSpecies = _VisitSpeciesToFindSpecies;
        visitor.VisitSymbol = _VisitSymbolToFindSpecies;
    }
    
    visitor._internal1 = (CADDR_T)species;
    visitor._internal2 = (CADDR_T)(&flag);
    
    if( IS_FAILED( kineticLaw->AcceptPostOrder( kineticLaw, &visitor ) ) ) {
        END_FUNCTION("_FindSpecies", SUCCESS );
        return FALSE;
    } 
    
    END_FUNCTION("_FindSpecies", SUCCESS );
    return flag;
}

static RET_VAL _VisitOpToFindSpecies( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    START_FUNCTION("_VisitOpToFindSpecies");
    END_FUNCTION("_VisitOpToFindSpecies", SUCCESS );
    return SUCCESS;
}

static RET_VAL _VisitIntToFindSpecies( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    START_FUNCTION("_VisitIntToFindSpecies");
    END_FUNCTION("_VisitIntToFindSpecies", SUCCESS );
    return SUCCESS;
}

static RET_VAL _VisitRealToFindSpecies( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    START_FUNCTION("_VisitRealToFindSpecies");
    END_FUNCTION("_VisitRealToFindSpecies", SUCCESS );
    return SUCCESS;
}


static RET_VAL _VisitSymbolToFindSpecies( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    START_FUNCTION("_VisitSymbolToFindSpecies");
    END_FUNCTION("_VisitSymbolToFindSpecies", SUCCESS );
    return SUCCESS;
}


static RET_VAL _VisitSpeciesToFindSpecies( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    BOOL *flag = NULL;
    SPECIES *species = NULL;
        
    START_FUNCTION("_VisitSpeciesToFindSpecies");
    
    species = (SPECIES*)(visitor->_internal1);
    flag = (BOOL*)(visitor->_internal2);
        
    *flag = ( species == GetSpeciesFromKineticLaw( kineticLaw ) ? TRUE : FALSE );     
    END_FUNCTION("_VisitSpeciesToFindSpecies", SUCCESS );
    return SUCCESS;
}




static LINKED_LIST *_CreateListOfMultipleTerms( KINETIC_LAW *kineticLaw  ) {
    static KINETIC_LAW_VISITOR visitor;
    LINKED_LIST *list = NULL;
    
    START_FUNCTION("_CreateListOfMultipleTerms");
    
    if( visitor.VisitOp == NULL ) {
        visitor.VisitOp = _VisitOpToCreateListOfMultipleTerms;
        visitor.VisitInt = _VisitIntToCreateListOfMultipleTerms;
        visitor.VisitReal = _VisitRealToCreateListOfMultipleTerms;
        visitor.VisitSpecies = _VisitSpeciesToCreateListOfMultipleTerms;
        visitor.VisitSymbol = _VisitSymbolToCreateListOfMultipleTerms;
    }
    
    if( ( list = CreateLinkedList() ) == NULL ) {
        END_FUNCTION("_CreateListOfMultipleTerms", FAILING );
        return NULL;
    }
    
    visitor._internal1 = (CADDR_T)list;
    
    if( IS_FAILED( kineticLaw->Accept( kineticLaw, &visitor ) ) ) {
        END_FUNCTION("_CreateListOfMultipleTerms", FAILING );
        return NULL;
    } 
    
    END_FUNCTION("_CreateListOfMultipleTerms", SUCCESS );
    return list;
}

static RET_VAL _VisitOpToCreateListOfMultipleTerms( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    RET_VAL ret = SUCCESS;
    BYTE opType = 0;
    KINETIC_LAW *left = NULL;
    KINETIC_LAW *right = NULL;
    LINKED_LIST *list = NULL;
    
    START_FUNCTION("_VisitOpToCreateListOfMultipleTerms");
    
    list = (LINKED_LIST*)(visitor->_internal1);
    
    opType = GetOpTypeFromKineticLaw( kineticLaw );    
    switch( opType ) {
        case KINETIC_LAW_OP_TIMES:
            left = GetOpLeftFromKineticLaw( kineticLaw );
            right = GetOpRightFromKineticLaw( kineticLaw );
            if( IS_FAILED( ( ret = left->Accept( left, visitor ) ) ) ) {
                END_FUNCTION("_VisitOpToCreateListOfMultipleTerms", ret );
                return ret;
            }
            if( IS_FAILED( ( ret = right->Accept( right, visitor ) ) ) ) {
                END_FUNCTION("_VisitOpToCreateListOfMultipleTerms", ret );
                return ret;
            }            
            END_FUNCTION("_VisitOpToCreateListOfMultipleTerms", SUCCESS );
        return ret;

        default:
            if( IS_FAILED( ( ret = AddElementInLinkedList( kineticLaw, list ) ) ) ) {
                END_FUNCTION("_VisitOpToCreateListOfMultipleTerms", ret );
                return ret;
            }
            END_FUNCTION("_VisitOpToCreateListOfMultipleTerms", SUCCESS );
        return ret;
    }
    
    END_FUNCTION("_VisitOpToCreateListOfMultipleTerms", SUCCESS );
    return SUCCESS;
}

static RET_VAL _VisitIntToCreateListOfMultipleTerms( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    RET_VAL ret = SUCCESS;
    LINKED_LIST *list = NULL;
    
    START_FUNCTION("_VisitIntToCreateListOfMultipleTerms");
    
    list = (LINKED_LIST*)(visitor->_internal1);
    if( IS_FAILED( ( ret = AddElementInLinkedList( kineticLaw, list ) ) ) ) {
        END_FUNCTION("_VisitIntToCreateListOfMultipleTerms", ret );
        return ret;
    }
    
    END_FUNCTION("_VisitIntToCreateListOfMultipleTerms", SUCCESS );
    return ret;
}

static RET_VAL _VisitRealToCreateListOfMultipleTerms( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    RET_VAL ret = SUCCESS;
    LINKED_LIST *list = NULL;
    
    START_FUNCTION("_VisitRealToCreateListOfMultipleTerms");
    
    list = (LINKED_LIST*)(visitor->_internal1);
    if( IS_FAILED( ( ret = AddElementInLinkedList( kineticLaw, list ) ) ) ) {
        END_FUNCTION("_VisitRealToCreateListOfMultipleTerms", ret );
        return ret;
    }
    
    END_FUNCTION("_VisitRealToCreateListOfMultipleTerms", SUCCESS );
    return ret;
}

static RET_VAL _VisitSpeciesToCreateListOfMultipleTerms( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    RET_VAL ret = SUCCESS;
    LINKED_LIST *list = NULL;
    
    START_FUNCTION("_VisitSpeciesToCreateListOfMultipleTerms");
    
    list = (LINKED_LIST*)(visitor->_internal1);
    if( IS_FAILED( ( ret = AddElementInLinkedList( kineticLaw, list ) ) ) ) {
        END_FUNCTION("_VisitSpeciesToCreateListOfMultipleTerms", ret );
        return ret;
    }
    
    END_FUNCTION("_VisitSpeciesToCreateListOfMultipleTerms", SUCCESS );
    return ret;
}

static RET_VAL _VisitSymbolToCreateListOfMultipleTerms( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    RET_VAL ret = SUCCESS;
    LINKED_LIST *list = NULL;
    
    START_FUNCTION("_VisitSymbolToCreateListOfMultipleTerms");
    
    list = (LINKED_LIST*)(visitor->_internal1);
    if( IS_FAILED( ( ret = AddElementInLinkedList( kineticLaw, list ) ) ) ) {
        END_FUNCTION("_VisitSymbolToCreateListOfMultipleTerms", ret );
        return ret;
    }
    
    END_FUNCTION("_VisitSymbolToCreateListOfMultipleTerms", SUCCESS );
    return ret;
}


static LINKED_LIST *_CreateListOfSumTerms( KINETIC_LAW *kineticLaw  ) {
    static KINETIC_LAW_VISITOR visitor;
    LINKED_LIST *list = NULL;
    
    START_FUNCTION("_CreateListOfSumTerms");
    
    if( visitor.VisitOp == NULL ) {
        visitor.VisitOp = _VisitOpToCreateListOfSumTerms;
        visitor.VisitInt = _VisitIntToCreateListOfSumTerms;
        visitor.VisitReal = _VisitRealToCreateListOfSumTerms;
        visitor.VisitSpecies = _VisitSpeciesToCreateListOfSumTerms;
        visitor.VisitSymbol = _VisitSymbolToCreateListOfSumTerms;
    }
    
    if( ( list = CreateLinkedList() ) == NULL ) {
        END_FUNCTION("_CreateListOfSumTerms", FAILING );
        return NULL;
    }
    
    visitor._internal1 = (CADDR_T)list;
    
    if( IS_FAILED( kineticLaw->Accept( kineticLaw, &visitor ) ) ) {
        END_FUNCTION("_CreateListOfSumTerms", FAILING );
        return NULL;
    } 
    
    END_FUNCTION("_CreateListOfSumTerms", SUCCESS );
    return list;
}

static RET_VAL _VisitOpToCreateListOfSumTerms( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    RET_VAL ret = SUCCESS;
    BYTE opType = 0;
    KINETIC_LAW *left = NULL;
    KINETIC_LAW *right = NULL;
    LINKED_LIST *list = NULL;
    
    START_FUNCTION("_VisitOpToCreateListOfSumTerms");
    
    list = (LINKED_LIST*)(visitor->_internal1);
    
    opType = GetOpTypeFromKineticLaw( kineticLaw );    
    switch( opType ) {
        case KINETIC_LAW_OP_PLUS:
            left = GetOpLeftFromKineticLaw( kineticLaw );
            right = GetOpRightFromKineticLaw( kineticLaw );
            if( IS_FAILED( ( ret = left->Accept( left, visitor ) ) ) ) {
                END_FUNCTION("_VisitOpToCreateListOfSumTerms", ret );
                return ret;
            }
            if( IS_FAILED( ( ret = right->Accept( right, visitor ) ) ) ) {
                END_FUNCTION("_VisitOpToCreateListOfSumTerms", ret );
                return ret;
            }            
            END_FUNCTION("_VisitOpToCreateListOfSumTerms", SUCCESS );
        return ret;

        default:
            if( IS_FAILED( ( ret = AddElementInLinkedList( kineticLaw, list ) ) ) ) {
                END_FUNCTION("_VisitOpToCreateListOfSumTerms", ret );
                return ret;
            }
            END_FUNCTION("_VisitOpToCreateListOfSumTerms", SUCCESS );
        return ret;
    }
    
    END_FUNCTION("_VisitOpToCreateListOfSumTerms", SUCCESS );
    return SUCCESS;
}

static RET_VAL _VisitIntToCreateListOfSumTerms( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    RET_VAL ret = SUCCESS;
    LINKED_LIST *list = NULL;
    
    START_FUNCTION("_VisitIntToCreateListOfSumTerms");
    
    list = (LINKED_LIST*)(visitor->_internal1);
    if( IS_FAILED( ( ret = AddElementInLinkedList( kineticLaw, list ) ) ) ) {
        END_FUNCTION("_VisitIntToCreateListOfSumTerms", ret );
        return ret;
    }
    
    END_FUNCTION("_VisitIntToCreateListOfSumTerms", SUCCESS );
    return ret;
}

static RET_VAL _VisitRealToCreateListOfSumTerms( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    RET_VAL ret = SUCCESS;
    LINKED_LIST *list = NULL;
    
    START_FUNCTION("_VisitRealToCreateListOfSumTerms");
    
    list = (LINKED_LIST*)(visitor->_internal1);
    if( IS_FAILED( ( ret = AddElementInLinkedList( kineticLaw, list ) ) ) ) {
        END_FUNCTION("_VisitRealToCreateListOfSumTerms", ret );
        return ret;
    }
    
    END_FUNCTION("_VisitRealToCreateListOfSumTerms", SUCCESS );
    return ret;
}

static RET_VAL _VisitSpeciesToCreateListOfSumTerms( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    RET_VAL ret = SUCCESS;
    LINKED_LIST *list = NULL;
    
    START_FUNCTION("_VisitSpeciesToCreateListOfSumTerms");
    
    list = (LINKED_LIST*)(visitor->_internal1);
    if( IS_FAILED( ( ret = AddElementInLinkedList( kineticLaw, list ) ) ) ) {
        END_FUNCTION("_VisitSpeciesToCreateListOfSumTerms", ret );
        return ret;
    }
    
    END_FUNCTION("_VisitSpeciesToCreateListOfSumTerms", SUCCESS );
    return ret;
}

static RET_VAL _VisitSymbolToCreateListOfSumTerms( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    RET_VAL ret = SUCCESS;
    LINKED_LIST *list = NULL;
    
    START_FUNCTION("_VisitSymbolToCreateListOfSumTerms");
    
    list = (LINKED_LIST*)(visitor->_internal1);
    if( IS_FAILED( ( ret = AddElementInLinkedList( kineticLaw, list ) ) ) ) {
        END_FUNCTION("_VisitSymbolToCreateListOfSumTerms", ret );
        return ret;
    }
    
    END_FUNCTION("_VisitSymbolToCreateListOfSumTerms", SUCCESS );
    return ret;
}


 
static KINETIC_LAW *_FindDivisor( KINETIC_LAW *kineticLaw ) {
    static KINETIC_LAW_VISITOR visitor;
    
    START_FUNCTION("_FindDivisor");
    
    if( visitor.VisitOp == NULL ) {
        visitor.VisitOp = _VisitOpToFindDivisor;
        visitor.VisitInt = _VisitIntToFindDivisor;
        visitor.VisitReal = _VisitRealToFindDivisor;
        visitor.VisitSpecies = _VisitSpeciesToFindDivisor;
        visitor.VisitSymbol = _VisitSymbolToFindDivisor;
    }
    
    visitor._internal1 = NULL;
    
    if( IS_FAILED( kineticLaw->Accept( kineticLaw, &visitor ) ) ) {
        END_FUNCTION("_FindDivisor", SUCCESS );
        return NULL;
    } 
    
    END_FUNCTION("_FindDivisor", SUCCESS );
    return (KINETIC_LAW*)(visitor._internal1);
}

static RET_VAL _VisitOpToFindDivisor( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    RET_VAL ret = SUCCESS;
    BYTE opType = 0x00;
    KINETIC_LAW *left = NULL;
    KINETIC_LAW *right = NULL;
    
    START_FUNCTION("_VisitOpToFindDivisor");
    
    right = GetOpRightFromKineticLaw( kineticLaw );
    opType = GetOpTypeFromKineticLaw( kineticLaw );
    if( opType == KINETIC_LAW_OP_DIVIDE ) {
        visitor->_internal1 = (CADDR_T)right;
        END_FUNCTION("_VisitOpToFindDivisor", SUCCESS );
        return SUCCESS;
    }
    
    left = GetOpLeftFromKineticLaw( kineticLaw );
    if( IS_FAILED( ( ret = left->Accept( left, visitor ) ) ) ) {
        END_FUNCTION("_VisitOpToFindDivisor", SUCCESS );
        return ret;
    }
    
    if( IS_FAILED( ( ret = right->Accept( right, visitor ) ) ) ) {
        END_FUNCTION("_VisitOpToFindDivisor", SUCCESS );
        return ret;
    }
        
    END_FUNCTION("_VisitOpToFindDivisor", SUCCESS );
    return ret;
}

static RET_VAL _VisitIntToFindDivisor( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    START_FUNCTION("_VisitIntToFindDivisor");
    END_FUNCTION("_VisitIntToFindDivisor", SUCCESS );
    return SUCCESS;
}

static RET_VAL _VisitRealToFindDivisor( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    START_FUNCTION("_VisitRealToFindDivisor");
    END_FUNCTION("_VisitRealToFindDivisor", SUCCESS );
    return SUCCESS;
}

static RET_VAL _VisitSpeciesToFindDivisor( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    START_FUNCTION("_VisitSpeciesToFindDivisor");
    END_FUNCTION("_VisitSpeciesToFindDivisor", SUCCESS );
    return SUCCESS;
}

static RET_VAL _VisitSymbolToFindDivisor( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    START_FUNCTION("_VisitSymbolToFindDivisor");
    END_FUNCTION("_VisitSymbolToFindDivisor", SUCCESS );
    return SUCCESS;
}



static BOOL _FindMultiplicationOfSpecies( KINETIC_LAW *kineticLaw, SPECIES *species, double *result ) {
    static KINETIC_LAW_VISITOR visitor;
#ifdef DEBUG    
    STRING *string = NULL;
#endif
    
    START_FUNCTION("_FindMultiplicationOfSpecies");
    
    if( visitor.VisitOp == NULL ) {
        visitor.VisitOp = _VisitOpToFindMultiplicationOfSpecies;
        visitor.VisitInt = _VisitIntToFindMultiplicationOfSpecies;
        visitor.VisitReal = _VisitRealToFindMultiplicationOfSpecies;
        visitor.VisitSpecies = _VisitSpeciesToFindMultiplicationOfSpecies;
        visitor.VisitSymbol = _VisitSymbolToFindMultiplicationOfSpecies;
    }
    *result = 0.0;
    
    visitor._internal1 = (CADDR_T)species;
    visitor._internal2 = (CADDR_T)result;
    
    if( IS_FAILED( kineticLaw->Accept( kineticLaw, &visitor ) ) ) {
        END_FUNCTION("_FindSpecies", SUCCESS );
        return FALSE;
    } 
#ifdef DEBUG    
    string = ToStringKineticLaw( kineticLaw );
    printf( "the multiplication of %s found in %s is %g" NEW_LINE, GetCharArrayOfString( GetSpeciesNodeName( species ) ), GetCharArrayOfString( string ), *result ); 
    FreeString( &string );
#endif
    
    END_FUNCTION("_FindMultiplicationOfSpecies", SUCCESS );
    return TRUE;
}

static RET_VAL _VisitOpToFindMultiplicationOfSpecies( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    RET_VAL ret = SUCCESS;
    BYTE opType = 0x00;
    double *resultSave = NULL;
    double temp = 0.0;
    KINETIC_LAW *left = NULL;
    KINETIC_LAW *right = NULL;
    
    START_FUNCTION("_VisitOpToFindMultiplicationOfSpecies");
    
    opType = GetOpTypeFromKineticLaw( kineticLaw );
    left = GetOpLeftFromKineticLaw( kineticLaw );
    right = GetOpRightFromKineticLaw( kineticLaw );
    
    if( opType == KINETIC_LAW_OP_TIMES  ) {
        if( IS_FAILED( ( ret = left->Accept( left, visitor ) ) ) ) {
            END_FUNCTION("_VisitOpToFindMultiplicationOfSpecies", ret );
            return ret;
        }
        if( IS_FAILED( ( ret = right->Accept( right, visitor ) ) ) ) {
            END_FUNCTION("_VisitOpToFindMultiplicationOfSpecies", ret );
            return ret;
        }
    }
    else if( opType == KINETIC_LAW_OP_POW ) {
        resultSave = (double*)(visitor->_internal2);
        temp = 0.0;
        visitor->_internal2 = (CADDR_T)(&temp);
        if( IS_FAILED( ( ret = left->Accept( left, visitor ) ) ) ) {
            END_FUNCTION("_VisitOpToFindMultiplicationOfSpecies", ret );
            return ret;
        }
        if( IsRealValueKineticLaw( right ) ) {
            temp *= GetRealValueFromKineticLaw( right );
        }
        else if( IsIntValueKineticLaw( right ) ) {
            temp *= (double)GetIntValueFromKineticLaw( right );
        }
        else {
            END_FUNCTION("_VisitOpToFindMultiplicationOfSpecies", E_WRONGDATA );
            return E_WRONGDATA;
        }
        *resultSave = (*resultSave) + temp;
        visitor->_internal2 = (CADDR_T)resultSave;
    }
    else {
        END_FUNCTION("_VisitOpToFindMultiplicationOfSpecies", E_WRONGDATA );
        return E_WRONGDATA;
    }
    
    END_FUNCTION("_VisitOpToFindMultiplicationOfSpecies", SUCCESS );
    return ret;
}

static RET_VAL _VisitIntToFindMultiplicationOfSpecies( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    RET_VAL ret = SUCCESS;
    START_FUNCTION("_VisitIntToFindMultiplicationOfSpecies");
    
    END_FUNCTION("_VisitIntToFindMultiplicationOfSpecies", SUCCESS );
    return ret;
}

static RET_VAL _VisitRealToFindMultiplicationOfSpecies( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    RET_VAL ret = SUCCESS;
    START_FUNCTION("_VisitRealToFindMultiplicationOfSpecies");
    
    END_FUNCTION("_VisitRealToFindMultiplicationOfSpecies", SUCCESS );
    return ret;
}


static RET_VAL _VisitSymbolToFindMultiplicationOfSpecies( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    RET_VAL ret = SUCCESS;
    START_FUNCTION("_VisitSymbolToFindMultiplicationOfSpecies");
    
    END_FUNCTION("_VisitSymbolToFindMultiplicationOfSpecies", SUCCESS );
    return ret;
}

static RET_VAL _VisitSpeciesToFindMultiplicationOfSpecies( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    RET_VAL ret = SUCCESS;
    double *result = NULL;
    SPECIES *species = NULL;
    SPECIES *target = NULL;
    
    START_FUNCTION("_VisitSpeciesToFindMultiplicationOfSpecies");
    
    species = GetSpeciesFromKineticLaw( kineticLaw );
    target = (SPECIES*)(visitor->_internal1);
    
    if( species == target ) {
        result = (double*)(visitor->_internal2);
        *result = *result + 1.0;
    }
    
    END_FUNCTION("_VisitSpeciesToFindMultiplicationOfSpecies", SUCCESS );
    return ret;
}


static int _GetNumberOfSpecies( KINETIC_LAW *kineticLaw ) {
    static KINETIC_LAW_VISITOR visitor;
    LINKED_LIST *list = NULL;
    int num = 0;
#ifdef DEBUG    
    STRING *string = NULL;
#endif
    
    if( visitor.VisitOp == NULL ) {
        visitor.VisitOp = _VisitOpToGetNumberOfSpecies;
        visitor.VisitInt = _VisitIntToGetNumberOfSpecies;
        visitor.VisitReal = _VisitRealToGetNumberOfSpecies;
        visitor.VisitSpecies = _VisitSpeciesToGetNumberOfSpecies;
        visitor.VisitSymbol = _VisitSymbolToGetNumberOfSpecies;
    }

    if( ( list = CreateLinkedList() ) == NULL ) {
        TRACE_0("could not create a list to store distinct species" );
        return 0;
    }    
    visitor._internal1 = (CADDR_T)list;
    
    if( IS_FAILED( kineticLaw->AcceptPostOrder( kineticLaw, &visitor ) ) ) {
        return FALSE;
    } 
    num = GetLinkedListSize( list );
    DeleteLinkedList( &list );
    return num;
}


static RET_VAL _VisitOpToGetNumberOfSpecies( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    return SUCCESS;
}

static RET_VAL _VisitIntToGetNumberOfSpecies( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    return SUCCESS;
}

static RET_VAL _VisitRealToGetNumberOfSpecies( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    return SUCCESS;
}

static RET_VAL _VisitSpeciesToGetNumberOfSpecies( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    SPECIES *species = NULL;
    SPECIES *target = NULL;
    LINKED_LIST *list = NULL;
            
    START_FUNCTION("_VisitSpeciesToFindSpecies");
    
    list = (LINKED_LIST*)(visitor->_internal1);
        
    target =GetSpeciesFromKineticLaw( kineticLaw );
    ResetCurrentElement( list );
    while( ( species = (SPECIES*)GetNextFromLinkedList( list ) ) != NULL ) {
        if( species == target ) {
            return SUCCESS;
        }
    }
    if( IS_FAILED( AddElementInLinkedList( (CADDR_T)target, list ) ) ) {
        return FAILING;
    }         
    return SUCCESS;
}

static RET_VAL _VisitSymbolToGetNumberOfSpecies( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    return SUCCESS;
}


