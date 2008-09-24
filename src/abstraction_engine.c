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
#include "abstraction_engine.h"

static ABSTRACTION_METHOD** _GetRegisteredMethods( ABSTRACTION_ENGINE *abstractionEngine );
static RET_VAL _PutKeepFlagInSpeciesNodes( ABSTRACTION_ENGINE *abstractionEngine, IR *ir );
static RET_VAL _PutPrintFlagInNodes( ABSTRACTION_ENGINE *abstractionEngine, IR *ir );
static RET_VAL _Abstract( ABSTRACTION_ENGINE *abstractionEngine, IR *ir );
static RET_VAL _Abstract_DEBUG( ABSTRACTION_ENGINE *abstractionEngine, IR *ir );

static RET_VAL _Abstract1( ABSTRACTION_ENGINE *abstractionEngine, IR *ir );
static RET_VAL _Abstract2( ABSTRACTION_ENGINE *abstractionEngine, IR *ir );
static RET_VAL _Abstract3( ABSTRACTION_ENGINE *abstractionEngine, IR *ir );


static RET_VAL _Close( ABSTRACTION_ENGINE *abstractionEngine );

static LINKED_LIST *_CreateListOfMethods( ABSTRACTION_ENGINE *abstractionEngine, int i );

RET_VAL InitAbstractionEngine( COMPILER_RECORD_T *record, ABSTRACTION_ENGINE *abstractionEngine ) {
    RET_VAL ret = SUCCESS;
    ABSTRACTION_METHOD_MANAGER *manager = NULL;
    
    START_FUNCTION("InitAbstractionEngine");
     
    if( abstractionEngine->record == NULL ) {
        abstractionEngine->record = record;
        
        abstractionEngine->GetRegisteredMethods = _GetRegisteredMethods;
#if 1        
        abstractionEngine->Abstract = _Abstract;
        abstractionEngine->Abstract1 = _Abstract1;
        abstractionEngine->Abstract2 = _Abstract2;
        abstractionEngine->Abstract3 = _Abstract3;
#else
        abstractionEngine->Abstract = _Abstract_DEBUG;
#endif        
        abstractionEngine->Close = _Close;
    
        if( ( abstractionEngine->manager = GetAbstractionMethodManagerInstance() ) == NULL ) {
            return ErrorReport( FAILING, "InitAbstractionEngine", "could not create abstraction method manager instance" );
        }
    
        manager = abstractionEngine->manager;
        if( IS_FAILED( ( ret = manager->Init( manager, abstractionEngine->record ) ) ) ) {
            END_FUNCTION("InitAbstractionEngine", ret );    
            return ret;
        }         
    }   
    
    END_FUNCTION("InitAbstractionEngine", SUCCESS );
    
    return ret;
}



static ABSTRACTION_METHOD** _GetRegisteredMethods( ABSTRACTION_ENGINE *abstractionEngine ) {
    ABSTRACTION_METHOD_MANAGER *manager = NULL;
    ABSTRACTION_METHOD **methods = NULL;
    
    START_FUNCTION("_GetRegisteredMethods");

    manager = abstractionEngine->manager;
    methods = manager->GetMethods( manager );
    
    END_FUNCTION("_GetRegisteredMethods", SUCCESS );
    
    return methods;
}

static RET_VAL _PutKeepFlagInSpeciesNodes( ABSTRACTION_ENGINE *abstractionEngine, IR *ir ) {
    RET_VAL ret = SUCCESS;
    int num = 0;
    char buf[256];
    char *speciesName = NULL;
    char *interestingSpeciesName = NULL;
    SPECIES *species;
    LINKED_LIST *list = NULL;
    ABSTRACTION_METHOD_MANAGER *manager = NULL;
    REB2SAC_PROPERTIES *properties = NULL;
    
    START_FUNCTION("_PutKeepFlagInSpeciesNodes");
    
    properties = abstractionEngine->record->properties;
    manager = abstractionEngine->manager;
    
    list = ir->GetListOfSpeciesNodes( ir );
    
    for( num = 1; ; num++ ) {
        sprintf( buf, "%s%i", REB2SAC_INTERESTING_SPECIES_KEY_PREFIX, num );
        if( ( interestingSpeciesName = properties->GetProperty( properties, buf ) ) == NULL ) {
            break;
        }
        ResetCurrentElement( list );
        while( ( species = (SPECIES*)GetNextFromLinkedList( list ) ) != NULL ) {
            speciesName = GetCharArrayOfString( GetSpeciesNodeName( species ) );
            if( strcmp( speciesName, interestingSpeciesName ) == 0 ) {
                TRACE_1( "species %s is property of interest", speciesName );
                if( IS_FAILED( ( ret = SetKeepFlagInSpeciesNode( species, TRUE ) ) ) ) {
                    END_FUNCTION("_PutKeepFlagInSpeciesNodes", ret );
                    return ret;
                }
                /* names may not be unique */
#if 0            
                break; 
#endif            
            }
        }
    }    
        
    END_FUNCTION("_PutKeepFlagInSpeciesNodes", SUCCESS );
    return ret;
}

