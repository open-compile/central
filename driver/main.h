//
// Created by xc5 on 2020/6/5.
//

#ifndef OCC_MAIN_H
#define OCC_MAIN_H

#include "../common/basic.h"

// Executing the whole flow
INT32 Execute(COMPILER_CONFIG &config);

// Run the component by command-line or direct invocation
INT32 Run_component(COMPONENTS_WHOLE component, COMPILER_CONFIG &config);

#endif //OCC_MAIN_H
