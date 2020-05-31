# 本项目

这个项目是深大参加比赛专用的代码主仓库
其他辅助项目都在szucompiler 这个群里，大家记得项目要设置Visibility 为 Private，防止知识产权侵权纠纷。

## 我们的设计

整体流程应该是 

1. Lexer -> Token Stream
2. Parser -> MLIR (High level, AST + Symbol Table)
3. Optimizer -> MLIR (Low level, AST + Symbol Table)
4. Code generation -> Assembly (ARMV7-EABI 32bit)

IR 设计

1. Opcode: [Opcode from WHIRL, to be used](common/opcode_gen)
2. Symbol Table: [common/symtab.h](common/symtab.h)

## 代码风格规范

- 松散化的 Google 风格
- C/C++ 函数名称首字母大写，其他字母小写，多个单词下划线连接
- 减少使用 int, long long 等字样，而是使用 INT32, UINT32, INT64, UINT64, INT8, UINT8 ...
- 使用 char *
- 使用 class
- 尽可能避免使用全局变量
- 基本的内存管理（应该问题不大）

## 参考资料

### 1. GCC
我们使用GCC 6.4.0作为编译基准，参考GCC 8.2.0 的参数
https://gcc.gnu.org/onlinedocs/gcc-8.2.0/gcc/Developer-Options.html

目前仓库同步的是 GCC-10.1 的开发版
GCC 作为比较基准（我们的benchmark基准）
如果GCC可以编译通过，那么我们也应该可以。
我们编译出的代码应该和GCC编译出的代码具有相似性，或者是运营结果一致。

### 2. GDB

### 3. Open64
这部分代码已经上传至我们的Gitlab.com
- open64

### 4. OpenCTI
这部分代码已经上传至我们的Gitlab.com  
- opencti-src  
- opencti-testware  

访问：
http://cti.anitago.com/CTI/

### 5. 类似的现有项目作为参照

一个简易的 C子集到ARMV7 的编译器，它直接生成ELF代码  
流程是： AST -> IR -> ARMV7-eabi  
https://gitlab.com/szucompiler/amacc  

### 6. 我们的 MLIR 设计

- IR 设计就是 Opcode + Symbol Table.
- 我们的 Symbol Table 定义，参照 WHIRL 的定义即可。
- 我们的 Opcode 定义，参照 WHIRL 的定义即可（HIR）。

- 方舟编译器的Opcode定义  
https://gitlab.com/szucompiler/openarkcompiler/-/blob/master/src/maple_ir/include/opcodes.def  

- WHIRL 的 Opcode 定义  
https://gitlab.com/szucompiler/docs/-/blob/master/open64A.pdf  
https://gitlab.com/szucompiler/open64/-/blob/master/osprey/common/com/opcode_gen
[Opcode from WHIRL, to be used](common/opcode_gen)

- GCC 的 Opcode 定义  
https://gitlab.com/szucompiler/open64/-/blob/master/osprey/kgccfe/gnu/cp/operators.def  

- LLVM 的 Opcode 定义  
To be reviewed. (not so important though)  