static RET_VAL _PutPrintFlagInNodes( ABSTRACTION_ENGINE *abstractionEngine, IR *ir ) {
    RET_VAL ret = SUCCESS;
    char *speciesName = NULL;
    SPECIES *species;
    LINKED_LIST *list = NULL;
    ABSTRACTION_METHOD_MANAGER *manager = NULL;
    REB2SAC_PROPERTIES *properties = NULL;
    char *valueString = NULL;
    char *curPtr = NULL;
    char *nextPtr = NULL;
    COMPARTMENT *compartment = NULL;
    COMPARTMENT_MANAGER *compartmentManager;
    REB2SAC_SYMBOL *symbol = NULL;
    REB2SAC_SYMTAB *symTab;
    
    START_FUNCTION("_PutPrintFlagInSpeciesNodes");
    
    properties = abstractionEngine->record->properties;
    manager = abstractionEngine->manager;

    if( ( compartmentManager = ir->GetCompartmentManager( ir ) ) == NULL ) {
        return ErrorReport( FAILING, "_InitializeRecord", "could not get the compartment manager" );
    }
    if( ( symTab = ir->GetGlobalSymtab( ir ) ) == NULL ) {
        return ErrorReport( FAILING, "_InitializeRecord", "could not get the symbol table" );
    }

    if( ( valueString = properties->GetProperty( properties, REB2SAC_PRINT_VARIABLES ) ) != NULL ) {
      list = compartmentManager->CreateListOfCompartments( compartmentManager );
      ResetCurrentElement( list );
      while( ( compartment = (COMPARTMENT*)GetNextFromLinkedList( list ) ) != NULL ) {
	if( IS_FAILED( ( ret = SetPrintCompartment( compartment, FALSE ) ) ) ) {
	  END_FUNCTION("_PutPrintFlagInSpeciesNodes", ret );
	  return ret;
	}
      }
      list = ir->GetListOfSpeciesNodes( ir );
      ResetCurrentElement( list );
      while( ( species = (SPECIES*)GetNextFromLinkedList( list ) ) != NULL ) {
	if( IS_FAILED( ( ret = SetPrintFlagInSpeciesNode( species, FALSE ) ) ) ) {
	  END_FUNCTION("_PutPrintFlagInSpeciesNodes", ret );
	  return ret;
	}
      }
      list = symTab->GenerateListOfSymbols( symTab );
      ResetCurrentElement( list );
      while( ( symbol = (REB2SAC_SYMBOL*)GetNextFromLinkedList( list ) ) != NULL ) {
	if( IS_FAILED( ( ret = SetPrintSymbol( symbol, FALSE ) ) ) ) {
	  END_FUNCTION("_PutPrintFlagInSpeciesNodes", ret );
	  return ret;
	}
      }

      curPtr = valueString;
      nextPtr = strchr(curPtr,',');
      while (nextPtr) {
	(*nextPtr) = '\0';
	list = compartmentManager->CreateListOfCompartments( compartmentManager );
	ResetCurrentElement( list );
	while( ( compartment = (COMPARTMENT*)GetNextFromLinkedList( list ) ) != NULL ) {
	  if( strcmp( GetCharArrayOfString( GetCompartmentID( compartment ) ), curPtr ) == 0 ) {
	    if( IS_FAILED( ( ret = SetPrintCompartment( compartment, TRUE ) ) ) ) {
	      END_FUNCTION("_PutPrintFlagInSpeciesNodes", ret );
	      return ret;
	    }
	  }
	}
	list = ir->GetListOfSpeciesNodes( ir );
	ResetCurrentElement( list );
	while( ( species = (SPECIES*)GetNextFromLinkedList( list ) ) != NULL ) {
	  speciesName = GetCharArrayOfString( GetSpeciesNodeName( species ) );
	  if( strcmp( speciesName, curPtr ) == 0 ) {
	    if( IS_FAILED( ( ret = SetPrintFlagInSpeciesNode( species, TRUE ) ) ) ) {
	      END_FUNCTION("_PutPrintFlagInSpeciesNodes", ret );
	      return ret;
	    }
	    if( IS_FAILED( ( ret = SetKeepFlagInSpeciesNode( species, TRUE ) ) ) ) {
	      END_FUNCTION("_PutPrintFlagInSpeciesNodes", ret );
	      return ret;
	    }
	  }
	}
	list = symTab->GenerateListOfSymbols( symTab );
	ResetCurrentElement( list );
	while( ( symbol = (REB2SAC_SYMBOL*)GetNextFromLinkedList( list ) ) != NULL ) {
	  if( strcmp( GetCharArrayOfString( GetSymbolID( symbol ) ), curPtr ) == 0 ) {
	    if( IS_FAILED( ( ret = SetPrintSymbol( symbol, TRUE ) ) ) ) {
	      END_FUNCTION("_PutPrintFlagInSpeciesNodes", ret );
	      return ret;
	    }
	  }
	}

	curPtr=nextPtr+2;
	nextPtr = strchr(curPtr,',');
      }

      list = compartmentManager->CreateListOfCompartments( compartmentManager );
      ResetCurrentElement( list );
      while( ( compartment = (COMPARTMENT*)GetNextFromLinkedList( list ) ) != NULL ) {
	if( strcmp( GetCharArrayOfString( GetCompartmentID( compartment ) ), curPtr ) == 0 ) {
	  if( IS_FAILED( ( ret = SetPrintCompartment( compartment, TRUE ) ) ) ) {
	    END_FUNCTION("_PutPrintFlagInSpeciesNodes", ret );
	    return ret;
	  }
	}
      }
      list = ir->GetListOfSpeciesNodes( ir );
      ResetCurrentElement( list );
      while( ( species = (SPECIES*)GetNextFromLinkedList( list ) ) != NULL ) {
	speciesName = GetCharArrayOfString( GetSpeciesNodeName( species ) );
	if( strcmp( speciesName, curPtr ) == 0 ) {
	  if( IS_FAILED( ( ret = SetPrintFlagInSpeciesNode( species, TRUE ) ) ) ) {
	    END_FUNCTION("_PutPrintFlagInSpeciesNodes", ret );
	    return ret;
	  }
	  if( IS_FAILED( ( ret = SetKeepFlagInSpeciesNode( species, TRUE ) ) ) ) {
	    END_FUNCTION("_PutPrintFlagInSpeciesNodes", ret );
	    return ret;
	  }
	}
      }
      list = symTab->GenerateListOfSymbols( symTab );
      ResetCurrentElement( list );
      while( ( symbol = (REB2SAC_SYMBOL*)GetNextFromLinkedList( list ) ) != NULL ) {
	if( strcmp( GetCharArrayOfString( GetSymbolID( symbol ) ), curPtr ) == 0 ) {
	  if( IS_FAILED( ( ret = SetPrintSymbol( symbol, TRUE ) ) ) ) {
	    END_FUNCTION("_PutPrintFlagInSpeciesNodes", ret );
	    return ret;
	  }
	}
      }

    } else {
      list = compartmentManager->CreateListOfCompartments( compartmentManager );
      ResetCurrentElement( list );
      while( ( compartment = (COMPARTMENT*)GetNextFromLinkedList( list ) ) != NULL ) {
	if (IsCompartmentConstant( compartment )) {
	  if( IS_FAILED( ( ret = SetPrintCompartment( compartment, FALSE ) ) ) ) {
	    END_FUNCTION("_PutPrintFlagInSpeciesNodes", ret );
	    return ret;
	  }
	} else {
	  if( IS_FAILED( ( ret = SetPrintCompartment( compartment, TRUE ) ) ) ) {
	    END_FUNCTION("_PutPrintFlagInSpeciesNodes", ret );
	    return ret;
	  }
	}
      }
      list = ir->GetListOfSpeciesNodes( ir );
      ResetCurrentElement( list );
      while( ( species = (SPECIES*)GetNextFromLinkedList( list ) ) != NULL ) {
	if( IS_FAILED( ( ret = SetPrintFlagInSpeciesNode( species, TRUE ) ) ) ) {
	  END_FUNCTION("_PutPrintFlagInSpeciesNodes", ret );
	  return ret;
	}
      }
      list = symTab->GenerateListOfSymbols( symTab );
      ResetCurrentElement( list );
      while( ( symbol = (REB2SAC_SYMBOL*)GetNextFromLinkedList( list ) ) != NULL ) {
	if (!IsSymbolConstant( symbol ) && 
	    !strcmp(GetCharArrayOfString(GetSymbolID( symbol )),"t")==0 &&
	    !strcmp(GetCharArrayOfString(GetSymbolID( symbol )),"time")==0) {
	  if( IS_FAILED( ( ret = SetPrintSymbol( symbol, TRUE ) ) ) ) {
	    END_FUNCTION("_PutPrintFlagInSpeciesNodes", ret );
	    return ret;
	  }
	} else {
	  if( IS_FAILED( ( ret = SetPrintSymbol( symbol, FALSE ) ) ) ) {
	    END_FUNCTION("_PutPrintFlagInSpeciesNodes", ret );
	    return ret;
	  }
	}
      }
    }
    END_FUNCTION("_PutPrintFlagInSpeciesNodes", SUCCESS );
    return ret;
}



