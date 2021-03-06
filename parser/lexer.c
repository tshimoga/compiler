
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "lexer.h"

#define KEYWORDS  5
#define RESERVED  26

char token[MAX_TOKEN_LENGTH];
int  tokenLength;
int  ttype;
int  line = 1;

static int active_token = 0;
static char *reserved[] =
	{	"",
		"IF",
		"WHILE",
		"DO",
		"THEN",
		"PRINT",
		"+",
		"-",
		"/",
		"*",
		"=",
		":",
		",",
		";",
		"[",
		"]",
		"(",
		")",
		"<>",
		">",
		"<",
		"<=",
		">=",
		".",
		"NUM",
		"ID",
		"ERROR"
	};


static void skip_space()
{
	char c;

	c = getchar();
	line += (c == '\n');
	while (!feof(stdin) && isspace(c))
	{
		c = getchar();
        line += (c == '\n');
	}
	// return character to input buffer if eof is not reached
	if (!feof(stdin))
		ungetc(c,stdin);
}

static int is_keyword(char *s)
{
	int i;

	for (i = 1; i <= KEYWORDS; i++)
		if (strcmp(reserved[i],s) == 0)
	   		return i;
	return 0;
}

static int scan_number()
{
	char c;

	c = getchar();
	if (isdigit(c))
	{
		if (c == '0')
		{
			token[tokenLength] = c;
			tokenLength++;
			token[tokenLength] = '\0';
		}
		else
		{
			while (isdigit(c) && tokenLength < MAX_TOKEN_LENGTH)
			{
				token[tokenLength] = c;
				tokenLength++;
				c = getchar();
			}
			token[tokenLength] = '\0';
			ungetc(c, stdin);
		}
		return NUM;
	}
	else
		return ERROR;
}

static int scan_id_keyword()
{
	char c;
	int tt;

	c = getchar();
	if (isalpha(c))
	{
		while (isalnum(c) && tokenLength < MAX_TOKEN_LENGTH)
		{
			token[tokenLength] = c;
			tokenLength++;
			c = getchar();
		}
		token[tokenLength] = '\0';
		ungetc(c, stdin);
		tt = is_keyword(token);
		if (tt == 0)
			tt = ID;
		return tt;
	}
	else
		return ERROR;
}

int getToken()
{
	char c;

	if (active_token)
	{
		active_token = 0;
		return ttype;
	}
	skip_space();
	tokenLength = 0;
	token[0] = '\0';
	c = getchar();
	switch (c)
	{
		case '.': ttype = DOT;			return ttype;
		case '+': ttype = PLUS;			return ttype;
		case '-': ttype = MINUS;		return ttype;
		case '/': ttype = DIV;			return ttype;
		case '*': ttype = MULT;			return ttype;
		case '=': ttype = EQUAL;		return ttype;
		case ':': ttype = COLON;		return ttype;
		case ',': ttype = COMMA;		return ttype;
		case ';': ttype = SEMICOLON;	return ttype;
		case '[': ttype = LBRAC;		return ttype;
		case ']': ttype = RBRAC;		return ttype;
		case '(': ttype = LPAREN;		return ttype;
		case ')': ttype = RPAREN;		return ttype;
		case '<':
			c = getchar();
			if (c == '=')
				ttype = LTEQ;
			else if (c == '>')
				ttype = NOTEQUAL;
			else
			{
				ungetc(c, stdin);
				ttype = LESS;
			}
			return ttype;
		case '>':
			c = getchar();
			if (c == '=')
				ttype = GTEQ;
			else
			{
				ungetc(c, stdin);
				ttype = GREATER;
			}
			return ttype;
		default :
			if (isdigit(c))
			{
				ungetc(c, stdin);
				ttype = scan_number();
			}
			else if (isalpha(c)) // token is either keyword or ID
			{
				ungetc(c, stdin);
				ttype = scan_id_keyword();
			}
			else if (c == EOF)
			{
				ttype = EOF;
			}
			else
			{
				ttype = ERROR;
			}
			return ttype;
	}
}

void ungetToken()
{
	active_token = 1;
}

/*
 * Write your code in a separate file, do NOT edit this file.
 *
 *
 *
 * If using C, include lexer.h in your C code and compile and
 * link your code with lexer.c like this:
 *
 * $ gcc -Wall lexer.c your_file.c
 *
 *
 *
 * If using C++, include lexer.h like this in your C++ code:
 *
 * extern "C" {
 *     #include "lexer.h"
 * }
 *
 * And compile and link the C and C++ code like this:
 *
 * $ gcc -Wall -c lexer.c
 * $ g++ -Wall lexer.o your_file.cpp
 *
 * The first command just compiles the C code. The second 
 * command compiles your C++ code and links it with the 
 * compiled lexer.
 *
 */

