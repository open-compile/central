%option noyywrap yylineno

%{
#include <stdio.h>   
#include <stdlib.h>
#include <string.h>
#include "parser.tab.h"
int old_status;
void yyerror(char *s, ...);

%}

%x COMMENT

%%
int         { return INT; }
float       { return FLOAT; }
double      { return DOUBLE; }
auto        { return AUTO; }
break       { return BREAK; }
case        { return CASE; }
const       { return CONST; }
else        { return ELSE; }
for         { return FOR; }
if          { return IF; }
long        { return LONG; }
return      { return RETURN; }
short       { return SHORT; }
signed      { return SIGNED; }
char        { return CHAR; }
unsigned    { return UNSIGNED; }
	
[a-zA-Z][a-zA-Z0-9_]*       { yylval.strval = strdup(yytext); return IDENTITY; }
-?[0-9]+                    { yylval.intval = atoi(yytext); return INT_NUMBER; }
-?[0-9]+\.[0-9]+            { yylval.floatval = atof(yytext); return FLOAT_NUMBER; }
\"(\\.|\"\"|[^"\n"])*\"     { yylval.strval = strdup(yytext); return STRING; }
\"(\\.|[^"\n])*$            { yyerror("Unterminated string %s", yytext); }

[-+&~|^/%*(),.;!]       { return yytext[0]; }
"&&"                    { return AND; }
"||"                    { return OR; }
"="                     { return ASSIGN; }

"//".*;
"/*"                    { old_status = YY_START; BEGIN COMMENT; }
<COMMENT>"*/"           { BEGIN old_status; }
[ \t\n]
.						{ yyerror("something goes wrong...\n"); }

%%
