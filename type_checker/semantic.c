/*----------------------------------------------------------------------------
Note: the code in this file is not to be shared with anyone or posted online.
(c) Rida Bazzi, 2015
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "syntax.h"

#define TRUE 1
#define FALSE 0


int CURRENT_TYPE = 15;
int symbolTableCount=0;


enum SYMBOL_TYPE
{
INT_TYPE=10, REAL_TYPE, STRING_TYPE, BOOLEAN_TYPE, LONG_TYPE
	
};

enum DECL_TYPE 
{
	IMPLICIT=1, EXPLICIT
	
};

/* ------------------------------------------------------- */
/* -------------------- LEXER SECTION -------------------- */
/* ------------------------------------------------------- */

#define KEYWORDS  11

enum TokenTypes
{
    VAR = 1, WHILE, INT, REAL, STRING, BOOLEAN, 
    TYPE, LONG, DO, CASE, SWITCH,
    PLUS, MINUS, DIV, MULT, EQUAL,
    COLON, COMMA, SEMICOLON,
    LBRAC, RBRAC, LPAREN, RPAREN, LBRACE, RBRACE,
    NOTEQUAL, GREATER, LESS, LTEQ, GTEQ, DOT,
    ID, NUM, REALNUM,
    ERROR
};

char *reserved[] = {"",
    "VAR", "WHILE", "INT", "REAL", "STRING", "BOOLEAN",
    "TYPE", "LONG", "DO", "CASE", "SWITCH",
    "+", "-", "/", "*", "=",
    ":", ",", ";",
    "[", "]", "(", ")", "{", "}",
    "<>", ">", "<", "<=", ">=", ".",
    "ID", "NUM", "REALNUM",
    "ERROR"
};


// Global Variables associated with the next input token
#define MAX_TOKEN_LENGTH 100
char token[MAX_TOKEN_LENGTH]; // token string
int ttype; // token type
int activeToken = FALSE;
int tokenLength;
int line_no = 1;

void skipSpace()
{
    char c;

    c = getchar();
    line_no += (c == '\n');
    while (!feof(stdin) && isspace(c))
    {
        c = getchar();
        line_no += (c == '\n');
    }
    ungetc(c, stdin);
}

int isKeyword(char *s)
{
    int i;

    for (i = 1; i <= KEYWORDS; i++)
        if (strcmp(reserved[i], s) == 0)
            return i;
    return FALSE;
}

/*
 * ungetToken() simply sets a flag so that when getToken() is called
 * the old ttype is returned and the old token is not overwritten.
 * NOTE: BETWEEN ANY TWO SEPARATE CALLS TO ungetToken() THERE MUST BE
 * AT LEAST ONE CALL TO getToken(). CALLING TWO ungetToken() WILL NOT
 * UNGET TWO TOKENS
 */
void ungetToken()
{
    activeToken = TRUE;
}

int scan_number()
{
    char c;

    c = getchar();
    if (isdigit(c))
    {
        // First collect leading digits before dot
        // 0 is a nNUM by itself
        if (c == '0')
        {
            token[tokenLength] = c;
            tokenLength++;
            token[tokenLength] = '\0';
        }
        else
        {
            while (isdigit(c))
            {
                token[tokenLength] = c;
                tokenLength++;
                c = getchar();
            }
            ungetc(c, stdin);
            token[tokenLength] = '\0';
        }
        // Check if leading digits are integer part of a REALNUM
        c = getchar();
        if (c == '.')
        {
            c = getchar();
            if (isdigit(c))
            {
                token[tokenLength] = '.';
                tokenLength++;
                while (isdigit(c))
                {
                    token[tokenLength] = c;
                    tokenLength++;
                    c = getchar();
                }
                token[tokenLength] = '\0';
                if (!feof(stdin))
                    ungetc(c, stdin);
                return REALNUM;
            }
            else
            {
                ungetc(c, stdin);
                c = '.';
                ungetc(c, stdin);
                return NUM;
            }
        }
        else
        {
            ungetc(c, stdin);
            return NUM;
        }
    }
    else
        return ERROR;
}

int scan_id_or_keyword()
{
    int ttype;
    char c;

    c = getchar();
    if (isalpha(c))
    {
        while (isalnum(c))
        {
            token[tokenLength] = c;
            tokenLength++;
            c = getchar();
        }
        if (!feof(stdin))
            ungetc(c, stdin);
        token[tokenLength] = '\0';
        ttype = isKeyword(token);
        if (ttype == 0)
            ttype = ID;
        return ttype;
    }
    else
        return ERROR;
}

int getToken()
{
    char c;

    if (activeToken)
    {
        activeToken = FALSE;
        return ttype;
    }
    skipSpace();
    tokenLength = 0;
    c = getchar();
    switch (c)
    {
        case '.':
            return DOT;
        case '+':
            return PLUS;
        case '-':
            return MINUS;
        case '/':
            return DIV;
        case '*':
            return MULT;
        case '=':
            return EQUAL;
        case ':':
            return COLON;
        case ',':
            return COMMA;
        case ';':
            return SEMICOLON;
        case '[':
            return LBRAC;
        case ']':
            return RBRAC;
        case '(':
            return LPAREN;
        case ')':
            return RPAREN;
        case '{':
            return LBRACE;
        case '}':
            return RBRACE;
        case '<':
            c = getchar();
            if (c == '=')
                return LTEQ;
            else if (c == '>')
                return NOTEQUAL;
            else
            {
                ungetc(c, stdin);
                return LESS;
            }
        case '>':
            c = getchar();
            if (c == '=')
                return GTEQ;
            else
            {
                ungetc(c, stdin);
                return GREATER;
            }
        default:
            if (isdigit(c))
            {
                ungetc(c, stdin);
                return scan_number();
            }
            else if (isalpha(c))
            {
                ungetc(c, stdin);
                return scan_id_or_keyword();
            }
            else if (c == EOF)
                return EOF;
            else
                return ERROR;
    }
}

