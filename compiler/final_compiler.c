#include<stdio.h>
#include <stdlib.h>
#include <string.h>
#include "compiler.h"
#define ALLOC(t) (t*) calloc(1, sizeof(t))


struct symbol_table {
	struct ValueNode *id;
	struct symbol_table *next;
};

struct symbol_table *symbols; 

struct StatementNode *target;

char *id;

//---------------------------------------------------------
// Parse

struct StatementNode* parse_body();
struct StatementNode* parse_stmt_list();

struct ValueNode* getVariable(char *token) {
	
	struct symbol_table *temp = symbols;
	
	while(temp!=NULL && strcmp(temp->id->name,token)!=0) {
		temp = temp->next;
	}
	
	return temp->id;
}

void parse_assignment_stmt(struct StatementNode* stmt) {
	stmt->type=ASSIGN_STMT;
	struct AssignmentStatement *assign_stmt = ALLOC(struct AssignmentStatement);
	assign_stmt->op=0;
	
	ttype=getToken();
	assign_stmt->left_hand_side = getVariable(token);
	
	getToken();
	ttype = getToken();
	struct ValueNode *left_operand;
	if(ttype==NUM) {
		left_operand = ALLOC(struct ValueNode);
		left_operand->value = atoi(token);
		left_operand->name = strdup(token);
	} else {
		left_operand = getVariable(token);
	}
	
	
	assign_stmt->operand1 = left_operand;
	
	ttype=getToken();
	if(ttype!=SEMICOLON) {
		
	assign_stmt->op = ttype;
	
	ttype=getToken();
	
	struct ValueNode *right_operand;
	
	if(ttype==NUM) {
		right_operand = ALLOC(struct ValueNode);
		right_operand->value = atoi(token);
		right_operand->name = strdup(token);
	} else {
		right_operand=getVariable(token);
	}
	assign_stmt->operand2 = right_operand;
	ttype=getToken();
	}
	
	stmt->assign_stmt=assign_stmt;
	
}

void parse_print_stmt(struct StatementNode *stmt) {
	stmt->type=PRINT_STMT; 
	struct PrintStatement *print_stmt = ALLOC(struct PrintStatement);
	
	ttype=getToken();
	ttype=getToken();
	
	
	print_stmt->id = getVariable(token);
	stmt->print_stmt=print_stmt;
	getToken();
		
}

void parse_condition(struct IfStatement *if_stmt) {
	ttype=getToken();
	
	struct ValueNode *operand1;
	struct ValueNode *operand2;
	
	
	if(ttype==NUM) {
		operand1 = ALLOC(struct ValueNode);
		operand1->name=token;
		operand1->value = atoi(token);
	} else {
		operand1 = getVariable(token);
	}
	
	ttype=getToken();
	if_stmt->condition_op = ttype;
	
	ttype=getToken();
	
	
	if(ttype==NUM) {
		operand2 = ALLOC(struct ValueNode);
		operand2->name=token;
		operand2->value = atoi(token);
	} else {
		operand2 = getVariable(token);
	}
	
	if_stmt->condition_operand1 = operand1;
	if_stmt->condition_operand2 = operand2;
}


void parse_if_stmt(struct StatementNode *stmt) {
	
	struct IfStatement *if_stmt = ALLOC(struct IfStatement);
	stmt->if_stmt = if_stmt;
	
	ttype = getToken();
	if(ttype==IF) {
		parse_condition(if_stmt);
	}
	
	if_stmt->true_branch = parse_body();
	
	struct StatementNode *no_op = ALLOC(struct StatementNode);
	no_op->type = NOOP_STMT;
	
	
	struct StatementNode *p = if_stmt->true_branch;
	while(p->next!=NULL) {
		p=p->next;
	}
	p->next = no_op;

	if_stmt->false_branch = no_op;
	
	
	stmt->next = no_op;
}

void parse_while_stmt(struct StatementNode *stmt) {
	
	struct IfStatement *if_stmt = ALLOC(struct IfStatement);
	
	ttype = getToken();
	if(ttype==WHILE) {
		parse_condition(if_stmt);
	}
	
	if_stmt->true_branch = parse_body();
	
	struct StatementNode *no_op = ALLOC(struct StatementNode);
	no_op->type = NOOP_STMT;
	
	struct StatementNode *gt_stmt = ALLOC(struct StatementNode);
	struct GotoStatement *gt = ALLOC(struct GotoStatement);
				
	gt_stmt->type = GOTO_STMT;
	gt_stmt->goto_stmt = gt;
	gt->target = stmt;
	
	struct StatementNode *p = if_stmt->true_branch;
	while(p->next!=NULL) {
		p=p->next;
	}
	p->next = gt_stmt;

	if_stmt->false_branch = no_op;
	stmt->if_stmt = if_stmt;
	stmt->next = no_op;
	
} 

