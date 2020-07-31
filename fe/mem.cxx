//
// Created by xc5 on 2020/6/14.
//

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cpp_pre.h"

#define MEMSZ		512

static void mem_extend(MEM_BLK_NC *mem)
{
	char *s = mem->s;
	mem->sz = mem->sz ? mem->sz + mem->sz : MEMSZ;
	mem->s = (char *) malloc(mem->sz);
	if (mem->n)
		memcpy(mem->s, s, mem->n);
	free(s);
}

void Cpp_mem_init(MEM_BLK_NC *mem)
{
	memset(mem, 0, sizeof(*mem));
}

void Cpp_mem_done(MEM_BLK_NC *mem)
{
	free(mem->s);
	memset(mem, 0, sizeof(*mem));
}

void mem_cut(MEM_BLK_NC *mem, INT64 pos)
{
	mem->n = pos < mem->n ? pos : mem->n;
}

void Mem_cpy(MEM_BLK_NC *mem, INT64 off, void *buf, INT64 len)
{
	while (mem->n + off + len + 1 >= mem->sz)
		mem_extend(mem);
	memcpy(mem->s + off, buf, len);
}

void mem_put(MEM_BLK_NC *mem, void *buf, INT64 len)
{
  Mem_cpy(mem, mem->n, buf, len);
	mem->n += len;
}

void Mem_putc(MEM_BLK_NC *mem, int c)
{
	if (mem->n + 2 >= mem->sz)
		mem_extend(mem);
	mem->s[mem->n++] = c;
}

void mem_putz(MEM_BLK_NC *mem, INT64 sz)
{
	while (mem->n + sz + 1 >= mem->sz)
		mem_extend(mem);
	memset(mem->s + mem->n, 0, sz);
	mem->n += sz;
}

/* return a pointer to MEM_BLK_NC's buffer; valid as INT64 as MEM_BLK_NC is not modified */
void *Mem_buf(MEM_BLK_NC *mem)
{
	if (!mem->s)
		return (void *) "";
	mem->s[mem->n] = '\0';
	return mem->s;
}

INT64 Mem_len(struct MEM_BLK_NC *mem)
{
	return mem->n;
}

void *Mem_get(struct MEM_BLK_NC *mem)
{
	void *ret;
	if (!mem->s)
		mem_extend(mem);
	ret = mem->s;
  Cpp_mem_init(mem);
	return ret;
}