/* ----------------------------------------------------------------- */
/* -------------------- SYNTAX ANALYSIS SECTION -------------------- */
/* ----------------------------------------------------------------- */

void syntax_error(const char* msg)
{
    printf("Syntax error while parsing %s line %d\n", msg, line_no);
    exit(1);
}



/* ------------------------------------------------------- */
/* -------------------- SYMBOL TABLE -------------------- */
/* ------------------------------------------------------- */
struct symbol_table_row
{
	char* name;
	int kind;
	int type_number;
	int decl_type; 
	int marked;
	
};

struct symbol_table_row* symbolTable[500];



int checkSymbolTableForIdList(char* ch) {
	int i=0;
	struct symbol_table_row* row;
	char* c;
	while(i<symbolTableCount) {
		row = symbolTable[i];		
		c = row->name;
		if(strcmp(ch,c)==0) {
			return TRUE;
		}
		
		i++;
	}
	
	return FALSE;
}


int checkSymbolTableForVarList(char* ch) {
	int i=0;
	struct symbol_table_row* row;
	char* c;
	while(i<symbolTableCount) {
		row = symbolTable[i];		
		c = row->name;
		if(strcmp(ch,c)==0) {
			if(row->kind==VAR) {
				printf("ERROR CODE 2\n");
				exit(1);
			} else {
				printf("ERROR CODE 1\n");
				exit(1);
			}
		}
		
		i++;
	}
	
	return FALSE;
} 


void addToSymbolTable(char* c, int ty, int typeNo, int decl_type) {
	struct symbol_table_row* row = ALLOC(struct symbol_table_row);
	row->name=c;
	row->kind=ty;
	row->type_number = typeNo;
	row->decl_type = decl_type;
	symbolTable[symbolTableCount++] = row;
}

int getTypeFromTable(char* ch) {
	int i=0;
	struct symbol_table_row* row;
	char* c;
	while(i<symbolTableCount) {
		row = symbolTable[i];		
		c = row->name;
		if(strcmp(ch,c)==0) {
			if(row->kind==TYPE)
				return row->type_number;
			else {
				printf("ERROR CODE 4\n");
				exit(1);
			}
				
		}
		
		i++;
	}
	addToSymbolTable(ch, TYPE, CURRENT_TYPE, IMPLICIT);
	CURRENT_TYPE++;
	row = symbolTable[i];
	return row->type_number;
} 

int getOnlyTypeFromTable(char* ch) {
	int i=0;
	struct symbol_table_row* row;
	char* c;
	while(i<symbolTableCount) {
		row = symbolTable[i];		
		c = row->name;
		if(strcmp(ch,c)==0) {
			if(row->kind==VAR)
				return row->type_number;
			else {
				printf("ERROR CODE 1\n");
				exit(1);
			}
		}
		
		i++;
	}
	addToSymbolTable(ch, VAR, CURRENT_TYPE, IMPLICIT);
	CURRENT_TYPE++;
	row = symbolTable[i];
	return row->type_number;
}

int getConditionTypeFromTable(char* ch) {
	int i=0;
	struct symbol_table_row* row;
	char* c;
	while(i<symbolTableCount) {
		row = symbolTable[i];		
		c = row->name;
		if(strcmp(ch,c)==0) {
			if(row->kind==VAR)
				return row->type_number;
			else {
				printf("ERROR CODE 1\n");
				exit(1);
			}
		}
		
		i++;
	}
	addToSymbolTable(ch, VAR, BOOLEAN_TYPE, IMPLICIT);
	row = symbolTable[i];
	return row->type_number;
}

int getSwitchTypeFromTable(char* ch) {
	int i=0;
	struct symbol_table_row* row;
	char* c;
	while(i<symbolTableCount) {
		row = symbolTable[i];		
		c = row->name;
		if(strcmp(ch,c)==0) {
			if(row->kind==VAR)
				return row->type_number;
			else {
				printf("ERROR CODE 1\n");
				exit(1);
			}
		}
		
		i++;
	}
	addToSymbolTable(ch, VAR, INT_TYPE, IMPLICIT);
	row = symbolTable[i];
	return row->type_number;
	
	
}

int getIntTypeFromTable(char* ch) {
	int i=0;
	struct symbol_table_row* row;
	char* c;
	while(i<symbolTableCount) {
		row = symbolTable[i];		
		c = row->name;
		if(strcmp(ch,c)==0) {
			if(row->kind==VAR)
				return row->type_number;
			else {
				syntax_error("ERROR CODE 1\n");
				exit(1);
			}
		}
		
		i++;
	}
	addToSymbolTable(ch, VAR, INT_TYPE, IMPLICIT);
	row = symbolTable[i];
	return row->type_number;
}

int getRealTypeFromTable(char* ch) {
	int i=0;
	struct symbol_table_row* row;
	char* c;
	while(i<symbolTableCount) {
		row = symbolTable[i];		
		c = row->name;
		if(strcmp(ch,c)==0) {
			if(row->kind==VAR)
				return row->type_number;
			else {
				printf("ERROR CODE 1\n");
				exit(1);
			}
		}
		
		i++;
	}
	addToSymbolTable(ch, VAR, REAL_TYPE, IMPLICIT);
	row = symbolTable[i];
	return row->type_number;
}


