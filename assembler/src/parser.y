%{
  #include <cstdio>
  #include <cstring>
  #include <iostream>
  #include <string>
  #include "list.h"
  #include "operand.h"
  #include "instruction.h"
  #include "assembler.h"

  using namespace std;

  extern int yylex();
  extern int yyparse();
  extern void yyerror(const char *s);
  extern FILE* yyin;
%}

%output   "./src/parser.cpp"
%defines  "./inc/parser.h"

%union {
  int num;
  int reg;
  char* directive;
  char* symbol;
  char* instruction;
  class instruction* op;
  class list* symbolList;
}

%token COMMA
%token COLON
%token DOLLAR
%token PERCENT
%token OPENBRACKET
%token CLOSEBRACKET
%token PLUS
%token TIMES
%token NEXT
%token<reg> REG
%token<num> NUM
%token<instruction> NOOPERAND
%token<instruction> JMP
%token<instruction> ONEREG
%token<instruction> TWOREG
%token<instruction> STACKINST
%token<instruction> MEMINST
%token<symbol> SYMBOL
%token<directive> LISTDIR
%token<directive> SECTION
%token<directive> WORD
%token<directive> SKIP
%token<directive> EQU
%token<directive> END

%type<op> instructionOP
%type<op> jmpOP
%type<op> oneregOP
%type<op> tworegOP
%type<op> stackOP
%type<op> meminstOP
%type<symbol> label 
%type<directive> directive
%type<symbolList>  sList
%%

program:
  program instruction
  |
  instruction;
instruction:
  instructionOP NEXT
  {
    assembler::instance()->getRules()->inst($1);
  }
  |
  label NEXT
  |
  label instructionOP NEXT
  {
    assembler::instance()->getRules()->inst($2);
  }
  |
  directive NEXT
  |
  NEXT;
label:
  SYMBOL COLON
  {
    assembler::instance()->getRules()->label($1);
  };
directive:
  END
  {
    assembler::instance()->getRules()->endD();
    YYACCEPT;
  }
  |
  SKIP NUM
  {
    assembler::instance()->getRules()->skipD($2);
  }
  |
  SECTION SYMBOL
  {
    assembler::instance()->getRules()->sectionD($2);
  }
  |
  EQU SYMBOL COMMA NUM
  {
    assembler::instance()->getRules()->equD($2, $4);
  }
  |
  LISTDIR sList
  {
    if(strcmp($1, ".global") == 0)
      assembler::instance()->getRules()->globalD($2);
    else
      assembler::instance()->getRules()->externD($2);
  }
  |
  WORD NUM
  {
    assembler::instance()->getRules()->wordD($2);
  }
  |
  WORD sList
  {
    assembler::instance()->getRules()->wordD($2);
  };
sList:
  SYMBOL
  {
    list* tmp = new list();
    tmp->add($1);
    $$ = tmp;
  }
  |
  sList COMMA SYMBOL
  {
    list* tmp = $1;
    tmp->add($3);
    $$ = tmp;
  };
instructionOP:
  NOOPERAND
  {
    operand* tmpOp = new operand(0, 0, 0, 0, "");
    instruction* tmpInst = new instruction($1, tmpOp);
    free($1);
    $$ = tmpInst;
  }
  |
  jmpOP
  {
    $$ = $1;
  }
  |
  oneregOP
  {
    $$ = $1;
  }
  |
  tworegOP
  {
    $$ = $1;
  }
  |
  stackOP
  {
    $$ = $1;
  }
  |
  meminstOP
  {
    $$ = $1;
  };
jmpOP:
  JMP NUM
  {
    operand* tmpOp = new operand(4, 0b11110000, 0b00000000, $2, "");
    instruction* tmpInst = new instruction($1, tmpOp);
    free($1);
    $$ = tmpInst;
  }
  |
  JMP SYMBOL
  {
    operand* tmpOp = new operand(4, 0b11110000, 0b00000000, 0, $2);
    instruction* tmpInst = new instruction($1, tmpOp);
    free($1);
    free($2);
    $$ = tmpInst;
  }
  |
  JMP PERCENT SYMBOL
  {
    operand* tmpOp = new operand(4, 0b11110111, 0b00000011, 0, $3);
    instruction* tmpInst = new instruction($1, tmpOp);
    free($1);
    free($3);
    $$ = tmpInst;
  }
  |
  JMP TIMES NUM
  {
    operand* tmpOp = new operand(4, 0b11110000, 0b00000100, $3, "");
    instruction* tmpInst = new instruction($1, tmpOp);
    free($1);
    $$ = tmpInst;
  }
  |
  JMP TIMES SYMBOL
  {
    operand* tmpOp = new operand(4, 0b11110000, 0b00000100, 0, $3);
    instruction* tmpInst = new instruction($1, tmpOp);
    free($1);
    free($3);
    $$ = tmpInst;
  }
  |
  JMP TIMES REG
  {
    operand* tmpOp = new operand(2, 0b11110000 | $3, 0b00000001, 0, "");
    instruction* tmpInst = new instruction($1, tmpOp);
    free($1);
    $$ = tmpInst;
  }
  |
  JMP TIMES OPENBRACKET REG CLOSEBRACKET
  {
    operand* tmpOp = new operand(2, 0b11110000 | $4, 0b00000010, 0, "");
    instruction* tmpInst = new instruction($1, tmpOp);
    free($1);
    $$ = tmpInst;
  }
  |
  JMP TIMES OPENBRACKET REG PLUS NUM CLOSEBRACKET
  {
    operand* tmpOp = new operand(4, 0b11110000 | $4, 0b00000011, $6, "");
    instruction* tmpInst = new instruction($1, tmpOp);
    free($1);
    $$ = tmpInst;
  }
  |
  JMP TIMES OPENBRACKET REG PLUS SYMBOL CLOSEBRACKET
  {
    operand* tmpOp = new operand(4, 0b11110000 | $4, 0b00000011, 0, $6);
    instruction* tmpInst = new instruction($1, tmpOp);
    free($1);
    free($6);
    $$ = tmpInst;
  };
