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
#include "common.h"
#include "linked_list.h"
#include "abstraction_method_manager.h"
#include "IR.h"
#include "species_node.h"
#include "reaction_node.h"
#include "kinetic_law.h"

static char * _GetModifierConstantPropagationMethodID( ABSTRACTION_METHOD *method );
static RET_VAL _ApplyModifierConstantPropagationMethod( ABSTRACTION_METHOD *method, IR *ir );      
static double _GetModifierConcentration( SPECIES *modifier );
static KINETIC_LAW *_CreateSymbolKineticLaw( IR *ir, SPECIES *modifier, double value );
static RET_VAL _DoConstantPropagation( ABSTRACTION_METHOD *method, KINETIC_LAW *kineticLaw, SPECIES *modifier, KINETIC_LAW *symKineticLaw );



ABSTRACTION_METHOD *ModifierConstantPropagationAbstractionMethodConstructor(  ABSTRACTION_METHOD_MANAGER *manager ) {
    static ABSTRACTION_METHOD method;
    
    START_FUNCTION("ModifierConstantPropagationAbstractionMethodConstructor");

    if( method.manager == NULL ) {
        method.manager = manager;
        method.GetID = _GetModifierConstantPropagationMethodID;
        method.Apply = _ApplyModifierConstantPropagationMethod;       
    }
    
    TRACE_0( "ModifierConstantPropagationAbstractionMethodConstructor invoked" );
    
    END_FUNCTION("ModifierConstantPropagationAbstractionMethodConstructor", SUCCESS );
    return &method;
}



static char * _GetModifierConstantPropagationMethodID( ABSTRACTION_METHOD *method ) {
    START_FUNCTION("_GetModifierConstantPropagationMethodID");
    
    END_FUNCTION("_GetModifierConstantPropagationMethodID", SUCCESS );
    return "modifier-constant-propagation";
}



