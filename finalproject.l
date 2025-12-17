%option noyywrap
%{
#include "finalproject.tab.h"
%}

separator    [ \t\n\r]
N_operator   [\+\-\*\/\>\<\=]
MOD_operator "mod"
parentheses  [\(\)]
number       0|([1-9][0-9]*)|(-[1-9][0-9]*)
ID           [a-z]([a-z]|[0-9]|-)*
bool_val     #t|#f

%%
"and" {
    return AND;
}

"or" {
    return OR;
}

"not" {
    return NOT;
}

"if" {
    return IF;
}

"define" {
    return DEFINE;
}

"fun" {
    return FUNCTION;
}

"print-num" {
    return PRINT_NUM;
}

"print-bool" {
    return PRINT_BOOL;
}

{separator} {;}

{N_operator} {
    return yytext[0];
}

{MOD_operator}  {
    return MOD;
}

{parentheses} {
    return yytext[0];
}

{number} {
    yylval.ival = atoi(yytext);
    return NUMBER;
}

{ID} {
    yylval.sval = strdup(yytext);
    return ID;
}

{bool_val} {
    if (yytext[1] == 't') {
        yylval.bval = 1;
    } else if (yytext[1] == 'f') {
        yylval.bval = 0;
    }
    return BOOLEAN;
} 

. {;}

%%