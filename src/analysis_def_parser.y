%{
#include "sad_ast.h"

static SAD_AST_ENV *_astEnv;
static SAD_AST *ast;
static SPECIES *species;
static REACTION *reaction;
        
       
       

static SPECIES *FindSpeciesFromID( char *id ) {
    int i = 0;
    SAD_AST_ENV *env = GetSadAstEnv();
    int size = env->speciesSize;
    SPECIES **speciesArray = env->speciesArray;
    SPECIES *target = NULL;
    char *targetID = NULL;
     
    for( ; i < size; i++ ) {
        target = speciesArray[i];
        targetID = GetCharArrayOfString( GetSpeciesNodeName( target ) );
        if( strcmp( targetID, id ) == 0 ) {
            return target;
        }
    }
    return NULL;
}

static REACTION *FindReactionFromID( char *id ) {
    int i = 0;
    SAD_AST_ENV *env = GetSadAstEnv();
    int size = env->speciesSize;
    REACTION **reactionArray = env->reactionArray;
    REACTION *target = NULL;
    char *targetID = NULL;
     
    for( ; i < size; i++ ) {
        target = reactionArray[i];
        targetID = GetCharArrayOfString( GetReactionNodeName( target ) );
        if( strcmp( targetID, id ) == 0 ) {
            return target;
        }
    }
    return NULL;
}
%}

%union	{
    char *string;	
    double value;
    SAD_AST *ast;
}

%token SAD_TERM SAD_DESC SAD_COND 
%token SAD_LPAREN SAD_RPAREN SAD_SEMI SAD_LCURLY SAD_RCURLY SAD_COMMA
%token SAD_AND SAD_NOT SAD_OR 
%token SAD_LE SAD_LT SAD_GE SAD_GT SAD_EQ
%token SAD_PLUS SAD_MINUS SAD_TIMES SAD_DIV 
%token SAD_TIME_VAR SAD_IDENTIFIER SAD_SPECIES SAD_REACTION SAD_STRING SAD_CONSTANT
%token SAD_CON_OP SAD_NUM_OP SAD_ATSIGN
%token SAD_EXP SAD_POW SAD_LOG
%token SAD_ERROR        
        
%type <value> SAD_CONSTANT         
%type <string> SAD_TIME_VAR         
%type <string> SAD_STRING         
%type <string> SAD_IDENTIFIER
%type <string> SAD_SPECIES
%type <string> SAD_REACTION
        
        
        
%left SAD_AND SAD_OR
%left SAD_LE SAD_LT SAD_GE SAD_GT SAD_EQ
%left SAD_PLUS SAD_MINUS
%left SAD_TIMES SAD_DIV
%left UMINUS SAD_NOT
%left SAD_NUM_OP SAD_CON_OP 
%left SAD_LPAREN 


%%

program : { 
        _astEnv = GetSadAstEnv();
    
    }
    opt_term_list
;            

opt_term_list :
    /* empty */  
    | opt_term_list term_dec {        
        _astEnv = GetSadAstEnv();
        if( IS_FAILED( AddElementInLinkedList( (CADDR_T)$<ast>2, _astEnv->termList->terms ) ) ) {
            PrintSadAstErrorMessage( "Error adding term %s in the list", ((SAD_AST_TERM*)$<ast>2)->id );
            return 1;
        }
    }
;

term_dec :
    SAD_TERM SAD_IDENTIFIER SAD_LCURLY desc_statement cond_statement SAD_RCURLY {
      ast = (SAD_AST*)CreateSadAstTerm( $2, $<string>4, (SAD_AST_EXP*)$<ast>5 );
        if( ast == NULL ) {
            PrintSadAstErrorMessage( "Error creating term %s", $2 );
            yyerror(NULL); return 1;
        } 
        $<ast>$ = ast; 
    }
;        
        
desc_statement :
    SAD_DESC SAD_STRING SAD_SEMI  {
        $<string>$ = $2;     
    }
;            
        
cond_statement :
    SAD_COND bool_exp SAD_SEMI {
        $<ast>$ = $<ast>2;     
    }
;        
        
bool_exp :
    logical_exp {
        $<ast>$ = $<ast>1;     
    } 
    | comp_exp {
        $<ast>$ = $<ast>1;     
    }                                 
;            
        