struct StatementNode* parse_case(struct StatementNode *stmt) {
	ttype = getToken();
	
	if(ttype==CASE) {
		stmt->type=IF_STMT; 
		struct IfStatement *if_stmt = ALLOC(struct IfStatement);
		ttype=getToken();
		struct ValueNode *num = ALLOC(struct ValueNode);
		num->name=token;
		num->value = atoi(token);
		
		
		if_stmt->condition_operand1 = getVariable(id);
		if_stmt->condition_operand2 = num;
		
		if_stmt->condition_op = NOTEQUAL;
		ttype = getToken();
		
		
		struct StatementNode *no_op = ALLOC(struct StatementNode);
		no_op->type = NOOP_STMT;	
		struct StatementNode *gt_stmt = ALLOC(struct StatementNode);
		struct GotoStatement *gt = ALLOC(struct GotoStatement);
				
		gt_stmt->type = GOTO_STMT;
		gt_stmt->goto_stmt = gt;
	if_stmt->false_branch = parse_body();
	struct StatementNode *p = if_stmt->false_branch;
	while(p->next!=NULL) {
		p=p->next;
	}
	p->next = gt_stmt;

	
	if_stmt->true_branch = no_op;
	stmt->if_stmt = if_stmt;
	stmt->next = no_op;
	} else {
		ttype=getToken();
		stmt=parse_body();
		
	}
	
		return stmt;
		
}

struct StatementNode* parse_case_list(struct StatementNode *stmt) {
	stmt = parse_case(stmt);
	ttype=getToken();
	
	if(ttype!=RBRACE) {
		struct StatementNode *stmt1 = ALLOC(struct StatementNode);
		ungetToken();
		stmt1 = parse_case_list(stmt1);
		stmt->next->next = stmt1;
		
	}
	
	return stmt;
}

void parse_switch_stmt(struct StatementNode *stmt) {
	target = ALLOC(struct StatementNode);
	ttype=getToken();
	if(ttype==SWITCH) {
		ttype=getToken();
		id = strdup(token);
		ttype=getToken(); //LBRACE
		parse_case_list(stmt);
	}
	
	
}


struct symbol_table* parse_id() {
	struct symbol_table *id = ALLOC(struct symbol_table);
	struct ValueNode *val = ALLOC(struct ValueNode);
	id->id = val;
	ttype = getToken();
	val->name = strdup(token);
	return id;
	
}


struct symbol_table* parse_id_list() {
	struct symbol_table *id = parse_id();
	ttype = getToken();
	if(ttype!=SEMICOLON) {
		struct symbol_table *id1 = parse_id_list();
		id->next = id1;
	}
	return id;
}




struct StatementNode* parse_body() { 
	struct StatementNode* stl;
	ttype=getToken();
	
	if(ttype==LBRACE) {
		stl = parse_stmt_list(); 
	} 
	ttype = getToken();
	if(ttype==RBRACE) {
		return stl;
	} 
}

struct StatementNode* parse_stmt() {
	struct StatementNode *stmt = ALLOC(struct StatementNode); 
	
	ttype = getToken();
	switch(ttype) {
			
		case ID:stmt->type=ASSIGN_STMT; 
				ungetToken(); 
				parse_assignment_stmt(stmt);
				break;
		case PRINT : 
				ungetToken(); 
				parse_print_stmt(stmt);
				break;
		case WHILE :stmt->type=IF_STMT; 
				ungetToken(); 
				parse_while_stmt(stmt);
				break;
		case IF : stmt->type=IF_STMT; 
				ungetToken(); 
				parse_if_stmt(stmt);
				break;
		case SWITCH :stmt->type=IF_STMT; 
				ungetToken(); 
				parse_switch_stmt(stmt);
				break;
		
	}
	
	return stmt;
	
}


struct StatementNode* parse_stmt_list() {
	
	struct StatementNode *st,*temp;
	st = parse_stmt(); 
	ttype = getToken();
	if(ttype!=RBRACE) {
		struct StatementNode* st1;
		ungetToken();
		st1 = parse_stmt_list();
		
		temp = st;
		while(temp->next!=NULL) {
			if(temp->type==IF_STMT) {
				struct StatementNode *temp1 = temp->if_stmt->false_branch;
				while(temp1->next!=NULL) {
					temp1 = temp1->next;
				}
				
				if(temp1->type==GOTO_STMT && temp1->goto_stmt->target==NULL) {
					temp1->goto_stmt->target = st1;
				}
			}
			temp = temp->next;
		}
		
		temp = st;
		while(temp->next!=NULL) {
			temp = temp->next;
		}
		temp->next = st1;	
		
	} else {
		ungetToken();
	}
	return st;
}


struct StatementNode* parse_generate_intermediate_representation() {
	symbols = parse_id_list();
	return parse_body();
		
}