void updateTypeInSymbolTable(int src, int dest) {
	int i=0;
	struct symbol_table_row* row;
	int type;
	while(i<symbolTableCount) {
		row = symbolTable[i];
		type=row->type_number;
		if(row->type_number==dest) {
			row->type_number= src;
		}
		i++;
	}
}





/* ----------------------------------------------------------------- */
/* -------------------- TYPES ANALYSIS SECTION -------------------- */
/* ----------------------------------------------------------------- */


int getType(struct type_nameNode* typeNameNode) {
	switch(typeNameNode->type) {
		case INT : return INT_TYPE;
		case STRING : return STRING_TYPE;
		case REAL : return REAL_TYPE;
		case BOOLEAN : return BOOLEAN_TYPE;
		case LONG : return LONG_TYPE;
		case ID : return getTypeFromTable(typeNameNode->id);
	}
	
	return 0;
}


void applyConstraintTwoAndLoadTypes(struct type_declNode* type_decl) {
	struct id_listNode* idList = type_decl->id_list;
	int type=getType(type_decl->type_name);
	
	while(idList!=NULL) {
		char* ch = idList->id;
		if(!checkSymbolTableForIdList(ch)) {
			addToSymbolTable(ch,TYPE,type,EXPLICIT);
		} else {
			printf("ERROR CODE 0\n");
			exit(1);
		}
		idList = idList->id_list;
	}
	
}

/*--------------------VAR Analysis Section--------------------------*/


void applyConstraintAndLoadVar(struct var_declNode* var_decl) {
	struct id_listNode* idList = var_decl->id_list;
	int type = getType(var_decl->type_name);
	
	while(idList!=NULL) {
		char* ch = idList->id;
		if(!checkSymbolTableForVarList(ch)) {
			addToSymbolTable(ch,VAR,type,EXPLICIT);	
		}
		idList = idList->id_list;
	}
}


/*---------------------Assignment stmt analysis Section--------------------------*/

int applyExpressionConstraints(struct exprNode* exprNode) {
	int currentType, exprType, destType;
	if(exprNode->tag==PRIMARY) {
		switch(exprNode->primary->tag) {
			case NUM : return INT_TYPE;
				
			case REALNUM : return REAL_TYPE;
				
			case ID : return getOnlyTypeFromTable(exprNode->primary->id);
				
		}
		
	  } //else {
		// if(exprNode->leftOperand->tag!=PRIMARY) {
			currentType = applyExpressionConstraints(exprNode->leftOperand);
		// } else {
			// switch(exprNode->leftOperand->primary->tag) {
				// case NUM : currentType= INT_TYPE;
					// break;
				// case REALNUM : currentType=REAL_TYPE;
					// break;
				// case ID : currentType = getOnlyTypeFromTable(exprNode->leftOperand->primary->id);
				
			// }
		
			
		// }
		exprType=applyExpressionConstraints(exprNode->rightOperand);
	// }
	
	if(currentType!=exprType) {
		if(currentType<15 && exprType<15) {
			printf("ERROR CODE 3\n");
			exit(1);
		} else if((currentType>=15 && exprType<15) || (currentType<15 && exprType>=15)) {
			if(currentType>=15) {
				updateTypeInSymbolTable(exprType,currentType);
				destType = exprType;
			} else {
				updateTypeInSymbolTable(currentType,exprType);
				destType = currentType;
			}
		} else if(currentType>=15 && exprType>=15) {
			if(currentType<exprType) {
				updateTypeInSymbolTable(currentType,exprType);
				destType = currentType;
			} else {
				updateTypeInSymbolTable(exprType,currentType);
				destType = exprType;
			}
		}
	} else {
		destType=currentType;
	}
	
	return destType;
}

void applyAssignmentConstraints(struct assign_stmtNode* assign_stmt) {
	char* c = assign_stmt->id;
	int idType = getOnlyTypeFromTable(c);
	int exprType = applyExpressionConstraints(assign_stmt->expr);
	
	if(idType!=exprType) {
		if(idType<15 && exprType<15) {
			printf("ERROR CODE 3\n");
			exit(1);
		} else if((idType>=15 && exprType<15) || (idType<15 && exprType>=15)) {
			if(idType>=15) {
				updateTypeInSymbolTable(exprType,idType);
				
			} else {
				updateTypeInSymbolTable(idType,exprType);
				
			}
		} else if(idType>=15 && exprType>=15) {
			
				updateTypeInSymbolTable(exprType,idType);
				
			}
		}
	}
	



/*---------------------Switch Condition Check -------------------*/

void checkSwitchIdIsInt(char* c) {
	int idType = getSwitchTypeFromTable(c);
	if(idType!=INT_TYPE) {
		printf("ERROR CODE 3\n");
		exit(1);
	}
	
}



/*---------------------Loop Condition Check -------------------*/

