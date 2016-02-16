#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#define true 1
#define false 0


int num_symbols=1;;
int num_non_terminals=0;
int num_terminals=0;
int num_rules=0; 

char *symbols[50]; 

int first[20][20];

int follow[20][20];

int gen_epsilon[20];

int lenOne[20];

int lengths[20];

struct rule {
	int rhs_length;
	int LHS;
	int RHS[20];
	int no_terminals;
	
};
struct rule rules[20];


void myPrinter() {
	
	int i=0;
	
	printf("Symbols #######################\n");
	for(i=0;i<num_symbols;i++) {
		printf("%d : %s\n",i,symbols[i]);
		
	}
	
	printf("grammar #######################\n");
		printf("num of rules is %d\n", num_rules);
		for(i=0;i<num_rules;i++) {
		struct rule temp = rules[i];
		printf("%s -> ",symbols[temp.LHS]);
		
		int k;
		for(k=0;k<temp.rhs_length;k++) 
			printf("%s ",symbols[temp.RHS[k]]);
		
		printf("Number of terminals is %d", temp.no_terminals);
	
		printf("\n");
		}
		
		i=0;
		
		printf("epsilons #######################\n");
		while(i++<=num_non_terminals) {
		if(gen_epsilon[i])
		printf("%s ", symbols[i]);
		
		}
	printf("\n");
		i=1;
	printf("lenOnes #######################\n");
		while(i <=num_non_terminals) {
		if(lenOne[i]==true)
			printf("%s ", symbols[i]);
			i++;
		}
	printf("\n");
		
}

void genEpsilon() {
	int changed=true;
	gen_epsilon[0]=true;
	int i,j;
	for (i = 1; i < num_symbols; i++)
		gen_epsilon[i] = false;
	
	while (changed) {  
		changed = false;
		for (i = 0; i < num_rules; i++) {
			if ( gen_epsilon[rules[i].LHS] ) {
				continue;
			} else if ( rules[i].rhs_length == 0 ) {
				gen_epsilon[rules[i].LHS] = true; 
				changed = true; 
			} else {
				int some_does_not_gen_epsilon = false;
				
				for (j = 0; j < rules[i].rhs_length; j++) {
					some_does_not_gen_epsilon |= !gen_epsilon[rules[i].RHS[j]]; 
				}
				if (!some_does_not_gen_epsilon) { 
					gen_epsilon[rules[i].LHS] = true;
					changed = true; 
				} 		
			}
		}
	}
}

void calcLenOne() {
	calculateLengths();
	int changed=true;
	lenOne[0]=false;
	int i,j;
	for (i = 0; i < num_symbols; i++) {
		if(i<=num_non_terminals) {
			lenOne[i]=false;
		} else {
			lenOne[i] = true;
		}
	}
	
		
	
	 while (changed) {  
		changed = false;
		for (i = 0; i < num_rules; i++) {
			if ( lenOne[rules[i].LHS] || rules[i].no_terminals > 1) {
				continue;
			} else if ( rules[i].rhs_length == 1 && lenOne[rules[i].RHS[0]]) {
					
					lenOne[rules[i].LHS] = true; 
					changed = true; 
				 
			} else if (rules[i].rhs_length > 1){
				int length=0;
				int canGenOne=false;
				for(j=0;j<rules[i].rhs_length;j++) {
					length=length+lengths[rules[i].RHS[j]];
					
				}
				
				if(length==1) {
					lenOne[rules[i].LHS] = true;
					changed=true;
				} else if(length==0) {
					int m=false;
					for(j=0;j<rules[i].rhs_length;j++) {
						
						if(lenOne[rules[i].RHS[j]]) {
							m=true;
							break;
						}
					}
					if(m) {
						lenOne[rules[i].LHS] = true;
						changed=true;
					}
				}	
				}
			}
		}
	 }
	 
void calculateLengths() {
	
	int i, changed=true, minLen;
	lengths[0]=0;
	
	
	for(i=1;i<num_symbols;i++) {
		if(i<=num_non_terminals){
			lengths[i]=-1;
		} else {
			lengths[i]=1;
			
		}
	}
	
	while(changed) {
		changed=false;
	for(i=0;i<num_rules;i++) {
		
		int rhs_len = rules[i].rhs_length;
		int j;
		if(rhs_len==0 && lengths[rules[i].LHS]!=0) {
			lengths[rules[i].LHS]=0;
			
			changed=true;
		} else {
			int k=0,minLen=0;
			for(j=0; j<rhs_len;j++) {
				k=1;
				if(lengths[rules[i].RHS[j]]==-1) {
					k=0;
					break;
				}
				minLen = minLen + lengths[rules[i].RHS[j]];
			}
			if(lengths[rules[i].LHS]>minLen || ( lengths[rules[i].LHS]==-1 && k==1)) {
				
				lengths[rules[i].LHS]=minLen;
				changed=true;
			}			
		}
	}}
	
}
	