static RET_VAL _Abstract( ABSTRACTION_ENGINE *abstractionEngine, IR *ir ) {
    RET_VAL ret = SUCCESS;
    BOOL changed = FALSE;
    int i = 0;
    int j = 0;
    LINKED_LIST *methods1 = NULL;
    LINKED_LIST *methods2 = NULL;
    LINKED_LIST *methods3 = NULL;
    ABSTRACTION_METHOD *method = NULL;
#if defined(REPORT_ABS)
    char *reporterType = NULL;
    ABSTRACTION_REPORTER *reporter = NULL;
#endif
#if defined(GEN_DOT) || defined(GEN_SBML) || defined(GEN_XHTML)   
    char filename[256];
    FILE *file = NULL;
#endif    
        
    START_FUNCTION("_Abstract");
    
#ifdef GEN_DOT
    if( ( file = fopen( "ir.dot", "w" ) ) == NULL ) {
        return ErrorReport( FAILING, "_Abstract", "dot file open error" ); 
    }
    if( IS_FAILED( ( ret = ir->GenerateDotFile( ir, file ) ) ) ) {
        END_FUNCTION("_Abstract", ret );
        return ret;
    }
    fclose( file );
#endif
    
#ifdef GEN_XHTML
    if( ( file = fopen( "ir.xhtml", "w" ) ) == NULL ) {
        return ErrorReport( FAILING, "_Abstract", "dot file open error" ); 
    }
    if( IS_FAILED( ( ret = ir->GenerateXHTML( ir, file ) ) ) ) {
        END_FUNCTION("_Abstract", ret );
        return ret;
    }
    fclose( file );
#endif


    if( IS_FAILED( ( ret = _PutKeepFlagInSpeciesNodes( abstractionEngine, ir ) ) ) ) {
        END_FUNCTION("_Abstract", ret );
        return ret;
    }

    if( IS_FAILED( ( ret = _PutPrintFlagInNodes( abstractionEngine, ir ) ) ) ) {
        END_FUNCTION("_Abstract", ret );
        return ret;
    }
    
    if( ( methods1 = _CreateListOfMethods( abstractionEngine, 1 ) ) == NULL ) {
        return ErrorReport( FAILING, "_Abstract", "could not create a method list1" );
    }
        
    if( ( methods2 = _CreateListOfMethods( abstractionEngine, 2 ) ) == NULL ) {
        return ErrorReport( FAILING, "_Abstract", "could not create a method list2" );
    }
    
    if( ( methods3 = _CreateListOfMethods( abstractionEngine, 3 ) ) == NULL ) {
        return ErrorReport( FAILING, "_Abstract", "could not create a method list3" );
    }

#if defined(REPORT_ABS)
    if( ( reporter = CreateAbstractionReporter( reporterType ) ) == NULL ) {
        return ErrorReport( FAILING, "_Abstract", "abstraction reporter creation error" ); 
    }
    if( IS_FAILED( ( ret = reporter->ReportInitial( reporter, ir ) ) ) ) {
        END_FUNCTION("_Abstract", ret );
        return ret;
    }
#endif
    
    
    TRACE_0( "pre-processing abstraction method" );
    i = 1;
    ResetCurrentElement( methods1 );
    while( ( method = (ABSTRACTION_METHOD*)GetNextFromLinkedList( methods1 ) ) != NULL ) {
        TRACE_1("applying method %s", method->GetID( method ) );                
        ir->ResetChangeFlag( ir );
        method->Apply( method, ir );

        if( ir->IsStructureChanged( ir ) ) {
#ifdef GEN_DOT
            memset( filename, 0, sizeof(filename) );
            sprintf( filename, "./ir-%s-1-%i.dot", method->GetID( method ), i );
            if( ( file = fopen( filename, "w" ) ) == NULL ) {
                return ErrorReport( FAILING, "_Abstract", "dot file open error" ); 
            }
            if( IS_FAILED( ( ret = ir->GenerateDotFile( ir, file ) ) ) ) {
                END_FUNCTION("_Abstract", ret );
                return ret;
            }
            fclose( file );
#endif        
#ifdef GEN_SBML
            memset( filename, 0, sizeof(filename) );
            sprintf( filename, "./ir-%s-1-%i.xml", method->GetID( method ), i );
            if( ( file = fopen( filename, "w" ) ) == NULL ) {
                return ErrorReport( FAILING, "_Abstract", "sbml file open error" ); 
            }
            if( IS_FAILED( ( ret = ir->GenerateSBML( ir, file ) ) ) ) {
                END_FUNCTION("_Abstract", ret );
                return ret;
            }
            fclose( file );
#endif        
#ifdef GEN_XHTML
            memset( filename, 0, sizeof(filename) );
            sprintf( filename, "./ir-%s-1-%i.xhtml", method->GetID( method ), i );
            if( ( file = fopen( filename, "w" ) ) == NULL ) {
                return ErrorReport( FAILING, "_Abstract", "xhtml file open error" ); 
            }
            if( IS_FAILED( ( ret = ir->GenerateXHTML( ir, file ) ) ) ) {
                END_FUNCTION("_Abstract", ret );
                return ret;
            }
            fclose( file );
#endif        
        }
#if defined(REPORT_ABS)
        if( IS_FAILED( ( ret = reporter->Report( reporter, method->GetID( method ), ir ) ) ) ) {
            END_FUNCTION("_Abstract", ret );
            return ret;
        }
#endif                
        i++;
    }
    
    TRACE_0( "main abstraction method" );
    i = 0;
    j = 1;
    do {
        TRACE_1( "main loop iteration %i", i );
        changed = FALSE;
        ResetCurrentElement( methods2 );
        while( ( method = (ABSTRACTION_METHOD*)GetNextFromLinkedList( methods2 ) ) != NULL ) {
            TRACE_1("applying method %s", method->GetID( method ) );                
            ir->ResetChangeFlag( ir );
            method->Apply( method, ir );
            if( !changed ) {
                changed = ir->IsStructureChanged( ir );
            }
            if( ir->IsStructureChanged( ir ) ) {
#ifdef GEN_DOT
                memset( filename, 0, sizeof(filename) );
                sprintf( filename, "./ir-%s-2-%i-%i.dot", method->GetID( method ), j, i );
                if( ( file = fopen( filename, "w" ) ) == NULL ) {
                    return ErrorReport( FAILING, "_Abstract", "dot file open error" ); 
                }
                if( IS_FAILED( ( ret = ir->GenerateDotFile( ir, file ) ) ) ) {
                    END_FUNCTION("_Abstract", ret );
                    return ret;
                }
                fclose( file );
#endif        
#ifdef GEN_SBML
                memset( filename, 0, sizeof(filename) );
                sprintf( filename, "./ir-%s-2-%i-%i.xml", method->GetID( method ), j, i );
                if( ( file = fopen( filename, "w" ) ) == NULL ) {
                    return ErrorReport( FAILING, "_Abstract", "sbml file open error" ); 
                }
                if( IS_FAILED( ( ret = ir->GenerateSBML( ir, file ) ) ) ) {
                    END_FUNCTION("_Abstract", ret );
                    return ret;
                }
                fclose( file );
#endif        
#ifdef GEN_XHTML
                memset( filename, 0, sizeof(filename) );
                sprintf( filename, "./ir-%s-2-%i-%i.xhtml", method->GetID( method ), j, i );
                if( ( file = fopen( filename, "w" ) ) == NULL ) {
                    return ErrorReport( FAILING, "_Abstract", "xhtml file open error" ); 
                }
                if( IS_FAILED( ( ret = ir->GenerateXHTML( ir, file ) ) ) ) {
                    END_FUNCTION("_Abstract", ret );
                    return ret;
                }
                fclose( file );
#endif        
            }
            j++;
#if defined(REPORT_ABS)
            if( IS_FAILED( ( ret = reporter->Report( reporter, method->GetID( method ), ir ) ) ) ) {
                END_FUNCTION("_Abstract", ret );
                return ret;
            }
#endif                            
        }
        i++;
    } while( changed );

#if defined(REPORT_ABS)
        if( IS_FAILED( ( ret = reporter->ReportFinal( reporter, ir ) ) ) ) {
            END_FUNCTION("_Abstract", ret );
            return ret;
        }
#endif                            
            
    TRACE_0( "post-processing abstraction method" );
    i = 1;
    ResetCurrentElement( methods3 );
    while( ( method = (ABSTRACTION_METHOD*)GetNextFromLinkedList( methods3 ) ) != NULL ) {
        TRACE_1("applying method %s", method->GetID( method ) );                
        ir->ResetChangeFlag( ir );
        method->Apply( method, ir );
        if( ir->IsStructureChanged( ir ) ) {
#ifdef GEN_DOT
            memset( filename, 0, sizeof(filename) );
            sprintf( filename, "./ir-%s-3-%i.dot", method->GetID( method ), i );
            if( ( file = fopen( filename, "w" ) ) == NULL ) {
                return ErrorReport( FAILING, "_Abstract", "dot file open error" ); 
            }
            if( IS_FAILED( ( ret = ir->GenerateDotFile( ir, file ) ) ) ) {
                END_FUNCTION("_Abstract", ret );
                return ret;
            }
            fclose( file );
#endif        
#ifdef GEN_SBML
            memset( filename, 0, sizeof(filename) );
            sprintf( filename, "./ir-%s-3-%i.xml", method->GetID( method ), i );
            if( ( file = fopen( filename, "w" ) ) == NULL ) {
                return ErrorReport( FAILING, "_Abstract", "sbml file open error" ); 
            }
            if( IS_FAILED( ( ret = ir->GenerateSBML( ir, file ) ) ) ) {
                END_FUNCTION("_Abstract", ret );
                return ret;
            }
            fclose( file );
#endif        
#ifdef GEN_XHTML
            memset( filename, 0, sizeof(filename) );
            sprintf( filename, "./ir-%s-3-%i.xhtml", method->GetID( method ), i );
            if( ( file = fopen( filename, "w" ) ) == NULL ) {
                return ErrorReport( FAILING, "_Abstract", "xhtml file open error" ); 
            }
            if( IS_FAILED( ( ret = ir->GenerateXHTML( ir, file ) ) ) ) {
                END_FUNCTION("_Abstract", ret );
                return ret;
            }
            fclose( file );
#endif        
        }
        i++;
    }
    
    DeleteLinkedList( &methods1 );
    DeleteLinkedList( &methods2 );
    DeleteLinkedList( &methods3 );
    
    END_FUNCTION("_Abstract", SUCCESS );
    
    return ret;
}


