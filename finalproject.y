%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "node.h"

int yylex();
void yyerror(const char* message);

%}

%union{
    int ival;
    int bval;
    char *sval;
    struct node *AST_Node;
}

%token<ival> NUMBER
%token<bval> BOOLEAN
%token<sval> ID
%token PRINT_NUM PRINT_BOOL DEFINE FUNCTION IF AND OR NOT MOD

%type <AST_Node> program stmts stmt print_stmt expr exprs num_operator logi_operator
%type <AST_Node> plus minus multiply divide modulus greater smaller equal
%type <AST_Node> and_operator or_operator not_operator
%type <AST_Node> def_stmt variable func_expr func_ids ids func_body func_call param func_name
%type <AST_Node> if_expr test_expr then_expr else_expr

%%
program : stmts         {root = $1;}
        ;

stmts   : stmts stmt    {$$ = new_node($1, $2, NODE_STMTS);}
        | stmt          {$$ = $1;}
        ;

stmt    : expr          {$$ = $1;}
        | def_stmt      {$$ = $1;}
        | print_stmt    {$$ = $1;}
        ;

print_stmt  : '(' PRINT_NUM expr ')'    {$$ = new_node($3, NULL, NODE_STMT_PRINT_NUM);}
            | '(' PRINT_BOOL expr ')'   {$$ = new_node($3, NULL, NODE_STMT_PRINT_BOOL);}
            ;

expr    : BOOLEAN       {$$ = new_node(NULL, NULL, NODE_BOOL); $$->value = $1;}
        | NUMBER        {$$ = new_node(NULL, NULL, NODE_INT); $$->value = $1;}
        | variable      {$$ = $1;}
        | num_operator  {$$ = $1;}
        | logi_operator {$$ = $1;}
        | func_expr     {$$ = $1;}
        | func_call     {$$ = $1;}
        | if_expr       {$$ = $1;}
        ;

exprs   : expr exprs    {$$ = new_node($1, $2, NODE_LIST);}
        | expr          {$$ = $1;}
        ;

num_operator    : plus      {$$ = $1;}
                | minus     {$$ = $1;}
                | multiply  {$$ = $1;}
                | divide    {$$ = $1;}
                | modulus   {$$ = $1;}
                | greater   {$$ = $1;}
                | smaller   {$$ = $1;}
                | equal     {$$ = $1;}
                ;

plus    : '(' '+' expr exprs ')'    {$$ = new_node($3, $4, NODE_OP_ADD);}
        ;

minus   : '(' '-' expr expr ')'     {$$ = new_node($3, $4, NODE_OP_SUB);}
        ;

multiply: '(' '*' expr exprs ')'    {$$ = new_node($3, $4, NODE_OP_MUL);}
        ;

divide  : '(' '/' expr expr ')'     {$$ = new_node($3, $4, NODE_OP_DIV);}
        ;

modulus : '(' MOD expr expr ')'     {$$ = new_node($3, $4, NODE_OP_MOD);}
        ;

greater : '(' '>' expr expr ')'     {$$ = new_node($3, $4, NODE_OP_GREATER);}
        ;

smaller : '(' '<' expr expr ')'     {$$ = new_node($3, $4, NODE_OP_SMALLER);}
        ;

equal   : '(' '=' expr exprs ')'    {$$ = new_node($3, $4, NODE_OP_EQUAL);}
        ;

logi_operator   : and_operator  {$$ = $1;}
                | or_operator   {$$ = $1;}
                | not_operator  {$$ = $1;}
                ;

and_operator    : '(' AND expr exprs ')'    {$$ = new_node($3, $4, NODE_OP_AND);}
                ;

or_operator     : '(' OR expr exprs ')'     {$$ = new_node($3, $4, NODE_OP_OR);}
                ;

not_operator    : '(' NOT expr ')'          {$$ = new_node($3, NULL, NODE_OP_NOT);}
                ;

def_stmt    : '(' DEFINE variable expr ')'  {$$ = new_node($3, $4, NODE_STMT_DEF);}
            ;

variable    : ID    {$$ = new_node(NULL, NULL, NODE_VAR); $$->identify = $1;}
            ;

func_expr   : '(' FUNCTION func_ids func_body ')'   {$$ = new_node($3, $4, NODE_FUNC_EXPR);}
            ;

func_ids    : '(' ids ')'   {$$ = $2;}
            ;

ids         : ids variable  {$$ = new_node($1, $2, NODE_LIST);}
            |               {$$ = new_node(NULL, NULL, NODE_NULL);}
            ;

func_body   : expr  {$$ = $1;}
            ;

func_call   : '(' func_expr param ')'   {$$ = new_node($2, $3, NODE_FUNC_CALL);}
            | '(' func_name param ')'   {$$ = new_node($2, $3, NODE_FUNC_CALL_NAMED);}
            ;

param       : expr param    {$$ = new_node($1, $2, NODE_LIST);}
            |               {$$ = new_node(NULL, NULL, NODE_NULL);}
            ;

func_name   : ID    {$$ = new_node(NULL, NULL, NODE_FUNC_NAME); $$->identify = $1;}
            ;

if_expr     : '(' IF test_expr then_expr else_expr ')' {$$ = new_node($3, $5, NODE_STMT_IF); $$->middle = $4;}
            ;

test_expr   : expr  {$$ = $1;}
            ;

then_expr   : expr  {$$ = $1;}
            ;

else_expr   : expr  {$$ = $1;}
            ;
%%

void yyerror (const char *message) {
    printf("%s\n", message);
}

int main(int argc, char *argv[]) {
    yyparse();
    traverse(root); 
    clear_tree(root);
    return(0);
}