int isSymbol(char* c) {
	
	int i=0;
	
	for(i=0;i<num_symbols;i++) {
		
		if ((strcmp(c,symbols[i]))==0) {
			return i;
		}
	}
	
	return -1;	
}

char* parseString(char c) {
	
	
	char* str;
	str = (char*) malloc(sizeof(char));
	int i=0;
	
	while(c==' ' || c=='\t' || c=='\n') {
		
		c=getchar();
	}
	
	while(c!=' ' && c!='\n' && c!='\t') {
		
		str[i++]=c;
		c=getchar();	
	}
	str[i]='\0';
	return str;
}

void parseNonTerminals() {
	char c;
	symbols[0]="#";
	
	while ((c=getchar())!='#') {
		symbols[num_symbols++] = parseString(c);
		num_non_terminals++;
		
	} 
}
void parseLHS() {
	
	
	char c=getchar();
	while(c==' ' || c=='\t' || c=='\n') {
		c=getchar();
	}
	if(c=='#') {
		return;
	}
	rules[num_rules].LHS=isSymbol(parseString(c));
	
	
}

void parseRHS() {
	
	rules[num_rules].rhs_length=0;
	rules[num_rules].no_terminals=0;
	
	char ch = getchar();
	while(ch==' '|| ch=='\n' || ch=='\t') {
		ch=getchar();
	}
	char* str;
	int j;
	if(ch!='#') {
		while(ch!='#') {
			while(ch==' '||ch=='\n' || ch=='\t') {
				ch=getchar();
			}
			if(ch=='#') {
				break;
			}
			str = parseString(ch);
			j=isSymbol(str);
			
			if(j==-1) {
				
				symbols[num_symbols] = str;
				num_symbols++;
				num_terminals++;
				j=num_symbols-1;
				
			}	
			
			rules[num_rules].RHS[rules[num_rules].rhs_length++] = j;
			
			if(j > num_non_terminals) {
				rules[num_rules].no_terminals++;
			}
			ch=getchar();
		}
	}
	
	num_rules++;
 }


void parseGrammar() {
	char ch;
	while((ch=getchar())!='#') {
		
		parseLHS();
		if((ch=getchar())=='#') {
			break;
		}
		char c;
		while((c=getchar())!='>') {
			
		}
		parseRHS();
		
	}
	
}



void parseInput() {
	parseNonTerminals();
	parseGrammar();
	genEpsilon();
	 // myPrinter();
}

 void calculateFirstSets(int p) {
	
	int i, changed=true;
	first[0][0]=0;
	first[0][1]=-1;
	for(i=1;i<num_symbols;i++) {
		if(i<=num_non_terminals) {
			first[i][0]=-1;
		} else {
			first[i][0]=i;
			first[i][1]=-1;
		}	
	}
	
while(changed) {
	changed=false;
	 for(i=0;i<num_rules;i++) {
		int j=0, done=true;
		if(rules[i].rhs_length>0) {
			
			while(done) {
			done=false;
			int m=0;
			
			while(first[rules[i].RHS[j]][m]!=-1) {
					int has=false, k=0;
					while(first[rules[i].LHS][k]!=-1) {
						if(first[rules[i].LHS][k]==first[rules[i].RHS[j]][m]) {
							has=true;
							break;
						}
						k++;
					}
					if(!has) {
						first[rules[i].LHS][k]=first[rules[i].RHS[j]][m];
						first[rules[i].LHS][k+1]=-1;
						changed=true;
					}
					m++;
			}
			if(gen_epsilon[rules[i].RHS[j]]) {
				j++;
				done=true;
			}
						
		}
		}
	}
 }
	if(p) {
	for(i=1;i<=num_non_terminals;i++) {
		int j=0, k;
		while(first[i][j+1]!=-1) {
			k=j+1;
			while(first[i][k]!=-1) {
				
				if(strcmp(symbols[first[i][j]], symbols[first[i][k]])>0) {
				
					int temp;
					temp=first[i][j];
					first[i][j]=first[i][k];
					first[i][k]=temp;
				}
				k++;
			}
			j++;
		}
		
				
		
		j=0;
		printf("FIRST(%s) = { ",symbols[i]);
		if(gen_epsilon[i] ) {
			
			printf("#, ");
		}
		
		while(first[i][j+1]!=-1) {
			
			printf("%s, ", symbols[first[i][j]]);
			j++;
		}
		printf("%s } ", symbols[first[i][j]]);
		
		printf("\n");
	}
	}	
}