static RET_VAL _Abstract1( ABSTRACTION_ENGINE *abstractionEngine, IR *ir ) {
    RET_VAL ret = SUCCESS;
    BOOL changed = FALSE;
    int i = 0;
    LINKED_LIST *methods1 = NULL;
    ABSTRACTION_METHOD *method = NULL;
#if defined(REPORT_ABS)
    char *reporterType = NULL;
    ABSTRACTION_REPORTER *reporter = NULL;
#endif
#if defined(GEN_DOT) || defined(GEN_SBML) || defined(GEN_XHTML)   
    char filename[256];
    FILE *file = NULL;
#endif    
        
    START_FUNCTION("_Abstract1");
    
#ifdef GEN_DOT
    if( ( file = fopen( "ir.dot", "w" ) ) == NULL ) {
        return ErrorReport( FAILING, "_Abstract1", "dot file open error" ); 
    }
    if( IS_FAILED( ( ret = ir->GenerateDotFile( ir, file ) ) ) ) {
        END_FUNCTION("_Abstract1", ret );
        return ret;
    }
    fclose( file );
#endif
    
#ifdef GEN_XHTML
    if( ( file = fopen( "ir.xhtml", "w" ) ) == NULL ) {
    return ErrorReport( FAILING, "_Abstract", "dot file open error" ); 
    }
    if( IS_FAILED( ( ret = ir->GenerateXHTML( ir, file ) ) ) ) {
        END_FUNCTION("_Abstract1", ret );
        return ret;
    }
    fclose( file );
#endif


    if( IS_FAILED( ( ret = _PutKeepFlagInSpeciesNodes( abstractionEngine, ir ) ) ) ) {
    END_FUNCTION("_Abstract1", ret );
    return ret;
    }
    
    if( ( methods1 = _CreateListOfMethods( abstractionEngine, 1 ) ) == NULL ) {
        return ErrorReport( FAILING, "_Abstract", "could not create a method list1" );
    }
        

#if defined(REPORT_ABS)
    if( ( reporter = CreateAbstractionReporter( reporterType ) ) == NULL ) {
    return ErrorReport( FAILING, "_Abstract1", "abstraction reporter creation error" ); 
    }
    if( IS_FAILED( ( ret = reporter->ReportInitial( reporter, ir ) ) ) ) {
        END_FUNCTION("_Abstract1", ret );
        return ret;
    }
#endif
    
    
    TRACE_0( "pre-processing abstraction method" );
    i = 1;
    ResetCurrentElement( methods1 );
    while( ( method = (ABSTRACTION_METHOD*)GetNextFromLinkedList( methods1 ) ) != NULL ) {
        TRACE_1("applying method %s", method->GetID( method ) );                
        ir->ResetChangeFlag( ir );
        method->Apply( method, ir );

        if( ir->IsStructureChanged( ir ) ) {
#ifdef GEN_DOT
            memset( filename, 0, sizeof(filename) );
            sprintf( filename, "./ir-%s-1-%i.dot", method->GetID( method ), i );
            if( ( file = fopen( filename, "w" ) ) == NULL ) {
                return ErrorReport( FAILING, "_Abstract1", "dot file open error" ); 
            }
            if( IS_FAILED( ( ret = ir->GenerateDotFile( ir, file ) ) ) ) {
                END_FUNCTION("_Abstract1", ret );
                return ret;
            }
            fclose( file );
#endif        
#ifdef GEN_SBML
            memset( filename, 0, sizeof(filename) );
            sprintf( filename, "./ir-%s-1-%i.xml", method->GetID( method ), i );
            if( ( file = fopen( filename, "w" ) ) == NULL ) {
                return ErrorReport( FAILING, "_Abstract1", "sbml file open error" ); 
            }
            if( IS_FAILED( ( ret = ir->GenerateSBML( ir, file ) ) ) ) {
                END_FUNCTION("_Abstract1", ret );
                return ret;
            }
            fclose( file );
#endif        
#ifdef GEN_XHTML
            memset( filename, 0, sizeof(filename) );
            sprintf( filename, "./ir-%s-1-%i.xhtml", method->GetID( method ), i );
            if( ( file = fopen( filename, "w" ) ) == NULL ) {
                return ErrorReport( FAILING, "_Abstract1", "xhtml file open error" ); 
            }
            if( IS_FAILED( ( ret = ir->GenerateXHTML( ir, file ) ) ) ) {
                END_FUNCTION("_Abstract1", ret );
                return ret;
            }
            fclose( file );
#endif        
        }
#if defined(REPORT_ABS)
        if( IS_FAILED( ( ret = reporter->Report( reporter, method->GetID( method ), ir ) ) ) ) {
    END_FUNCTION("_Abstract1", ret );
    return ret;
        }
#endif                
        i++;
    }
    DeleteLinkedList( &methods1 );
    END_FUNCTION("_Abstract1", SUCCESS );
    
    return ret;
}

