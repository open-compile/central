//
// Created by xc5 on 2020/6/5.
//

#ifndef OCC_FE_EXPORT_H
#define OCC_FE_EXPORT_H

extern "C" {
    int femain(int argc, const char *args[]);
    void yyerror(char *s, ...);
}

#endif //OCC_FE_EXPORT_H
