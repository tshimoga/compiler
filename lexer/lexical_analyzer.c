#include <stdio.h>
#include <stdlib.h>
#include<string.h>
#include<ctype.h>
#include "lexer.h"

/*Each node in the linked list contains members 
(i) tokenString which holds the token text
(ii) tokenType whose values are either "NUM" or "ID" depending on the input
(iii) lineNumber which holds the nodes line number
(iv) a pointer to the next node in the linked list. 
*/
struct Node {
	char tokenString[MAX_TOKEN_LENGTH];
	char* tokenType;
	int lineNumber;
	struct Node* next;
};

//root node is Global
struct Node* root;

/*Adds new Node
*/
struct Node* addNode() {
	struct Node* newNode = (struct Node*)malloc(sizeof(struct Node)); 
	char* string; 
	if(ttype==24) {
	string = "NUM";	
	} else {
	string = "ID";
	}
	newNode->next=NULL;
	newNode->tokenType=string;
	int i=0;
	while(i<tokenLength) {
		newNode->tokenString[i]=token[i];
		i++;
	}
	newNode->lineNumber=line;
	if(root==NULL) {
		root=newNode;
	} else {
		struct Node* iterator = root; 
		while(iterator->next!=NULL) {
			iterator=iterator->next;
		}
		iterator->next=newNode;
	}
	
	return newNode;
}

/*
Prints contents of the list in reverse order in the format
<line> <ttype_str> <token>
*/
void printResult(struct Node* node) {
	if(node==NULL) {
		return;
	}
	printResult(node->next);
	printf("%d %s %s \n",node->lineNumber, node->tokenType, node->tokenString);
	
}


int main(int argc, char* argv[]) {
	
	int c;
	
	while((c=getToken())!=EOF && c!=ERROR) {
		if((c==24) || ((c==25) && ((strcmp(token,"cse340")==0) || (strcmp(token,"programming")==0) || (strcmp(token,"language")==0)))) {
			addNode();
		}	
	}
	
	printResult(root);
	return 1;
}