static RET_VAL _Abstract2( ABSTRACTION_ENGINE *abstractionEngine, IR *ir ) {
    RET_VAL ret = SUCCESS;
    BOOL changed = FALSE;
    int i = 0;
    int j = 0;
    LINKED_LIST *methods2 = NULL;
    ABSTRACTION_METHOD *method = NULL;
#if defined(REPORT_ABS)
    char *reporterType = NULL;
    ABSTRACTION_REPORTER *reporter = NULL;
#endif
#if defined(GEN_DOT) || defined(GEN_SBML) || defined(GEN_XHTML)   
    char filename[256];
    FILE *file = NULL;
#endif    
        
    START_FUNCTION("_Abstract2");
    
    if( IS_FAILED( ( ret = _PutKeepFlagInSpeciesNodes( abstractionEngine, ir ) ) ) ) {
    END_FUNCTION("_Abstract2", ret );
    return ret;
    }
    
    if( ( methods2 = _CreateListOfMethods( abstractionEngine, 2 ) ) == NULL ) {
        return ErrorReport( FAILING, "_Abstract2", "could not create a method list2" );
    }

#if defined(REPORT_ABS)
    if( ( reporter = CreateAbstractionReporter( reporterType ) ) == NULL ) {
    return ErrorReport( FAILING, "_Abstract2", "abstraction reporter creation error" ); 
    }
#endif
    
    TRACE_0( "main abstraction method" );
    i = 0;
    j = 1;
    do {
        TRACE_1( "main loop iteration %i", i );
        changed = FALSE;
        ResetCurrentElement( methods2 );
        while( ( method = (ABSTRACTION_METHOD*)GetNextFromLinkedList( methods2 ) ) != NULL ) {
            TRACE_1("applying method %s", method->GetID( method ) );                
            ir->ResetChangeFlag( ir );
            method->Apply( method, ir );
            if( !changed ) {
                changed = ir->IsStructureChanged( ir );
            }
            if( ir->IsStructureChanged( ir ) ) {
#ifdef GEN_DOT
                memset( filename, 0, sizeof(filename) );
                sprintf( filename, "./ir-%s-2-%i-%i.dot", method->GetID( method ), j, i );
                if( ( file = fopen( filename, "w" ) ) == NULL ) {
                    return ErrorReport( FAILING, "_Abstract2", "dot file open error" ); 
                }
                if( IS_FAILED( ( ret = ir->GenerateDotFile( ir, file ) ) ) ) {
                    END_FUNCTION("_Abstract2", ret );
                    return ret;
                }
                fclose( file );
#endif        
#ifdef GEN_SBML
                memset( filename, 0, sizeof(filename) );
                sprintf( filename, "./ir-%s-2-%i-%i.xml", method->GetID( method ), j, i );
                if( ( file = fopen( filename, "w" ) ) == NULL ) {
                    return ErrorReport( FAILING, "_Abstract2", "sbml file open error" ); 
                }
                if( IS_FAILED( ( ret = ir->GenerateSBML( ir, file ) ) ) ) {
                    END_FUNCTION("_Abstract2", ret );
                    return ret;
                }
                fclose( file );
#endif        
#ifdef GEN_XHTML
                memset( filename, 0, sizeof(filename) );
                sprintf( filename, "./ir-%s-2-%i-%i.xhtml", method->GetID( method ), j, i );
                if( ( file = fopen( filename, "w" ) ) == NULL ) {
                    return ErrorReport( FAILING, "_Abstract2", "xhtml file open error" ); 
                }
                if( IS_FAILED( ( ret = ir->GenerateXHTML( ir, file ) ) ) ) {
                    END_FUNCTION("_Abstract2", ret );
                    return ret;
                }
                fclose( file );
#endif        
            }
            j++;
#if defined(REPORT_ABS)
            if( IS_FAILED( ( ret = reporter->Report( reporter, method->GetID( method ), ir ) ) ) ) {
                END_FUNCTION("_Abstract2", ret );
                return ret;
            }
#endif                            
        }
        i++;
    } while( changed );

#if defined(REPORT_ABS)
    if( IS_FAILED( ( ret = reporter->ReportFinal( reporter, ir ) ) ) ) {
        END_FUNCTION("_Abstract2", ret );
        return ret;
    }
#endif                            
    DeleteLinkedList( &methods2 );
    
    END_FUNCTION("_Abstract2", SUCCESS );
    
    return ret;
}