logical_exp :
        bool_exp SAD_AND bool_exp  {
        ast = (SAD_AST*)CreateSadAstBinaryLogicalExp( LOGICAL_EXP_TYPE_SAD_AST_AND, (SAD_AST_EXP*)$<ast>1, (SAD_AST_EXP*)$<ast>3 );
        if( ast == NULL ) {
            PrintSadAstErrorMessage( "Error creating and expression" );
            yyerror(NULL); return 1;
        } 
        $<ast>$ = ast; 
    }         
    | bool_exp SAD_OR bool_exp {
        ast = (SAD_AST*)CreateSadAstBinaryLogicalExp( LOGICAL_EXP_TYPE_SAD_AST_OR, (SAD_AST_EXP*)$<ast>1, (SAD_AST_EXP*)$<ast>3 );
        if( ast == NULL ) {
            PrintSadAstErrorMessage( "Error creating or expression" );
            yyerror(NULL); return 1;
        } 
        $<ast>$ = ast; 
    }        
    | SAD_NOT bool_exp {
        ast = (SAD_AST*)CreateSadAstUnaryLogicalExp( LOGICAL_EXP_TYPE_SAD_AST_NOT, (SAD_AST_EXP*)$<ast>2 );
        if( ast == NULL ) {
            PrintSadAstErrorMessage( "Error creating not expression" );
            yyerror(NULL); return 1;
        } 
        $<ast>$ = ast; 
    }     
    | SAD_LPAREN logical_exp SAD_RPAREN {
        $<ast>$ = $<ast>2;     
    }                                          
;        
    
comp_exp :
    num_exp SAD_LE num_exp {
        ast = (SAD_AST*)CreateSadAstCompExp( COMP_EXP_TYPE_SAD_AST_LE, (SAD_AST_EXP*)$<ast>1, (SAD_AST_EXP*)$<ast>3 );
        if( ast == NULL ) {
            PrintSadAstErrorMessage( "Error creating less than or equal to expression" );
            yyerror(NULL); return 1;
        } 
        $<ast>$ = ast; 
    }       
    | num_exp SAD_LT num_exp {
        ast = (SAD_AST*)CreateSadAstCompExp( COMP_EXP_TYPE_SAD_AST_LT, (SAD_AST_EXP*)$<ast>1, (SAD_AST_EXP*)$<ast>3 );
        if( ast == NULL ) {
            PrintSadAstErrorMessage( "Error creating less than expression" );
            yyerror(NULL); return 1;
        } 
        $<ast>$ = ast; 
    }      
    | num_exp SAD_GE num_exp {
        ast = (SAD_AST*)CreateSadAstCompExp( COMP_EXP_TYPE_SAD_AST_GE, (SAD_AST_EXP*)$<ast>1, (SAD_AST_EXP*)$<ast>3 );
        if( ast == NULL ) {
            PrintSadAstErrorMessage( "Error creating greater than or equal to expression" );
            yyerror(NULL); return 1;
        } 
        $<ast>$ = ast; 
    }     
    | num_exp SAD_GT num_exp {
        ast = (SAD_AST*)CreateSadAstCompExp( COMP_EXP_TYPE_SAD_AST_GT, (SAD_AST_EXP*)$<ast>1, (SAD_AST_EXP*)$<ast>3 );
        if( ast == NULL ) {
            PrintSadAstErrorMessage( "Error creating greater than expression" );
            yyerror(NULL); return 1;
        } 
        $<ast>$ = ast; 
    }    
    | num_exp SAD_EQ num_exp {
        ast = (SAD_AST*)CreateSadAstCompExp( COMP_EXP_TYPE_SAD_AST_EQ, (SAD_AST_EXP*)$<ast>1, (SAD_AST_EXP*)$<ast>3 );
        if( ast == NULL ) {
            PrintSadAstErrorMessage( "Error creating equal expression" );
            yyerror(NULL); return 1;
        } 
        $<ast>$ = ast; 
    }   
    | SAD_LPAREN comp_exp SAD_RPAREN  {
        $<ast>$ = $<ast>2;     
    }                                   
;        