void checkConditionConstraints(struct conditionNode* condition) {
	
	int relop = condition->relop;
	int leftOperandTag = condition->left_operand->tag;
	if(relop==0) {
		if(leftOperandTag!=ID) {
			printf("ERROR CODE 3\n");
			exit(1);
		} else {
			char* id = condition->left_operand->id;
			int idType = getConditionTypeFromTable(id);
			if(idType< 15) {
				if(idType!=BOOLEAN_TYPE) {
					printf("ERROR CODE 3\n");
					exit(1);
				}
			} else {
				updateTypeInSymbolTable(BOOLEAN_TYPE, idType);	
			}
		}
	} else {
		int rightOperandTag = condition->right_operand->tag;
		if(leftOperandTag==INT) {
			if(rightOperandTag==REAL) {
				printf("ERROR CODE 3");
				exit(1);
			} else if(rightOperandTag==ID){
				int rightOperandIdType = getIntTypeFromTable(condition->right_operand->id);
				if(rightOperandIdType!=INT_TYPE) {
					printf("ERROR CODE 3\n");
					exit(1);
				}
			}
		} else if(leftOperandTag==REAL) {
			if(rightOperandTag==INT) {
				printf("ERROR CODE 3\n");
				exit(1);
			} else if(rightOperandTag==ID){
				int rightOperandIdType = getRealTypeFromTable(condition->right_operand->id);
				if(rightOperandIdType!=REAL_TYPE) {
					printf("ERROR CODE 3\n");
					exit(1);
				}
			}
			
		} else {
			int leftOperandIdType;
			if(rightOperandTag==INT) {
				leftOperandIdType = getIntTypeFromTable(condition->right_operand->id);
				if(leftOperandIdType!=INT_TYPE) {
					printf("ERROR CODE 3\n");
					exit(1);
				}
			} else if(rightOperandTag==ID){
				leftOperandIdType = getOnlyTypeFromTable(condition->left_operand->id);
				int rightOperandIdType = getOnlyTypeFromTable(condition->right_operand->id);
				
				if(leftOperandIdType!=rightOperandIdType) {
					if(leftOperandIdType<15 && rightOperandIdType<15) {
					printf("ERROR CODE 3\n");
					exit(1);
					} else if((leftOperandIdType>=15 && rightOperandIdType<15) || (leftOperandIdType<15 && rightOperandIdType>=15)) {
						if(leftOperandIdType>=15) {
							updateTypeInSymbolTable(rightOperandIdType,leftOperandIdType);
				
						} else {
							updateTypeInSymbolTable(leftOperandIdType,rightOperandIdType);
				
						}
					} else if(leftOperandIdType>=15 && rightOperandIdType>=15) {
						if(leftOperandIdType<rightOperandIdType) {
							updateTypeInSymbolTable(leftOperandIdType,rightOperandIdType);
						} else {
							updateTypeInSymbolTable(rightOperandIdType,leftOperandIdType);
				
						}
					}
				}
				
			} else {
				leftOperandIdType = getRealTypeFromTable(condition->right_operand->id);
				if(leftOperandIdType != REAL_TYPE) {
					printf("ERROR CODE 3\n");
					exit(1);
				}
			}
			
		}		
		
	}
	
	
}



/* -------------------- PRINTING PARSE TREE -------------------- */
void print_parse_tree(struct programNode* program)
{
    print_decl(program->decl);
    print_body(program->body);
}

void print_decl(struct declNode* dec)
{
    if (dec->type_decl_section != NULL)
    {
        print_type_decl_section(dec->type_decl_section);
    }
    if (dec->var_decl_section != NULL)
    {
        print_var_decl_section(dec->var_decl_section);
    }
}

void print_body(struct bodyNode* body)
{
    printf("{\n");
    print_stmt_list(body->stmt_list);
    printf("}\n");
}

void print_var_decl_section(struct var_decl_sectionNode* varDeclSection)
{
    printf("VAR\n");
    if (varDeclSection->var_decl_list != NULL)
        print_var_decl_list(varDeclSection->var_decl_list);
}

void print_var_decl_list(struct var_decl_listNode* varDeclList)
{
    print_var_decl(varDeclList->var_decl);
    if (varDeclList->var_decl_list != NULL)
        print_var_decl_list(varDeclList->var_decl_list);
}

void print_var_decl(struct var_declNode* varDecl)
{
    print_id_list(varDecl->id_list);
    printf(": ");
    print_type_name(varDecl->type_name);
    printf(";\n");
}

void print_type_decl_section(struct type_decl_sectionNode* typeDeclSection)
{
    printf("TYPE\n");
    if (typeDeclSection->type_decl_list != NULL)
        print_type_decl_list(typeDeclSection->type_decl_list);
}

void print_type_decl_list(struct type_decl_listNode* typeDeclList)
{
    print_type_decl(typeDeclList->type_decl);
    if (typeDeclList->type_decl_list != NULL)
        print_type_decl_list(typeDeclList->type_decl_list);
}

void print_type_decl(struct type_declNode* typeDecl)
{
    print_id_list(typeDecl->id_list);
    printf(": ");
    print_type_name(typeDecl->type_name);
    printf(";\n");
}

void print_type_name(struct type_nameNode* typeName)
{
    if (typeName->type != ID)
        printf("%s ", reserved[typeName->type]);
    else
        printf("%s ", typeName->id);
}

void print_id_list(struct id_listNode* idList)
{
    printf("%s ", idList->id);
    if (idList->id_list != NULL)
    {
        printf(", ");
        print_id_list(idList->id_list);
    }
}

void print_stmt_list(struct stmt_listNode* stmt_list)
{
    print_stmt(stmt_list->stmt);
    if (stmt_list->stmt_list != NULL)
        print_stmt_list(stmt_list->stmt_list);

}

void print_assign_stmt(struct assign_stmtNode* assign_stmt)
{
    printf("%s ", assign_stmt->id);
    printf("= ");
    print_expression_prefix(assign_stmt->expr);
    printf("; \n");
}

void print_stmt(struct stmtNode* stmt)
{
    switch (stmt->stmtType)
    {
        case ASSIGN:
            print_assign_stmt(stmt->assign_stmt);
            break;
        case WHILE:
            print_while_stmt(stmt->while_stmt);
            break;
        case DO:
            print_do_stmt(stmt->while_stmt);
            break;
        case SWITCH:
            print_switch_stmt(stmt->switch_stmt);
            break;
    }
}