static RET_VAL _ApplyModifierConstantPropagationMethod( ABSTRACTION_METHOD *method, IR *ir ) {
    RET_VAL ret = SUCCESS;
    double value = 0.0;
    SPECIES *species = NULL;
    SPECIES *species2 = NULL;
    REACTION *reaction = NULL;
    IR_EDGE *edge = NULL;    
    KINETIC_LAW *kineticLaw = NULL;
    KINETIC_LAW *symKineticLaw = NULL;
    LINKED_LIST *speciesList = NULL;
    LINKED_LIST *edges = NULL;
    LINKED_LIST *list = NULL;
    LINKED_LIST *ruleList = NULL;
    LINKED_LIST *eventList = NULL;
    LINKED_LIST *list2 = NULL;
    EVENT_MANAGER *eventManager;
    EVENT *event;
    EVENT_ASSIGNMENT *eventAssignment;
    RULE_MANAGER *ruleManager;
    CONSTRAINT_MANAGER *constraintManager;
    CONSTRAINT *constraint;
    RULE *rule = NULL;
    BOOL foundIt;
#ifdef DEBUG
    STRING *kineticLawString = NULL;
#endif        
    START_FUNCTION("_ApplyModifierConstantPropagationMethod");
    
    speciesList = ir->GetListOfSpeciesNodes( ir );
    
    ResetCurrentElement( speciesList );    
    while( ( species = (SPECIES*)GetNextFromLinkedList( speciesList ) ) != NULL ) {
        if( IsKeepFlagSetInSpeciesNode( species ) ) {
            continue;
        }
        
        edges = GetReactantEdges( (IR_NODE*)species );
        if( GetLinkedListSize( edges ) > 0 ) {
            continue;
        }
        edges = GetProductEdges( (IR_NODE*)species );
        if( GetLinkedListSize( edges ) > 0 ) {
            continue;
        }

	/* Check if species used in an event */
	foundIt = FALSE;
	if( ( eventManager = ir->GetEventManager( ir ) ) == NULL ) {
	  return ErrorReport( FAILING, "_InitializeRecord", "could not get the event manager" );
	}
	eventList = eventManager->CreateListOfEvents( eventManager );
	ResetCurrentElement( eventList );
	while( ( event = (EVENT*)GetNextFromLinkedList( eventList ) ) != NULL ) {
	  list2 = GetEventAssignments( event );
	  ResetCurrentElement( list2 );
	  while( ( eventAssignment = (EVENT_ASSIGNMENT*)GetNextFromLinkedList( list2 ) ) != NULL ) {
	    if ( strcmp( GetCharArrayOfString(eventAssignment->var), 
			 GetCharArrayOfString(GetSpeciesNodeID( species ) ) ) == 0 ) {
	      foundIt = TRUE;
	      break;
	    }
	  }
	  if (foundIt) break;
	}
	if (foundIt) continue;

	/* Check if species used in a rule */
	if( ( ruleManager = ir->GetRuleManager( ir ) ) == NULL ) {
	  return ErrorReport( FAILING, "_InitializeRecord", "could not get the rule manager" );
	}
	ruleList = ruleManager->CreateListOfRules( ruleManager );
	ResetCurrentElement( ruleList );
	while( ( rule = (RULE*)GetNextFromLinkedList( ruleList ) ) != NULL ) {
	  if ( ( GetRuleType( rule ) == RULE_TYPE_ASSIGNMENT ) ||
	       ( GetRuleType( rule ) == RULE_TYPE_RATE ) ) {
	    if ( strcmp( GetCharArrayOfString(GetRuleVar( rule )),
			 GetCharArrayOfString(GetSpeciesNodeID( species ) ) ) == 0 ) {
	      foundIt = TRUE;
	      break;
	    }
	  }
	}
	if (foundIt) continue;

        /*
            now conditions are satisfied, so propagate constant
        */
        value = _GetModifierConcentration( species );
        if( ( symKineticLaw = _CreateSymbolKineticLaw( ir, species, value ) ) == NULL ) {
            END_FUNCTION("_ApplyModifierConstantPropagationMethod", ret );
            return ret;
        } 
        TRACE_1( "species %s satisfies constant propagation condition", GetCharArrayOfString( GetSpeciesNodeName( species ) ) );
        edges = GetModifierEdges( (IR_NODE*)species );
        ResetCurrentElement( edges );
        while( ( edge = GetNextEdge( edges ) ) != NULL ) {
            reaction = GetReactionInIREdge( edge );
            TRACE_1( "changing kinetic law of reaction %s", GetCharArrayOfString( GetReactionNodeName( reaction ) ) );
            kineticLaw = GetKineticLawInReactionNode( reaction );
#ifdef DEBUG
            kineticLawString = ToStringKineticLaw( kineticLaw );
            printf( "kinetic law before constant propagation: %s%s", GetCharArrayOfString( kineticLawString ), NEW_LINE );
            FreeString( &kineticLawString );
#endif             
            if( IS_FAILED( ( ret = _DoConstantPropagation( method, kineticLaw, species, symKineticLaw ) ) ) ) {
                END_FUNCTION("_ApplyModifierConstantPropagationMethod", ret );
                return ret;
            }
#ifdef DEBUG
            kineticLawString = ToStringKineticLaw( kineticLaw );
            printf( "kinetic law after constant propagation: %s%s", GetCharArrayOfString( kineticLawString ), NEW_LINE );
            FreeString( &kineticLawString );
#endif             
        }

	ResetCurrentElement( ruleList );
	while( ( rule = (RULE*)GetNextFromLinkedList( ruleList ) ) != NULL ) {
	  kineticLaw = GetMathInRule( rule );
	  if( IS_FAILED( ( ret = _DoConstantPropagation( method, kineticLaw, species, symKineticLaw ) ) ) ) {
	    END_FUNCTION("_ApplyModifierConstantPropagationMethod", ret );
	    return ret;
	  }
	}
	ResetCurrentElement( eventList );
	while( ( event = (EVENT*)GetNextFromLinkedList( eventList ) ) != NULL ) {
	  kineticLaw = GetTriggerInEvent( event );
	  if( IS_FAILED( ( ret = _DoConstantPropagation( method, kineticLaw, species, symKineticLaw ) ) ) ) {
	    END_FUNCTION("_ApplyModifierConstantPropagationMethod", ret );
	    return ret;
	  }
	  kineticLaw = GetDelayInEvent( event );
	  if( IS_FAILED( ( ret = _DoConstantPropagation( method, kineticLaw, species, symKineticLaw ) ) ) ) {
	    END_FUNCTION("_ApplyModifierConstantPropagationMethod", ret );
	    return ret;
	  }
	  list2 = GetEventAssignments( event );
	  ResetCurrentElement( list2 );
	  while( ( eventAssignment = (EVENT_ASSIGNMENT*)GetNextFromLinkedList( list2 ) ) != NULL ) {
	    kineticLaw = eventAssignment->assignment;
	    if( IS_FAILED( ( ret = _DoConstantPropagation( method, kineticLaw, species, symKineticLaw ) ) ) ) {
	      END_FUNCTION("_ApplyModifierConstantPropagationMethod", ret );
	      return ret;
	    }
	  }
	}

	if( ( constraintManager = ir->GetConstraintManager( ir ) ) == NULL ) {
	  return ErrorReport( FAILING, "_ApplyModifierConstantPropagationMethod", "could not get the constraint manager" );
	}
	list = constraintManager->CreateListOfConstraints( constraintManager );
	ResetCurrentElement( list );
	while( ( constraint = (CONSTRAINT*)GetNextFromLinkedList( list ) ) != NULL ) {
	  kineticLaw = GetMathInConstraint( constraint );
	  if( IS_FAILED( ( ret = _DoConstantPropagation( method, kineticLaw, species, symKineticLaw ) ) ) ) {
	    END_FUNCTION("_ApplyModifierConstantPropagationMethod", ret );
	    return ret;
	  }
	}

	list = ir->GetListOfSpeciesNodes( ir );
        ResetCurrentElement( list );    
	while( ( species2 = (SPECIES*)GetNextFromLinkedList( speciesList ) ) != NULL ) {
	  kineticLaw = (KINETIC_LAW*)GetInitialAssignmentInSpeciesNode( species2 );
	  if( IS_FAILED( ( ret = _DoConstantPropagation( method, kineticLaw, species, symKineticLaw ) ) ) ) {
	    END_FUNCTION("_ApplyModifierConstantPropagationMethod", ret );
	    return ret;
	  }
	}

        FreeKineticLaw( &symKineticLaw );
        if( IS_FAILED( ( ret = ir->RemoveSpecies( ir, species ) ) ) ) {
            END_FUNCTION("_ApplyModifierConstantPropagationMethod", ret );
            return ret;
        }
    }
         
    END_FUNCTION("_ApplyModifierConstantPropagationMethod", SUCCESS );
    return ret;
}      



