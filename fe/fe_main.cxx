//
// Created by xc5 on 2020/6/14.
//

#include <iostream>
#include <fstream>
#include "ASTNodes.h"
#include "options.h"
#include "stdarg.h"
#include "ir.h"

extern shared_ptr<NBlock> programBlock;
extern int yyparse();
// extern void yyparse_init(const char* filename);
// extern void yyparse_cleanup();
//
//void createCoreFunctions(CodeGenContext& context);

INT32 femain(COMPILER_CONFIG &conf, FILE_MANAGER &file_man, const char *file_name) {
  extern FILE *yyin;
  if ((yyin = fopen(file_name, "r")) == NULL) {
    Comp_Failure("Failed to open source code : %s", file_name);
  }
  yyparse();

  // std::cout << programBlock << std::endl;
  programBlock->print("--");
  auto root = programBlock->jsonGen();

//    cout << root;

////    cout << root << endl;
//    CodeGenContext context;
////    createCoreFunctions(context);
//    context.generateCode(*programBlock);
//    ObjGen(context);

  string jsonFile = "visualization/A_tree.json";
  std::ofstream astJson(jsonFile);
  if( astJson.is_open() ){
    astJson << root;
    astJson.close();
    cout << "json write to " << jsonFile << endl;
  }

  return 0;
}


void yyerror(char *s, ...)
{
  extern int yylineno;

  va_list ap;
  va_start(ap, s);

  fprintf(stderr, "%d: error: ", yylineno);
  vfprintf(stderr, s, ap);
  fprintf(stderr, "\n");
}