void print_expression_prefix(struct exprNode* expr)
{
    if (expr->tag == EXPR)
    {
        printf("%s ", reserved[expr->op]);
        print_expression_prefix(expr->leftOperand);
        print_expression_prefix(expr->rightOperand);
    }
    else if (expr->tag == PRIMARY)
    {
        if (expr->primary->tag == ID)
            printf("%s ", expr->primary->id);
        else if (expr->primary->tag == NUM)
            printf("%d ", expr->primary->ival);
        else if (expr->primary->tag == REALNUM)
            printf("%.4f ", expr->primary->fval);
    }
}

void print_while_stmt(struct while_stmtNode* while_stmt)
{
    printf("WHILE ");
	print_condition(while_stmt->condition);
	print_body(while_stmt->body);
	printf("\n");
}

void print_do_stmt(struct while_stmtNode* do_stmt)
{
	printf("DO \n");
    print_body(do_stmt->body);
	printf("while ");
	print_condition(do_stmt->condition);
	printf(";\n");
	
}

void print_primary(struct primaryNode* primary_node) {
	
	switch(primary_node->tag) {
			case ID: printf("%s", primary_node->id);
				break;
			case NUM: printf("%d", primary_node->ival);
				break;
			case REALNUM : printf("%f", primary_node->fval);
				break;
	}
	
}


void print_condition(struct conditionNode* condition)
{
		
		if(condition->relop!=0) {
			printf("%s ", reserved[condition->relop]);
			print_primary(condition->left_operand);
			print_primary(condition->right_operand);
		} else {
			print_primary(condition->left_operand);
		}
}


void print_case(struct caseNode* cas)
{
    printf(" CASE %d: \n", cas->num);
	print_body(cas->body);
	
}

void print_case_list(struct case_listNode* case_list)
{
    print_case(case_list->cas);
	if(case_list->case_list!=NULL) {
		print_case_list(case_list->case_list);
	}
}

void print_switch_stmt(struct switch_stmtNode* switc)
{
    printf("SWITCH %s \n", switc->id);
	printf("{\n");
	print_case_list(switc->case_list);
	printf("}\n");
}

void print_symbol_table() {
	int i,j;
	
	for(i=10;i<CURRENT_TYPE;i++) {
			char* output[500];
			int count=0;
			for(j=0;j<symbolTableCount;j++) {
				struct symbol_table_row* row = symbolTable[j];
				if(row->marked==FALSE && row->type_number==i && row->kind==TYPE && row->decl_type==EXPLICIT) {
					output[count++] = strdup(row->name); 	
					row->marked = TRUE;
				}
			}
			
			for(j=0;j<symbolTableCount;j++) {
				struct symbol_table_row* row = symbolTable[j];
				if(row->marked==FALSE && row->type_number==i && row->kind==TYPE && row->decl_type==IMPLICIT) {
					output[count++] = strdup(row->name); 	
					row->marked = TRUE;
				}
			}
			
			for(j=0;j<symbolTableCount;j++) {
				struct symbol_table_row* row = symbolTable[j];
				if(row->marked==FALSE && row->type_number==i && row->kind==VAR && row->decl_type==EXPLICIT) {
					output[count++] = strdup(row->name); 	
					row->marked = TRUE;
				}
			}
			
			for(j=0;j<symbolTableCount;j++) {
				struct symbol_table_row* row = symbolTable[j];
				if(row->marked==FALSE && row->type_number==i && row->kind==VAR && row->decl_type==IMPLICIT) {
					output[count++] = strdup(row->name); 	
					row->marked = TRUE;
				}
			}
			
			if(count!=0) { 
			int k;
			char* type;
			j=0;
				switch(i) {
					case INT_TYPE: type = "INT";
						break;
					case REAL_TYPE: type = "REAL";
						break;
					case STRING_TYPE: type = "STRING";
						break;
					case BOOLEAN_TYPE: type = "BOOLEAN";
						break;
					case LONG_TYPE: type = "LONG";
						break;
					default : type = output[j++];
						break;
				}
			
				printf("%s : %s", type, output[j++]);
				for(k=j;k<count;k++) {
					printf(" %s",output[k]);
				}
				printf(" #\n");
				
			} else {
				
			}
	}
}

/* -------------------- PARSING AND BUILDING PARSE TREE -------------------- */

// Note that the following function is not
// called case because case is a keyword in C/C++
struct caseNode* cas()
{
	struct caseNode* casNode = ALLOC(struct caseNode);
	
    ttype=getToken();
	if(ttype==CASE) {
		ttype=getToken();
		if(ttype==NUM) {
			casNode->num = atoi(token);
			ttype=getToken();
			if(ttype==COLON) {
				casNode->body=body();
			} else {
				syntax_error("cas. COLON expected");
			}
		} else {
			syntax_error("cas. NUM expected");
		}
	} else {
		syntax_error("cas. CASE expected");
	}
    return casNode;
}

struct case_listNode* case_list()
{
    struct case_listNode* caseList = ALLOC(struct case_listNode);
	caseList->cas = cas();
	ttype=getToken();
	if(ttype==RBRACE) {
		ungetToken();
		
	} else if(ttype==CASE) {
		ungetToken();
		caseList->case_list = case_list();
	} else {
		syntax_error("case_list. RBRACE or CASE expected");
	}
	
    return caseList;
}