static RET_VAL _Abstract3( ABSTRACTION_ENGINE *abstractionEngine, IR *ir ) {
    RET_VAL ret = SUCCESS;
    BOOL changed = FALSE;
    int i = 0;
    LINKED_LIST *methods3 = NULL;
    ABSTRACTION_METHOD *method = NULL;

#if defined(GEN_DOT) || defined(GEN_SBML) || defined(GEN_XHTML)   
    char filename[256];
    FILE *file = NULL;
#endif    
        
    START_FUNCTION("_Abstract3");
    

    if( IS_FAILED( ( ret = _PutKeepFlagInSpeciesNodes( abstractionEngine, ir ) ) ) ) {
        END_FUNCTION("_Abstract3", ret );
        return ret;
    }
    
    if( ( methods3 = _CreateListOfMethods( abstractionEngine, 3 ) ) == NULL ) {
        return ErrorReport( FAILING, "_Abstract3", "could not create a method list3" );
    }

    TRACE_0( "post-processing abstraction method" );
    i = 1;
    ResetCurrentElement( methods3 );
    while( ( method = (ABSTRACTION_METHOD*)GetNextFromLinkedList( methods3 ) ) != NULL ) {
        TRACE_1("applying method %s", method->GetID( method ) );                
        ir->ResetChangeFlag( ir );
        method->Apply( method, ir );
        if( ir->IsStructureChanged( ir ) ) {
#ifdef GEN_DOT
            memset( filename, 0, sizeof(filename) );
            sprintf( filename, "./ir-%s-3-%i.dot", method->GetID( method ), i );
            if( ( file = fopen( filename, "w" ) ) == NULL ) {
                return ErrorReport( FAILING, "_Abstract3", "dot file open error" ); 
            }
            if( IS_FAILED( ( ret = ir->GenerateDotFile( ir, file ) ) ) ) {
                END_FUNCTION("_Abstract3", ret );
                return ret;
            }
            fclose( file );
#endif        
#ifdef GEN_SBML
            memset( filename, 0, sizeof(filename) );
            sprintf( filename, "./ir-%s-3-%i.xml", method->GetID( method ), i );
            if( ( file = fopen( filename, "w" ) ) == NULL ) {
                return ErrorReport( FAILING, "_Abstract3", "sbml file open error" ); 
            }
            if( IS_FAILED( ( ret = ir->GenerateSBML( ir, file ) ) ) ) {
                END_FUNCTION("_Abstract3", ret );
                return ret;
            }
            fclose( file );
#endif        
#ifdef GEN_XHTML
            memset( filename, 0, sizeof(filename) );
            sprintf( filename, "./ir-%s-3-%i.xhtml", method->GetID( method ), i );
            if( ( file = fopen( filename, "w" ) ) == NULL ) {
                return ErrorReport( FAILING, "_Abstract3", "xhtml file open error" ); 
            }
            if( IS_FAILED( ( ret = ir->GenerateXHTML( ir, file ) ) ) ) {
                END_FUNCTION("_Abstract3", ret );
                return ret;
            }
            fclose( file );
#endif        
        }
        i++;
    }
    
    DeleteLinkedList( &methods3 );
    
    END_FUNCTION("_Abstract3", SUCCESS );
    
    return ret;
}


