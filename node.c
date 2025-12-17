#include "node.h"

struct node *root = NULL; // root node

// store variables, parameters, and functions
struct symbol_data var_table[100];
struct symbol_data par_table[100];
struct func_data func_table[100];
int var_table_index = 0;
int par_table_index = 0;
int func_table_index = 0;
int old_index = 0;

int cal_result = 0; // caculate result
int equ_first_result = 0; // whether the first_result number of equation
int equ_expect_num = 0; // number that is going to compare
int param_count = 0; // count how many parameter in function
int is_func = 0; // whether variable is in function

struct node *new_node(struct node *left, struct node *right, node_type type) {
    struct node *temp_node = (struct node *)malloc(sizeof(struct node));
    temp_node->type = type;
    temp_node->value = 0;
    temp_node->identify = "";
    temp_node->left = left;
    temp_node->right = right;
    temp_node->middle = NULL;
    return temp_node;
}

void clear_tree(struct node *father) { 
    if(father != NULL) {
        clear_tree(father->left);
        clear_tree(father->middle);
        clear_tree(father->right);
        free(father);
    }
}

void traverse(struct node *father) {
    if(father == NULL) return;

    switch (father->type) {
        case NODE_STMT_PRINT_NUM:
            traverse(father->left);
            printf("%d\n", father->left->value);
            //printf("test\n");
            break;

        case NODE_STMT_PRINT_BOOL:
            traverse(father->left);
            if(father->left->value) {
                printf("#t\n");
            } else {
                printf("#f\n");
            }
            break;
        
        case NODE_OP_ADD:
            traverse(father->left);
            traverse(father->right);
            cal_result = 0;
            do_add(father);
            father->value = cal_result;
            break;

        case NODE_OP_MUL:
            traverse(father->left);
            traverse(father->right);
            cal_result = 1;
            do_mul(father);
            father->value = cal_result;
            break;
        
        case NODE_OP_SUB:
            traverse(father->left);
            traverse(father->right);
            do_sub(father);
            father->value = cal_result;
            break;

        case NODE_OP_DIV:
            traverse(father->left);
            traverse(father->right);
            do_div(father);
            father->value = cal_result;
            break;

        case NODE_OP_MOD:
            traverse(father->left);
            traverse(father->right);
            do_mod(father);
            father->value = cal_result;
            break;

        case NODE_OP_GREATER:
            traverse(father->left);
            traverse(father->right);
            do_greater(father);
            father->value = cal_result;
            break;

        case NODE_OP_SMALLER:
            traverse(father->left);
            traverse(father->right);
            do_smaller(father);
            father->value = cal_result;
            break;

        case NODE_OP_EQUAL:
            traverse(father->left);
            traverse(father->right);
            equ_first_result = 1; // first number to compare
            cal_result = 1;
            do_equal(father);
            father->value = cal_result;
            break;

        case NODE_OP_AND:
            traverse(father->left);
            traverse(father->right);
            cal_result = 1;
            do_and(father);
            father->value = cal_result;
            break;

        case NODE_OP_OR:
            traverse(father->left);
            traverse(father->right);
            cal_result = 0;
            do_or(father);
            father->value = cal_result;
            break;
        
        case NODE_OP_NOT:
            traverse(father->left);
            father->value = !father->left->value;
            break;

        case NODE_STMT_IF:
            traverse(father->left); //test_exp
            traverse(father->middle); //then_exp
            traverse(father->right); //else_exp
            if(father->left->value) {
                father->value = father->middle->value; // true -> first scope (then)
            } else {
                father->value = father->right->value; // false -> second scope (else)
            }
            break;

        case NODE_VAR: // call variable
            if(is_func) {
                for(int i = 0; i < par_table_index; ++i){
                    if(strcmp(par_table[i].var_name, father->identify) == 0){
                        father->value = par_table[i].var_value;
                        break;
                    }
                }
            } else {
                for(int i = 0; i < var_table_index; ++i){
                    if(strcmp(var_table[i].var_name, father->identify) == 0){
                        //printf("%s == %s | ", var_table[i].var_name, father->identify);
                        father->value = var_table[i].var_value;
                        break;
                    }
                }
            }
            break;

        case NODE_STMT_DEF:
            if(father->right->type == NODE_FUNC_EXPR) { //define function, left: func_name, right: func_expr
                if(father->right->left->type == NODE_NULL) { // no parameter,so its a variable
                    traverse(father->right->right); // calculate fun_body
                    var_table[var_table_index].var_name = father->left->identify;
                    var_table[var_table_index].var_value = father->right->right->value;
                    var_table_index++;
                } else { // store in fun_map
                    func_table[func_table_index].func_name = father->left->identify; // variable
                    func_table[func_table_index].func_expr = father->right; // fun_exp
                    func_table_index++;
                }
            } else { // define variable, left: var_name, right: var_value
                traverse(father->right);
                var_table[var_table_index].var_name = father->left->identify;
                var_table[var_table_index].var_value = father->right->value;
                var_table_index++;
            }
            break;
        
        case NODE_FUNC_CALL:
            old_index = par_table_index;
            // left: func_expr, right: parameter
            if(father->left->left->type == NODE_NULL){
                ; // no ids or parameters
            } else {
                param_count = 0;
                store_id(father->left->left);
                par_table_index -= param_count;
                bind_param(father->right); 
            }
            is_func = 1;
            traverse(father->left->right);
            father->value = father->left->right->value;
            is_func = 0;
            par_table_index = old_index;
            break;

        case NODE_FUNC_CALL_NAMED: // call by function name
            old_index = par_table_index;
            if(father->right->type == NODE_NULL) { // no parameter
                for(int i = 0; i < var_table_index; ++i) {
                    if(strcmp(var_table[i].var_name, father->left->identify) == 0) {
                        father->value = var_table[i].var_value;
                        break;
                    }
                }
            } else { // bind parameters
                struct node *temp_node = NULL;
                int i;
                for(int i = 0; i < func_table_index; ++i) { // find fun_expr
                    if(strcmp(func_table[i].func_name, father->left->identify) == 0){
                        temp_node = func_table[i].func_expr;
                        break;
                    }
                }
                param_count = 0;
                store_id(temp_node->left); // fun_ids
                par_table_index -= param_count;
                bind_param(father->right); // param
                is_func = 1;
                traverse(temp_node->right); // fun_body
                father->value = temp_node->right->value;
                //free(temp_node);
                is_func = 0;
            }
            par_table_index = old_index;
            break;
        
        default:
            traverse(father->left);
            traverse(father->right);
            break;

    }
}