oneregOP:
  ONEREG REG
  {
    operand* tmpOp = new operand(1, 0b00001111 | ($2 << 4), 0, 0, "");
    instruction* tmpInst = new instruction($1, tmpOp);
    free($1);
    $$ = tmpInst;
  };
tworegOP:
  TWOREG REG COMMA REG
  {
    operand* tmpOp = new operand(1, ($2 << 4) | $4, 0, 0, "");
    instruction* tmpInst = new instruction($1, tmpOp);
    free($1);
    $$ = tmpInst;
  };
stackOP:
  STACKINST REG
  {
    int tmp;
    if(strcmp($1, "pop") == 0)
      tmp = 4;
    else 
      tmp = 1;
    operand* tmpOp = new operand(2, ($2 << 4) | 6, 0b00000010 | (tmp << 4), 0, "");
    instruction* tmpInst = new instruction($1, tmpOp);
    free($1);
    $$ = tmpInst;
  };
meminstOP:
  MEMINST REG COMMA NUM
  {
    operand* tmpOp = new operand(4, 0b00000000 | ($2 << 4), 0b00000100, $4, "");
    instruction* tmpInst = new instruction($1, tmpOp);
    free($1);
    $$ = tmpInst;
  }
  |
  MEMINST REG COMMA SYMBOL
  {
    operand* tmpOp = new operand(4, 0b00000000 | ($2 << 4), 0b00000100, 0, $4);
    instruction* tmpInst = new instruction($1, tmpOp);
    free($1);
    free($4);
    $$ = tmpInst;
  }
  |
  MEMINST REG COMMA PERCENT SYMBOL
  {
    operand* tmpOp = new operand(4, 0b00000111 | ($2 << 4), 0b00000011, 0, $5);
    instruction* tmpInst = new instruction($1, tmpOp);
    free($1);
    free($5);
    $$ = tmpInst;
  }
  |
  MEMINST REG COMMA DOLLAR NUM
  {
    operand* tmpOp = new operand(4, 0b00000000 | ($2 << 4), 0b00000000, $5, "");
    instruction* tmpInst = new instruction($1, tmpOp);
    free($1);
    $$ = tmpInst;
  }
  |
  MEMINST REG COMMA DOLLAR SYMBOL
  {
    operand* tmpOp = new operand(4, 0b00000000 | ($2 << 4), 0b00000000, 0, $5);
    instruction* tmpInst = new instruction($1, tmpOp);
    free($1);
    free($5);
    $$ = tmpInst;
  }
  |
  MEMINST REG COMMA REG
  {
    operand* tmpOp = new operand(2,($2 << 4) | $4, 0b00000001, 0, "");
    instruction* tmpInst = new instruction($1, tmpOp);
    free($1);
    $$ = tmpInst;
  }
  |
  MEMINST REG COMMA OPENBRACKET REG CLOSEBRACKET
  {
    operand* tmpOp = new operand(2,($2 << 4) | $5, 0b00000010, 0, "");
    instruction* tmpInst = new instruction($1, tmpOp);
    free($1);
    $$ = tmpInst;
  }
  |
  MEMINST REG COMMA OPENBRACKET REG PLUS NUM CLOSEBRACKET
  {
    operand* tmpOp = new operand(4, ($2 << 4) | $5, 0b00000011, $7, "");
    instruction* tmpInst = new instruction($1, tmpOp);
    free($1);
    $$ = tmpInst;
  }
  |
  MEMINST REG COMMA OPENBRACKET REG PLUS SYMBOL CLOSEBRACKET
  {
    operand* tmpOp = new operand(4, ($2 << 4) | $5, 0b00000011, 0, $7);
    instruction* tmpInst = new instruction($1, tmpOp);
    free($1);
    free($7);
    $$ = tmpInst;
  };
%%