static LINKED_LIST *_CreateListOfMethods( ABSTRACTION_ENGINE *abstractionEngine, int i ) {
    LINKED_LIST *list = NULL;
    int num = 0;
    REB2SAC_PROPERTIES *properties = NULL;
    ABSTRACTION_METHOD_MANAGER *manager = NULL;
    ABSTRACTION_METHOD *method = NULL;
    char buf[512];
    char *methodID = NULL;
    
    START_FUNCTION("_CreateListOfMethods");
        
    if( ( list = CreateLinkedList() ) == NULL ) {
        END_FUNCTION("_CreateListOfMethods", FAILING );
        return NULL;    
    }
    
    properties = abstractionEngine->record->properties;
    manager = abstractionEngine->manager;
    
    for( num = 1; ; num++ ) {        
        sprintf( buf, "%s%i.%i", REB2SAC_ABSTRACTION_METHOD_KEY_PREFIX, i, num );
        if( ( methodID = properties->GetProperty( properties, buf ) ) == NULL ) {
            break;
        }
        if( ( method = manager->LookupMethod( manager, methodID ) ) == NULL ) {
            DeleteLinkedList( &list );
            END_FUNCTION("_CreateListOfMethods", FAILING );
            return NULL;
        }
        TRACE_2( "putting %s in method list %i", methodID, i );
        AddElementInLinkedList( (CADDR_T)method, list ); 
    }
    END_FUNCTION("_CreateListOfMethods", SUCCESS );
    return list;
}