static double _GetModifierConcentration( SPECIES *modifier ) {
    double concentration = 0.0;
    KINETIC_LAW *law;

    START_FUNCTION("_GetModifierConcentration");
    
    if (GetInitialAssignmentInSpeciesNode( modifier ) != NULL) {
      law = (KINETIC_LAW*)GetInitialAssignmentInSpeciesNode( modifier );
      SimplifyInitialAssignment(law);
      if (law->valueType == KINETIC_LAW_VALUE_TYPE_REAL) {
	concentration = GetRealValueFromKineticLaw(law);
      } else if (law->valueType == KINETIC_LAW_VALUE_TYPE_INT) {
	concentration = (double)GetIntValueFromKineticLaw(law);
      }
    } else {
      if( HasOnlySubstanceUnitsInSpeciesNode( modifier ) ) {
	concentration = GetInitialAmountInSpeciesNode( modifier );
      } else {
	concentration = GetInitialConcentrationInSpeciesNode( modifier );
      }
    }
    //concentration = GetInitialConcentrationInSpeciesNode( modifier );
    END_FUNCTION("_GetModifierConcentration", SUCCESS );
    return concentration;
}

static KINETIC_LAW *_CreateSymbolKineticLaw( IR *ir, SPECIES *modifier, double value ) {
    char *proposedID =NULL;
    REB2SAC_SYMBOL *sym = NULL;
    REB2SAC_SYMTAB *symtab = NULL;
    KINETIC_LAW *kineticLaw = NULL;
    
    START_FUNCTION("_CreateSymbolKineticLaw");
    
    proposedID = GetCharArrayOfString( GetSpeciesNodeName( modifier ) );
    symtab = ir->GetGlobalSymtab( ir );
    if( ( sym = symtab->AddRealValueSymbol( symtab, proposedID, value, TRUE ) ) == NULL ) {
        END_FUNCTION("_CreateSymbolKineticLaw", FAILING );    
        return NULL;
    }
    
    if( ( kineticLaw = CreateSymbolKineticLaw( sym ) ) == NULL ) {
        END_FUNCTION("_CreateSymbolKineticLaw", FAILING );    
        return NULL;
    }
    
    END_FUNCTION("_CreateSymbolKineticLaw", SUCCESS );    
    return kineticLaw;
}



static RET_VAL _DoConstantPropagation( ABSTRACTION_METHOD *method, KINETIC_LAW *kineticLaw, SPECIES *modifier, KINETIC_LAW *symKineticLaw ) {
    RET_VAL ret = SUCCESS;
    KINETIC_LAW_VISITOR *visitor = NULL;
    
    START_FUNCTION("_DoConstantPropagation");
    
    if (kineticLaw) {
      if( IS_FAILED( ( ret = ReplaceSpeciesWithKineticLawInKineticLaw( kineticLaw, modifier, symKineticLaw ) ) ) ) {
        END_FUNCTION("_DoConstantPropagation", ret );
        return ret;
      }
    }
    END_FUNCTION("_DoConstantPropagation", SUCCESS );
    return ret;
}


