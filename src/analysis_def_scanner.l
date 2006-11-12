%{
#include <stdio.h>
#include <stdlib.h>
#include "sad_ast.h"
#include "analysis_def_parser.tab.h"	
static int lineNum = 1;
static char *_string = NULL;

#if LEX_MAIN_TEST                
YYSTYPE yylval;
#endif
%}


AND "&&"
NOT "!"
OR "||"
LPAREN "("
RPAREN ")"
LCURLY "{"
RCURLY "}"
SEMI ";"
COMMA ","
PLUS "+"
MINUS "-"
TIMES "*"
DIV "/"
LT "<"
GT ">"
LE "<="
GE ">="
EQ "="
NUM_OP "\#"
CON_OP "\%"
TIME "t"
TERM "term"
DESC "desc"
COND "cond"       
LOG "log"
EXP "exp"
POW "pow"
        
        
LINE_TERMINATOR (\n)
COMMENT ("//".*)$
ALPHA [a-zA-Z]
LETTER {ALPHA}
DIGIT [0-9]
IDENTIFIER (({LETTER}|"_")({LETTER}|{DIGIT}|"_")*)
SPECIES \${IDENTIFIER}
REACTION \@{IDENTIFIER}
CONSTANT 0*{DIGIT}+ 
INTEGER (0|[1-9][0-9]*)
REAL1 {INTEGER}"."[0-9]*
REAL2 "."[0-9]+
REAL3 {REAL1}[eE][+-]?{INTEGER} 
REAL4 {REAL2}[eE][+-]?{INTEGER}
REAL5 {INTEGER}[eE][+-]?{INTEGER}
WHITE_SPACE [ \t\r]
SEPARATOR ({WHITE_SPACE}|{LINE_TERMINATOR})+
STRING_LITERAL (\"([^\"\n\\]|\\.)*\") 
ERROR_CONSTANT ({CONSTANT}{IDENTIFIER})
ERROR_STRING (\"([^\"\n\\]|\\.)*)$ 
ERROR_STRINGX (\"((\\(\\\\)*)|{WHITE_SPACE})*)$
        
%%

        
        

{AND} {
#if LEX_DEBUG
        printf("at line %i: %s\n", lineNum, yytext );            
#endif
    return SAD_AND;            
}

{NOT} {
#if LEX_DEBUG
        printf("at line %i: %s\n", lineNum, yytext );            
#endif
    return SAD_NOT;            
}

{OR} {
#if LEX_DEBUG
        printf("at line %i: %s\n", lineNum, yytext );            
#endif
    return SAD_OR;            
}

{LPAREN} {
#if LEX_DEBUG
        printf("at line %i: %s\n", lineNum, yytext );            
#endif
    return SAD_LPAREN;            
}

{RPAREN} {
#if LEX_DEBUG
        printf("at line %i: %s\n", lineNum, yytext );            
#endif
    return SAD_RPAREN;            
}

{LCURLY} {
#if LEX_DEBUG
        printf("at line %i: %s\n", lineNum, yytext );            
#endif
    return SAD_LCURLY;            
}

{RCURLY} {
#if LEX_DEBUG
        printf("at line %i: %s\n", lineNum, yytext );            
#endif
    return SAD_RCURLY;            
}

{SEMI} {
#if LEX_DEBUG
        printf("at line %i: %s\n", lineNum, yytext );            
#endif
    return SAD_SEMI;            
}

{COMMA} {
#if LEX_DEBUG
        printf("at line %i: %s\n", lineNum, yytext );            
#endif
    return SAD_COMMA;            
}


{PLUS} {
#if LEX_DEBUG
        printf("at line %i: %s\n", lineNum, yytext );            
#endif
    return SAD_PLUS;            
}

{MINUS} {
#if LEX_DEBUG
        printf("at line %i: %s\n", lineNum, yytext );            
#endif
    return SAD_MINUS;            
}

{TIMES} {
#if LEX_DEBUG
        printf("at line %i: %s\n", lineNum, yytext );            
#endif
    return SAD_TIMES;            
}

{DIV} {
#if LEX_DEBUG
        printf("at line %i: %s\n", lineNum, yytext );            
#endif
    return SAD_DIV;            
}

{LT} {
#if LEX_DEBUG
        printf("at line %i: %s\n", lineNum, yytext );            
#endif
    return SAD_LT;            
}

{GT} {
#if LEX_DEBUG
        printf("at line %i: %s\n", lineNum, yytext );            
#endif
    return SAD_GT;            
}

{LE} {
#if LEX_DEBUG
        printf("at line %i: %s\n", lineNum, yytext );            
#endif
    return SAD_LE;            
}

{GE} {
#if LEX_DEBUG
        printf("at line %i: %s\n", lineNum, yytext );            
#endif
    return SAD_GE;            
}

{EQ} {
#if LEX_DEBUG
        printf("at line %i: %s\n", lineNum, yytext );            
#endif
    return SAD_EQ;            
}

{NUM_OP} {
#if LEX_DEBUG
        printf("at line %i: %s\n", lineNum, yytext );            
#endif
    return SAD_NUM_OP;            
}

{CON_OP} {
#if LEX_DEBUG
        printf("at line %i: %s\n", lineNum, yytext );            
#endif
    return SAD_CON_OP;            
}

{TIME} {
#if LEX_DEBUG
        printf("time var at line %i: %s\n", lineNum, yytext );            
#endif
    return SAD_TIME_VAR;            
}

{LOG} {
#if LEX_DEBUG
        printf("log at line %i: %s\n", lineNum, yytext );            
#endif
    return SAD_LOG;            
}

{EXP} {
#if LEX_DEBUG
        printf("exp at line %i: %s\n", lineNum, yytext );            
#endif
    return SAD_EXP;            
}
        
{POW} {
#if LEX_DEBUG
        printf("pow at line %i: %s\n", lineNum, yytext );            
#endif
    return SAD_POW;            
}

{TERM} {
#if LEX_DEBUG
        printf("term at line %i: %s\n", lineNum, yytext );            
#endif
    return SAD_TERM;            
}
        
{DESC} {
#if LEX_DEBUG
        printf("desc at line %i: %s\n", lineNum, yytext );            
#endif
    return SAD_DESC;            
}
        
{COND} {
#if LEX_DEBUG
        printf("cond at line %i: %s\n", lineNum, yytext );            
#endif
    return SAD_COND;            
}
        

{IDENTIFIER} {
    _string = (char*)MALLOC( yyleng + 1 );
    strcpy( _string, yytext );
    yylval.string = _string; 
#if LEX_DEBUG
        printf("ID at line %i: %s\n", lineNum, _string );            
#endif
    return SAD_IDENTIFIER;            
}

{SPECIES} {
    _string = (char*)MALLOC( yyleng );
    strcpy( _string, yytext + 1 );
    yylval.string = _string; 
#if LEX_DEBUG
        printf("species at line %i: %s\n", lineNum, _string );            
#endif
    return SAD_SPECIES;            
}

{REACTION} {
    _string = (char*)MALLOC( yyleng );
    strcpy( _string, yytext + 1 );
    yylval.string = _string; 
#if LEX_DEBUG
        printf("reaction at line %i: %s\n", lineNum, _string );            
#endif
    return SAD_REACTION;            
}

{INTEGER} {
    yylval.value = strtod( yytext, NULL ); 
#if LEX_DEBUG
        printf("int at line %i: %g\n", lineNum, yylval.value );            
#endif
    return SAD_CONSTANT;            
}
        

{REAL1} |
{REAL2} |
{REAL3} |
{REAL4} |
{REAL5} {
    yylval.value = strtod( yytext, NULL ); 
#if LEX_DEBUG
        printf("real at line %i: %g\n", lineNum, yylval.value );            
#endif
    return SAD_CONSTANT;            
}

{STRING_LITERAL} {
    yytext[yyleng - 1] = '\0'; 
    _string = (char*)MALLOC( yyleng - 1 );
    strcpy( _string, yytext + 1 );
    yylval.string = _string; 
#if LEX_DEBUG
        printf("string at line %i: %s\n", lineNum, _string );            
#endif
    return SAD_STRING;            
}

{ERROR_STRING} {
#if LEX_DEBUG
        printf("error string at line %i: %s\n", lineNum, yytext );            
#endif
    return SAD_ERROR;            
}

{ERROR_STRINGX} {
#if LEX_DEBUG
        printf("error string at line %i: %s\n", lineNum, yytext );            
#endif
    return SAD_ERROR;            
}

{WHITE_SPACE} ;

{COMMENT} {
#if LEX_DEBUG
    printf("COMMENT at line %i: %s\n", lineNum, yytext );            
#endif
}

{LINE_TERMINATOR} {
    lineNum++;
}

. {
#if LEX_DEBUG
        printf("error at line %i: %s\n", lineNum, yytext );            
#endif
    return SAD_ERROR;            
}


%% 
    
int yywrap() {
    return 1; 
}
        
void yyerror( char *str ) {
    fprintf( stderr, "error at line %i: %s\n", lineNum, yytext );
}        
        
#if LEX_MAIN_TEST                
int main( int argc, char *argv[] ) {
    FILE *file = NULL;
    int ret = -1;
                
    if( argc > 1 ) {
        if( ( file = fopen( argv[1], "r" ) ) == NULL ) {
            fprintf( stderr, "file %s could not be read\n", argv[1] );
            return 1; 
        }
        yyin = file;
    }
    do {
        ret = yylex();
    } while( ret != YY_NULL );
    
    return 0;
}        
#endif

  
  
  