struct switch_stmtNode* switch_stmt()
{
    struct switch_stmtNode* switchStmt = ALLOC(struct switch_stmtNode);
	ttype=getToken();
	if(ttype==SWITCH) {
		ttype=getToken();
		if(ttype==ID) {
			switchStmt->id = (char*) malloc(tokenLength*sizeof(char));
			strcpy(switchStmt->id, token);
			checkSwitchIdIsInt(switchStmt->id);
			ttype=getToken();
			if(ttype==LBRACE) {
				ttype=getToken();
				if(ttype==CASE) {
					ungetToken();
					switchStmt->case_list = case_list();
					ttype=getToken();
					if(ttype==RBRACE) {
						return switchStmt;
					} else {
						syntax_error("switch_stmt. RBRACE expected");
					}
				} else {
					syntax_error("switch_stmt. CASE expected");
				}	
			} else {
				syntax_error("switch_stmt. LBRACE expected");
			}
			
		} else {
			syntax_error("switch_stmt. ID expected");
		}
	} else {
		syntax_error("switch_stmt. SWITCH expected");
	}
	
	
    return NULL;
}

struct while_stmtNode* do_stmt()
{
    struct while_stmtNode* do_stmt = ALLOC(struct while_stmtNode);
	ttype=getToken();
	if(ttype == DO) {
		do_stmt->body=body();
		ttype=getToken();
		
		if(ttype==WHILE) {
			do_stmt->condition=condition();
		} else {
			syntax_error("do_stmt. WHILE expected");
		}
		
		ttype=getToken();
		
		if(ttype==SEMICOLON) {
			return do_stmt;
		} else {
			syntax_error("do_stmt. SEMICOLON expected");
		}
			
	} else {
		syntax_error("do_stmt. DO expected");
	}
	
    return NULL;
}

struct primaryNode* primary()
{
    struct primaryNode* primaryNode = ALLOC(struct primaryNode);
	ttype = getToken();
	primaryNode->tag = ttype;
	if(ttype==ID) {
		primaryNode->id = (char*) malloc(tokenLength*sizeof(char));
		primaryNode->id = strcpy(primaryNode->id, token);
	} else if(ttype==NUM) {
		primaryNode->ival= atoi(token);
	} else{ 
		primaryNode->fval = atoi(token);
	}
    return primaryNode;
}

struct conditionNode* condition()
{
    struct conditionNode* condition = ALLOC(struct conditionNode);
	ttype=getToken();
	
	if(ttype==ID || ttype==NUM || ttype==REALNUM ) {
		ungetToken();
		condition->left_operand = primary();
		ttype=getToken();
		if(ttype==SEMICOLON || ttype==LBRACE) {
			ungetToken();
			
		} else if(ttype==GREATER || ttype==GTEQ || ttype==LESS || ttype==NOTEQUAL || ttype==LTEQ) {
			condition->relop = ttype;
			ttype=getToken();
			if(ttype==ID || ttype==NUM || ttype==REALNUM ) {
				ungetToken();
				condition->right_operand = primary();
			
			} else {
				syntax_error("condition. ID, NUM, REALNUM expected");
			}
		}
	} else {
		syntax_error("condition. ID, NUM or REALNUM expected");
	}
	
	checkConditionConstraints(condition);
    return condition;
}

struct while_stmtNode* while_stmt()
{
    struct while_stmtNode* whileStmt = ALLOC(struct while_stmtNode);
	
	ttype=getToken();
	if(ttype==WHILE) {
		whileStmt->condition = condition();
		
		whileStmt->body = body();
	} else {
		syntax_error("while_stmt. WHILE expected");
	}
    return whileStmt;
}

struct exprNode* factor()
{
    struct exprNode* facto;

    ttype = getToken();
    if (ttype == LPAREN)
    {
        facto = expr();
        ttype = getToken();
        if (ttype == RPAREN)
            return facto;
        else
            syntax_error("factor. RPAREN expected");
    }
    else if (ttype == NUM)
    {
        facto = ALLOC(struct exprNode);
        facto->primary = ALLOC(struct primaryNode);
        facto->tag = PRIMARY;
        facto->op = NOOP;
        facto->leftOperand = NULL;
        facto->rightOperand = NULL;
        facto->primary->tag = NUM;
        facto->primary->ival = atoi(token);
        return facto;
    }
    else if (ttype == REALNUM)
    {
        facto = ALLOC(struct exprNode);
        facto->primary = ALLOC(struct primaryNode);
        facto->tag = PRIMARY;
        facto->op = NOOP;
        facto->leftOperand = NULL;
        facto->rightOperand = NULL;
        facto->primary->tag = REALNUM;
        facto->primary->fval = atof(token);
        return facto;
    }
    else if (ttype == ID)
    {
        facto = ALLOC(struct exprNode);
        facto->primary = ALLOC(struct primaryNode);
        facto->tag = PRIMARY;
        facto->op = NOOP;
        facto->leftOperand = NULL;
        facto->rightOperand = NULL;
        facto->primary->tag = ID;
        facto->primary->id = strdup(token);
        return facto;
    }
    else
        syntax_error("factor. NUM, REALNUM, or ID, expected");
    return NULL; // control never reaches here, this is just for the sake of GCC
}

struct exprNode* term()
{
    struct exprNode* ter;
    struct exprNode* f;

    ttype = getToken();
    if (ttype == ID || ttype == LPAREN || ttype == NUM || ttype == REALNUM)
    {
        ungetToken();
        f = factor();
        ttype = getToken();
        if (ttype == MULT || ttype == DIV)
        {
            ter = ALLOC(struct exprNode);
            ter->op = ttype;
            ter->leftOperand = f;
            ter->rightOperand = term();
            ter->tag = EXPR;
            ter->primary = NULL;
            return ter;
        }
        else if (ttype == SEMICOLON || ttype == PLUS ||
                 ttype == MINUS || ttype == RPAREN)
        {
            ungetToken();
            return f;
        }
        else
            syntax_error("term. MULT or DIV expected");
    }
    else
        syntax_error("term. ID, LPAREN, NUM, or REALNUM expected");
    return NULL; // control never reaches here, this is just for the sake of GCC
}

