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
#include "law_of_mass_action_util.h"
#include "kinetic_law_evaluater.h"
#include <math.h>

static BOOL _FindRateConstant( KINETIC_LAW *kineticLaw, double *result );
static RET_VAL _VisitOpToFindRateConstant( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitIntToFindRateConstant( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitRealToFindRateConstant( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitSpeciesToFindRateConstant( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitSymbolToFindRateConstant( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );

static BOOL _FindSpecies( KINETIC_LAW *kineticLaw, SPECIES *species );
static RET_VAL _VisitOpToFindSpecies( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitIntToFindSpecies( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitRealToFindSpecies( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitSpeciesToFindSpecies( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitSymbolToFindSpecies( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
  
static BOOL _ContainSpecies( KINETIC_LAW *kineticLaw );
static RET_VAL _VisitOpToContainSpecies( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitIntToContainSpecies( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitRealToContainSpecies( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitSpeciesToContainSpecies( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitSymbolToContainSpecies( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );



static KINETIC_LAW *_FindDivisor( KINETIC_LAW *kineticLaw );
static RET_VAL _VisitOpToFindDivisor( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitIntToFindDivisor( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitRealToFindDivisor( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitSpeciesToFindDivisor( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitSymbolToFindDivisor( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );

static BOOL _FindCriticalConcentrationLevel( KINETIC_LAW *kineticLaw, SPECIES *species, double *result );
static RET_VAL _AddListOfCriticalConcentrationLevels( KINETIC_LAW *kineticLaw, SPECIES *species, LINKED_LIST *list );
static RET_VAL __AddListOfCriticalConcentrationLevels( KINETIC_LAW *kineticLaw, SPECIES *species, LINKED_LIST *list );


static BOOL _IsKineticLawMultipleOfSpecies(  KINETIC_LAW *kineticLaw, SPECIES *species );
static RET_VAL _VisitOpMultipleOfSpecies( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitIntMultipleOfSpecies( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitRealMultipleOfSpecies( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitSpeciesMultipleOfSpecies( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitSymbolMultipleOfSpecies( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );

static KINETIC_LAW *_CreateMassActionRatio( KINETIC_LAW *forward, SPECIES *op, double rateRatio  );
static RET_VAL _VisitOpToCreateMassActionRatio( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitIntToCreateMassActionRatio( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitRealToCreateMassActionRatio( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitSpeciesToCreateMassActionRatio( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitSymbolToCreateMassActionRatio( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );

static KINETIC_LAW *_CreateRateConstantKineticLaw( KINETIC_LAW *kineticLaw );
static RET_VAL _VisitOpToCreateRateConstantKineticLaw( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitIntToCreateRateConstantKineticLaw( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitRealToCreateRateConstantKineticLaw( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitSpeciesToCreateRateConstantKineticLaw( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitSymbolToCreateRateConstantKineticLaw( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );

static KINETIC_LAW *_CreateMassActionRatioWithRateConstantInKineticLaw( KINETIC_LAW *forward, SPECIES *op, KINETIC_LAW *rateRatio  );
static RET_VAL _VisitOpToCreateMassActionRatioWithRateConstantInKineticLaw( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitIntToCreateMassActionRatioWithRateConstantInKineticLaw( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitRealToCreateMassActionRatioWithRateConstantInKineticLaw( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitSpeciesToCreateMassActionRatioWithRateConstantInKineticLaw( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitSymbolToCreateMassActionRatioWithRateConstantInKineticLaw( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );

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

static BOOL _FindMultiplicationOfSpecies( KINETIC_LAW *kineticLaw, SPECIES *species, double *result );
static RET_VAL _VisitOpToFindMultiplicationOfSpecies( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitIntToFindMultiplicationOfSpecies( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitRealToFindMultiplicationOfSpecies( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitSpeciesToFindMultiplicationOfSpecies( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );
static RET_VAL _VisitSymbolToFindMultiplicationOfSpecies( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw );



 
BOOL FindRateConstantRatio( KINETIC_LAW *kineticLaw, double *result ) {
    double associationRate = 0.0;
    double dissociationRate = 0.0;    
    KINETIC_LAW *left = NULL;        
    KINETIC_LAW *right = NULL;        
#ifdef DEBUG
    STRING *lawString = NULL;
#endif    
    START_FUNCTION("FindRateConstantRatio");
    
    if( !IsOpKineticLaw( kineticLaw ) ) {
        END_FUNCTION("FindRateConstantRatio", SUCCESS );
        return FALSE;
    }
    if( GetOpTypeFromKineticLaw( kineticLaw ) != KINETIC_LAW_OP_MINUS ) {
        END_FUNCTION("FindRateConstantRatio", SUCCESS );
        return FALSE;
    }
    
    left = GetOpLeftFromKineticLaw( kineticLaw );
    if(!_FindRateConstant( left, &associationRate ) ) {
        END_FUNCTION("FindRateConstantRatio", SUCCESS );
        return FALSE;
    }
    
    right = GetOpRightFromKineticLaw( kineticLaw );
    if(!_FindRateConstant( right, &dissociationRate ) ) {
        END_FUNCTION("FindRateConstantRatio", SUCCESS );
        return FALSE;
    }
    
    *result = (associationRate / dissociationRate);
#ifdef DEBUG
    lawString = ToStringKineticLaw( kineticLaw );
    printf( "rate constant ratio of %s is %g" NEW_LINE, GetCharArrayOfString( lawString ), *result ); 
    FreeString( &lawString );
#endif    
            
    END_FUNCTION("FindRateConstantRatio", SUCCESS );
    return TRUE;
}

KINETIC_LAW *CreateDissociationConstantRatioKineticLaw( KINETIC_LAW *kineticLaw ) {
    KINETIC_LAW *rateConstantRatio = NULL;
    KINETIC_LAW *associationRate = NULL;
    KINETIC_LAW *dissociationRate = NULL;    
    KINETIC_LAW *left = NULL;        
    KINETIC_LAW *right = NULL;        
#ifdef DEBUG
    STRING *lawString = NULL;
#endif    
    START_FUNCTION("CreateDissociationConstantRatioKineticLaw");
    
    if( !IsOpKineticLaw( kineticLaw ) ) {
        END_FUNCTION("CreateDissociationConstantRatioKineticLaw", SUCCESS );
        return NULL;
    }
    if( GetOpTypeFromKineticLaw( kineticLaw ) != KINETIC_LAW_OP_MINUS ) {
        END_FUNCTION("CreateDissociationConstantRatioKineticLaw", SUCCESS );
        return NULL;
    }
    
    left = GetOpLeftFromKineticLaw( kineticLaw );
    if( ( associationRate = _CreateRateConstantKineticLaw( left ) ) == NULL ) {
        END_FUNCTION("CreateDissociationConstantRatioKineticLaw", SUCCESS );
        return NULL;
    }
    
    right = GetOpRightFromKineticLaw( kineticLaw );
    if( ( dissociationRate = _CreateRateConstantKineticLaw( right ) ) == NULL ) {
        END_FUNCTION("CreateDissociationConstantRatioKineticLaw", SUCCESS );
        return NULL;
    }
    
    if( ( rateConstantRatio = CreateOpKineticLaw( KINETIC_LAW_OP_DIVIDE, dissociationRate, associationRate  ) ) == NULL ) {
        END_FUNCTION("CreateDissociationConstantRatioKineticLaw", FAILING );
        return NULL;
    }
    
#ifdef DEBUG
    lawString = ToStringKineticLaw( kineticLaw );
    printf( "dissociation rate constant ratio of %s is ", GetCharArrayOfString( lawString ) ); 
    FreeString( &lawString );
    lawString = ToStringKineticLaw( rateConstantRatio );
    printf( "%s" NEW_LINE, GetCharArrayOfString( lawString ) ); 
    FreeString( &lawString );
#endif    
            
    END_FUNCTION("CreateDissociationConstantRatioKineticLaw", SUCCESS );
    return rateConstantRatio;
}


KINETIC_LAW *CreateRateConstantRatioKineticLaw( KINETIC_LAW *kineticLaw ) {
    KINETIC_LAW *rateConstantRatio = NULL;
    KINETIC_LAW *associationRate = NULL;
    KINETIC_LAW *dissociationRate = NULL;    
    KINETIC_LAW *left = NULL;        
    KINETIC_LAW *right = NULL;        
#ifdef DEBUG
    STRING *lawString = NULL;
#endif    
    START_FUNCTION("CreateRateConstantRatioKineticLaw");
    
    if( !IsOpKineticLaw( kineticLaw ) ) {
        END_FUNCTION("CreateRateConstantRatioKineticLaw", SUCCESS );
        return NULL;
    }
    if( GetOpTypeFromKineticLaw( kineticLaw ) != KINETIC_LAW_OP_MINUS ) {
        END_FUNCTION("CreateRateConstantRatioKineticLaw", SUCCESS );
        return NULL;
    }
    
    left = GetOpLeftFromKineticLaw( kineticLaw );
    if( ( associationRate = _CreateRateConstantKineticLaw( left ) ) == NULL ) {
        END_FUNCTION("CreateRateConstantRatioKineticLaw", SUCCESS );
        return NULL;
    }
    
    right = GetOpRightFromKineticLaw( kineticLaw );
    if( ( dissociationRate = _CreateRateConstantKineticLaw( right ) ) == NULL ) {
        END_FUNCTION("CreateRateConstantRatioKineticLaw", SUCCESS );
        return NULL;
    }
    
    if( ( rateConstantRatio = CreateOpKineticLaw( KINETIC_LAW_OP_DIVIDE, associationRate, dissociationRate ) ) == NULL ) {
        END_FUNCTION("CreateRateConstantRatioKineticLaw", FAILING );
        return NULL;
    }
    
#ifdef DEBUG
    lawString = ToStringKineticLaw( kineticLaw );
    printf( "rate constant ratio of %s is ", GetCharArrayOfString( lawString ) ); 
    FreeString( &lawString );
    lawString = ToStringKineticLaw( rateConstantRatio );
    printf( "%s" NEW_LINE, GetCharArrayOfString( lawString ) ); 
    FreeString( &lawString );
#endif    
            
    END_FUNCTION("CreateRateConstantRatioKineticLaw", SUCCESS );
    return rateConstantRatio;
}


BOOL FindRateConstant( KINETIC_LAW *kineticLaw, double *result ) {
#ifdef DEBUG
    STRING *lawString = NULL;
#endif    
    START_FUNCTION("FindRateConstant");
    
    if(!_FindRateConstant( kineticLaw, result ) ) {
        END_FUNCTION("FindRateConstant", SUCCESS );
        return FALSE;
    }
#ifdef DEBUG
    lawString = ToStringKineticLaw( kineticLaw );
    printf( "rate constant of %s is %g" NEW_LINE, GetCharArrayOfString( lawString ), *result ); 
    FreeString( &lawString );
#endif    
            
    END_FUNCTION("FindRateConstant", SUCCESS );
    return TRUE;
}

KINETIC_LAW *CreateRateConstantKineticLaw( KINETIC_LAW *kineticLaw ) {
    KINETIC_LAW *rateConstant = NULL;
#ifdef DEBUG
    STRING *lawString = NULL;
#endif    
    START_FUNCTION("CreateRateConstantKineticLaw");
    
    if( ( rateConstant = _CreateRateConstantKineticLaw( kineticLaw ) ) == NULL ) {
        END_FUNCTION("CreateRateConstantKineticLaw", FAILING );
        return NULL;
    }

#ifdef DEBUG
    lawString = ToStringKineticLaw( kineticLaw );
    printf( "rate constant of %s is ", GetCharArrayOfString( lawString ) ); 
    FreeString( &lawString );
    lawString = ToStringKineticLaw( rateConstant );
    printf( "%s" NEW_LINE, GetCharArrayOfString( lawString ) ); 
    FreeString( &lawString );
#endif    
            
    END_FUNCTION("CreateRateConstantKineticLaw", SUCCESS );
    return rateConstant;
}

#if 0
BOOL FindCriticalConcentrationLevel( REACTION *reaction, SPECIES *species, double *result ) {
    double rateRatio = 0.0;    
    KINETIC_LAW *kineticLaw = NULL;
    KINETIC_LAW *temp = NULL;
    LINKED_LIST *list = NULL;
    
    START_FUNCTION("FindCriticalConcentrationLevel");

    kineticLaw = GetKineticLawInReactionNode( reaction );    
    if( IsReactionReversibleInReactionNode( reaction ) ) {
        if( !FindRateConstantRatio( kineticLaw, &rateRatio ) ) {
            END_FUNCTION("FindCriticalConcentrationLevel", SUCCESS );
            return FALSE;
        }
        temp = GetOpLeftFromKineticLaw( kineticLaw );
        if( _FindSpecies( temp, species ) ) {
            *result = 1.0 / rateRatio;
            END_FUNCTION("FindCriticalConcentrationLevel", SUCCESS );
            return TRUE;
        }
        temp = GetOpRightFromKineticLaw( kineticLaw );        
        if( _FindSpecies( temp, species ) ) {
            *result = rateRatio;
            END_FUNCTION("FindCriticalConcentrationLevel", SUCCESS );
            return TRUE;
        }
        END_FUNCTION("FindCriticalConcentrationLevel", SUCCESS );
        return FALSE;
    }
    else {
        if( ( list = _CreateListOfMultipleTerms( kineticLaw ) ) == NULL ) {
            END_FUNCTION("FindCriticalConcentrationLevel", SUCCESS );
            return FALSE;            
        }
        ResetCurrentElement( list );
        while( ( kineticLaw = (KINETIC_LAW*)GetNextFromLinkedList( list ) ) != NULL ) {
            if( ( temp = _FindDivisor( kineticLaw ) ) != NULL ) {
                if( _FindCriticalConcentrationLevel( temp, species, result ) ) {
                    DeleteLinkedList( &list );
                    END_FUNCTION("FindCriticalConcentrationLevel", SUCCESS );
                    return TRUE;            
                }         
            }
        }
        DeleteLinkedList( &list );
        END_FUNCTION("FindCriticalConcentrationLevel", SUCCESS );
        return FALSE;            
    }
        
    END_FUNCTION("FindCriticalConcentrationLevel", SUCCESS );
    return TRUE;
}
#endif



RET_VAL AddListOfCriticalConcentrationLevels( REACTION *reaction, SPECIES *species, LINKED_LIST *conList ) {
    RET_VAL ret = SUCCESS;
    double rateRatio = 0.0;    
    double *result = NULL;
    KINETIC_LAW *kineticLaw = NULL;
    KINETIC_LAW *temp = NULL;
    LINKED_LIST *list = NULL;
    
    START_FUNCTION("AddListOfCriticalConcentrationLevels");

    kineticLaw = GetKineticLawInReactionNode( reaction );    
    if( IsReactionReversibleInReactionNode( reaction ) ) {
        if( !FindRateConstantRatio( kineticLaw, &rateRatio ) ) {
            END_FUNCTION("AddListOfCriticalConcentrationLevels", SUCCESS );
            return ret;
        }
        temp = GetOpLeftFromKineticLaw( kineticLaw );
        if( _FindSpecies( temp, species ) ) {
            if( ( result = (double*)MALLOC( sizeof( double ) ) ) == NULL ) {
                return ErrorReport( FAILING, "AddListOfCriticalConcentrationLevels", "could not allocate memory space for critical concentration level" );
            }            
            *result = 1.0 / rateRatio;
            if( IS_FAILED( ( ret = AddElementInLinkedList( (CADDR_T)result, conList ) ) ) ) {
                END_FUNCTION("AddListOfCriticalConcentrationLevels", ret );
                return ret;
            }
            TRACE_3( "critical concentration of %s found in %s is %f", 
                GetCharArrayOfString( GetSpeciesNodeName( species ) ), GetCharArrayOfString( GetReactionNodeName( reaction ) ), *result );
            END_FUNCTION("AddListOfCriticalConcentrationLevels", SUCCESS );
            return ret;
        }
        temp = GetOpRightFromKineticLaw( kineticLaw );        
        if( _FindSpecies( temp, species ) ) {
            if( ( result = (double*)MALLOC( sizeof( double ) ) ) == NULL ) {
                return ErrorReport( FAILING, "AddListOfCriticalConcentrationLevels", "could not allocate memory space for critical concentration level" );
            }
            *result = rateRatio;
            if( IS_FAILED( ( ret = AddElementInLinkedList( (CADDR_T)result, conList ) ) ) ) {
                END_FUNCTION("AddListOfCriticalConcentrationLevels", ret );
                return ret;
            }
            TRACE_3( "critical concentration of %s found in %s is %f", 
                GetCharArrayOfString( GetSpeciesNodeName( species ) ), GetCharArrayOfString( GetReactionNodeName( reaction ) ), *result );
            END_FUNCTION("AddListOfCriticalConcentrationLevels", SUCCESS );
            return ret;
        }
        END_FUNCTION("AddListOfCriticalConcentrationLevels", SUCCESS );
        return ret;
    }
    else {
        if( ( list = _CreateListOfMultipleTerms( kineticLaw ) ) == NULL ) {
            END_FUNCTION("AddListOfCriticalConcentrationLevels", SUCCESS );
            return ret;            
        }
        ResetCurrentElement( list );
        while( ( kineticLaw = (KINETIC_LAW*)GetNextFromLinkedList( list ) ) != NULL ) {
            if( ( temp = _FindDivisor( kineticLaw ) ) != NULL ) {
                TRACE_2( "finding critical concentration levels of %s in %s", 
                    GetCharArrayOfString( GetSpeciesNodeName( species ) ), GetCharArrayOfString( GetReactionNodeName( reaction ) ) );
                if( IS_FAILED( ( ret = _AddListOfCriticalConcentrationLevels( temp, species, conList ) ) ) ) {
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
        
    END_FUNCTION("AddListOfCriticalConcentrationLevels", SUCCESS );
    return ret;
}


KINETIC_LAW *CreateMassActionRatioKineticLaw( REACTION *reaction, SPECIES *op ) {
    double rateRatio = 0.0;
    KINETIC_LAW *law = NULL;
    KINETIC_LAW *left = NULL;        
    KINETIC_LAW *massActionRatio = NULL;
#ifdef DEBUG
    STRING *lawString = NULL;
#endif    
    START_FUNCTION("CreateMassActionRatioKineticLaw");
    
    law = GetKineticLawInReactionNode( reaction );
    if( !FindRateConstantRatio( law, &rateRatio ) ) {
        END_FUNCTION("CreateMassActionRatioKineticLaw", FAILING );
        return NULL;
    }
    
    left = GetOpLeftFromKineticLaw( law );
    if( ( massActionRatio = _CreateMassActionRatio( left, op, rateRatio ) ) == NULL ) {
        END_FUNCTION("CreateMassActionRatioKineticLaw", FAILING );
        return NULL;
    } 
    
#ifdef DEBUG
    lawString = ToStringKineticLaw( massActionRatio );
    printf( "mass action ratio of %s is %s" NEW_LINE, GetCharArrayOfString( GetSpeciesNodeName( op ) ), GetCharArrayOfString( lawString ) ); 
    FreeString( &lawString );
#endif    
        
    END_FUNCTION("_CreateMassActionRatio", SUCCESS );
    return massActionRatio;
}


KINETIC_LAW *CreateMassActionRatioKineticLawWithRateConstantInKineticLaw( REACTION *reaction, SPECIES *op ) {
    KINETIC_LAW *kineticLaw = NULL;
    KINETIC_LAW *left = NULL;        
    KINETIC_LAW *right = NULL;        
    KINETIC_LAW *term = NULL;
    LINKED_LIST *list = NULL;
    KINETIC_LAW *massActionRatio = NULL;
#ifdef DEBUG
    STRING *lawString = NULL;
#endif    
    
    START_FUNCTION("CreateMassActionRatioKineticLawWithRateConstantInKineticLaw");
    
    kineticLaw = GetKineticLawInReactionNode( reaction );
    if( ( massActionRatio = CreateRateConstantRatioKineticLaw( kineticLaw ) ) == NULL ) {
        END_FUNCTION("CreateMassActionRatioKineticLawWithRateConstantInKineticLaw", FAILING );
        return NULL;
    }
    
    left = GetOpLeftFromKineticLaw( kineticLaw );
    if( ( list = _CreateListOfMultipleTerms( left ) ) == NULL ) {
        END_FUNCTION("CreateMassActionRatioKineticLawWithRateConstantInKineticLaw", FAILING );
        return NULL;
    } 
    
    ResetCurrentElement( list );
    while( ( term = (KINETIC_LAW*)GetNextFromLinkedList( list ) ) != NULL ) {
        if( !_ContainSpecies( term ) ) {
            continue;
        }
        if( IsSpeciesKineticLaw( term ) ) {
            if( op == GetSpeciesFromKineticLaw( term ) ) {
                continue;
            }
        }
        if( ( massActionRatio = CreateOpKineticLaw( KINETIC_LAW_OP_TIMES, massActionRatio, CloneKineticLaw( term ) ) ) == NULL ) {
#ifdef DEBUG
            lawString = ToStringKineticLaw( massActionRatio );
            printf( "failed to %s", GetCharArrayOfString( lawString ) );
            FreeString( &lawString );
            lawString = ToStringKineticLaw( term );
            printf( "* %s" NEW_LINE, GetCharArrayOfString( lawString ) );
            FreeString( &lawString );
#endif    
            END_FUNCTION("CreateMassActionRatioKineticLawWithRateConstantInKineticLaw", FAILING );
            return NULL;
        } 
    }
    
    DeleteLinkedList( &list );
#ifdef DEBUG
    lawString = ToStringKineticLaw( massActionRatio );
    printf( "mass action ratio of %s in %s is" NEW_LINE, GetCharArrayOfString( GetSpeciesNodeName( op ) ), GetCharArrayOfString( GetReactionNodeName( reaction ) ) );
    printf( "%s" NEW_LINE, GetCharArrayOfString( lawString ) ); 
    FreeString( &lawString );
#endif    

    END_FUNCTION("CreateMassActionRatioKineticLawWithRateConstantInKineticLaw", SUCCESS );
    return massActionRatio;

#if 0    
    KINETIC_LAW *rateRatio = NULL;
    KINETIC_LAW *law = NULL;
    KINETIC_LAW *left = NULL;        
    KINETIC_LAW *massActionRatio = NULL;
#ifdef DEBUG
    STRING *lawString = NULL;
#endif    
    START_FUNCTION("CreateMassActionRatioKineticLawWithRateConstantInKineticLaw");

    law = GetKineticLawInReactionNode( reaction );
    if( ( rateRatio = CreateRateConstantRatioKineticLaw( law ) ) == NULL ) {
        END_FUNCTION("CreateMassActionRatioKineticLawWithRateConstantInKineticLaw", FAILING );
        return NULL;
    }
    
    left = GetOpLeftFromKineticLaw( law );
    if( ( massActionRatio = _CreateMassActionRatioWithRateConstantInKineticLaw( left, op, rateRatio ) ) == NULL ) {
        END_FUNCTION("CreateMassActionRatioKineticLaw", FAILING );
        return NULL;
    } 
    
#ifdef DEBUG
    lawString = ToStringKineticLaw( massActionRatio );
    printf( "mass action ratio of %s in %s is" NEW_LINE, GetCharArrayOfString( GetSpeciesNodeName( op ) ), GetCharArrayOfString( GetReactionNodeName( reaction ) ) );
    printf( "%s" NEW_LINE, GetCharArrayOfString( lawString ) ); 
    FreeString( &lawString );
#endif    
    
    END_FUNCTION("CreateMassActionRatioKineticLawWithRateConstantInKineticLaw", SUCCESS );
    return massActionRatio;
#endif
}


KINETIC_LAW *CreateTotalConcentrationKineticLaw( SPECIES *enzyme, REB2SAC_SYMTAB *symtab, double totalCon ) {
    char buf[256];
    REB2SAC_SYMBOL *sym = NULL;
    KINETIC_LAW *kineticLaw = NULL;
    
    START_FUNCTION("_CreateTotalConcentrationKineticLaw");
    
    strcpy( buf, GetCharArrayOfString( GetSpeciesNodeName( enzyme ) ) );
    strcat( buf, "_total" );
    if( ( sym = symtab->AddRealValueSymbol( symtab, buf, totalCon, TRUE ) ) == NULL ) {
        END_FUNCTION("_CreateTotalConcentrationKineticLaw", FAILING );
        return NULL;
    }
    
    if( ( kineticLaw = CreateSymbolKineticLaw( sym ) ) == NULL ) {
        END_FUNCTION("_CreateTotalConcentrationKineticLaw", FAILING );
        return NULL;
    }
    
    END_FUNCTION("_CreateTotalConcentrationKineticLaw", SUCCESS );
    return kineticLaw;
}

KINETIC_LAW *CreateConcentrationKineticLaw( SPECIES *species, REB2SAC_SYMTAB *symtab, double concentration ) {
    char buf[256];
    REB2SAC_SYMBOL *sym = NULL;
    KINETIC_LAW *kineticLaw = NULL;
    
    START_FUNCTION("CreateConcentrationKineticLaw");
    
    strcpy( buf, GetCharArrayOfString( GetSpeciesNodeName( species ) ) );
    strcat( buf, "_con" );
    if( ( sym = symtab->AddRealValueSymbol( symtab, buf, concentration, TRUE ) ) == NULL ) {
        END_FUNCTION("CreateConcentrationKineticLaw", FAILING );
        return NULL;
    }
    
    if( ( kineticLaw = CreateSymbolKineticLaw( sym ) ) == NULL ) {
        END_FUNCTION("CreateConcentrationKineticLaw", FAILING );
        return NULL;
    }
    
    END_FUNCTION("CreateConcentrationKineticLaw", SUCCESS );
    return kineticLaw;
}




static BOOL  _FindRateConstant( KINETIC_LAW *kineticLaw, double *result ) {
#if 0    
    KINETIC_LAW_EVALUATER *evaluater = NULL;
    double rateConstant = 1.0;
    KINETIC_LAW *term = NULL; 
    LINKED_LIST *terms = NULL;

    START_FUNCTION("_FindRateConstant");

    if( ( evaluater = CreateKineticLawEvaluater() ) == NULL ) {
        END_FUNCTION("_FindRateConstant", FAILING );
        return FALSE;
    } 
    
    if( ( terms = _CreateListOfMultipleTerms( kineticLaw ) ) == NULL ) {
        END_FUNCTION("_FindRateConstant", FAILING );
        return FALSE;
    }
    ResetCurrentElement( terms );
    while( ( term = (KINETIC_LAW*)GetNextFromLinkedList( terms ) ) != NULL ) {
        if( !_ContainSpecies( term ) ) {
            rateConstant *= evaluater->Evaluate( evaluater, term );
        }        
    }
    FreeKineticLawEvaluater( &evaluater );
    *result = rateConstant;
    END_FUNCTION("_FindRateConstant", SUCCESS );
    return TRUE;

#else    
    static KINETIC_LAW_VISITOR rateConstantFinderVisitor;
    double rateConstant = 0.0; 

    START_FUNCTION("_FindRateConstant");
    
    if( rateConstantFinderVisitor.VisitOp == NULL ) {
        rateConstantFinderVisitor.VisitOp = _VisitOpToFindRateConstant;
        rateConstantFinderVisitor.VisitInt = _VisitIntToFindRateConstant;
        rateConstantFinderVisitor.VisitReal = _VisitRealToFindRateConstant;
        rateConstantFinderVisitor.VisitSpecies = _VisitSpeciesToFindRateConstant;
        rateConstantFinderVisitor.VisitSymbol = _VisitSymbolToFindRateConstant;
    }
    rateConstant = 1.0;
    rateConstantFinderVisitor._internal1 = (CADDR_T)(&rateConstant);
    
    if( IS_FAILED( kineticLaw->Accept( kineticLaw, &rateConstantFinderVisitor ) ) ) {
        END_FUNCTION("_FindRateConstant", SUCCESS );
        return FALSE;
    } 
    *result = rateConstant;
    END_FUNCTION("_FindRateConstant", SUCCESS );
    return TRUE;
#endif
}

static RET_VAL _VisitOpToFindRateConstant( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    RET_VAL ret = SUCCESS;
    BYTE opType = 0;
    double leftValue = 0.0;
    double rightValue = 0.0;
    double *result = NULL;
    KINETIC_LAW *left = NULL;
    KINETIC_LAW *right = NULL;
    
    START_FUNCTION("_VisitOpToFindRateConstant");
        
    opType = GetOpTypeFromKineticLaw( kineticLaw );    
    switch( opType ) {
        case KINETIC_LAW_OP_TIMES:
            result = (double*)(visitor->_internal1);
            
            left = GetOpLeftFromKineticLaw( kineticLaw );
            visitor->_internal1 = (CADDR_T)(&leftValue);
            if( IS_FAILED( ( ret = left->Accept( left, visitor ) ) ) ) {
                END_FUNCTION("_VisitOpToFindRateConstant", ret );
                return ret;
            }
            right = GetOpRightFromKineticLaw( kineticLaw );
            visitor->_internal1 = (CADDR_T)(&rightValue);
            if( IS_FAILED( ( ret = right->Accept( right, visitor ) ) ) ) {
                END_FUNCTION("_VisitOpToFindRateConstant", ret );
                return ret;
            }
            *result = leftValue * rightValue;            
            END_FUNCTION("_VisitOpToFindRateConstant", SUCCESS );
        return ret;
        
        case KINETIC_LAW_OP_DIVIDE:
            result = (double*)(visitor->_internal1);
            
            left = GetOpLeftFromKineticLaw( kineticLaw );
            visitor->_internal1 = (CADDR_T)(&leftValue);
            if( IS_FAILED( ( ret = left->Accept( left, visitor ) ) ) ) {
                END_FUNCTION("_VisitOpToFindRateConstant", ret );
                return ret;
            }
            right = GetOpRightFromKineticLaw( kineticLaw );
            visitor->_internal1 = (CADDR_T)(&rightValue);
            if( IS_FAILED( ( ret = right->Accept( right, visitor ) ) ) ) {
                END_FUNCTION("_VisitOpToFindRateConstant", ret );
                return ret;
            }
            *result = leftValue / rightValue;            
            END_FUNCTION("_VisitOpToFindRateConstant", SUCCESS );
        return ret;

        case KINETIC_LAW_OP_POW:
            result = (double*)(visitor->_internal1);
            
            left = GetOpLeftFromKineticLaw( kineticLaw );
            visitor->_internal1 = (CADDR_T)(&leftValue);
            if( IS_FAILED( ( ret = left->Accept( left, visitor ) ) ) ) {
                END_FUNCTION("_VisitOpToFindRateConstant", ret );
                return ret;
            }
            right = GetOpRightFromKineticLaw( kineticLaw );
            visitor->_internal1 = (CADDR_T)(&rightValue);
            if( IS_FAILED( ( ret = right->Accept( right, visitor ) ) ) ) {
                END_FUNCTION("_VisitOpToFindRateConstant", ret );
                return ret;
            }
            *result = pow( leftValue, rightValue );            
            END_FUNCTION("_VisitOpToFindRateConstant", SUCCESS );
        return SUCCESS;

        default:
            END_FUNCTION("_VisitOpToFindRateConstant", E_WRONGDATA );
        return E_WRONGDATA;
    }
    END_FUNCTION("_VisitOpToFindRateConstant", FAILING );
    return FAILING;
}

static RET_VAL _VisitIntToFindRateConstant( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    double value = 0.0;
    double *rateConstant = NULL;
            
    START_FUNCTION("_VisitIntToFindRateConstant");
    
    value = (double)GetIntValueFromKineticLaw( kineticLaw );
    rateConstant = (double*)(visitor->_internal1);
    *rateConstant = value;
    
    END_FUNCTION("_VisitIntToFindRateConstant", SUCCESS );
    return SUCCESS;
}

static RET_VAL _VisitRealToFindRateConstant( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    double value = 0.0;
    double *rateConstant = NULL;
            
    START_FUNCTION("_VisitRealToFindRateConstant");
    
    value = GetRealValueFromKineticLaw( kineticLaw );
    rateConstant = (double*)(visitor->_internal1);
    *rateConstant = value;
    
    END_FUNCTION("_VisitRealToFindRateConstant", SUCCESS );
    return SUCCESS;
}

static RET_VAL _VisitSpeciesToFindRateConstant( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    double *rateConstant = NULL;
    
    START_FUNCTION("_VisitSpeciesToFindRateConstant");
    
    rateConstant = (double*)(visitor->_internal1);
    *rateConstant = 1.0;
    
    END_FUNCTION("_VisitSpeciesToFindRateConstant", SUCCESS );
    return SUCCESS;
}


static RET_VAL _VisitSymbolToFindRateConstant( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    double value = 0.0;
    double *rateConstant = NULL;
    REB2SAC_SYMBOL *sym = NULL;
            
    START_FUNCTION("_VisitSymbolToFindRateConstant");
    
    rateConstant = (double*)(visitor->_internal1);
    sym = GetSymbolFromKineticLaw( kineticLaw );
    if( !IsSymbolConstant( sym ) ) {
        *rateConstant = 1.0;
        END_FUNCTION("_VisitSymbolToFindRateConstant", SUCCESS );
        return SUCCESS;
    }
    if( !IsRealValueSymbol( sym ) ) {
        *rateConstant = 1.0;
        END_FUNCTION("_VisitSymbolToFindRateConstant", SUCCESS );
        return SUCCESS;
    }
    
    value = GetRealValueInSymbol( sym );    
    rateConstant = (double*)(visitor->_internal1);
    *rateConstant = value;
    
    END_FUNCTION("_VisitSymbolToFindRateConstant", SUCCESS );
    return SUCCESS;
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


static BOOL _ContainSpecies( KINETIC_LAW *kineticLaw ) {
    static KINETIC_LAW_VISITOR visitor;
    BOOL flag = FALSE;
    
    START_FUNCTION("_ContainSpecies");
    
    if( visitor.VisitOp == NULL ) {
        visitor.VisitOp = _VisitOpToContainSpecies;
        visitor.VisitInt = _VisitIntToContainSpecies;
        visitor.VisitReal = _VisitRealToContainSpecies;
        visitor.VisitSpecies = _VisitSpeciesToContainSpecies;
        visitor.VisitSymbol = _VisitSymbolToContainSpecies;
    }
    
    visitor._internal1 = (CADDR_T)(&flag);
    
    if( IS_FAILED( kineticLaw->AcceptPostOrder( kineticLaw, &visitor ) ) ) {
        END_FUNCTION("_ContainSpecies", SUCCESS );
        return FALSE;
    } 
    
    END_FUNCTION("_ContainSpecies", SUCCESS );
    return flag;
}

static RET_VAL _VisitOpToContainSpecies( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    START_FUNCTION("_VisitOpToContainSpecies");
    END_FUNCTION("_VisitOpToContainSpecies", SUCCESS );
    return SUCCESS;
}

static RET_VAL _VisitIntToContainSpecies( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    START_FUNCTION("_VisitIntToContainSpecies");
    END_FUNCTION("_VisitIntToContainSpecies", SUCCESS );
    return SUCCESS;
}

static RET_VAL _VisitRealToContainSpecies( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    START_FUNCTION("_VisitRealToContainSpecies");
    END_FUNCTION("_VisitRealToContainSpecies", SUCCESS );
    return SUCCESS;
}

static RET_VAL _VisitSpeciesToContainSpecies( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    BOOL *flag = NULL;
    
    START_FUNCTION("_VisitSpeciesToContainSpecies");
    
    flag = (BOOL*)(visitor->_internal1);
    *flag = TRUE;
    
    END_FUNCTION("_VisitSpeciesToContainSpecies", SUCCESS );
    return SUCCESS;
}

static RET_VAL _VisitSymbolToContainSpecies( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    START_FUNCTION("_VisitSymbolToContainSpecies");
    END_FUNCTION("_VisitSymbolToContainSpecies", SUCCESS );
    return SUCCESS;
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


static BOOL _FindCriticalConcentrationLevel( KINETIC_LAW *kineticLaw, SPECIES *species, double *result ) {
    static KINETIC_LAW_VISITOR visitor;
    double rateConstant = 1.0;
    double power = 0.0;
    KINETIC_LAW *term = NULL;
    LINKED_LIST *terms = NULL;
    START_FUNCTION("_FindCriticalConcentrationLevel");
    
    if( ( terms = _CreateListOfMultipleTerms( kineticLaw ) ) == NULL ) {
        END_FUNCTION("_FindCriticalConcentrationLevel", FAILING );
        return FALSE;
    }

    ResetCurrentElement( terms );
    while( ( term = (KINETIC_LAW*)GetNextFromLinkedList( terms ) ) != NULL ) {
        if( _FindMultiplicationOfSpecies( term, species, &power ) ) {
#if 0
        if( _IsKineticLawMultipleOfSpecies( term, species ) ) {
#endif
            if( FindRateConstant( kineticLaw, &rateConstant ) ) {
                if( IS_REAL_EQUAL( rateConstant, 0.0 ) ) {
                    END_FUNCTION("_FindCriticalConcentrationLevel", SUCCESS );
                    return FALSE;
                }
                if( IS_REAL_EQUAL( power, 0.0 ) ) {
                    DeleteLinkedList( &terms );
                    END_FUNCTION("_FindCriticalConcentrationLevel", SUCCESS );
                    return FALSE;
                }
                *result = pow( 1.0 / rateConstant, 1.0 / power );
                DeleteLinkedList( &terms );
                END_FUNCTION("_FindCriticalConcentrationLevel", SUCCESS );
                return TRUE;
            } 
        }        
    }
    
    DeleteLinkedList( &terms );
    END_FUNCTION("_FindCriticalConcentrationLevel", SUCCESS );
    return FALSE;
}



static RET_VAL _AddListOfCriticalConcentrationLevels( KINETIC_LAW *kineticLaw, SPECIES *species, LINKED_LIST *list ) {
    RET_VAL ret = SUCCESS;
    double value = 0.0;    
    double *criticalCon = NULL;
    KINETIC_LAW *term = NULL;
    LINKED_LIST *terms = NULL;
    STRING *string = NULL;
    
    START_FUNCTION("_AddListOfCriticalConcentrationLevels");
    
    if( ( terms = _CreateListOfSumTerms( kineticLaw ) ) == NULL ) {
        string = ToStringKineticLaw( kineticLaw );
        return ErrorReport( FAILING, "_AddListOfCriticalConcentrationLevels", "could not create a list of sum terms for %s", GetCharArrayOfString( string ) );
    }
    
    ResetCurrentElement( terms );
    while( ( term = (KINETIC_LAW*)GetNextFromLinkedList( terms ) ) != NULL ) {
        if( IS_FAILED( ( ret = __AddListOfCriticalConcentrationLevels( term, species, list ) ) ) ) {
            END_FUNCTION("_AddListOfCriticalConcentrationLevels", ret );
            return ret;
        }
        
        if( _FindCriticalConcentrationLevel( term, species, &value ) ) {
            if( ( criticalCon = (double*)MALLOC( sizeof(double) ) ) == NULL ) {
                return ErrorReport( FAILING, "_AddListOfCriticalConcentrationLevels", "could not allocate space for critical concentration %f", value );
            }
            *criticalCon = value;
#ifdef DEBUG
            string = ToStringKineticLaw( term );
            printf("critical concentration level %f is found from %s" NEW_LINE, value, GetCharArrayOfString( string ) );
            FreeString( &string );
#endif
            if( IS_FAILED( ( ret = AddElementInLinkedList( (CADDR_T)criticalCon, list ) ) ) ) {
                END_FUNCTION("_AddListOfCriticalConcentrationLevels", ret );
                return ret;
            }
        }
    }
    DeleteLinkedList( &terms );
    
    END_FUNCTION("_AddListOfCriticalConcentrationLevels", SUCCESS );
    return ret;
}

static RET_VAL __AddListOfCriticalConcentrationLevels( KINETIC_LAW *kineticLaw, SPECIES *species, LINKED_LIST *list ) {
    RET_VAL ret = SUCCESS;
    BOOL foundTarget = FALSE;
    double rateConstant = 1.0;
    double *concentration = NULL;
    KINETIC_LAW *term = NULL;
    KINETIC_LAW *numer = NULL;
    KINETIC_LAW *denom = NULL;
    LINKED_LIST *terms = NULL;
#ifdef DEBUG
    STRING *string = NULL;
#endif    
    
    START_FUNCTION("__AddListOfCriticalConcentrationLevels");
    
    if( ( terms = _CreateListOfMultipleTerms( kineticLaw ) ) == NULL ) {
        return ErrorReport( FAILING, "__AddListOfCriticalConcentrationLevels", "could not create a list of multiple terms" );
    }

    ResetCurrentElement( terms );
    while( ( term = (KINETIC_LAW*)GetNextFromLinkedList( terms ) ) != NULL ) {
        if( IsOpKineticLaw( term ) ) {
            if( GetOpTypeFromKineticLaw( term ) == KINETIC_LAW_OP_DIVIDE ) {
                denom = GetOpRightFromKineticLaw( term );
                if( _FindSpecies( denom, species ) ) {
                    if( IS_FAILED( ( ret = _AddListOfCriticalConcentrationLevels( denom, species, list ) ) ) ) {
                        END_FUNCTION("__AddListOfCriticalConcentrationLevels", ret );
                        return ret;
                    }                    
                }
                numer = GetOpLeftFromKineticLaw( term );
                if( _FindSpecies( numer, species ) ) {
                    if( IS_FAILED( ( ret = _AddListOfCriticalConcentrationLevels( numer, species, list ) ) ) ) {
                        END_FUNCTION("__AddListOfCriticalConcentrationLevels", ret );
                        return ret;
                    }                    
                }
                
            }
        }        
        
        if( _IsKineticLawMultipleOfSpecies( term, species ) ) {
            foundTarget = TRUE;
            continue;
        }
    }
    
    DeleteLinkedList( &terms );
        
    if( !foundTarget ) {
        END_FUNCTION("_FindCriticalConcentrationLevel", SUCCESS );
        return ret;
    }
        
    if( FindRateConstant( kineticLaw, &rateConstant ) ) {
        if( IS_REAL_EQUAL( rateConstant, 0.0 ) ) {
            END_FUNCTION("__AddListOfCriticalConcentrationLevels", SUCCESS );
            return ret;
        } 
        if( ( concentration = (double*)MALLOC( sizeof(double) ) ) == NULL ) {
            return ErrorReport( FAILING, "__AddListOfCriticalConcentrationLevels", "could not allocate space for critical concentration %f", rateConstant );
        }
        *concentration = 1.0 / rateConstant;
#ifdef DEBUG
            string = ToStringKineticLaw( kineticLaw );
            printf("critical concentration level %f is found from %s" NEW_LINE, *concentration, GetCharArrayOfString( string ) );
            FreeString( &string );
#endif
        if( IS_FAILED( ( ret = AddElementInLinkedList( (CADDR_T)concentration, list ) ) ) ) {
                END_FUNCTION("__AddListOfCriticalConcentrationLevels", ret );
                return ret;
        }
    }
    
    END_FUNCTION("_FindCriticalConcentrationLevel", SUCCESS );
    return ret;
}


static BOOL _IsKineticLawMultipleOfSpecies(  KINETIC_LAW *kineticLaw, SPECIES *species ) {
    static KINETIC_LAW_VISITOR visitor;
    
    START_FUNCTION("_IsKineticLawMultipleOfSpecies");
    
    if( visitor.VisitOp == NULL ) {
        visitor.VisitOp = _VisitOpMultipleOfSpecies;
        visitor.VisitInt = _VisitIntMultipleOfSpecies;
        visitor.VisitReal = _VisitRealMultipleOfSpecies;
        visitor.VisitSpecies = _VisitSpeciesMultipleOfSpecies;
        visitor.VisitSymbol = _VisitSymbolMultipleOfSpecies;
    }
    
    visitor._internal1 = (CADDR_T)species;
    
    if( IS_FAILED( kineticLaw->Accept( kineticLaw, &visitor ) ) ) {
        END_FUNCTION("_IsKineticLawMultipleOfSpecies", FAILING );
        return FALSE;
    } 
    
    END_FUNCTION("_IsKineticLawMultipleOfSpecies", SUCCESS );
    return TRUE;
}

static RET_VAL _VisitOpMultipleOfSpecies( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    RET_VAL ret = SUCCESS;
    BYTE opType = 0;
    KINETIC_LAW *left = NULL;
    KINETIC_LAW *right = NULL;
    
    START_FUNCTION("_VisitOpMultipleOfSpecies");
        
    opType = GetOpTypeFromKineticLaw( kineticLaw );    
    switch( opType ) {
        case KINETIC_LAW_OP_TIMES:
            left = GetOpLeftFromKineticLaw( kineticLaw );
            right = GetOpRightFromKineticLaw( kineticLaw );
            if( IS_FAILED( ( ret = left->Accept( left, visitor ) ) ) ) {
                END_FUNCTION("_VisitOpMultipleOfSpecies", ret );
                return ret;
            }
            if( IS_FAILED( ( ret = right->Accept( right, visitor ) ) ) ) {
                END_FUNCTION("_VisitOpMultipleOfSpecies", ret );
                return ret;
            }            
            END_FUNCTION("_VisitOpMultipleOfSpecies", SUCCESS );
        return ret;

        case KINETIC_LAW_OP_POW:
            left = GetOpLeftFromKineticLaw( kineticLaw );
            if( IS_FAILED( ( ret = left->Accept( left, visitor ) ) ) ) {
                END_FUNCTION("_VisitOpMultipleOfSpecies", ret );
                return ret;
            }
            END_FUNCTION("_VisitOpMultipleOfSpecies", SUCCESS );
        return ret;

        default:
            END_FUNCTION("_VisitOpMultipleOfSpecies", E_WRONGDATA );
        return E_WRONGDATA;
    }
    END_FUNCTION("_VisitOpMultipleOfSpecies", FAILING );
    return FAILING;
}

static RET_VAL _VisitIntMultipleOfSpecies( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    START_FUNCTION("_VisitIntMultipleOfSpecies");
    END_FUNCTION("_VisitIntMultipleOfSpecies", E_WRONGDATA );
    return E_WRONGDATA;
}

static RET_VAL _VisitRealMultipleOfSpecies( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    START_FUNCTION("_VisitRealMultipleOfSpecies");
    END_FUNCTION("_VisitRealMultipleOfSpecies", E_WRONGDATA );
    return E_WRONGDATA;
}


static RET_VAL _VisitSymbolMultipleOfSpecies( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    START_FUNCTION("_VisitSymbolMultipleOfSpecies");
    END_FUNCTION("_VisitSymbolMultipleOfSpecies", E_WRONGDATA );
    return E_WRONGDATA;
}


static RET_VAL _VisitSpeciesMultipleOfSpecies( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    SPECIES *species = NULL;
    SPECIES *target = NULL;

    START_FUNCTION("_VisitSpeciesMultipleOfSpecies");
    
    species = GetSpeciesFromKineticLaw( kineticLaw );
    target = (SPECIES*)(visitor->_internal1);
    
    if( species != target ) {
        END_FUNCTION("_VisitSpeciesMultipleOfSpecies", E_WRONGDATA );
        return E_WRONGDATA;
    }
    
    END_FUNCTION("_VisitSpeciesMultipleOfSpecies", SUCCESS );
    return SUCCESS;
}


static KINETIC_LAW *_CreateMassActionRatio( KINETIC_LAW *forward, SPECIES *op, double rateRatio  ) {
    static KINETIC_LAW_VISITOR visitor;
    KINETIC_LAW *massActionRatio = NULL;
    
    START_FUNCTION("_CreateMassActionRatio");
    
    if( visitor.VisitOp == NULL ) {
        visitor.VisitOp = _VisitOpToCreateMassActionRatio;
        visitor.VisitInt = _VisitIntToCreateMassActionRatio;
        visitor.VisitReal = _VisitRealToCreateMassActionRatio;
        visitor.VisitSpecies = _VisitSpeciesToCreateMassActionRatio;
        visitor.VisitSymbol = _VisitSymbolToCreateMassActionRatio;
    }
    
    if( ( massActionRatio = CreateRealValueKineticLaw( rateRatio ) ) == NULL ) {
        END_FUNCTION("_CreateMassActionRatio", FAILING );
        return NULL;
    }
    
    visitor._internal1 = (CADDR_T)(&massActionRatio);
    visitor._internal2 = (CADDR_T)op;
    
    if( IS_FAILED( forward->Accept( forward, &visitor ) ) ) {
        END_FUNCTION("_CreateMassActionRatio", FAILING );
        return NULL;
    } 
    
    END_FUNCTION("_CreateMassActionRatio", SUCCESS );
    return massActionRatio;
}

static RET_VAL _VisitOpToCreateMassActionRatio( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    RET_VAL ret = SUCCESS;
    BYTE opType = 0x00;
    SPECIES *species = NULL;
    SPECIES *target = NULL;
    KINETIC_LAW *left = NULL;
    KINETIC_LAW *right = NULL;
    KINETIC_LAW **massActionRatio = NULL;
    
    START_FUNCTION("_VisitOpToCreateMassActionRatio");
    opType = GetOpTypeFromKineticLaw( kineticLaw );    
    switch( opType ) {
        case KINETIC_LAW_OP_TIMES:
            left = GetOpLeftFromKineticLaw( kineticLaw );
            right = GetOpRightFromKineticLaw( kineticLaw );
            if( IS_FAILED( ( ret = left->Accept( left, visitor ) ) ) ) {
                END_FUNCTION("_VisitOpToCreateMassActionRatio", ret );
                return ret;
            }
            if( IS_FAILED( ( ret = right->Accept( right, visitor ) ) ) ) {
                END_FUNCTION("_VisitOpToCreateMassActionRatio", ret );
                return ret;
            }            
            END_FUNCTION("_VisitOpToCreateMassActionRatio", SUCCESS );
        return ret;

        case KINETIC_LAW_OP_POW:
            left = GetOpLeftFromKineticLaw( kineticLaw );
            
            if( !IsSpeciesKineticLaw( left ) ) {
                END_FUNCTION("_VisitOpToCreateMassActionRatio", E_WRONGDATA );
                return E_WRONGDATA;
            }
            species = GetSpeciesFromKineticLaw( left );
            target = (SPECIES*)(visitor->_internal2);
            if( species == target ) {
                END_FUNCTION("_VisitOpToCreateMassActionRatio", E_WRONGDATA );
                return E_WRONGDATA;
            } 
            massActionRatio = (KINETIC_LAW**)(visitor->_internal1);
            if( ( *massActionRatio = CreateOpKineticLaw( KINETIC_LAW_OP_TIMES, *massActionRatio, CloneKineticLaw( kineticLaw ) ) ) == NULL ) {
                END_FUNCTION("_VisitOpToCreateMassActionRatio", FAILING );
                return FAILING;
            }
            
            END_FUNCTION("_VisitOpToCreateMassActionRatio", SUCCESS );
        return ret;

        default:
            END_FUNCTION("_VisitOpToCreateMassActionRatio", E_WRONGDATA );
        return E_WRONGDATA;
    }
    END_FUNCTION("_VisitOpToCreateMassActionRatio", E_WRONGDATA );
    return ret;
}

static RET_VAL _VisitIntToCreateMassActionRatio( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    START_FUNCTION("_VisitIntToCreateMassActionRatio");
    END_FUNCTION("_VisitIntToCreateMassActionRatio", SUCCESS );
    return SUCCESS;
}

static RET_VAL _VisitRealToCreateMassActionRatio( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    START_FUNCTION("_VisitRealToCreateMassActionRatio");
    END_FUNCTION("_VisitRealToCreateMassActionRatio", SUCCESS );
    return SUCCESS;
}

static RET_VAL _VisitSymbolToCreateMassActionRatio( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    START_FUNCTION("_VisitSymbolToCreateMassActionRatio");
    END_FUNCTION("_VisitSymbolToCreateMassActionRatio", SUCCESS );
    return SUCCESS;
}

static RET_VAL _VisitSpeciesToCreateMassActionRatio( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    SPECIES *species = NULL;
    SPECIES *target = NULL;
    KINETIC_LAW **massActionRatio = NULL;
    
    START_FUNCTION("_VisitSpeciesToCreateMassActionRatio");
    
    species = GetSpeciesFromKineticLaw( kineticLaw );
    target = (SPECIES*)(visitor->_internal2);
    if( species == target ) {
        END_FUNCTION("_VisitSpeciesToCreateMassActionRatio", SUCCESS );
        return SUCCESS;
    } 

    massActionRatio = (KINETIC_LAW**)(visitor->_internal1);
    if( ( *massActionRatio = CreateOpKineticLaw( KINETIC_LAW_OP_TIMES, *massActionRatio, CloneKineticLaw( kineticLaw ) ) ) == NULL ) {
        END_FUNCTION("_VisitSpeciesToCreateMassActionRatio", FAILING );
        return FAILING;
    }
            
    END_FUNCTION("_VisitSpeciesToCreateMassActionRatio", SUCCESS );
    return SUCCESS;
}



static KINETIC_LAW *_CreateRateConstantKineticLaw( KINETIC_LAW *kineticLaw ) {
    static KINETIC_LAW_VISITOR visitor;
    KINETIC_LAW *rateConstant = NULL;; 

    START_FUNCTION("_CreateRateConstantKineticLaw");
    
    if( visitor.VisitOp == NULL ) {
        visitor.VisitOp = _VisitOpToCreateRateConstantKineticLaw;
        visitor.VisitInt = _VisitIntToCreateRateConstantKineticLaw;
        visitor.VisitReal = _VisitRealToCreateRateConstantKineticLaw;
        visitor.VisitSpecies = _VisitSpeciesToCreateRateConstantKineticLaw;
        visitor.VisitSymbol = _VisitSymbolToCreateRateConstantKineticLaw;
    }
    visitor._internal1 = (CADDR_T)(&rateConstant);
    
    if( IS_FAILED( kineticLaw->Accept( kineticLaw, &visitor ) ) ) {
        END_FUNCTION("_CreateRateConstantKineticLaw", SUCCESS );
        return NULL;
    } 
    END_FUNCTION("_CreateRateConstantKineticLaw", SUCCESS );
    return rateConstant;
}

static RET_VAL _VisitOpToCreateRateConstantKineticLaw( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    RET_VAL ret = SUCCESS;
    BYTE opType = 0;
    KINETIC_LAW **result = NULL;
    KINETIC_LAW *left = NULL;
    KINETIC_LAW *leftValue = NULL;
    KINETIC_LAW *right = NULL;
    KINETIC_LAW *rightValue = NULL;
    
    START_FUNCTION("_VisitOpToCreateRateConstantKineticLaw");
        
    opType = GetOpTypeFromKineticLaw( kineticLaw );    
    switch( opType ) {
        case KINETIC_LAW_OP_TIMES:
            result = (KINETIC_LAW**)(visitor->_internal1);
            
            left = GetOpLeftFromKineticLaw( kineticLaw );
            visitor->_internal1 = (CADDR_T)(&leftValue);            
            if( IS_FAILED( ( ret = left->Accept( left, visitor ) ) ) ) {
                END_FUNCTION("_VisitOpToCreateRateConstantKineticLaw", ret );
                return ret;
            }
            
            right = GetOpRightFromKineticLaw( kineticLaw );
            visitor->_internal1 = (CADDR_T)(&rightValue);            
            if( IS_FAILED( ( ret = right->Accept( right, visitor ) ) ) ) {
                END_FUNCTION("_VisitOpToCreateRateConstantKineticLaw", ret );
                return ret;
            }
            
            if( ( leftValue == NULL ) && ( rightValue == NULL ) ) {
                *result = NULL;
            }
            else if( leftValue == NULL ) {
                *result = rightValue;
            }
            else if( rightValue == NULL ) {
                *result = leftValue;
            }
            else {
                if( ( *result = CreateOpKineticLaw( opType, leftValue, rightValue ) ) == NULL ) {
                    END_FUNCTION("_VisitOpToCreateRateConstantKineticLaw", FAILING );
                    return FAILING;
                }
            }                        
            END_FUNCTION("_VisitOpToCreateRateConstantKineticLaw", SUCCESS );
        return ret;

        case KINETIC_LAW_OP_DIVIDE:
            result = (KINETIC_LAW**)(visitor->_internal1);
            
            left = GetOpLeftFromKineticLaw( kineticLaw );
            visitor->_internal1 = (CADDR_T)(&leftValue);            
            if( IS_FAILED( ( ret = left->Accept( left, visitor ) ) ) ) {
                END_FUNCTION("_VisitOpToCreateRateConstantKineticLaw", ret );
                return ret;
            }
            
            right = GetOpRightFromKineticLaw( kineticLaw );
            visitor->_internal1 = (CADDR_T)(&rightValue);            
            if( IS_FAILED( ( ret = right->Accept( right, visitor ) ) ) ) {
                END_FUNCTION("_VisitOpToCreateRateConstantKineticLaw", ret );
                return ret;
            }
            
            if( ( leftValue == NULL ) && ( rightValue == NULL ) ) {
                *result = NULL;
            }
            else if( leftValue == NULL ) {
                if( ( *result = CreateOpKineticLaw( opType, CreateRealValueKineticLaw( 1.0 ), rightValue ) ) == NULL ) {
                    END_FUNCTION("_VisitOpToCreateRateConstantKineticLaw", FAILING );
                    return FAILING;
                }
            }
            else if( rightValue == NULL ) {
                *result = leftValue;
            }
            else {
                if( ( *result = CreateOpKineticLaw( opType, leftValue, rightValue ) ) == NULL ) {
                    END_FUNCTION("_VisitOpToCreateRateConstantKineticLaw", FAILING );
                    return FAILING;
                }
            }                        
            END_FUNCTION("_VisitOpToCreateRateConstantKineticLaw", SUCCESS );
        return ret;
        
        case KINETIC_LAW_OP_POW:
            END_FUNCTION("_VisitOpToCreateRateConstantKineticLaw", SUCCESS );
        return SUCCESS;

        default:
            END_FUNCTION("_VisitOpToCreateRateConstantKineticLaw", E_WRONGDATA );
        return E_WRONGDATA;
    }
    END_FUNCTION("_VisitOpToCreateRateConstantKineticLaw", FAILING );
    return FAILING;
}

static RET_VAL _VisitIntToCreateRateConstantKineticLaw( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    KINETIC_LAW **rateConstant = NULL;
            
    START_FUNCTION("_VisitIntToCreateRateConstantKineticLaw");
    
    rateConstant = (KINETIC_LAW**)(visitor->_internal1);
    if( ( *rateConstant = CloneKineticLaw( kineticLaw ) ) == NULL ) {
        END_FUNCTION("_VisitIntToCreateRateConstantKineticLaw", FAILING );
        return FAILING;
    }
    END_FUNCTION("_VisitIntToCreateRateConstantKineticLaw", SUCCESS );
    return SUCCESS;
}

static RET_VAL _VisitRealToCreateRateConstantKineticLaw( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    KINETIC_LAW **rateConstant = NULL;
            
    START_FUNCTION("_VisitRealToCreateRateConstantKineticLaw");
    
    rateConstant = (KINETIC_LAW**)(visitor->_internal1);
    if( ( *rateConstant = CloneKineticLaw( kineticLaw ) ) == NULL ) {
        END_FUNCTION("_VisitRealToCreateRateConstantKineticLaw", FAILING );
        return FAILING;
    }
    
    END_FUNCTION("_VisitRealToCreateRateConstantKineticLaw", SUCCESS );
    return SUCCESS;
}

static RET_VAL _VisitSpeciesToCreateRateConstantKineticLaw( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    KINETIC_LAW **rateConstant = NULL;
    
    START_FUNCTION("_VisitSpeciesToCreateRateConstantKineticLaw");
    
    rateConstant = (KINETIC_LAW**)(visitor->_internal1);
    *rateConstant = NULL;
    
    END_FUNCTION("_VisitSpeciesToCreateRateConstantKineticLaw", SUCCESS );
    return SUCCESS;
}

static RET_VAL _VisitSymbolToCreateRateConstantKineticLaw( KINETIC_LAW_VISITOR *visitor, KINETIC_LAW *kineticLaw ) {
    KINETIC_LAW **rateConstant = NULL;
    REB2SAC_SYMBOL *sym = NULL;
            
    START_FUNCTION("_VisitSymbolToCreateRateConstantKineticLaw");

    rateConstant = (KINETIC_LAW**)(visitor->_internal1);
    
    sym = GetSymbolFromKineticLaw( kineticLaw );
    if( !IsSymbolConstant( sym ) ) {
        *rateConstant = NULL;
        END_FUNCTION("_VisitSymbolToCreateRateConstantKineticLaw", SUCCESS );
        return SUCCESS;
    }
    if( !IsRealValueSymbol( sym ) ) {
        *rateConstant = NULL;
        END_FUNCTION("_VisitSymbolToCreateRateConstantKineticLaw", SUCCESS );
        return SUCCESS;
    }    
    if( ( *rateConstant = CloneKineticLaw( kineticLaw ) ) == NULL ) {
        END_FUNCTION("_VisitSymbolToCreateRateConstantKineticLaw", FAILING );
        return FAILING;
    }
    
    END_FUNCTION("_VisitSymbolToCreateRateConstantKineticLaw", SUCCESS );
    return SUCCESS;
}


static KINETIC_LAW *_CreateMassActionRatioWithRateConstantInKineticLaw( KINETIC_LAW *forward, SPECIES *op, KINETIC_LAW *rateRatio  ) {
    static KINETIC_LAW_VISITOR visitor;
    KINETIC_LAW *massActionRatio = NULL;
    
    START_FUNCTION("_CreateMassActionRatio");
    
    if( visitor.VisitOp == NULL ) {
        visitor.VisitOp = _VisitOpToCreateMassActionRatio;
        visitor.VisitInt = _VisitIntToCreateMassActionRatio;
        visitor.VisitReal = _VisitRealToCreateMassActionRatio;
        visitor.VisitSpecies = _VisitSpeciesToCreateMassActionRatio;
        visitor.VisitSymbol = _VisitSymbolToCreateMassActionRatio;
    }
    
    massActionRatio = rateRatio;
    visitor._internal1 = (CADDR_T)(&massActionRatio);
    visitor._internal2 = (CADDR_T)op;
    
    if( IS_FAILED( forward->Accept( forward, &visitor ) ) ) {
        END_FUNCTION("_CreateMassActionRatio", FAILING );
        return NULL;
    } 
    
    END_FUNCTION("_CreateMassActionRatio", SUCCESS );
    return massActionRatio;
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
            if( IS_FAILED( ( ret = AddElementInLinkedList( (CADDR_T)kineticLaw, list ) ) ) ) {
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
    if( IS_FAILED( ( ret = AddElementInLinkedList( (CADDR_T)kineticLaw, list ) ) ) ) {
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
    if( IS_FAILED( ( ret = AddElementInLinkedList( (CADDR_T)kineticLaw, list ) ) ) ) {
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
    if( IS_FAILED( ( ret = AddElementInLinkedList( (CADDR_T)kineticLaw, list ) ) ) ) {
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
    if( IS_FAILED( ( ret = AddElementInLinkedList( (CADDR_T)kineticLaw, list ) ) ) ) {
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
            if( IS_FAILED( ( ret = AddElementInLinkedList( (CADDR_T)kineticLaw, list ) ) ) ) {
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
    if( IS_FAILED( ( ret = AddElementInLinkedList( (CADDR_T)kineticLaw, list ) ) ) ) {
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
    if( IS_FAILED( ( ret = AddElementInLinkedList( (CADDR_T)kineticLaw, list ) ) ) ) {
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
    if( IS_FAILED( ( ret = AddElementInLinkedList( (CADDR_T)kineticLaw, list ) ) ) ) {
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
    if( IS_FAILED( ( ret = AddElementInLinkedList( (CADDR_T)kineticLaw, list ) ) ) ) {
        END_FUNCTION("_VisitSymbolToCreateListOfSumTerms", ret );
        return ret;
    }
    
    END_FUNCTION("_VisitSymbolToCreateListOfSumTerms", SUCCESS );
    return ret;
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
    
    if( opType == KINETIC_LAW_OP_TIMES ) {
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