void printNonTerminalsWithLengthOne() {
	calcLenOne();
	char* str;
	int i=1;
	while(i <=num_non_terminals) {
		str="NO";
		if(lenOne[i]==true) {
			str="YES";
		}
		printf("%s: %s\n", symbols[i], str);
		i++;
		}
}

void calculateFollowSets() {
	int i;
	calculateFirstSets(false);
	symbols[num_symbols++]="$";
	follow[1][0] = num_symbols-1;
	follow[1][1] = -1;
	for(i=2;i<=num_non_terminals;i++) {
		
			follow[i][0]=-1;
	}
	int changed=true;
	while(changed) {
		changed=false;
		for(i=0;i<num_rules;i++) {
			
			if(rules[i].rhs_length>0) {
			int j=rules[i].rhs_length-1;
			int changed1=true;
			while(changed1) {
				changed1=false;
			if(rules[i].RHS[j]>0 && rules[i].RHS[j]<=num_non_terminals) {
				int m=0;
				while(follow[rules[i].LHS][m]!=-1) {
					int has=false, k=0;
					while(follow[rules[i].RHS[j]][k]!=-1) {
						if(follow[rules[i].RHS[j]][k]==follow[rules[i].LHS][m]) {
							has=true;
							break;
						}
						k++;
					}
					if(!has) {
						follow[rules[i].RHS[j]][k]=follow[rules[i].LHS][m];
						follow[rules[i].RHS[j]][k+1]=-1;
						changed=true;
					}
					m++;
					}
				}
				
				if(gen_epsilon[rules[i].RHS[j]]) {
						j--;
						changed1=true;
					}
			}
			}
		}
		
		for(i=0;i<num_rules;i++) {
			if(rules[i].rhs_length>0) {
			
					int j=rules[i].rhs_length-1; 
					while(j>0) {
						if(rules[i].RHS[j-1]>0 && rules[i].RHS[j-1]<=num_non_terminals) {
							int m=0;
							while(first[rules[i].RHS[j]][m]!=-1) {
								int has=false, k=0;
								while(follow[rules[i].RHS[j-1]][k]!=-1) {
									if(follow[rules[i].RHS[j-1]][k]==first[rules[i].RHS[j]][m]) {
										has=true;
										break;
								}
								k++;
							}
							if(!has) {
								follow[rules[i].RHS[j-1]][k]=first[rules[i].RHS[j]][m];
								
			  					follow[rules[i].RHS[j-1]][k+1]=-1;
								changed=true;
							}
							m++;
							}
						}
						j--;
					}
			}	
		}
		
	}
		
		for(i=1;i<=num_non_terminals;i++) {
			int j=0;
			
			while(follow[i][j+1]!=-1) {
				int k=j+1;
				while(follow[i][k]!=-1) {
					if(strcmp(symbols[follow[i][j]],symbols[follow[i][k]])>0) {
						int temp;
						temp = follow[i][j];
						follow[i][j] = follow[i][k];
						follow[i][k] = temp;
					}
					k++;
				}
				j++;
			}
			
			j=0;
			printf("FOLLOW(%s) = { ", symbols[i]);
			while(follow[i][j+1]!=-1) {
				printf("%s, ", symbols[follow[i][j]]);
				j++;
			}
			printf("%s }", symbols[follow[i][j]]);
			printf("\n");
		}
}


int main(int argc, char* argv[]) {

		int task;
		
		
		
		if(argc<2) {
			printf("Error: missing argument\n");
			return 1;
		}
		
	
		 parseInput();
		
		task = atoi(argv[1]);
		
		switch(task) {
		
		case 1: 
			//calculateLengths();
			 printNonTerminalsWithLengthOne();
			break;
		
		case 2: 
			
			calculateFirstSets(true);
			break;
			
		case 3:
			calculateFollowSets();
			break;
		
		default:
			printf("Error: unrecognized task number %d\n",task);
			break;
		
		}
		
		return 1;
}