struct exprNode* expr()
{
    struct exprNode* exp;
    struct exprNode* t;

    ttype = getToken();
    if (ttype == ID || ttype == LPAREN || ttype == NUM || ttype == REALNUM)
    {
        ungetToken();
        t = term();
        ttype = getToken();
        if (ttype == PLUS || ttype == MINUS)
        {
            exp = ALLOC(struct exprNode);
            exp->op = ttype;
            exp->leftOperand = t;
            exp->rightOperand = expr();
            exp->tag = EXPR;
            exp->primary = NULL;
            return exp;
        }
        else if (ttype == SEMICOLON || ttype == MULT ||
                 ttype == DIV || ttype == RPAREN)
        {
            ungetToken();
            return t;
        }
        else
            syntax_error("expr. PLUS, MINUS, or SEMICOLON expected");
    }
    else
        syntax_error("expr. ID, LPAREN, NUM, or REALNUM expected");
    return NULL; // control never reaches here, this is just for the sake of GCC
}

struct assign_stmtNode* assign_stmt()
{
    struct assign_stmtNode* assignStmt;

    ttype = getToken();
    if (ttype == ID)
    {
        assignStmt = ALLOC(struct assign_stmtNode);
        assignStmt->id = strdup(token);
        ttype = getToken();
        if (ttype == EQUAL)
        {
            assignStmt->expr = expr();
            ttype = getToken();
            if (ttype == SEMICOLON) {
				applyAssignmentConstraints(assignStmt);
                return assignStmt;
            } else
                syntax_error("asign_stmt. SEMICOLON expected");
        }
        else
            syntax_error("assign_stmt. EQUAL expected");
    }
    else
        syntax_error("assign_stmt. ID expected");
    return NULL; // control never reaches here, this is just for the sake of GCC
}

struct stmtNode* stmt()
{
    struct stmtNode* stm;

    stm = ALLOC(struct stmtNode);
    ttype = getToken();
    if (ttype == ID) // assign_stmt
    {
        ungetToken();
        stm->assign_stmt = assign_stmt();
        stm->stmtType = ASSIGN;
    }
    else if (ttype == WHILE) // while_stmt
    {
        ungetToken();
        stm->while_stmt = while_stmt();
        stm->stmtType = WHILE;
    }
    else if (ttype == DO)  // do_stmt
    {
        ungetToken();
        stm->while_stmt = do_stmt();
        stm->stmtType = DO;
    }
    else if (ttype == SWITCH) // switch_stmt
    {
        ungetToken();
        stm->switch_stmt = switch_stmt();
        stm->stmtType = SWITCH;
    }
    else
        syntax_error("stmt. ID, WHILE, DO or SWITCH expected");
    return stm;
}

struct stmt_listNode* stmt_list()
{
    struct stmt_listNode* stmtList;

    ttype = getToken();
    if (ttype == ID || ttype == WHILE ||
        ttype == DO || ttype == SWITCH)
    {
        ungetToken();
        stmtList = ALLOC(struct stmt_listNode);
        stmtList->stmt = stmt();
        ttype = getToken();
        if (ttype == ID || ttype == WHILE ||
            ttype == DO || ttype == SWITCH)
        {
            ungetToken();
            stmtList->stmt_list = stmt_list();
            return stmtList;
        }
        else // If the next token is not in FOLLOW(stmt_list),
             // let the caller handle it.
        {
            ungetToken();
            stmtList->stmt_list = NULL;
            return stmtList;
        }
    }
    else
        syntax_error("stmt_list. ID, WHILE, DO or SWITCH expected");
    return NULL; // control never reaches here, this is just for the sake of GCC
}

struct bodyNode* body()
{
    struct bodyNode* bod;

    ttype = getToken();
    if (ttype == LBRACE)
    {
        bod = ALLOC(struct bodyNode);
        bod->stmt_list = stmt_list();
        ttype = getToken();
        if (ttype == RBRACE)
            return bod;
        else
            syntax_error("body. RBRACE expected");
    }
    else
        syntax_error("body. LBRACE expected");
    return NULL; // control never reaches here, this is just for the sake of GCC
}

struct type_nameNode* type_name()
{
    struct type_nameNode* tName;

    tName = ALLOC(struct type_nameNode);
    ttype = getToken();
    if (ttype == ID || ttype == INT || ttype == REAL ||
            ttype == STRING || ttype == BOOLEAN || ttype == LONG)
    {
        tName->type = ttype;
        if (ttype == ID)
            tName->id = strdup(token);
        else
            tName->id = NULL;
        return tName;
    }
    else
        syntax_error("type_name. type name expected");
    return NULL; // control never reaches here, this is just for the sake of GCC
}

struct id_listNode* id_list()
{
    struct id_listNode* idList;

    idList = ALLOC(struct id_listNode);
    ttype = getToken();
    if (ttype == ID)
    {
        idList->id = strdup(token);
        ttype = getToken();
        if (ttype == COMMA)
        {
            idList->id_list = id_list();
            return idList;
        }
        else if (ttype == COLON)
        {
            ungetToken();
            idList->id_list = NULL;
            return idList;
        }
        else
            syntax_error("id_list. COMMA or COLON expected");
    }
    else
        syntax_error("id_list. ID expected");
    return NULL; // control never reaches here, this is just for the sake of GCC
}

struct type_declNode* type_decl()
{
    struct type_declNode* typeDecl;

