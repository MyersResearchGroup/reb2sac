%{
%}

%union	{
    char *string;	
    double value;
}

%token SAD_TERM SAD_DESC SAD_COND 
%token SAD_LPAREN SAD_RPAREN SAD_SEMI SAD_LCURLY SAD_RCURLY SAD_COMMA
%token SAD_AND SAD_NOT SAD_OR 
%token SAD_LE SAD_LT SAD_GE SAD_GT SAD_EQ
%token SAD_PLUS SAD_MINUS SAD_TIMES SAD_DIV 
%token SAD_TIME_VAR SAD_IDENTIFIER SAD_SPECIES SAD_REACTION SAD_STRING SAD_CONSTANT
%token SAD_CON_OP SAD_NUM_OP
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

program : 
      opt_term_list
;            

opt_term_list :
        /* empty */  
    | opt_term_list term_dec
;

term_dec :
    SAD_TERM SAD_IDENTIFIER SAD_LCURLY desc_statement cond_statement SAD_RCURLY
;        
        
desc_statement :
    SAD_DESC SAD_STRING SAD_SEMI
;            
        
cond_statement :
    SAD_COND bool_exp SAD_SEMI
;        
        
bool_exp :
      logical_exp
    | comp_exp
;            
        
logical_exp :
      bool_exp SAD_AND bool_exp 
    | bool_exp SAD_OR bool_exp 
    | SAD_NOT bool_exp     
    | SAD_LPAREN logical_exp SAD_RPAREN            
;        
    
comp_exp :
      num_exp SAD_LE num_exp 
    | num_exp SAD_LT num_exp 
    | num_exp SAD_GE num_exp 
    | num_exp SAD_GT num_exp 
    | num_exp SAD_EQ num_exp 
    | SAD_LPAREN comp_exp SAD_RPAREN            
;        

num_exp : 
      num_exp SAD_PLUS num_exp 
    | num_exp SAD_MINUS num_exp 
    | num_exp SAD_TIMES num_exp 
    | num_exp SAD_DIV num_exp 
    | SAD_MINUS num_exp %prec UMINUS 
    | SAD_LPAREN num_exp SAD_RPAREN            
    | SAD_EXP SAD_LPAREN num_exp SAD_RPAREN            
    | SAD_POW SAD_LPAREN num_exp SAD_COMMA num_exp SAD_RPAREN            
    | SAD_LOG SAD_LPAREN num_exp SAD_RPAREN            
    | SAD_CONSTANT 
    | SAD_TIME_VAR 
    | SAD_SPECIES
    | SAD_REACTION    
    | SAD_CON_OP SAD_SPECIES
    | SAD_NUM_OP SAD_SPECIES
    | SAD_NUM_OP SAD_REACTION
;        
        
    
%%