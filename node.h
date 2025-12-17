#ifndef NODE_H
#define NODE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* ===== define node type ===== */
typedef enum { // node type enum
    NODE_INT,  
    NODE_BOOL,  
    NODE_VAR,  
    NODE_STMTS,  
    NODE_LIST,  
    NODE_NULL, 

    NODE_OP_ADD,
    NODE_OP_SUB,
    NODE_OP_MUL,
    NODE_OP_DIV,
    NODE_OP_MOD,
    NODE_OP_GREATER,
    NODE_OP_SMALLER,
    NODE_OP_EQUAL,
    
    NODE_OP_AND,
    NODE_OP_OR,
    NODE_OP_NOT,

    NODE_STMT_PRINT_NUM,
    NODE_STMT_PRINT_BOOL,
    NODE_STMT_DEF,
    NODE_STMT_IF,
    
    NODE_FUNC_EXPR,
    NODE_FUNC_NAME,
    NODE_FUNC_CALL,
    NODE_FUNC_CALL_NAMED
} node_type;


/* ===== define struct ===== */
struct node{
    node_type type;
    char *identify; // identify var name
    int value; // store NUM or BOOLEAN
    struct node *left, *right, *middle; // other nodes, middle for if-else
};

struct symbol_data{ // symbol data
    char *var_name;
    int var_value;
};

struct func_data{ // function data
    char *func_name;
    struct node *func_expr;
};


/* ===== define function ===== */
struct node *new_node(struct node *left, struct node *right, node_type type); // create a new node

void traverse(struct node *father); // post order
void clear_tree(struct node *father); // clean the tree

void do_add(struct node *father);
void do_sub(struct node *father);
void do_mul(struct node *father);
void do_div(struct node *father);
void do_mod(struct node *father);
void do_greater(struct node *father);
void do_smaller(struct node *father);
void do_equal(struct node *father);
void do_and(struct node *father);
void do_or(struct node *father);

void store_id(struct node *father);
void bind_param(struct node *father);

/* ===== define variables ===== */
struct node *root; // root node
// store variables, parameters, and functions
extern struct symbol_data var_table[100];
extern struct symbol_data par_table[100];
extern struct func_data func_table[100];
extern int var_table_index;
extern int par_table_index;
extern int func_table_index;
extern int old_index;

extern int cal_result; // caculate result
extern int equ_first_result; // whether the first_result number of equation
extern int equ_expect_num; // number that is going to compare
extern int param_count; // count how many parameter in function
extern int is_func; // whether variable is in function

#endif