num_exp : 
    num_exp SAD_PLUS num_exp {
        ast = (SAD_AST*)CreateSadAstBinaryNumExp( NUM_EXP_TYPE_SAD_AST_PLUS, (SAD_AST_EXP*)$<ast>1, (SAD_AST_EXP*)$<ast>3 );
        if( ast == NULL ) {
            PrintSadAstErrorMessage( "Error creating plus expression" );
            yyerror(NULL); return 1;
        } 
        $<ast>$ = ast; 
    }  
    | num_exp SAD_MINUS num_exp {
        ast = (SAD_AST*)CreateSadAstBinaryNumExp( NUM_EXP_TYPE_SAD_AST_MINUS, (SAD_AST_EXP*)$<ast>1, (SAD_AST_EXP*)$<ast>3 );
        if( ast == NULL ) {
            PrintSadAstErrorMessage( "Error creating minus expression" );
            yyerror(NULL); return 1;
        } 
        $<ast>$ = ast; 
    } 
    | num_exp SAD_TIMES num_exp {
        ast = (SAD_AST*)CreateSadAstBinaryNumExp( NUM_EXP_TYPE_SAD_AST_TIMES, (SAD_AST_EXP*)$<ast>1, (SAD_AST_EXP*)$<ast>3 );
        if( ast == NULL ) {
            PrintSadAstErrorMessage( "Error creating times expression" );
            yyerror(NULL); return 1;
        } 
        $<ast>$ = ast; 
    }
    | num_exp SAD_DIV num_exp {
        ast = (SAD_AST*)CreateSadAstBinaryNumExp( NUM_EXP_TYPE_SAD_AST_DIV, (SAD_AST_EXP*)$<ast>1, (SAD_AST_EXP*)$<ast>3 );
        if( ast == NULL ) {
            PrintSadAstErrorMessage( "Error creating div expression" );
            yyerror(NULL); return 1;
        } 
        $<ast>$ = ast; 
    }
    | SAD_MINUS num_exp %prec UMINUS {
        ast = (SAD_AST*)CreateSadAstUnaryNumExp( NUM_EXP_TYPE_SAD_AST_UMINUS, (SAD_AST_EXP*)$<ast>2 );
        if( ast == NULL ) {
            PrintSadAstErrorMessage( "Error creating unary minus expression" );
            yyerror(NULL); return 1;
        } 
        $<ast>$ = ast; 
    }
    | SAD_LPAREN num_exp SAD_RPAREN {
        $<ast>$ = $<ast>2;     
    }                        
    | SAD_EXP SAD_LPAREN num_exp SAD_RPAREN {
        ast = (SAD_AST*)CreateSadAstFuncExp( "exp", 1, (SAD_AST_EXP*)$<ast>3 );
        if( ast == NULL ) {
            PrintSadAstErrorMessage( "Error creating exp function" );
            yyerror(NULL); return 1;
        } 
        $<ast>$ = ast; 
    }                       
    | SAD_POW SAD_LPAREN num_exp SAD_COMMA num_exp SAD_RPAREN {
        ast = (SAD_AST*)CreateSadAstFuncExp( "pow", 2, (SAD_AST_EXP*)$<ast>3, (SAD_AST_EXP*)$<ast>5 );
        if( ast == NULL ) {
            PrintSadAstErrorMessage( "Error creating pow function" );
            yyerror(NULL); return 1;
        } 
        $<ast>$ = ast; 
    }           
    | SAD_LOG SAD_LPAREN num_exp SAD_RPAREN {
        ast = (SAD_AST*)CreateSadAstFuncExp( "log", 1, (SAD_AST_EXP*)$<ast>3 );
        if( ast == NULL ) {
            PrintSadAstErrorMessage( "Error creating log function" );
            yyerror(NULL); return 1;
        } 
        $<ast>$ = ast; 
    }            
    | SAD_CONSTANT { 
        ast = (SAD_AST*)CreateSadAstConstant( $1 );
        if( ast == NULL ) {
            PrintSadAstErrorMessage( "Error creating time variable" );
            yyerror(NULL); return 1;
        } 
        $<ast>$ = ast; 
    }
    | SAD_TIME_VAR { 
        ast = (SAD_AST*)CreateSadAstTimeVar( );
        if( ast == NULL ) {
            PrintSadAstErrorMessage( "Error creating time variable" );
            yyerror(NULL); return 1;
        } 
        $<ast>$ = ast; 
    }
    | SAD_CON_OP SAD_IDENTIFIER { 
        species = FindSpeciesFromID( $2 );
        if( species == NULL ) {
            PrintSadAstErrorMessage( "%s is not a valid species ID", $2 );
            yyerror(NULL); return 1;
        } 
        ast = (SAD_AST*)CreateSadAstSpeciesCon( species );
        if( ast == NULL ) {
            PrintSadAstErrorMessage( "Error creating species node for %s", $2 );
            yyerror(NULL); return 1;
        } 
        $<ast>$ = ast; 
        FREE($2);
    }
    | SAD_NUM_OP SAD_SPECIES { 
        species = FindSpeciesFromID( $2 );
        if( species == NULL ) {
            PrintSadAstErrorMessage( "%s is not a valid species ID", $2 );
            yyerror(NULL); return 1;
        } 
        ast = (SAD_AST*)CreateSadAstSpeciesCnt( species );
        if( ast == NULL ) {
            PrintSadAstErrorMessage( "Error creating species node for %s", $2 );
            yyerror(NULL); return 1;
        } 
        $<ast>$ = ast; 
        FREE($2);
    }
    | SAD_IDENTIFIER { 
        species = FindSpeciesFromID( $1 );
        if( species == NULL ) {
            PrintSadAstErrorMessage( "%s is not a valid species ID", $1 );
            yyerror(NULL); return 1;
        } 
        ast = (SAD_AST*)CreateSadAstSpeciesCnt( species );
        if( ast == NULL ) {
            PrintSadAstErrorMessage( "Error creating species node for %s", $1 );
            yyerror(NULL); return 1;
        } 
        $<ast>$ = ast; 
        FREE($1);
    }
    | SAD_ATSIGN SAD_REACTION { 
        reaction = FindReactionFromID( $2 );
        if( reaction == NULL ) {
            PrintSadAstErrorMessage( "%s is not a valid reaction ID", $2 );
            yyerror(NULL); return 1;
        } 
        ast = (SAD_AST*)CreateSadAstReactionCnt( reaction );
        if( ast == NULL ) {
            PrintSadAstErrorMessage( "Error creating reaction node for %s", $2 );
            yyerror(NULL); return 1;
        }
        $<ast>$ = ast; 
        FREE($2);
    }
;        
        

    
%%
