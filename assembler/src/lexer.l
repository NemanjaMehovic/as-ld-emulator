%{
  #include <iostream>
  #include <cstdlib>
  #include <string>
  #include "parser.h"

  void cisti();
  void yyerror(const char*);
  int lineNum = 1;
%}

%option noyywrap outfile="./src/lexer.cpp" header-file="./inc/lexer.h"

DEC [0-9]+
HEX 0[xX][0-9a-fA-F]+
REGNUM [0-7]
NOTHING [ \t]
NEXT    (#[^\n]*|\n)
NOOPERAND (halt|iret|ret)
JMP (call|jmp|jeq|jne|jgt)
ONEREG  (int|not)
TWOREG  (xchg|add|sub|mul|div|cmp|and|or|xor|test|shl|shr)
MEMINST (ldr|str)
STACKOP (push|pop)
LISTDIR (\.global|\.extern)
SYMBOLS  [_a-zA-Z][_0-9a-zA-Z]*

%%
{NOTHING}   ;
{NEXT}      {
                lineNum++;
                return NEXT;
            }
{DEC}       {   
                int tmp = strtol(yytext, NULL, 10);
                if(tmp > 0xFFFF)
                    yyerror("Only 16 bit values alowed");
                yylval.num = tmp;
                return NUM; 
            }
{HEX}       {
                int tmp = strtol(yytext, NULL, 16);
                if(tmp > 0xFFFF)
                    yyerror("Only 16 bit values alowed");
                yylval.num = tmp;
                return NUM;
            }
"r"{REGNUM} {
                yylval.reg = strtol(yytext + 1, NULL, 10);
                return REG; 
            }
"psw"       {
                yylval.reg = 8;
                return REG;
            }
{LISTDIR}   {
                yylval.directive = strdup(yytext);
                return LISTDIR;
            }
".section"  {
                yylval.directive = strdup(yytext);
                return SECTION;
            }
".word"     {
                yylval.directive = strdup(yytext);
                return WORD;
            }
".skip"     {
                yylval.directive = strdup(yytext);
                return SKIP;
            }
".equ"      {
                yylval.directive = strdup(yytext);
                return EQU;
            }
".end"      {
                yylval.directive = strdup(yytext);
                return END;
            }
{NOOPERAND} {
                yylval.instruction = strdup(yytext);
                return NOOPERAND;
            }
{JMP}       {
                yylval.instruction = strdup(yytext);
                return JMP;
            }
{ONEREG}    {
                yylval.instruction = strdup(yytext);
                return ONEREG;
            }
{TWOREG}    {
                yylval.instruction = strdup(yytext);
                return TWOREG;
            }
{MEMINST}   {
                yylval.instruction = strdup(yytext);
                return MEMINST;
            }
{STACKOP}   {
                yylval.instruction = strdup(yytext);
                return STACKINST;
            }
{SYMBOLS}   {
                yylval.symbol = strdup(yytext);
                return SYMBOL;
            }
","         {   return COMMA; }
":"         {   return COLON; }
"$"         {   return DOLLAR; }
"%"         {   return PERCENT; }
"["         {   return OPENBRACKET; }
"]"         {   return CLOSEBRACKET; }
"+"         {   return PLUS; }
"*"         {   return TIMES; }
.         {   yyerror("syntax error"); }
%%

void yyerror(const char* s)
{
    std::cerr << s << " on line " << lineNum << std::endl; 
    exit(1);
}

void cisti()
{
    YY_FLUSH_BUFFER;
}