//
// Created by xc5 on 2020/6/14.
//

#include "basic.h"


/* predefined array limits; (p.f. means per function) */
#define NARGS		32		/* Count on function/macro arguments */
#define NTMPS		64		/* Count on expression temporaries */
#define NFIELDS		128		/* Count on fields in structs */
#define NAMELEN		128		/* The maximum of identifiers */
#define NDEFS		4096		/* Count on macros */
#define MARGLEN		1024		/* Sizing of macro arguments */
#define MDEFLEN		2048		/* Sizing of macro definitions */
#define NBUFS		32		/* Macro expansion stack depth */
#define NLOCS		1024		/* Count on header search paths */

#define LEN(a)		(sizeof(a) / sizeof((a)[0]))
#define ALIGN(x, a)	(((x) + (a) - 1) & ~((a) - 1))
#define MIN(a, b)	((a) < (b) ? (a) : (b))
#define MAX(a, b)	((a) < (b) ? (b) : (a))

void *mextend(void *old, INT64 oldsz, INT64 newsz, INT64 memsz);

/* variable length buffer */
struct MEM_BLK_NC {
	char *s;		/* allocated buffer */
	INT64 sz;		/* buffer size */
	INT64 n;			/* length of data stored in s */
};

void Cpp_mem_init(MEM_BLK_NC *mem);
void Cpp_mem_done(MEM_BLK_NC *mem);
void Mem_cut(MEM_BLK_NC *mem, INT64 pos);
void *Mem_buf(MEM_BLK_NC *mem);
void Mem_put(MEM_BLK_NC *mem, void *buf, INT64 len);
void Mem_putc(MEM_BLK_NC *mem, INT32 c);
void Mem_putz(MEM_BLK_NC *mem, INT64 sz);
void Mem_cpy(MEM_BLK_NC *mem, INT64 off, void *buf, INT64 len);
INT64 Mem_len(MEM_BLK_NC *mem);
void *Mem_get(MEM_BLK_NC *mem);

/* SECTION ONE: Tokenisation */
void tok_init(char *path);
void tok_done();
char *tok_see();		/* return the current token; a static buffer */
char *tok_get();		/* return and consume the current token */
INT64 tok_len();		/* the length of the last token */
INT64 tok_num(char *tok, INT64 *n);
INT64 tok_addr();
void tok_jump(INT64 addr);

INT32 Cpp_preprocess_init(const char *path);
void Cpp_path(const char *s);
void Cpp_define(const char *name, const char *def);
char *Cpp_loc(INT64 addr);
INT32 Cpp_read(const char **buf, INT64 *len);