static RET_VAL _Abstract_DEBUG( ABSTRACTION_ENGINE *abstractionEngine, IR *ir ) {
    RET_VAL ret = SUCCESS;
    int i = 0;
    int j = 0;
    BOOL changed = FALSE;
    ABSTRACTION_METHOD *method = NULL;
    ABSTRACTION_METHOD **methods = NULL;
#if defined(GEN_DOT) || defined(GEN_SBML)    
    char filename[256];
    FILE *file = NULL;
#endif
                
    START_FUNCTION("_Abstract_DEBUG");
    
#ifdef GEN_DOT
    if( ( file = fopen( "ir.dot", "w" ) ) == NULL ) {
        return ErrorReport( FAILING, "_Abstract_DEBUG", "dot file open error" ); 
    }
    if( IS_FAILED( ( ret = ir->GenerateDotFile( ir, file ) ) ) ) {
        END_FUNCTION("_Abstract_DEBUG", ret );
        return ret;
    }
    fclose( file );
#endif

    
    if( IS_FAILED( ( ret = _PutKeepFlagInSpeciesNodes( abstractionEngine, ir ) ) ) ) {
        END_FUNCTION("_Abstract_DEBUG", ret );
        return ret;
    }
    
    methods = _GetRegisteredMethods( abstractionEngine );
    j = 0;
    do {
        changed = FALSE;
        i = 0;        
        TRACE_1("abstraction iteration %i", j );                
        while( ( method = methods[i++] ) != NULL ) {
            TRACE_1("applying method %s", method->GetID( method ) );                
            ir->ResetChangeFlag( ir );
            method->Apply( method, ir );
            if( !changed ) {
                changed = ir->IsStructureChanged( ir );
            }
#ifdef GEN_DOT
            if( ir->IsStructureChanged( ir ) ) {
                memset( filename, 0, sizeof(filename) );
                sprintf( filename, "./ir-%s-%i.dot", method->GetID( method ), j );
                if( ( file = fopen( filename, "w" ) ) == NULL ) {
                    return ErrorReport( FAILING, "_Abstract_DEBUG", "dot file open error" ); 
                }
                if( IS_FAILED( ( ret = ir->GenerateDotFile( ir, file ) ) ) ) {
                    END_FUNCTION("_Abstract_DEBUG", ret );
                    return ret;
                }
                fclose( file );
            }
#endif

#ifdef GEN_SBML
            if( ir->IsStructureChanged( ir ) ) {
                memset( filename, 0, sizeof(filename) );
                sprintf( filename, "./ir-%s-%i.xml", method->GetID( method ), j );
                if( ( file = fopen( filename, "w" ) ) == NULL ) {
                    return ErrorReport( FAILING, "_Abstract_DEBUG", "sbml file open error" ); 
                }
                if( IS_FAILED( ( ret = ir->GenerateSBML( ir, file ) ) ) ) {
                    END_FUNCTION("_Abstract_DEBUG", ret );
                    return ret;
                }
                fclose( file );
            }
#endif        
        }
        j++;         
    } while( changed );                
    
    END_FUNCTION("_Abstract_DEBUG", SUCCESS );
    
    return ret;
}


static RET_VAL _Close( ABSTRACTION_ENGINE *abstractionEngine ) {
    RET_VAL ret = SUCCESS;
    ABSTRACTION_METHOD_MANAGER *manager = NULL;
#if defined(REPORT_ABS)
    ABSTRACTION_REPORTER *reporter = NULL;
#endif                            
    
    START_FUNCTION("_Close");

    manager = abstractionEngine->manager;
    
#if defined(REPORT_ABS)
    reporter = CreateAbstractionReporter( NULL );
    if( IS_FAILED( ( ret = CloseAbstractionReporter( &reporter ) ) ) ) {
        END_FUNCTION("_Abstract", ret );
        return ret;
    }
#endif                            
    
    if( IS_FAILED( ( ret = manager->Free( manager ) ) ) ) {
        END_FUNCTION("_Close", ret );    
        return ret;
    } 
    abstractionEngine->record = NULL;
        
    END_FUNCTION("_Close", SUCCESS );    
    return ret;
}