    typeDecl = ALLOC(struct type_declNode);
    ttype = getToken();
    if (ttype == ID)
    {
        ungetToken();
        typeDecl->id_list = id_list();
        ttype = getToken();
        if (ttype == COLON)
        {
            typeDecl->type_name = type_name();
            ttype = getToken();
            if (ttype == SEMICOLON) {
				applyConstraintTwoAndLoadTypes(typeDecl);
                return typeDecl;
			} else {
                syntax_error("type_decl. SEMICOLON expected");
			}
        } else {
            syntax_error("type_decl. COLON expected");
		}
    } else {
        syntax_error("type_decl. ID expected");
	}
    return NULL; // control never reaches here, this is just for the sake of GCC
}

struct var_declNode* var_decl()
{
    struct var_declNode* varDecl;

    varDecl = ALLOC(struct var_declNode);
    ttype = getToken();
    if (ttype == ID)
    {
        ungetToken();
        varDecl->id_list = id_list();
        ttype = getToken();
        if (ttype == COLON)
        {
            varDecl->type_name = type_name();
            ttype = getToken();
            if (ttype == SEMICOLON) {
				applyConstraintAndLoadVar(varDecl);
                return varDecl;
            } else
                syntax_error("var_decl. SEMICOLON expected");
        }
        else
            syntax_error("var_decl. COLON expected");
    }
    else
        syntax_error("var_decl. ID expected");
    return NULL; // control never reaches here, this is just for the sake of GCC
}

struct var_decl_listNode* var_decl_list()
{
    struct var_decl_listNode* varDeclList;

    varDeclList = ALLOC(struct var_decl_listNode);
    ttype = getToken();
    if (ttype == ID)
    {
        ungetToken();
        varDeclList->var_decl = var_decl();
        ttype = getToken();
        if (ttype == ID)
        {
            ungetToken();
            varDeclList->var_decl_list = var_decl_list();
            return varDeclList;
        }
        else
        {
            ungetToken();
            varDeclList->var_decl_list = NULL;
            return varDeclList;
        }
    }
    else
        syntax_error("var_decl_list. ID expected");
    return NULL; // control never reaches here, this is just for the sake of GCC
}

struct type_decl_listNode* type_decl_list()
{
    struct type_decl_listNode* typeDeclList;

    typeDeclList = ALLOC(struct type_decl_listNode);
    ttype = getToken();
    if (ttype == ID)
    {
        ungetToken();
        typeDeclList->type_decl = type_decl();
        ttype = getToken();
        if (ttype == ID)
        {
            ungetToken();
            typeDeclList->type_decl_list = type_decl_list();
            return typeDeclList;
        }
        else
        {
            ungetToken();
            typeDeclList->type_decl_list = NULL;
            return typeDeclList;
        }
    }
    else
        syntax_error("type_decl_list. ID expected");
    return NULL; // control never reaches here, this is just for the sake of GCC
}

struct var_decl_sectionNode* var_decl_section()
{
    struct var_decl_sectionNode *varDeclSection;

    varDeclSection = ALLOC(struct var_decl_sectionNode);
    ttype = getToken();
    if (ttype == VAR)
    {
        // no need to ungetToken()
        varDeclSection->var_decl_list = var_decl_list();
        return varDeclSection;
    }
    else
        syntax_error("var_decl_section. VAR expected");
    return NULL; // control never reaches here, this is just for the sake of GCC
}

struct type_decl_sectionNode* type_decl_section()
{
    struct type_decl_sectionNode *typeDeclSection;

    typeDeclSection = ALLOC(struct type_decl_sectionNode);
    ttype = getToken();
    if (ttype == TYPE)
    {
        typeDeclSection->type_decl_list = type_decl_list();
        return typeDeclSection;
    }
    else
        syntax_error("type_decl_section. TYPE expected");
    return NULL; // control never reaches here, this is just for the sake of GCC
}

struct declNode* decl()
{
    struct declNode* dec;

    dec = ALLOC(struct declNode);
    dec->type_decl_section = NULL;
    dec->var_decl_section = NULL;
    ttype = getToken();
    if (ttype == TYPE)
    {
        ungetToken();
        dec->type_decl_section = type_decl_section();
        ttype = getToken();
        if (ttype == VAR)
        {
            // type_decl_list is epsilon
            // or type_decl already parsed and the
            // next token is checked
            ungetToken();
            dec->var_decl_section = var_decl_section();
        }
        else
        {
            ungetToken();
            dec->var_decl_section = NULL;
        }
        return dec;
    }
    else
    {
        dec->type_decl_section = NULL;
        if (ttype == VAR)
        {
            // type_decl_list is epsilon
            // or type_decl already parsed and the
            // next token is checked
            ungetToken();
            dec->var_decl_section = var_decl_section();
            return dec;
        }
        else
        {
            if (ttype == LBRACE)
            {
                ungetToken();
                dec->var_decl_section = NULL;
                return dec;
            }
            else
                syntax_error("decl. LBRACE expected");
        }
    }
    return NULL; // control never reaches here, this is just for the sake of GCC
}

struct programNode* program()
{
    struct programNode* prog;

    prog = ALLOC(struct programNode);
    ttype = getToken();
    if (ttype == TYPE || ttype == VAR || ttype == LBRACE)
    {
        ungetToken();
        prog->decl = decl();
        prog->body = body();
        return prog;
    }
    else
        syntax_error("program. TYPE or VAR or LBRACE expected");
    return NULL; // control never reaches here, this is just for the sake of GCC
}




int main()
{
    struct programNode* parseTree;
    parseTree = program();
    //print_parse_tree(parseTree);
	print_symbol_table();
    
    return 0;
}