void do_add(struct node *father) {
    if(father->left != NULL){
        if(father->left->type != NODE_LIST) {
            cal_result = cal_result + father->left->value;
        } else {
            do_add(father->left);
        }
    }

    if(father->right != NULL ){
        if(father->right->type != NODE_LIST) {
            cal_result = cal_result + father->right->value;
        } else {
            do_add(father->right);
        }
    }
}

void do_mul(struct node *father) {
    if(father->left != NULL){
        if(father->left->type != NODE_LIST) {
            cal_result = cal_result * father->left->value;
        } else {
            do_mul(father->left);
        }
    }

    if(father->right != NULL ){
        if(father->right->type != NODE_LIST) {
            cal_result = cal_result * father->right->value;
        } else {
            do_mul(father->right);
        }
    }
}

void do_sub(struct node *father) {
    if(father->left != NULL && father->right != NULL)
        cal_result = father->left->value - father->right->value;
}

void do_div(struct node *father) {
    if(father->left != NULL && father->right != NULL)
        cal_result = father->left->value / father->right->value;
}

void do_mod(struct node *father) {
    if(father->left != NULL && father->right != NULL)
        cal_result = father->left->value % father->right->value;
}

void do_greater(struct node *father) {
    if(father->left != NULL && father->right != NULL) {
        if(father->left->value > father->right->value) {
            cal_result = 1;
        } else {
            cal_result = 0;
        }
    }
}

void do_smaller(struct node *father) {
    if(father->left != NULL && father->right != NULL) {
        if(father->left->value < father->right->value) {
            cal_result = 1;
        } else {
            cal_result = 0;
        }
    }
}

void do_equal(struct node *father) {
    if(father->left != NULL) {
        if(father->left->type != NODE_LIST) {
            if(equ_first_result == 1) { // first_result number
                equ_expect_num = father->left->value;
                equ_first_result = 0;
            } else {
                if(father->left->value != equ_expect_num) cal_result = 0;
            }
        } else {
            do_equal(father->left); // the whole list to compare 
        }
    }

    if(father->right != NULL) {
        if(father->right->type != NODE_LIST) {
            if(equ_first_result == 1) { // first_result number
                equ_expect_num = father->right->value;
                equ_first_result = 0;
            } else {
                if(father->right->value != equ_expect_num) cal_result = 0;
            }
        } else {
            do_equal(father->right); // the whole list to compare 
        }
    }
}

void do_and(struct node *father) {
    if(father->left != NULL) {
        if(father->left->type != NODE_LIST) {
            cal_result = cal_result & father->left->value;
        } else {
            do_and(father->left);
        }
    }

    if(father->right != NULL) {
        if(father->right->type != NODE_LIST) {
            cal_result = cal_result & father->right->value;
        } else {
            do_and(father->right);
        }
    }
}

void do_or(struct node *father) {
    if(father->left != NULL) {
        if(father->left->type != NODE_LIST) {
            cal_result = cal_result | father->left->value;
        } else {
            do_or(father->left);
        }
    }

    if(father->right != NULL) {
        if(father->right->type != NODE_LIST) {
            cal_result = cal_result | father->right->value;
        } else {
            do_or(father->right);
        }
    }
}

void store_id(struct node *father) {
    if(father->left->type == NODE_LIST) store_id(father->left);

    par_table[par_table_index].var_name = father->right->identify;
    par_table_index++;
    param_count++;
}

void bind_param(struct node *father) {
    traverse(father->left);
    par_table[par_table_index].var_value = father->left->value;
    par_table_index++;

    if(father->right->type == NODE_LIST) bind_param(father->right);
}