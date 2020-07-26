//
// Created by xc5 on 2020/7/16.
//
#include "register.h"
#include "targ_reg.h"
#include "targ_abi.h"
#include "consts.h"


ISA_REGISTER_CLASS REGISTER_CLASS_vec[ISA_REGISTER_CLASS_MAX + 1] = {
  ISA_REGISTER_CLASS_integer,
  ISA_REGISTER_CLASS_float,
  ISA_REGISTER_CLASS_rflags,
  ISA_REGISTER_CLASS_rip,
  ISA_REGISTER_CLASS_x87,
  ISA_REGISTER_CLASS_x87_cw,
  ISA_REGISTER_CLASS_mmx,
  ISA_REGISTER_CLASS_mxcsr,
};

/*
 * Cached information about each ISA_REGISTER_CLASS:
 */
ISA_REGISTER_CLASS_INFO ISA_REGISTER_CLASS_info[] = {
  // isa  sz  min      max s  ms  name          reg_name
  { 0x00,  0,   0,   0, 0, 0, "UNDEFINED", { 0 } },
  { 0x01, 64,   0,  15, 1, 0, "integer",
                                           { "%rax", "%rbx", "%rbp", "%rsp", "%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9", "%r10", "%r11", "%r12",
                                             "%r13", "%r14", "%r15" } },
  { 0x01, 64,   0,  15, 1, 0, "float",
                                           { "%ymm0", "%ymm1", "%ymm2", "%ymm3", "%ymm4", "%ymm5", "%ymm6", "%ymm7", "%ymm8", "%ymm9", "%ymm10", "%ymm11", "%ymm12",
                                             "%ymm13", "%ymm14", "%ymm15" } },
  { 0x01, 64,   0,   0, 0, 0, "rflags",
                                           { "%rflags" } },
  { 0x01, 64,   0,   0, 0, 0, "rip",
                                           { "%rip" } },
  { 0x01, 128,   0,   7, 1, 0, "x87",
                                           { "%st0", "%st1", "%st2", "%st3", "%st4", "%st5", "%st6", "%st7" } },
  { 0x01, 16,   0,   0, 0, 0, "x87_cw",
                                           { "%x87_cw" } },
  { 0x01, 64,   0,   7, 1, 0, "mmx",
                                           { "%mm0", "%mm1", "%mm2", "%mm3", "%mm4", "%mm5", "%mm6", "%mm7" } },
  { 0x01, 32,   0,   0, 1, 0, "mxcsr",
                                           { "%mxcsr" } },
};

UINT8 ISA_REGISTER_CLASS_info_index[] = {
  0,  /* ISA_REGISTER_CLASS_UNDEFINED */
  1,  /* ISA_REGISTER_CLASS_integer */
  2,  /* ISA_REGISTER_CLASS_float */
  3,  /* ISA_REGISTER_CLASS_rflags */
  4,  /* ISA_REGISTER_CLASS_rip */
  5, /* ISA_REGISTER_CLASS_x87 */
  6,/* ISA_REGISTER_CLASS_x87_cw */
  7,  /* ISA_REGISTER_CLASS_mmx */
  8,  /* ISA_REGISTER_CLASS_mxcsr */
};

//  name         rclass                count      members reg_name
ISA_REGISTER_SUBCLASS_INFO ISA_REGISTER_SUBCLASS_info[] = {
  { "UNDEFINED", ISA_REGISTER_CLASS_UNDEFINED, 0, { 0 }, { nullptr } },
  { "rax", ISA_REGISTER_CLASS_integer, 1,
                                                  { 0 },
                                                         { nullptr } },
  { "rdx", ISA_REGISTER_CLASS_integer, 1,
                                                  { 6 },
                                                         { nullptr } },
  { "rbx", ISA_REGISTER_CLASS_integer, 1,
                                                  { 1 },
                                                         { nullptr } },
  { "rcx", ISA_REGISTER_CLASS_integer, 1,
                                                  { 7 },
                                                         { nullptr } },
  { "rbp", ISA_REGISTER_CLASS_integer, 1,
                                                  { 2 },
                                                         { nullptr } },
  { "rsp", ISA_REGISTER_CLASS_integer, 1,
                                                  { 3 },
                                                         { nullptr } },
  { "rdi", ISA_REGISTER_CLASS_integer, 1,
                                                  { 4 },
                                                         { nullptr } },
  { "r11", ISA_REGISTER_CLASS_integer, 1,
                                                  { 11 },
                                                         { nullptr } },
  { "xmm0", ISA_REGISTER_CLASS_float, 1,
                                                  { 0 },
                                                         { nullptr } },
  { "m32_8bit_regs", ISA_REGISTER_CLASS_integer, 4,
                                                  { 0, 1, 7, 6 },
                                                         { nullptr } },
};


static const char* const isa_subset_names[] = {
  "x86_64",  "UNDEFINED"
};

ISA_SUBSET ISA_SUBSET_Value = ISA_SUBSET_UNDEFINED;

const char* ISA_SUBSET_Name( ISA_SUBSET subset ) {
  return isa_subset_names[(INT)subset];
}
static const unsigned char isa_subset_opcode_table[2][454] = {
  { /* x86_64 */
    0377, /* add8 add16 add32 adc32 add64 addx32 addxx32 addxxx32 */
    0377, /* addx64 addxx64 addxxx64 addi8 addi16 addi32 adci32 addi64 */
    0377, /* addxr8 addxxr8 addxxxr8 addxr8_n32 addxr16 addxxr16 addxxxr16 addxr16_n32 */
    0377, /* addxr32 addxxr32 addxxxr32 addxr32_n32 addxr64 addxxr64 addxxxr64 addxr64_off */
    0377, /* addixr8 addixxr8 addixxxr8 addixr8_n32 addixr16 addixxr16 addixxxr16 addixr16_n32 */
    0377, /* addixr32 addixxr32 addixxxr32 addixr32_n32 addixr64 addixxr64 addixxxr64 addixr64_off */
    0377, /* add128v8 addx128v8 addxx128v8 addxxx128v8 add128v16 addx128v16 addxx128v16 addxxx128v16 */
    0377, /* add128v32 addx128v32 addxx128v32 addxxx128v32 add128v64 addx128v64 addxx128v64 addxxx128v64 */
    0377, /* add64v8 add64v16 add64v32 paddsb paddsw paddq psubsb psubsw */
    0377, /* psubq paddusb paddusw psubusb psubusw pmullw pmulhw pmulhuw */
    0377, /* pmuludq pmaddwd paddsb128 paddsw128 paddq128 psubsb128 psubsw128 psubq128 */
    0377, /* paddusb128 paddusw128 psubusb128 psubusw128 pmullw128 pmulhw128 pmulhuw128 pmuludq128 */
    0377, /* pmaddwd128 and8 and16 and32 andx32 andxx32 andxxx32 and64 */
    0377, /* andx64 andxx64 andxxx64 andx8 andx16 andxx8 andxxx8 andxx16 */
    0377, /* andxxx16 and128v8 andx128v8 andxx128v8 andxxx128v8 andxr8 andxxr8 andxxxr8 */
    0377, /* andxr8_n32 andxr16 andxxr16 andxxxr16 andxr16_n32 andxr32 andxxr32 andxxxr32 */
    0377, /* andxr32_n32 andxr64 andxxr64 andxxxr64 andxr64_off andixr8 andixxr8 andixxxr8 */
    0377, /* andixr8_n32 andixr16 andixxr16 andixxxr16 andixr16_n32 andixr32 andixxr32 andixxxr32 */
    0377, /* andixr32_n32 andixr64 andixxr64 andixxxr64 andixr64_off and128v16 andx128v16 andxx128v16 */
    0377, /* andxxx128v16 and128v32 andx128v32 andxx128v32 andxxx128v32 and128v64 andx128v64 andxx128v64 */
    0377, /* andxxx128v64 andi8 andi16 andi32 andi64 bswap32 bswap64 call */
    0377, /* icall icallx icallxx icallxxx cmp8 cmpx8 cmpxx8 cmpxxx8 */
    0377, /* cmp16 cmpx16 cmpxx16 cmpxxx16 cmp32 cmpx32 cmpxx32 cmpxxx32 */
    0377, /* cmp64 cmpx64 cmpxx64 cmpxxx64 cmpi8 cmpxr8 cmpxi8 cmpxxr8 */
    0377, /* cmpxxi8 cmpxxxr8 cmpxxxi8 cmpi16 cmpxr16 cmpxi16 cmpxxr16 cmpxxi16 */
    0377, /* cmpxxxr16 cmpxxxi16 cmpi32 cmpxr32 cmpxi32 cmpxxr32 cmpxxi32 cmpxxxr32 */
    0377, /* cmpxxxi32 cmpi64 cmpxr64 cmpxi64 cmpxxr64 cmpxxi64 cmpxxxr64 cmpxxxi64 */
    0377, /* cmovb cmovae cmovp cmovnp cmove cmovne cmovbe cmova */
    0377, /* cmovl cmovge cmovle cmovg cmovs cmovns div32 div64 */
    0377, /* enter idiv32 idiv64 imul32 imulx32 imul64 imuli32 imuli64 */
    0377, /* imulx64 mul128v16 inc8 dec8 inc16 dec16 inc32 dec32 */
    0377, /* inc64 dec64 incxr8 incxxr8 incxxxr8 incxr8_n32 incxr16 incxxr16 */
    0377, /* incxxxr16 incxr16_n32 incxr32 incxxr32 incxxxr32 incxr32_n32 incxr64 incxxr64 */
    0377, /* incxxxr64 incxr64_off decxr8 decxxr8 decxxxr8 decxr8_n32 decxr16 decxxr16 */
    0377, /* decxxxr16 decxr16_n32 decxr32 decxxr32 decxxxr32 decxr32_n32 decxr64 decxxr64 */
    0377, /* decxxxr64 decxr64_off jb jae jp jnp je jne */
    0377, /* jbe ja jl jge jle jg jcxz jecxz */
    0377, /* jrcxz js jns jmp ijmp ijmpx ijmpxx ijmpxxx */
    0377, /* ld64 ldx64 ldxx64 ld64_2m ld64_2sse lea32 lea64 leax32 */
    0377, /* leax64 leaxx32 leaxx64 leave ldc32 ldc64 mul32 mulx64 */
    0377, /* mov32 mov64 mov64_m ld32_64_off ld64_off store64_off storei64_off ld8_32_n32 */
    0377, /* ldu8_32_n32 ld16_32_n32 ldu16_32_n32 ld32_n32 ldss_n32 ldsd_n32 ldaps_n32 ldapd_n32 */
    0377, /* ldups_n32 ldupd_n32 lddqa_n32 lddqu_n32 ldlps_n32 ldlpd_n32 ldhps_n32 ldhpd_n32 */
    0377, /* ld64_2m_n32 ld64_2sse_n32 store8_n32 storei8_n32 store16_n32 storei16_n32 store32_n32 storei32_n32 */
    0377, /* stss_n32 stsd_n32 staps_n32 stapd_n32 stdqa_n32 stdqu_n32 stlps_n32 sthps_n32 */
    0377, /* stlpd_n32 sthpd_n32 store64_fm_n32 store64_fsse_n32 ld32_gs_seg_off ld64_fs_seg_off movsbl ld8_32 */
    0377, /* ldx8_32 ldxx8_32 movzbl ldu8_32 ldxu8_32 ldxxu8_32 movswl ld16_32 */
    0377, /* ldx16_32 ldxx16_32 movzwl ldu16_32 ldxu16_32 ldxxu16_32 movsbq ld8_64 */
    0377, /* ldx8_64 ldxx8_64 ld8_64_off movzbq ldu8_64 ldxu8_64 ldxxu8_64 ldu8_64_off */
    0377, /* movswq ld16_64 ldx16_64 ldxx16_64 ld16_64_off movzwq ldu16_64 ldxu16_64 */
    0377, /* ldxxu16_64 ldu16_64_off movslq ld32_64 ldx32_64 ldxx32_64 ld32 ldx32 */
    0377, /* ldxx32 movzlq neg8 neg16 neg32 neg64 negxr8 negxxr8 */
    0377, /* negxxxr8 negxr8_n32 negxr16 negxxr16 negxxxr16 negxr16_n32 negxr32 negxxr32 */
    0377, /* negxxxr32 negxr32_n32 negxr64 negxxr64 negxxxr64 negxr64_off not8 not16 */
    0377, /* not32 not64 notxr8 notxxr8 notxxxr8 notxr8_n32 notxr16 notxxr16 */
    0377, /* notxxxr16 notxr16_n32 notxr32 notxxr32 notxxxr32 notxr32_n32 notxr64 notxxr64 */
    0377, /* notxxxr64 notxr64_off or8 or16 or32 orx32 orxx32 orxxx32 */
    0377, /* or64 orx64 orxx64 orxxx64 orx8 orx16 orxx8 orxxx8 */
    0377, /* orxx16 orxxx16 orxr8 orxxr8 orxxxr8 orxr8_n32 orxr16 orxxr16 */
    0377, /* orxxxr16 orxr16_n32 orxr32 orxxr32 orxxxr32 orxr32_n32 orxr64 orxxr64 */
    0377, /* orxxxr64 orxr64_off orixr8 orixxr8 orixxxr8 orixr8_n32 orixr16 orixxr16 */
    0377, /* orixxxr16 orixr16_n32 orixr32 orixxr32 orixxxr32 orixr32_n32 orixr64 orixxr64 */
    0377, /* orixxxr64 orixr64_off or128v8 orx128v8 orxx128v8 orxxx128v8 or128v16 orx128v16 */
    0377, /* orxx128v16 orxxx128v16 or128v32 orx128v32 orxx128v32 orxxx128v32 or128v64 orx128v64 */
    0377, /* orxx128v64 orxxx128v64 ori8 ori16 ori32 ori64 popl popq */
    0377, /* pushl pushq ret reti ror8 ror16 ror32 ror64 */
    0377, /* rori8 rori16 rori32 rori64 rol8 rol16 rol32 rol64 */
    0377, /* roli8 roli16 roli32 roli64 prefetch prefetchw prefetcht0 prefetcht1 */
    0377, /* prefetchnta prefetchx prefetchxx prefetchwx prefetchwxx prefetcht0x prefetcht0xx prefetcht1x */
    0377, /* prefetcht1xx prefetchntax prefetchntaxx setb setae setp setnp sete */
    0377, /* setne setbe seta setl setge setle setg setc */
    0377, /* seto sets setz store8 storei8 storex8 storeix8 storexx8 */
    0377, /* storeixx8 store16 storei16 storex16 storeix16 storexx16 storeixx16 store32 */
    0377, /* storei32 storex32 storeix32 storexx32 storeixx32 store64 storei64 storex64 */
    0377, /* storeix64 storexx64 storeixx64 store64_fm store64_fsse storenti32 storentix32 storentixx32 */
    0377, /* storenti64 storentix64 storentixx64 storenti128 sar32 sar64 sari32 sari64 */
    0377, /* shl32 shld32 shldi32 shrd32 shrdi32 shl64 shli32 shli64 */
    0377, /* shr32 shr64 shri32 shri64 sub8 sub16 sub32 sbb32 */
    0377, /* sub64 subx32 subx64 subxx32 subxxx32 subxx64 subxxx64 subi8 */
    0377, /* subi16 subi32 sbbi32 subi64 subxr8 subxxr8 subxxxr8 subxr8_n32 */
    0377, /* subxr16 subxxr16 subxxxr16 subxr16_n32 subxr32 subxxr32 subxxxr32 subxr32_n32 */
    0377, /* subxr64 subxxr64 subxxxr64 subxr64_off subixr8 subixxr8 subixxxr8 subixr8_n32 */
    0377, /* subixr16 subixxr16 subixxxr16 subixr16_n32 subixr32 subixxr32 subixxxr32 subixr32_n32 */
    0377, /* subixr64 subixxr64 subixxxr64 subixr64_off sub128v8 subx128v8 subxx128v8 subxxx128v8 */
    0377, /* sub128v16 subx128v16 subxx128v16 subxxx128v16 sub128v32 subx128v32 subxx128v32 subxxx128v32 */
    0377, /* sub128v64 subx128v64 subxx128v64 subxxx128v64 sub64v8 sub64v16 sub64v32 test8 */
    0377, /* testx8 testxx8 testxxx8 test16 testx16 testxx16 testxxx16 test32 */
    0377, /* testx32 testxx32 testxxx32 test64 testx64 testxx64 testxxx64 testi8 */
    0377, /* testi16 testi32 testi64 xor8 xor16 xor32 xorx32 xorxx32 */
    0377, /* xorxxx32 xor64 xorx64 xorxx64 xorxxx64 xorx8 xorx16 xorxx8 */
    0377, /* xorxxx8 xorxx16 xorxxx16 xorxr8 xorxxr8 xorxxxr8 xorxr8_n32 xorxr16 */
    0377, /* xorxxr16 xorxxxr16 xorxr16_n32 xorxr32 xorxxr32 xorxxxr32 xorxr32_n32 xorxr64 */
    0377, /* xorxxr64 xorxxxr64 xorxr64_off xorixr8 xorixxr8 xorixxxr8 xorixr8_n32 xorixr16 */
    0377, /* xorixxr16 xorixxxr16 xorixr16_n32 xorixr32 xorixxr32 xorixxxr32 xorixr32_n32 xorixr64 */
    0377, /* xorixxr64 xorixxxr64 xorixr64_off xor128v8 xorx128v8 xorxx128v8 xorxxx128v8 xor128v16 */
    0377, /* xorx128v16 xorxx128v16 xorxxx128v16 xor128v32 xorx128v32 xorxx128v32 xorxxx128v32 xor128v64 */
    0377, /* xorx128v64 xorxx128v64 xorxxx128v64 xori8 xori16 xori32 xori64 fxor128v32 */
    0377, /* fxorx128v32 fxorxx128v32 fxorxxx128v32 fxor128v64 fxorx128v64 fxorxx128v64 fxorxxx128v64 xorps */
    0377, /* xorpd addsd addss addxsd addxss addxxsd addxxxsd addxxss */
    0377, /* addxxxss faddsub128v32 faddsubx128v32 faddsubxx128v32 faddsubxxx128v32 faddsub128v64 faddsubx128v64 faddsubxx128v64 */
    0377, /* faddsubxxx128v64 fadd128v32 faddx128v32 faddxx128v32 faddxxx128v32 fadd128v64 faddx128v64 faddxx128v64 */
    0377, /* faddxxx128v64 fhadd128v32 fhaddx128v32 fhaddxx128v32 fhaddxxx128v32 fhadd128v64 fhaddx128v64 fhaddxx128v64 */
    0377, /* fhaddxxx128v64 fand128v32 fandx128v32 fandxx128v32 fandxxx128v32 fand128v64 fandx128v64 fandxx128v64 */
    0377, /* fandxxx128v64 andps andpd andnps andnpd for128v32 forx128v32 forxx128v32 */
    0377, /* forxxx128v32 for128v64 forx128v64 forxx128v64 forxxx128v64 orps orpd comisd */
    0377, /* comixsd comixxsd comixxxsd comiss comixss comixxss comixxxss cmpss */
    0377, /* cmpsd cmpps cmppd cmpeq128v8 cmpeq128v16 cmpeq128v32 cmpeqx128v8 cmpeqx128v16 */
    0377, /* cmpeqx128v32 cmpeqxx128v8 cmpeqxx128v16 cmpeqxx128v32 cmpeqxxx128v8 cmpeqxxx128v16 cmpeqxxx128v32 cmpgt128v8 */
    0377, /* cmpgt128v16 cmpgt128v32 cmpgtx128v8 cmpgtx128v16 cmpgtx128v32 cmpgtxx128v8 cmpgtxx128v16 cmpgtxx128v32 */
    0377, /* cmpgtxxx128v8 cmpgtxxx128v16 cmpgtxxx128v32 pcmpeqb pcmpeqw pcmpeqd pcmpgtb pcmpgtw */
    0377, /* pcmpgtd fmovsldup fmovshdup fmovddup fmovsldupx fmovshdupx fmovddupx fmovsldupxx */
    0377, /* fmovshdupxx fmovddupxx fmovsldupxxx fmovshdupxxx fmovddupxxx cltd cqto cvtss2sd */
    0377, /* cvtsd2ss cvtsd2ss_x cvtsd2ss_xx cvtsd2ss_xxx cvtsi2sd cvtsi2sd_x cvtsi2sd_xx cvtsi2sd_xxx */
    0377, /* cvtsi2ss cvtsi2ss_x cvtsi2ss_xx cvtsi2ss_xxx cvtsi2sdq cvtsi2sdq_x cvtsi2sdq_xx cvtsi2sdq_xxx */
    0377, /* cvtsi2ssq cvtsi2ssq_x cvtsi2ssq_xx cvtsi2ssq_xxx cvtss2si cvtsd2si cvtss2siq cvtsd2siq */
    0377, /* cvttss2si cvttsd2si cvttss2siq cvttsd2siq cvtdq2pd cvtdq2ps cvtps2pd cvtpd2ps */
    0377, /* cvtps2dq cvttps2dq cvtpd2dq cvttpd2dq cvtdq2pd_x cvtdq2ps_x cvtps2pd_x cvtpd2ps_x */
    0377, /* cvtps2dq_x cvtpd2dq_x cvttps2dq_x cvttpd2dq_x cvtdq2pd_xx cvtdq2ps_xx cvtps2pd_xx cvtpd2ps_xx */
    0377, /* cvtps2dq_xx cvtpd2dq_xx cvttps2dq_xx cvttpd2dq_xx cvtdq2pd_xxx cvtdq2ps_xxx cvtps2pd_xxx cvtpd2ps_xxx */
    0377, /* cvtps2dq_xxx cvtpd2dq_xxx cvttps2dq_xxx cvttpd2dq_xxx cvtpi2ps cvtps2pi cvttps2pi cvtpi2pd */
    0377, /* cvtpd2pi cvttpd2pi ldsd ldsdx ldsdxx ldss ldssx ldssxx */
    0377, /* lddqa lddqu ldlps ldhps ldlpd ldhpd stdqa stdqu */
    0377, /* stlps sthps stlpd storelpd sthpd stntpd stntps storent64_fm */
    0377, /* lddqax lddqux ldlpsx ldhpsx ldlpdx ldhpdx stdqax stntpdx */
    0377, /* stntpsx stdqux stlpsx sthpsx stlpdx sthpdx lddqaxx lddquxx */
    0377, /* ldlpsxx ldhpsxx ldlpdxx ldhpdxx ldaps ldapsx ldapsxx ldapd */
    0377, /* ldapdx ldapdxx ldups ldupsx ldupsxx ldupd ldupdx ldupdxx */
    0377, /* stdqaxx stntpdxx stntpsxx stdquxx stlpsxx sthpsxx stlpdxx sthpdxx */
    0377, /* staps stapsx stapsxx stapd stapdx stapdxx stups stupsx */
    0377, /* stupsxx stups_n32 stupd stupdx stupdxx stupd_n32 maxsd maxss */
    0377, /* fmax128v32 fmaxx128v32 fmaxxx128v32 fmaxxxx128v32 fmax128v64 fmaxx128v64 fmaxxx128v64 fmaxxxx128v64 */
    0377, /* max64v8 max64v16 min64v8 min64v16 minsd minss fmin128v32 fminx128v32 */
    0377, /* fminxx128v32 fminxxx128v32 fmin128v64 fminx128v64 fminxx128v64 fminxxx128v64 movx2g64 movx2g */
    0377, /* movg2x64 movg2x movsd movss movdq movapd movaps movq2dq */
    0377, /* movdq2q divsd divxsd divxxsd divxxxsd divss divxss divxxss */
    0377, /* divxxxss fdiv128v32 fdivx128v32 fdivxx128v32 fdivxxx128v32 fdiv128v64 fdivx128v64 fdivxx128v64 */
    0377, /* fdivxxx128v64 mulsd mulss fmul128v32 fmulx128v32 fmulxx128v32 fmulxxx128v32 fmul128v64 */
    0377, /* fmulx128v64 fmulxx128v64 fmulxxx128v64 mulxsd mulxss mulxxsd mulxxxsd mulxxss */
    0377, /* mulxxxss subsd subss subxsd subxss subxxsd subxxxsd subxxss */
    0377, /* subxxxss ucomisd ucomixsd ucomixxsd ucomixxxsd ucomiss ucomixss ucomixxss */
    0377, /* ucomixxxss fsub128v32 fsubx128v32 fsubxx128v32 fsubxxx128v32 fsub128v64 fsubx128v64 fsubxx128v64 */
    0377, /* fsubxxx128v64 fhsub128v32 fhsubx128v32 fhsubxx128v32 fhsubxxx128v32 fhsub128v64 fhsubx128v64 fhsubxx128v64 */
    0377, /* fhsubxxx128v64 stss stntss stssx stntssx stssxx stntssxx stsd */
    0377, /* stntsd stsdx stntsdx stsdxx stntsdxx rcpss frcp128v32 sqrtsd */
    0377, /* sqrtss rsqrtss fsqrt128v32 frsqrt128v32 fsqrt128v64 punpcklwd punpcklbw punpckldq */
    0377, /* punpcklbw128 punpcklwd128 punpckldq128 punpckhbw punpckhwd punpckhdq punpckhbw128 punpckhwd128 */
    0377, /* punpckhdq128 punpcklqdq punpckhqdq packsswb packssdw packuswb packsswb128 packssdw128 */
    0377, /* packuswb128 pshufd pshufw pshuflw pshufhw pslldq psllw psllwi */
    0377, /* pslld pslldi psllq psllqi psrlw psrlwi psrld psrldi */
    0377, /* psrlq psrlqi psraw psrawi psrad psradi psllw_mmx psllwi_mmx */
    0377, /* pslld_mmx pslldi_mmx psllq_mmx psllqi_mmx psrlw_mmx psrlwi_mmx psrld_mmx psrldi_mmx */
    0377, /* psrlq_mmx psrlqi_mmx psraw_mmx psrawi_mmx psrad_mmx psradi_mmx pand_mmx pandn_mmx */
    0377, /* por_mmx pxor_mmx pand pandn por pxor unpckhpd unpckhps */
    0377, /* unpcklpd unpcklps shufpd shufps movhlps movlhps psrldq psrlq128v64 */
    0377, /* subus128v16 pavgb pavgw psadbw pavgb128 pavgw128 psadbw128 pextrw */
    0377, /* pinsrw pmovmskb pmovmskb128 movi32_2m movi64_2m movm_2i32 movm_2i64 pshufw64v16 */
    0377, /* movmskps movmskpd maskmovdqu maskmovq extrq insertq vfmaddss vfmaddxss */
    0377, /* vfmaddxxss vfmaddxxxss vfmaddxrss vfmaddxxrss vfmaddxxxrss vfmaddsd vfmaddxsd vfmaddxxsd */
    0377, /* vfmaddxxxsd vfmaddxrsd vfmaddxxrsd vfmaddxxxrsd vfnmaddss vfnmaddxss vfnmaddxxss vfnmaddxxxss */
    0377, /* vfnmaddxrss vfnmaddxxrss vfnmaddxxxrss vfnmaddsd vfnmaddxsd vfnmaddxxsd vfnmaddxxxsd vfnmaddxrsd */
    0377, /* vfnmaddxxrsd vfnmaddxxxrsd vfmaddps vfmaddxps vfmaddxxps vfmaddxxxps vfmaddxrps vfmaddxxrps */
    0377, /* vfmaddxxxrps vfmaddpd vfmaddxpd vfmaddxxpd vfmaddxxxpd vfmaddxrpd vfmaddxxrpd vfmaddxxxrpd */
    0377, /* vfmaddsubps vfmaddsubxps vfmaddsubxxps vfmaddsubxxxps vfmaddsubxrps vfmaddsubxxrps vfmaddsubxxxrps vfmaddsubpd */
    0377, /* vfmaddsubxpd vfmaddsubxxpd vfmaddsubxxxpd vfmaddsubxrpd vfmaddsubxxrpd vfmaddsubxxxrpd vfnmaddps vfnmaddxps */
    0377, /* vfnmaddxxps vfnmaddxxxps vfnmaddxrps vfnmaddxxrps vfnmaddxxxrps vfnmaddpd vfnmaddxpd vfnmaddxxpd */
    0377, /* vfnmaddxxxpd vfnmaddxrpd vfnmaddxxrpd vfnmaddxxxrpd vfmsubss vfmsubxss vfmsubxxss vfmsubxxxss */
    0377, /* vfmsubxrss vfmsubxxrss vfmsubxxxrss vfmsubsd vfmsubxsd vfmsubxxsd vfmsubxxxsd vfmsubxrsd */
    0377, /* vfmsubxxrsd vfmsubxxxrsd vfnmsubss vfnmsubxss vfnmsubxxss vfnmsubxxxss vfnmsubxrss vfnmsubxxrss */
    0377, /* vfnmsubxxxrss vfnmsubsd vfnmsubxsd vfnmsubxxsd vfnmsubxxxsd vfnmsubxrsd vfnmsubxxrsd vfnmsubxxxrsd */
    0377, /* vfmsubps vfmsubxps vfmsubxxps vfmsubxxxps vfmsubxrps vfmsubxxrps vfmsubxxxrps vfmsubpd */
    0377, /* vfmsubxpd vfmsubxxpd vfmsubxxxpd vfmsubxrpd vfmsubxxrpd vfmsubxxxrpd vfmsubaddps vfmsubaddxps */
    0377, /* vfmsubaddxxps vfmsubaddxxxps vfmsubaddxrps vfmsubaddxxrps vfmsubaddxxxrps vfmsubaddpd vfmsubaddxpd vfmsubaddxxpd */
    0377, /* vfmsubaddxxxpd vfmsubaddxrpd vfmsubaddxxrpd vfmsubaddxxxrpd vfnmsubps vfnmsubxps vfnmsubxxps vfnmsubxxxps */
    0377, /* vfnmsubxrps vfnmsubxxrps vfnmsubxxxrps vfnmsubpd vfnmsubxpd vfnmsubxxpd vfnmsubxxxpd vfnmsubxrpd */
    0377, /* vfnmsubxxrpd vfnmsubxxxrpd vzeroupper mfence lfence sfence monitor mwait */
    0377, /* asm intrncall spadjust savexmms zero32 zero64 xzero32 xzero64 */
    0377, /* xzero128v32 xzero128v64 fadd faddp flds flds_n32 fldl fldl_n32 */
    0377, /* fldt fldt_n32 fld fst fstp fstps fstps_n32 fstpl */
    0377, /* fstpl_n32 fstpt fstpt_n32 fsts fsts_n32 fstl fstl_n32 fxch */
    0377, /* fmov fsub fsubr fsubp fsubrp fmul fmulp fdiv */
    0377, /* fdivp fdivr fdivrp fucomi fucomip fchs frndint fnstcw */
    0377, /* fldcw fistps fistpl fists fistl fistpll filds fildl */
    0377, /* fildll fldz fabs fsqrt fcmovb fcmovbe fcmovnb fcmovnbe */
    0377, /* fcmove fcmovne fcmovu fcmovnu fcos fsin cmpeqpd cmpltpd */
    0377, /* cmplepd cmpunordpd cmpneqpd cmpnltpd cmpnlepd cmpordpd cmpeqps cmpltps */
    0377, /* cmpleps cmpunordps cmpneqps cmpnltps cmpnleps cmpordps cmpeqsd cmpltsd */
    0377, /* cmplesd cmpunordsd cmpneqsd cmpnltsd cmpnlesd cmpordsd cmpeqss cmpltss */
    0377, /* cmpless cmpunordss cmpneqss cmpnltss cmpnless cmpordss emms stmxcsr */
    0377, /* ldmxcsr clflush fisttps fisttpl fisttpll pabs128v8 pabsx128v8 pabsxx128v8 */
    0377, /* pabsxxx128v8 pabs128v16 pabsx128v16 pabsxx128v16 pabsxxx128v16 pabs128v32 pabsx128v32 pabsxx128v32 */
    0377, /* pabsxxx128v32 psign128v8 psignx128v8 psignxx128v8 psignxxx128v8 psign128v16 psignx128v16 psignxx128v16 */
    0377, /* psignxxx128v16 psign128v32 psignx128v32 psignxx128v32 psignxxx128v32 pshuf128v8 pshufx128v8 pshufxx128v8 */
    0377, /* pshufxxx128v8 phsub128v16 phsubx128v16 phsubxx128v16 phsubxxx128v16 phsub128v32 phsubx128v32 phsubxx128v32 */
    0377, /* phsubxxx128v32 phsubs128v16 phsubsx128v16 phsubsxx128v16 phsubsxxx128v16 phadd128v16 phaddx128v16 phaddxx128v16 */
    0377, /* phaddxxx128v16 phadd128v32 phaddx128v32 phaddxx128v32 phaddxxx128v32 phadds128v16 phaddsx128v16 phaddsxx128v16 */
    0377, /* phaddsxxx128v16 pmulhrsw128 pmulhrswx128 pmulhrswxx128 pmulhrswxxx128 pmaddubsw128 pmaddubswx128 pmaddubswxx128 */
    0377, /* pmaddubswxxx128 palignr128 palignrx128 palignrxx128 palignrxxx128 muldq muldqx muldqxx */
    0377, /* muldqxxx ldntdqa ldntdqax ldntdqaxx stntdq stntdqx stntdqxx minu128v8 */
    0377, /* minux128v8 minuxx128v8 minuxxx128v8 mins128v8 minsx128v8 minsxx128v8 minsxxx128v8 maxu128v8 */
    0377, /* maxux128v8 maxuxx128v8 maxuxxx128v8 maxs128v8 maxsx128v8 maxsxx128v8 maxsxxx128v8 mins128v16 */
    0377, /* minsx128v16 minsxx128v16 minsxxx128v16 maxs128v16 maxsx128v16 maxsxx128v16 maxsxxx128v16 minu128v16 */
    0377, /* minux128v16 minuxx128v16 minuxxx128v16 maxu128v16 maxux128v16 maxuxx128v16 maxuxxx128v16 minu128v32 */
    0377, /* minux128v32 minuxx128v32 minuxxx128v32 maxu128v32 maxux128v32 maxuxx128v32 maxuxxx128v32 mins128v32 */
    0377, /* minsx128v32 minsxx128v32 minsxxx128v32 maxs128v32 maxsx128v32 maxsxx128v32 maxsxxx128v32 pmovsxbw */
    0377, /* pmovsxbwx pmovsxbwxx pmovsxbwxxx pmovzxbw pmovzxbwx pmovzxbwxx pmovzxbwxxx pmovsxbd */
    0377, /* pmovsxbdx pmovsxbdxx pmovsxbdxxx pmovzxbd pmovzxbdx pmovzxbdxx pmovzxbdxxx pmovsxbq */
    0377, /* pmovsxbqx pmovsxbqxx pmovsxbqxxx pmovzxbq pmovzxbqx pmovzxbqxx pmovzxbqxxx pmovsxwd */
    0377, /* pmovsxwdx pmovsxwdxx pmovsxwdxxx pmovzxwd pmovzxwdx pmovzxwdxx pmovzxwdxxx pmovsxwq */
    0377, /* pmovsxwqx pmovsxwqxx pmovsxwqxxx pmovzxwq pmovzxwqx pmovzxwqxx pmovzxwqxxx pmovsxdq */
    0377, /* pmovsxdqx pmovsxdqxx pmovsxdqxxx pmovzxdq pmovzxdqx pmovzxdqxx pmovzxdqxxx mul128v32 */
    0377, /* mulx128v32 mulxx128v32 mulxxx128v32 cmpeq128v64 cmpeqx128v64 cmpeqxx128v64 cmpeqxxx128v64 packusdw */
    0377, /* packusdwx packusdwxx packusdwxxx phminposuw phminposuwx phminposuwxx phminposuwxxx ptest128 */
    0377, /* ptestx128 ptestxx128 ptestxxx128 roundsd roundxsd roundxxsd roundxxxsd mpsadbw */
    0377, /* mpsadbwx mpsadbwxx mpsadbwxxx insr128v8 insrx128v8 insrxx128v8 insrxxx128v8 insr128v16 */
    0377, /* insrx128v16 insrxx128v16 insrxxx128v16 insr128v32 insrx128v32 insrxx128v32 insrxxx128v32 insr128v64 */
    0377, /* insrx128v64 insrxx128v64 insrxxx128v64 extr128v8 extrx128v8 extrxx128v8 extrxxx128v8 extr128v16 */
    0377, /* extrx128v16 extrxx128v16 extrxxx128v16 extr128v32 extrx128v32 extrxx128v32 extrxxx128v32 extr128v64 */
    0377, /* extrx128v64 extrxx128v64 extrxxx128v64 finsr128v32 finsrx128v32 finsrxx128v32 finsrxxx128v32 fextr128v32 */
    0377, /* fextrx128v32 fextrxx128v32 fextrxxx128v32 fblendv128v32 fblendvx128v32 fblendvxx128v32 fblendvxxx128v32 fblendv128v64 */
    0377, /* fblendvx128v64 fblendvxx128v64 fblendvxxx128v64 blendv128v8 blendvx128v8 blendvxx128v8 blendvxxx128v8 round128v32 */
    0377, /* roundx128v32 roundxx128v32 roundxxx128v32 roundss roundxss roundxxss roundxxxss fblend128v64 */
    0377, /* fblendx128v64 fblendxx128v64 fblendxxx128v64 blend128v16 blendx128v16 blendxx128v16 blendxxx128v16 fdp128v32 */
    0377, /* fdpx128v32 fdpxx128v32 fdpxxx128v32 fdp128v64 fdpx128v64 fdpxx128v64 fdpxxx128v64 round128v64 */
    0377, /* roundx128v64 roundxx128v64 roundxxx128v64 fblend128v32 fblendx128v32 fblendxx128v32 fblendxxx128v32 cmpgt128v64 */
    0377, /* cmpgtx128v64 cmpgtxx128v64 cmpgtxxx128v64 crc32w crc32wx crc32wxx crc32wxxx crc32d */
    0377, /* crc32dx crc32dxx crc32dxxx crc32q crc32qx crc32qxx crc32qxxx crc32b */
    0377, /* crc32bx crc32bxx crc32bxxx cmpestrm cmpestrmx cmpestrmxx cmpestrmxxx cmpestri */
    0377, /* cmpestrix cmpestrixx cmpestrixxx cmpistrm cmpistrmx cmpistrmxx cmpistrmxxx cmpistri */
    0377, /* cmpistrix cmpistrixx cmpistrixxx popcnt16 popcntx16 popcntxx16 popcntxxx16 popcnt32 */
    0377, /* popcntx32 popcntxx32 popcntxxx32 popcnt64 popcntx64 popcntxx64 popcntxxx64 aesimc */
    0377, /* aesimcx aesimcxx aesimcxxx aeskeygenassist aeskeygenassistx aeskeygenassistxx aeskeygenassistxxx aesenc */
    0377, /* aesencx aesencxx aesencxxx aesenclast aesenclastx aesenclastxx aesenclastxxx aesdec */
    0377, /* aesdecx aesdecxx aesdecxxx aesdeclast aesdeclastx aesdeclastxx aesdeclastxxx pclmulqdq */
    0377, /* pclmulqdqx pclmulqdqxx pclmulqdqxxx vphaddbd vphaddbdx vphaddbdxx vphaddbdxxx vphaddbq */
    0377, /* vphaddbqx vphaddbqxx vphaddbqxxx vphaddbw vphaddbwx vphaddbwxx vphaddbwxxx vphadddq */
    0377, /* vphadddqx vphadddqxx vphadddqxxx vphaddubd vphaddubdx vphaddubdxx vphaddubdxxx vphaddubq */
    0377, /* vphaddubqx vphaddubqxx vphaddubqxxx vphaddubw vphaddubwx vphaddubwxx vphaddubwxxx vphaddudq */
    0377, /* vphaddudqx vphaddudqxx vphaddudqxxx vphadduwd vphadduwdx vphadduwdxx vphadduwdxxx vphadduwq */
    0377, /* vphadduwqx vphadduwqxx vphadduwqxxx vphaddwd vphaddwdx vphaddwdxx vphaddwdxxx vphaddwq */
    0377, /* vphaddwqx vphaddwqxx vphaddwqxxx vphsubbw vphsubbwx vphsubbwxx vphsubbwxxx vphsubdq */
    0377, /* vphsubdqx vphsubdqxx vphsubdqxxx vphsubwd vphsubwdx vphsubwdxx vphsubwdxxx vfrczpd */
    0377, /* vfrczpdx vfrczpdxx vfrczpdxxx vfrczps vfrczpsx vfrczpsxx vfrczpsxxx vfrczsd */
    0377, /* vfrczsdx vfrczsdxx vfrczsdxxx vfrczss vfrczssx vfrczssxx vfrczssxxx vprotbi */
    0377, /* vprotbix vprotbixx vprotbixxx vprotdi vprotdix vprotdixx vprotdixxx vprotqi */
    0377, /* vprotqix vprotqixx vprotqixxx vprotwi vprotwix vprotwixx vprotwixxx vpcomb */
    0377, /* vpcombx vpcombxx vpcombxxx vpcomd vpcomdx vpcomdxx vpcomdxxx vpcomq */
    0377, /* vpcomqx vpcomqxx vpcomqxxx vpcomw vpcomwx vpcomwxx vpcomwxxx vpcomub */
    0377, /* vpcomubx vpcomubxx vpcomubxxx vpcomud vpcomudx vpcomudxx vpcomudxxx vpcomuq */
    0377, /* vpcomuqx vpcomuqxx vpcomuqxxx vpcomuw vpcomuwx vpcomuwxx vpcomuwxxx vpperm */
    0377, /* vppermx vppermxx vppermxxx vppermxr vppermxxr vppermxxxr vprotb vprotbx */
    0377, /* vprotbxx vprotbxxx vprotbxr vprotbxxr vprotbxxxr vprotd vprotdx vprotdxx */
    0377, /* vprotdxxx vprotdxr vprotdxxr vprotdxxxr vprotq vprotqx vprotqxx vprotqxxx */
    0377, /* vprotqxr vprotqxxr vprotqxxxr vprotw vprotwx vprotwxx vprotwxxx vprotwxr */
    0377, /* vprotwxxr vprotwxxxr vpshab vpshabx vpshabxx vpshabxxx vpshabxr vpshabxxr */
    0377, /* vpshabxxxr vpshad vpshadx vpshadxx vpshadxxx vpshadxr vpshadxxr vpshadxxxr */
    0377, /* vpshaq vpshaqx vpshaqxx vpshaqxxx vpshaqxr vpshaqxxr vpshaqxxxr vpshaw */
    0377, /* vpshawx vpshawxx vpshawxxx vpshawxr vpshawxxr vpshawxxxr vpshlb vpshlbx */
    0377, /* vpshlbxx vpshlbxxx vpshlbxr vpshlbxxr vpshlbxxxr vpshld vpshldx vpshldxx */
    0377, /* vpshldxxx vpshldxr vpshldxxr vpshldxxxr vpshlq vpshlqx vpshlqxx vpshlqxxx */
    0377, /* vpshlqxr vpshlqxxr vpshlqxxxr vpshlw vpshlwx vpshlwxx vpshlwxxx vpshlwxr */
    0377, /* vpshlwxxr vpshlwxxxr vpcmov vpcmovx vpcmovxx vpcmovxxx vpcmovxr vpcmovxxr */
    0377, /* vpcmovxxxr vpmacsdd vpmacsddx vpmacsddxx vpmacsddxxx vpmacsdqh vpmacsdqhx vpmacsdqhxx */
    0377, /* vpmacsdqhxxx vpmacsdql vpmacsdqlx vpmacsdqlxx vpmacsdqlxxx vpmacssdd vpmacssddx vpmacssddxx */
    0377, /* vpmacssddxxx vpmacssdqh vpmacssdqhx vpmacssdqhxx vpmacssdqhxxx vpmacssdql vpmacssdqlx vpmacssdqlxx */
    0377, /* vpmacssdqlxxx vpmacsswd vpmacsswdx vpmacsswdxx vpmacsswdxxx vpmacssww vpmacsswwx vpmacsswwxx */
    0377, /* vpmacsswwxxx vpmacswd vpmacswdx vpmacswdxx vpmacswdxxx vpmacsww vpmacswwx vpmacswwxx */
    0377, /* vpmacswwxxx vpmadcsswd vpmadcsswdx vpmadcsswdxx vpmadcsswdxxx vpmadcswd vpmadcswdx vpmadcswdxx */
    0377, /* vpmadcswdxxx vfadd128v64 vfaddx128v64 vfaddxx128v64 vfaddxxx128v64 vfadd128v32 vfaddx128v32 vfaddxx128v32 */
    0377, /* vfaddxxx128v32 vfaddsd vfaddxsd vfaddxxsd vfaddxxxsd vfaddss vfaddxss vfaddxxss */
    0377, /* vfaddxxxss vfaddsub128v64 vfaddsubx128v64 vfaddsubxx128v64 vfaddsubxxx128v64 vfaddsub128v32 vfaddsubx128v32 vfaddsubxx128v32 */
    0377, /* vfaddsubxxx128v32 vaesenc vaesencx vaesencxx vaesencxxx vaesenclast vaesenclastx vaesenclastxx */
    0377, /* vaesenclastxxx vaesdec vaesdecx vaesdecxx vaesdecxxx vaesdeclast vaesdeclastx vaesdeclastxx */
    0377, /* vaesdeclastxxx vaesimc vaesimcx vaesimcxx vaesimcxxx vaeskeygenassist vaeskeygenassistx vaeskeygenassistxx */
    0377, /* vaeskeygenassistxxx vandpd vfand128v64 vfandx128v64 vfandxx128v64 vfandxxx128v64 vandps vfand128v32 */
    0377, /* vfandx128v32 vfandxx128v32 vfandxxx128v32 vandnpd vfandn128v64 vfandnx128v64 vfandnxx128v64 vfandnxxx128v64 */
    0377, /* vandnps vfandn128v32 vfandnx128v32 vfandnxx128v32 vfandnxxx128v32 vfblend128v64 vfblendx128v64 vfblendxx128v64 */
    0377, /* vfblendxxx128v64 vfblend128v32 vfblendx128v32 vfblendxx128v32 vfblendxxx128v32 vfblendv128v64 vfblendvx128v64 vfblendvxx128v64 */
    0377, /* vfblendvxxx128v64 vfblendv128v32 vfblendvx128v32 vfblendvxx128v32 vfblendvxxx128v32 vfbroadcastss vfbroadcastxss vfbroadcastxxss */
    0377, /* vfbroadcastsd vfbroadcastxsd vfbroadcastxxsd vfbroadcastf128 vfbroadcastxf128 vfbroadcastxxf128 vcmppd vfcmp128v64 */
    0377, /* vfcmpx128v64 vfcmpxx128v64 vfcmpxxx128v64 vcmpps vfcmp128v32 vfcmpx128v32 vfcmpxx128v32 vfcmpxxx128v32 */
    0377, /* vcmpsd vcmpxsd vcmpxxsd vcmpxxxsd vcmpss vcmpxss vcmpxxss vcmpxxxss */
    0377, /* vcomisd vcomixsd vcomixxsd vcomixxxsd vcomiss vcomixss vcomixxss vcomixxxss */
    0377, /* vcvtdq2pd vcvtdq2pdx vcvtdq2pdxx vcvtdq2pdxxx vcvtdq2ps vcvtdq2psx vcvtdq2psxx vcvtdq2psxxx */
    0377, /* vcvtpd2dq vcvtpd2dqx vcvtpd2dqxx vcvtpd2dqxxx vcvtpd2dqy vcvtpd2dqyx vcvtpd2dqyxx vcvtpd2dqyxxx */
    0377, /* vcvtpd2ps vcvtpd2psx vcvtpd2psxx vcvtpd2psxxx vcvtpd2psy vcvtpd2psyx vcvtpd2psyxx vcvtpd2psyxxx */
    0377, /* vcvtps2dq vcvtps2dqx vcvtps2dqxx vcvtps2dqxxx vcvtps2pd vcvtps2pdx vcvtps2pdxx vcvtps2pdxxx */
    0377, /* vcvtsd2si vcvtsd2siq vcvtsd2ss vcvtsd2ssx vcvtsd2ssxx vcvtsd2ssxxx vcvtsi2sd vcvtsi2sdx */
    0377, /* vcvtsi2sdxx vcvtsi2sdxxx vcvtsi2sdq vcvtsi2sdqx vcvtsi2sdqxx vcvtsi2sdqxxx vcvtsi2ss vcvtsi2ssx */
    0377, /* vcvtsi2ssxx vcvtsi2ssxxx vcvtsi2ssq vcvtsi2ssqx vcvtsi2ssqxx vcvtsi2ssqxxx vcvtss2sd vcvtss2sdx */
    0377, /* vcvtss2sdxx vcvtss2sdxxx vcvtss2si vcvtss2siq vcvttpd2dq vcvttpd2dqx vcvttpd2dqxx vcvttpd2dqxxx */
    0377, /* vcvttpd2dqy vcvttpd2dqyx vcvttpd2dqyxx vcvttpd2dqyxxx vcvttps2dq vcvttps2dqx vcvttps2dqxx vcvttps2dqxxx */
    0377, /* vcvttsd2si vcvttsd2siq vcvttss2si vcvttss2siq vfdiv128v64 vfdivx128v64 vfdivxx128v64 vfdivxxx128v64 */
    0377, /* vfdiv128v32 vfdivx128v32 vfdivxx128v32 vfdivxxx128v32 vdivsd vdivxsd vdivxxsd vdivxxxsd */
    0377, /* vdivss vdivxss vdivxxss vdivxxxss vfdp128v64 vfdpx128v64 vfdpxx128v64 vfdpxxx128v64 */
    0377, /* vfdp128v32 vfdpx128v32 vfdpxx128v32 vfdpxxx128v32 vfextrf128 vfextrxf128 vfextrxxf128 vfextrxxxf128 */
    0377, /* vfextr128v32 vfextrx128v32 vfextrxx128v32 vfextrxxx128v32 vfhadd128v64 vfhaddx128v64 vfhaddxx128v64 vfhaddxxx128v64 */
    0377, /* vfhadd128v32 vfhaddx128v32 vfhaddxx128v32 vfhaddxxx128v32 vfhsub128v64 vfhsubx128v64 vfhsubxx128v64 vfhsubxxx128v64 */
    0377, /* vfhsub128v32 vfhsubx128v32 vfhsubxx128v32 vfhsubxxx128v32 vfinsrf128 vfinsrxf128 vfinsrxxf128 vfinsrxxxf128 */
    0377, /* vfinsr128v32 vfinsrx128v32 vfinsrxx128v32 vfinsrxxx128v32 vlddqu vlddqux vlddquxx vlddqu_n32 */
    0377, /* vldmxcsr vmaskmovdqu vfmaskld128v32 vfmaskldx128v32 vfmaskldxx128v32 vfmaskld128v64 vfmaskldx128v64 vfmaskldxx128v64 */
    0377, /* vfmaskst128v32 vfmaskstx128v32 vfmaskstxx128v32 vfmaskst128v64 vfmaskstx128v64 vfmaskstxx128v64 vfmax128v64 vfmaxx128v64 */
    0377, /* vfmaxxx128v64 vfmaxxxx128v64 vfmax128v32 vfmaxx128v32 vfmaxxx128v32 vfmaxxxx128v32 vfmaxsd vfmaxxsd */
    0377, /* vfmaxxxsd vfmaxxxxsd vfmaxss vfmaxxss vfmaxxxss vfmaxxxxss vfmin128v64 vfminx128v64 */
    0377, /* vfminxx128v64 vfminxxx128v64 vfmin128v32 vfminx128v32 vfminxx128v32 vfminxxx128v32 vfminsd vfminxsd */
    0377, /* vfminxxsd vfminxxxsd vfminss vfminxss vfminxxss vfminxxxss vmovapd vldapd */
    0377, /* vldapdx vldapdxx vldapd_n32 vstapd vstapdx vstapdxx vstapd_n32 vmovaps */
    0377, /* vldaps vldapsx vldapsxx vldaps_n32 vstaps vstapsx vstapsxx vstaps_n32 */
    0377, /* vmovg2x vmovg2x64 vmovx2g vmovx2g64 vld64_2sse vldx64_2sse vldxx64_2sse vld64_2sse_n32 */
    0377, /* vst64_fsse vstx64_fsse vstxx64_fsse vst64_fsse_n32 vmovddup vmovddupx vmovddupxx vmovddupxxx */
    0377, /* vlddqa vlddqax vlddqaxx vlddqa_n32 vmovdqa vstdqa vstdqax vstdqaxx */
    0377, /* vstdqa_n32 vstdqu vstdqux vstdquxx vstdqu_n32 vldhpd vldhpdx vldhpdxx */
    0377, /* vldhpd_n32 vsthpd vsthpdx vsthpdxx vsthpd_n32 vldhps vldhpsx vldhpsxx */
    0377, /* vldhps_n32 vsthps vsthpsx vsthpsxx vsthps_n32 vldlpd vldlpdx vldlpdxx */
    0377, /* vldlpd_n32 vstorelpd vstlpd vstlpdx vstlpdxx vstlpd_n32 vldlps vldlpsx */
    0377, /* vldlpsxx vldlps_n32 vstlps vstlpsx vstlpsxx vstlps_n32 vmovmskpd vmovmskps */
    0377, /* vstorenti128 vstorentxi128 vstorentxxi128 vldntdqa vldntdqax vldntdqaxx vstntdq vstntdqx */
    0377, /* vstntdqxx vstntpd vstntpdx vstntpdxx vstntps vstntpsx vstntpsxx vstntsd */
    0377, /* vstntsdx vstntsdxx vstntss vstntssx vstntssxx vmovsd vldsd vldsdx */
    0377, /* vldsdxx vldsd_n32 vstsd vstsdx vstsdxx vstsd_n32 vmovss vldss */
    0377, /* vldssx vldssxx vldss_n32 vstss vstssx vstssxx vstss_n32 vmovlhps */
    0377, /* vmovhlps vmovshdup vmovshdupx vmovshdupxx vmovshdupxxx vmovsldup vmovsldupx vmovsldupxx */
    0377, /* vmovsldupxxx vldupd vldupdx vldupdxx vldupd_n32 vstupd vstupdx vstupdxx */
    0377, /* vstupd_n32 vldups vldupsx vldupsxx vldups_n32 vstups vstupsx vstupsxx */
    0377, /* vstups_n32 vmpsadbw vmpsadbwx vmpsadbwxx vmpsadbwxxx vfmul128v64 vfmulx128v64 vfmulxx128v64 */
    0377, /* vfmulxxx128v64 vfmul128v32 vfmulx128v32 vfmulxx128v32 vfmulxxx128v32 vmulsd vmulxsd vmulxxsd */
    0377, /* vmulxxxsd vmulss vmulxss vmulxxss vmulxxxss vorpd vfor128v64 vforx128v64 */
    0377, /* vforxx128v64 vforxxx128v64 vorps vfor128v32 vforx128v32 vforxx128v32 vforxxx128v32 vabs128v8 */
    0377, /* vabsx128v8 vabsxx128v8 vabsxxx128v8 vabs128v32 vabsx128v32 vabsxx128v32 vabsxxx128v32 vabs128v16 */
    0377, /* vabsx128v16 vabsxx128v16 vabsxxx128v16 vpackssdw vpackssdwx vpackssdwxx vpackssdwxxx vpacksswb */
    0377, /* vpacksswbx vpacksswbxx vpacksswbxxx vpackusdw vpackusdwx vpackusdwxx vpackusdwxxx vpackuswb */
    0377, /* vpackuswbx vpackuswbxx vpackuswbxxx vadd128v8 vaddx128v8 vaddxx128v8 vaddxxx128v8 vadd128v32 */
    0377, /* vaddx128v32 vaddxx128v32 vaddxxx128v32 vadd128v64 vaddx128v64 vaddxx128v64 vaddxxx128v64 vadd128v16 */
    0377, /* vaddx128v16 vaddxx128v16 vaddxxx128v16 vadds128v8 vaddsx128v8 vaddsxx128v8 vaddsxxx128v8 vadds128v16 */
    0377, /* vaddsx128v16 vaddsxx128v16 vaddsxxx128v16 vaddus128v8 vaddusx128v8 vaddusxx128v8 vaddusxxx128v8 vaddus128v16 */
    0377, /* vaddusx128v16 vaddusxx128v16 vaddusxxx128v16 vpalignr128 vpalignrx128 vpalignrxx128 vpalignrxxx128 vand128v8 */
    0377, /* vandx128v8 vandxx128v8 vandxxx128v8 vand128v16 vandx128v16 vandxx128v16 vandxxx128v16 vand128v32 */
    0377, /* vandx128v32 vandxx128v32 vandxxx128v32 vand128v64 vandx128v64 vandxx128v64 vandxxx128v64 vandn128v8 */
    0377, /* vandnx128v8 vandnxx128v8 vandnxxx128v8 vandn128v16 vandnx128v16 vandnxx128v16 vandnxxx128v16 vandn128v32 */
    0377, /* vandnx128v32 vandnxx128v32 vandnxxx128v32 vandn128v64 vandnx128v64 vandnxx128v64 vandnxxx128v64 vpavgb */
    0377, /* vpavgbx vpavgbxx vpavgbxxx vpavgw vpavgwx vpavgwxx vpavgwxxx vblendv128v8 */
    0377, /* vblendvx128v8 vblendvxx128v8 vblendvxxx128v8 vblend128v16 vblendx128v16 vblendxx128v16 vblendxxx128v16 vpclmulqdq */
    0377, /* vpclmulqdqx vpclmulqdqxx vpclmulqdqxxx vcmpeq128v8 vcmpeqx128v8 vcmpeqxx128v8 vcmpeqxxx128v8 vcmpeq128v32 */
    0377, /* vcmpeqx128v32 vcmpeqxx128v32 vcmpeqxxx128v32 vcmpeq128v64 vcmpeqx128v64 vcmpeqxx128v64 vcmpeqxxx128v64 vcmpeq128v16 */
    0377, /* vcmpeqx128v16 vcmpeqxx128v16 vcmpeqxxx128v16 vcmpestri vcmpestrix vcmpestrixx vcmpestrixxx vcmpestrm */
    0377, /* vcmpestrmx vcmpestrmxx vcmpestrmxxx vcmpgt128v8 vcmpgtx128v8 vcmpgtxx128v8 vcmpgtxxx128v8 vcmpgt128v32 */
    0377, /* vcmpgtx128v32 vcmpgtxx128v32 vcmpgtxxx128v32 vcmpgt128v64 vcmpgtx128v64 vcmpgtxx128v64 vcmpgtxxx128v64 vcmpgt128v16 */
    0377, /* vcmpgtx128v16 vcmpgtxx128v16 vcmpgtxxx128v16 vcmpistri vcmpistrix vcmpistrixx vcmpistrixxx vcmpistrm */
    0377, /* vcmpistrmx vcmpistrmxx vcmpistrmxxx vfperm128v64 vfpermx128v64 vfpermxx128v64 vfpermxxx128v64 vfpermi128v64 */
    0377, /* vfpermix128v64 vfpermixx128v64 vfpermixxx128v64 vfperm128v32 vfpermx128v32 vfpermxx128v32 vfpermxxx128v32 vfpermi128v32 */
    0377, /* vfpermix128v32 vfpermixx128v32 vfpermixxx128v32 vfperm2f128 vfperm2xf128 vfperm2xxf128 vfperm2xxxf128 vextr128v8 */
    0377, /* vextrx128v8 vextrxx128v8 vextrxxx128v8 vextr128v32 vextrx128v32 vextrxx128v32 vextrxxx128v32 vextr128v64 */
    0377, /* vextrx128v64 vextrxx128v64 vextrxxx128v64 vextr128v16 vextrx128v16 vextrxx128v16 vextrxxx128v16 vphadd128v32 */
    0377, /* vphaddx128v32 vphaddxx128v32 vphaddxxx128v32 vphadds128v16 vphaddsx128v16 vphaddsxx128v16 vphaddsxxx128v16 vphadd128v16 */
    0377, /* vphaddx128v16 vphaddxx128v16 vphaddxxx128v16 vphminposuw vphminposuwx vphminposuwxx vphminposuwxxx vphsub128v32 */
    0377, /* vphsubx128v32 vphsubxx128v32 vphsubxxx128v32 vphsubs128v16 vphsubsx128v16 vphsubsxx128v16 vphsubsxxx128v16 vphsub128v16 */
    0377, /* vphsubx128v16 vphsubxx128v16 vphsubxxx128v16 vinsr128v8 vinsrx128v8 vinsrxx128v8 vinsrxxx128v8 vinsr128v32 */
    0377, /* vinsrx128v32 vinsrxx128v32 vinsrxxx128v32 vinsr128v64 vinsrx128v64 vinsrxx128v64 vinsrxxx128v64 vinsr128v16 */
    0377, /* vinsrx128v16 vinsrxx128v16 vinsrxxx128v16 vpmaddwd vpmaddwdx vpmaddwdxx vpmaddwdxxx vpmaddubsw128 */
    0377, /* vpmaddubswx128 vpmaddubswxx128 vpmaddubswxxx128 vmaxs128v8 vmaxsx128v8 vmaxsxx128v8 vmaxsxxx128v8 vmaxs128v16 */
    0377, /* vmaxsx128v16 vmaxsxx128v16 vmaxsxxx128v16 vmaxs128v32 vmaxsx128v32 vmaxsxx128v32 vmaxsxxx128v32 vmaxu128v8 */
    0377, /* vmaxux128v8 vmaxuxx128v8 vmaxuxxx128v8 vmaxu128v16 vmaxux128v16 vmaxuxx128v16 vmaxuxxx128v16 vmaxu128v32 */
    0377, /* vmaxux128v32 vmaxuxx128v32 vmaxuxxx128v32 vmins128v8 vminsx128v8 vminsxx128v8 vminsxxx128v8 vmins128v16 */
    0377, /* vminsx128v16 vminsxx128v16 vminsxxx128v16 vmins128v32 vminsx128v32 vminsxx128v32 vminsxxx128v32 vminu128v8 */
    0377, /* vminux128v8 vminuxx128v8 vminuxxx128v8 vminu128v16 vminux128v16 vminuxx128v16 vminuxxx128v16 vminu128v32 */
    0377, /* vminux128v32 vminuxx128v32 vminuxxx128v32 vpmovmskb128 vpmovsxbd vpmovsxbdx vpmovsxbdxx vpmovsxbdxxx */
    0377, /* vpmovsxbq vpmovsxbqx vpmovsxbqxx vpmovsxbqxxx vpmovsxbw vpmovsxbwx vpmovsxbwxx vpmovsxbwxxx */
    0377, /* vpmovsxdq vpmovsxdqx vpmovsxdqxx vpmovsxdqxxx vpmovsxwd vpmovsxwdx vpmovsxwdxx vpmovsxwdxxx */
    0377, /* vpmovsxwq vpmovsxwqx vpmovsxwqxx vpmovsxwqxxx vpmovzxbd vpmovzxbdx vpmovzxbdxx vpmovzxbdxxx */
    0377, /* vpmovzxbq vpmovzxbqx vpmovzxbqxx vpmovzxbqxxx vpmovzxbw vpmovzxbwx vpmovzxbwxx vpmovzxbwxxx */
    0377, /* vpmovzxdq vpmovzxdqx vpmovzxdqxx vpmovzxdqxxx vpmovzxwd vpmovzxwdx vpmovzxwdxx vpmovzxwdxxx */
    0377, /* vpmovzxwq vpmovzxwqx vpmovzxwqxx vpmovzxwqxxx vmulhuw vmulhuwx vmulhuwxx vmulhuwxxx */
    0377, /* vmulhrsw vmulhrswx vmulhrswxx vmulhrswxxx vmulhw vmulhwx vmulhwxx vmulhwxxx */
    0377, /* vmulld vmulldx vmulldxx vmulldxxx vmul128v16 vmulx128v16 vmulxx128v16 vmulxxx128v16 */
    0377, /* vmul128v32 vmulx128v32 vmulxx128v32 vmulxxx128v32 vmuludq vmuludqx vmuludqxx vmuludqxxx */
    0377, /* vmuldq vmuldqx vmuldqxx vmuldqxxx vor128v8 vorx128v8 vorxx128v8 vorxxx128v8 */
    0377, /* vor128v16 vorx128v16 vorxx128v16 vorxxx128v16 vor128v32 vorx128v32 vorxx128v32 vorxxx128v32 */
    0377, /* vor128v64 vorx128v64 vorxx128v64 vorxxx128v64 vpsadbw vpsadbwx vpsadbwxx vpsadbwxxx */
    0377, /* vpshuf128v8 vpshufx128v8 vpshufxx128v8 vpshufxxx128v8 vpshuf128v32 vpshufx128v32 vpshufxx128v32 vpshufxxx128v32 */
    0377, /* vpshufw64v16 vpshufwx64v16 vpshufwxx64v16 vpshufwxxx64v16 vpshufhw vpshufhwx vpshufhwxx vpshufhwxxx */
    0377, /* vpshuflw vpshuflwx vpshuflwxx vpshuflwxxx vpsign128v8 vpsignx128v8 vpsignxx128v8 vpsignxxx128v8 */
    0377, /* vpsign128v32 vpsignx128v32 vpsignxx128v32 vpsignxxx128v32 vpsign128v16 vpsignx128v16 vpsignxx128v16 vpsignxxx128v16 */
    0377, /* vpslldq vpsrldq vpslld vpslldx vpslldxx vpslldxxx vpslldi vpsllq */
    0377, /* vpsllqx vpsllqxx vpsllqxxx vpsllqi vpsllw vpsllwx vpsllwxx vpsllwxxx */
    0377, /* vpsllwi vpsrad vpsradx vpsradxx vpsradxxx vpsradi vpsraw vpsrawx */
    0377, /* vpsrawxx vpsrawxxx vpsrawi vpsrld vpsrldx vpsrldxx vpsrldxxx vpsrldi */
    0377, /* vpsrlq vpsrlqx vpsrlqxx vpsrlqxxx vpsrlqi vpsrlw vpsrlwx vpsrlwxx */
    0377, /* vpsrlwxxx vpsrlwi vsub128v8 vsubx128v8 vsubxx128v8 vsubxxx128v8 vsub128v32 vsubx128v32 */
    0377, /* vsubxx128v32 vsubxxx128v32 vsub128v64 vsubx128v64 vsubxx128v64 vsubxxx128v64 vsub128v16 vsubx128v16 */
    0377, /* vsubxx128v16 vsubxxx128v16 vsubs128v8 vsubsx128v8 vsubsxx128v8 vsubsxxx128v8 vsubs128v16 vsubsx128v16 */
    0377, /* vsubsxx128v16 vsubsxxx128v16 vsubus128v8 vsubusx128v8 vsubusxx128v8 vsubusxxx128v8 vsubus128v16 vsubusx128v16 */
    0377, /* vsubusxx128v16 vsubusxxx128v16 vptest128 vptestx128 vptestxx128 vptestxxx128 vtestpd vtestxpd */
    0377, /* vtestxxpd vtestxxxpd vtestps vtestxps vtestxxps vtestxxxps vpunpckh64v8 vpunpckhx64v8 */
    0377, /* vpunpckhxx64v8 vpunpckhxxx64v8 vpunpckh64v32 vpunpckhx64v32 vpunpckhxx64v32 vpunpckhxxx64v32 vpunpckh64v16 vpunpckhx64v16 */
    0377, /* vpunpckhxx64v16 vpunpckhxxx64v16 vpunpckh64v64 vpunpckhx64v64 vpunpckhxx64v64 vpunpckhxxx64v64 vpunpckl64v8 vpunpcklx64v8 */
    0377, /* vpunpcklxx64v8 vpunpcklxxx64v8 vpunpckl64v32 vpunpcklx64v32 vpunpcklxx64v32 vpunpcklxxx64v32 vpunpckl64v16 vpunpcklx64v16 */
    0377, /* vpunpcklxx64v16 vpunpcklxxx64v16 vpunpckl64v64 vpunpcklx64v64 vpunpcklxx64v64 vpunpcklxxx64v64 vxor128v8 vxorx128v8 */
    0377, /* vxorxx128v8 vxorxxx128v8 vxor128v16 vxorx128v16 vxorxx128v16 vxorxxx128v16 vxor128v32 vxorx128v32 */
    0377, /* vxorxx128v32 vxorxxx128v32 vxor128v64 vxorx128v64 vxorxx128v64 vxorxxx128v64 vfrcp128v32 vfrcpx128v32 */
    0377, /* vfrcpxx128v32 vfrcpxxx128v32 vfrcpss vfrcpxss vfrcpxxss vfrcpxxxss vround128v64 vroundx128v64 */
    0377, /* vroundxx128v64 vroundxxx128v64 vround128v32 vroundx128v32 vroundxx128v32 vroundxxx128v32 vroundsd vroundxsd */
    0377, /* vroundxxsd vroundxxxsd vroundss vroundxss vroundxxss vroundxxxss vfrsqrt128v32 vfrsqrtx128v32 */
    0377, /* vfrsqrtxx128v32 vfrsqrtxxx128v32 vfrsqrtss vfrsqrtxss vfrsqrtxxss vfrsqrtxxxss vshufpd vfshuf128v64 */
    0377, /* vfshufx128v64 vfshufxx128v64 vfshufxxx128v64 vshufps vfshuf128v32 vfshufx128v32 vfshufxx128v32 vfshufxxx128v32 */
    0377, /* vfsqrt128v64 vfsqrtx128v64 vfsqrtxx128v64 vfsqrtxxx128v64 vfsqrt128v32 vfsqrtx128v32 vfsqrtxx128v32 vfsqrtxxx128v32 */
    0377, /* vfsqrtsd vfsqrtxsd vfsqrtxxsd vfsqrtxxxsd vfsqrtss vfsqrtxss vfsqrtxxss vfsqrtxxxss */
    0377, /* vstmxcsr vfsub128v64 vfsubx128v64 vfsubxx128v64 vfsubxxx128v64 vfsub128v32 vfsubx128v32 vfsubxx128v32 */
    0377, /* vfsubxxx128v32 vsubsd vsubxsd vsubxxsd vsubxxxsd vsubss vsubxss vsubxxss */
    0377, /* vsubxxxss vucomisd vucomixsd vucomixxsd vucomixxxsd vucomiss vucomixss vucomixxss */
    0377, /* vucomixxxss vunpckh128v64 vunpckhx128v64 vunpckhxx128v64 vunpckhxxx128v64 vunpckh128v32 vunpckhx128v32 vunpckhxx128v32 */
    0377, /* vunpckhxxx128v32 vunpckl128v64 vunpcklx128v64 vunpcklxx128v64 vunpcklxxx128v64 vunpckl128v32 vunpcklx128v32 vunpcklxx128v32 */
    0377, /* vunpcklxxx128v32 vxorpd vfxor128v64 vfxorx128v64 vfxorxx128v64 vfxorxxx128v64 vxzero128v64 vxzero64 */
    0377, /* vxorps vfxor128v32 vfxorx128v32 vfxorxx128v32 vfxorxxx128v32 vxzero128v32 vxzero32 vzeroall */
    0377, /* vcmpeqpd vcmpltpd vcmplepd vcmpunordpd vcmpneqpd vcmpnltpd vcmpnlepd vcmpordpd */
    0377, /* vcmpeqps vcmpltps vcmpleps vcmpunordps vcmpneqps vcmpnltps vcmpnleps vcmpordps */
    0377, /* vcmpeqsd vcmpltsd vcmplesd vcmpunordsd vcmpneqsd vcmpnltsd vcmpnlesd vcmpordsd */
    0377, /* vcmpeqss vcmpltss vcmpless vcmpunordss vcmpneqss vcmpnltss vcmpnless vcmpordss */
    0377, /* xfmadd132pd xfmadd132xpd xfmadd132xxpd xfmadd132xxxpd xfmadd213pd xfmadd213xpd xfmadd213xxpd xfmadd213xxxpd */
    0377, /* xfmadd231pd xfmadd231xpd xfmadd231xxpd xfmadd231xxxpd xfmadd132ps xfmadd132xps xfmadd132xxps xfmadd132xxxps */
    0377, /* xfmadd213ps xfmadd213xps xfmadd213xxps xfmadd213xxxps xfmadd231ps xfmadd231xps xfmadd231xxps xfmadd231xxxps */
    0377, /* xfmadd132sd xfmadd132xsd xfmadd132xxsd xfmadd132xxxsd xfmadd213sd xfmadd213xsd xfmadd213xxsd xfmadd213xxxsd */
    0377, /* xfmadd231sd xfmadd231xsd xfmadd231xxsd xfmadd231xxxsd xfmadd132ss xfmadd132xss xfmadd132xxss xfmadd132xxxss */
    0377, /* xfmadd213ss xfmadd213xss xfmadd213xxss xfmadd213xxxss xfmadd231ss xfmadd231xss xfmadd231xxss xfmadd231xxxss */
    0377, /* xfmaddsub132pd xfmaddsub132xpd xfmaddsub132xxpd xfmaddsub132xxxpd xfmaddsub213pd xfmaddsub213xpd xfmaddsub213xxpd xfmaddsub213xxxpd */
    0377, /* xfmaddsub231pd xfmaddsub231xpd xfmaddsub231xxpd xfmaddsub231xxxpd xfmaddsub132ps xfmaddsub132xps xfmaddsub132xxps xfmaddsub132xxxps */
    0377, /* xfmaddsub213ps xfmaddsub213xps xfmaddsub213xxps xfmaddsub213xxxps xfmaddsub231ps xfmaddsub231xps xfmaddsub231xxps xfmaddsub231xxxps */
    0377, /* xfmsubadd132pd xfmsubadd132xpd xfmsubadd132xxpd xfmsubadd132xxxpd xfmsubadd213pd xfmsubadd213xpd xfmsubadd213xxpd xfmsubadd213xxxpd */
    0377, /* xfmsubadd231pd xfmsubadd231xpd xfmsubadd231xxpd xfmsubadd231xxxpd xfmsubadd132ps xfmsubadd132xps xfmsubadd132xxps xfmsubadd132xxxps */
    0377, /* xfmsubadd213ps xfmsubadd213xps xfmsubadd213xxps xfmsubadd213xxxps xfmsubadd231ps xfmsubadd231xps xfmsubadd231xxps xfmsubadd231xxxps */
    0377, /* xfmsub132pd xfmsub132xpd xfmsub132xxpd xfmsub132xxxpd xfmsub213pd xfmsub213xpd xfmsub213xxpd xfmsub213xxxpd */
    0377, /* xfmsub231pd xfmsub231xpd xfmsub231xxpd xfmsub231xxxpd xfmsub132ps xfmsub132xps xfmsub132xxps xfmsub132xxxps */
    0377, /* xfmsub213ps xfmsub213xps xfmsub213xxps xfmsub213xxxps xfmsub231ps xfmsub231xps xfmsub231xxps xfmsub231xxxps */
    0377, /* xfmsub132sd xfmsub132xsd xfmsub132xxsd xfmsub132xxxsd xfmsub213sd xfmsub213xsd xfmsub213xxsd xfmsub213xxxsd */
    0377, /* xfmsub231sd xfmsub231xsd xfmsub231xxsd xfmsub231xxxsd xfmsub132ss xfmsub132xss xfmsub132xxss xfmsub132xxxss */
    0377, /* xfmsub213ss xfmsub213xss xfmsub213xxss xfmsub213xxxss xfmsub231ss xfmsub231xss xfmsub231xxss xfmsub231xxxss */
    0377, /* xfnmadd132pd xfnmadd132xpd xfnmadd132xxpd xfnmadd132xxxpd xfnmadd213pd xfnmadd213xpd xfnmadd213xxpd xfnmadd213xxxpd */
    0377, /* xfnmadd231pd xfnmadd231xpd xfnmadd231xxpd xfnmadd231xxxpd xfnmadd132ps xfnmadd132xps xfnmadd132xxps xfnmadd132xxxps */
    0377, /* xfnmadd213ps xfnmadd213xps xfnmadd213xxps xfnmadd213xxxps xfnmadd231ps xfnmadd231xps xfnmadd231xxps xfnmadd231xxxps */
    0377, /* xfnmadd132sd xfnmadd132xsd xfnmadd132xxsd xfnmadd132xxxsd xfnmadd213sd xfnmadd213xsd xfnmadd213xxsd xfnmadd213xxxsd */
    0377, /* xfnmadd231sd xfnmadd231xsd xfnmadd231xxsd xfnmadd231xxxsd xfnmadd132ss xfnmadd132xss xfnmadd132xxss xfnmadd132xxxss */
    0377, /* xfnmadd213ss xfnmadd213xss xfnmadd213xxss xfnmadd213xxxss xfnmadd231ss xfnmadd231xss xfnmadd231xxss xfnmadd231xxxss */
    0377, /* xfnmsub132pd xfnmsub132xpd xfnmsub132xxpd xfnmsub132xxxpd xfnmsub213pd xfnmsub213xpd xfnmsub213xxpd xfnmsub213xxxpd */
    0377, /* xfnmsub231pd xfnmsub231xpd xfnmsub231xxpd xfnmsub231xxxpd xfnmsub132ps xfnmsub132xps xfnmsub132xxps xfnmsub132xxxps */
    0377, /* xfnmsub213ps xfnmsub213xps xfnmsub213xxps xfnmsub213xxxps xfnmsub231ps xfnmsub231xps xfnmsub231xxps xfnmsub231xxxps */
    0377, /* xfnmsub132sd xfnmsub132xsd xfnmsub132xxsd xfnmsub132xxxsd xfnmsub213sd xfnmsub213xsd xfnmsub213xxsd xfnmsub213xxxsd */
    0377, /* xfnmsub231sd xfnmsub231xsd xfnmsub231xxsd xfnmsub231xxxsd xfnmsub132ss xfnmsub132xss xfnmsub132xxss xfnmsub132xxxss */
    0377, /* xfnmsub213ss xfnmsub213xss xfnmsub213xxss xfnmsub213xxxss xfnmsub231ss xfnmsub231xss xfnmsub231xxss xfnmsub231xxxss */
    0377, /* movabsq store8_abs store16_abs store32_abs store64_abs ld8_abs ld16_abs ld32_abs */
    0377, /* ld64_abs lock_add32 lock_adc32 lock_add64 lock_xchg32 lock_xchg64 lock_cmpxchg32 lock_cmpxchg64 */
    0377, /* lock_and32 lock_and64 lock_or32 lock_or64 lock_xor32 lock_xor64 lock_sub32 lock_sub64 */
    0377, /* lock_add8 lock_add16 lock_xchg8 lock_xchg16 lock_cmpxchg8 lock_cmpxchg16 lock_and8 lock_and16 */
    0377, /* lock_or8 lock_or16 lock_xor8 lock_xor16 lock_sub8 lock_sub16 lock_xadd8 lock_xadd16 */
    0377, /* lock_xadd32 lock_xadd64 bsf32 bsf64 bsr32 bsr64 tls_global_dynamic_64 tls_global_dynamic_32 */
    0377, /* tls_local_dynamic_64 tls_local_dynamic_32 begin_pregtn end_pregtn bwd_bar fwd_bar label nop */
    0001, /* noop */
  },
  { /* UNDEFINED */
    0
  }
};

REGISTER_SUBCLASS_INFO REGISTER_SUBCLASS_info[ISA_REGISTER_SUBCLASS_MAX + 1];
REGISTER_CLASS_INFO REGISTER_CLASS_info[ISA_REGISTER_CLASS_MAX + 1];
CLASS_REG_PAIR      CLASS_REG_PAIR_zero;
CLASS_REG_PAIR      CLASS_REG_PAIR_ep;
CLASS_REG_PAIR      CLASS_REG_PAIR_gp;
CLASS_REG_PAIR      CLASS_REG_PAIR_sp;
CLASS_REG_PAIR      CLASS_REG_PAIR_fp;
CLASS_REG_PAIR      CLASS_REG_PAIR_ra;
CLASS_REG_PAIR      CLASS_REG_PAIR_v0;
CLASS_REG_PAIR      CLASS_REG_PAIR_static_link;
CLASS_REG_PAIR      CLASS_REG_PAIR_pfs;
CLASS_REG_PAIR      CLASS_REG_PAIR_lc;
CLASS_REG_PAIR      CLASS_REG_PAIR_ec;
CLASS_REG_PAIR      CLASS_REG_PAIR_true;
CLASS_REG_PAIR      CLASS_REG_PAIR_fzero;
CLASS_REG_PAIR      CLASS_REG_PAIR_fone;
CLASS_REG_PAIR      CLASS_REG_PAIR_f0;


INT32 ISA_SUBSET_Member( ISA_SUBSET subset, UINT8 opcode )
{
  INT byte_index = ((UINT) opcode) / 8;
  INT bit_index = ((UINT) opcode) % 8;
  INT byte = isa_subset_opcode_table[(INT32) subset][byte_index];
  return (byte >> bit_index) & 1;
}

void ISA_REGISTER_Initialize(void)
{
  INT rc;
  INT mask = 1 << (INT) ISA_SUBSET_Value;
  for (rc = ISA_REGISTER_CLASS_MIN; rc <= ISA_REGISTER_CLASS_MAX; ++rc) {
    INT i = ISA_REGISTER_CLASS_info_index[rc];
    const ISA_REGISTER_CLASS_INFO *info = &ISA_REGISTER_CLASS_info[i];
    while ((info->isa_mask & mask) == 0) ++info, ++i;
    ISA_REGISTER_CLASS_info_index[rc] = i;
  }
}



/* Track the "allocatable" state of each register.
 */
enum {
  AS_default = 0,	/* the default is what targ_info says */
  AS_allocatable = 1,
  AS_not_allocatable = 2
};

static UINT8 reg_alloc_status[ISA_REGISTER_CLASS_MAX + 1][REGISTER_MAX + 1];

// list of registers that should not be allocated, both globally and locally.
//static vector< pair< ISA_REGISTER_CLASS, REGISTER> > dont_allocate_these_registers;
//static vector< pair< ISA_REGISTER_CLASS, REGISTER> > dont_allocate_these_registers_in_pu;


/* ====================================================================
 * ====================================================================
 *
 * Initialization and termination
 *
 * ====================================================================
 * ====================================================================
 */

/* ====================================================================
 *
 *  REGISTER_SET_Range
 *
 *  Return the a set of the registers: low .. high
 *
 * ====================================================================
 */
REGISTER_SET
REGISTER_SET_Range(UINT low, UINT high)
{
#if ISA_REGISTER_MAX < 64
  AssertThat(low >= REGISTER_MIN && low <= high && high <= REGISTER_MAX,
          ("REGISTER_SET_Range: bad range specification"));

  UINT leading_zeros = (sizeof(REGISTER_SET_WORD) * 8) - high;
  UINT trailing_zeros = low - REGISTER_MIN;
  return   ((REGISTER_SET_WORD)-1 << (leading_zeros + trailing_zeros))
    >> leading_zeros;
#else /* ISA_REGISTER_MAX < 64 */
  INT i;
  REGISTER_SET set;
  for (i = 0; i <= MAX_REGISTER_SET_IDX; ++i) {
    UINT this_low = (i * 64) + REGISTER_MIN;
    UINT this_high = this_low + 63;
    if (low > this_high || high < this_low) {
      set.v[i] = 0;
    } else {
      UINT leading_zeros = high > this_high ? 0 : this_high - high;
      UINT trailing_zeros = low < this_low ? 0 : low - this_low;
      set.v[i] =   ((REGISTER_SET_WORD)-1 << (leading_zeros + trailing_zeros))
		>> leading_zeros;
    }
  }
  return set;
#endif /* ISA_REGISTER_MAX < 64 */
}

/* ====================================================================
 *
 *  REGISTER_SET_CHOOSE_ENGINE
 *
 *  The guts of the REGISTER_SET_Choose... functions. Return the index
 *  (1-based) of the first set bit in 'set'.
 *
 * ====================================================================
 */
inline REGISTER REGISTER_SET_Choose_Engine(
  REGISTER_SET set
)
{
  INT i = 0;
  do {
    REGISTER_SET_WORD w = REGISTER_SET_ELEM(set, i);
    INT regbase = REGISTER_MIN + (i * sizeof(REGISTER_SET_WORD) * 8);
    do {
      REGISTER_SET_WORD lowb = w & 0xff;
      if ( lowb ) return regbase + U8_lease_bit[lowb];
    } while (regbase += 8, w >>= 8);
  } while (++i <= MAX_REGISTER_SET_IDX);
  return REGISTER_UNDEFINED;
}


/* ====================================================================
 *
 *  REGISTER_SET_Choose
 *
 *  See interface description
 *
 * ====================================================================
 */
extern REGISTER
REGISTER_SET_Choose(
  REGISTER_SET set
)
{
  return REGISTER_SET_Choose_Engine(set);
}

/* ====================================================================
 *
 *  Initialize_Register_Class
 *
 *  Initialize the register class 'rclass'. A register class may be
 *  intialized multiple times.
 *
 * ====================================================================
 */
static void
Initialize_Register_Class(
  ISA_REGISTER_CLASS rclass
)
{
  INT32              i;
  const ISA_REGISTER_CLASS_INFO *icinfo = ISA_REGISTER_CLASS_Info(rclass);
  const char        *rcname         = ISA_REGISTER_CLASS_INFO_Name(icinfo);
  INT		             bit_size       = ISA_REGISTER_CLASS_INFO_Bit_Size(icinfo);
  INT                first_isa_reg  = ISA_REGISTER_CLASS_INFO_First_Reg(icinfo);
  INT                last_isa_reg   = ISA_REGISTER_CLASS_INFO_Last_Reg(icinfo);
  INT                register_count = last_isa_reg - first_isa_reg + 1;
  REGISTER_SET       allocatable    = REGISTER_SET_EMPTY_SET;
  REGISTER_SET       caller         = REGISTER_SET_EMPTY_SET;
  REGISTER_SET       callee         = REGISTER_SET_EMPTY_SET;
  REGISTER_SET       func_argument  = REGISTER_SET_EMPTY_SET;
  REGISTER_SET       func_value     = REGISTER_SET_EMPTY_SET;
  REGISTER_SET       shrink_wrap    = REGISTER_SET_EMPTY_SET;
  REGISTER_SET	     stacked        = REGISTER_SET_EMPTY_SET;
  REGISTER_SET	     rotating       = REGISTER_SET_EMPTY_SET;

  /* Verify we have a valid rclass and that the type used to implement
   * a register set is large enough.
   */
  AssertThat(rclass >= ISA_REGISTER_CLASS_MIN && rclass <= ISA_REGISTER_CLASS_MAX,
            ("invalide register class %d", (INT)rclass));
  AssertThat((sizeof(REGISTER_SET) * 8) >= register_count,
            ("REGISTER_SET type cannot represent all registers in "
             "the class %s %d", rcname, (INT32) rclass));

  REGISTER_CLASS_name(rclass) = rcname;

  /* Now make sets of various register properties:
   */
  for ( i = 0; i < register_count; ++i ) {
    INT      isa_reg        = i + first_isa_reg;
    REGISTER reg            = i + REGISTER_MIN;
    BOOL     is_allocatable = ABI_PROPERTY_Is_allocatable(rclass, isa_reg);
    INT      alloc_status   = reg_alloc_status[rclass][reg];

    /* CG likes to pretend that a class with only one register can't
     * be allocated, so perpetuate that illusion.
     */
    if ( register_count <= 1 ) is_allocatable = FALSE;

    switch ( alloc_status ) {
      case AS_allocatable:
        is_allocatable = TRUE;
        break;
      case AS_not_allocatable:
        is_allocatable = FALSE;
        break;
      case AS_default:
        break;
      default:
        AssertThat(FALSE, ("unhandled allocations status: %d", alloc_status));
    }

    if ( is_allocatable ) {
      allocatable = REGISTER_SET_Union1(allocatable,reg);
#ifdef ABI_PROPERTY_global_ptr
      if ( ABI_PROPERTY_Is_global_ptr(rclass, isa_reg) ) {
        if ( GP_Is_Preserved ) {
          /* neither caller nor callee saved (always preserved). */
        } else if ( Is_Caller_Save_GP ) {
          /* caller-saved. */
          caller = REGISTER_SET_Union1(caller, reg);
        } else {
          /* callee-saved. */
          callee = REGISTER_SET_Union1(callee, reg);
        }
      }
      else
#endif
      {
        // has calling convention
        if ( ABI_PROPERTY_Is_callee(rclass, isa_reg) ) {
          callee = REGISTER_SET_Union1(callee, reg);
          shrink_wrap = REGISTER_SET_Union1(shrink_wrap, reg);
        }
        if ( ABI_PROPERTY_Is_caller(rclass, isa_reg) )
          caller = REGISTER_SET_Union1(caller, reg);
        if ( ABI_PROPERTY_Is_func_arg(rclass, isa_reg) )
          func_argument = REGISTER_SET_Union1(func_argument, reg);
        if ( ABI_PROPERTY_Is_func_val(rclass, isa_reg) )
          func_value = REGISTER_SET_Union1(func_value, reg);
      }
    }

    REGISTER_bit_size(rclass, reg) = bit_size;
    REGISTER_machine_id(rclass, reg) = isa_reg;
    REGISTER_allocatable(rclass, reg) = is_allocatable;

    if ( ABI_PROPERTY_Is_frame_ptr(rclass, isa_reg) ) {
      Set_CLASS_REG_PAIR_reg(CLASS_REG_PAIR_fp, reg);
      Set_CLASS_REG_PAIR_rclass(CLASS_REG_PAIR_fp, rclass);
    }
    else if ( ABI_PROPERTY_Is_static_link(rclass, isa_reg) ) {
      Set_CLASS_REG_PAIR_reg(CLASS_REG_PAIR_static_link, reg);
      Set_CLASS_REG_PAIR_rclass(CLASS_REG_PAIR_static_link, rclass);
    }
//    else if ( ABI_PROPERTY_Is_global_ptr(rclass, isa_reg) ) {
//      Set_CLASS_REG_PAIR_reg(CLASS_REG_PAIR_gp, reg);
//      Set_CLASS_REG_PAIR_rclass(CLASS_REG_PAIR_gp, rclass);
//    }
//    else if ( ABI_PROPERTY_Is_ret_addr(rclass, isa_reg) ) {
//      Set_CLASS_REG_PAIR_reg(CLASS_REG_PAIR_ra, reg);
//      Set_CLASS_REG_PAIR_rclass(CLASS_REG_PAIR_ra, rclass);
//    }
    else if ( ABI_PROPERTY_Is_stack_ptr(rclass, isa_reg) ) {
      Set_CLASS_REG_PAIR_reg(CLASS_REG_PAIR_sp, reg);
      Set_CLASS_REG_PAIR_rclass(CLASS_REG_PAIR_sp, rclass);
    }
  }

  REGISTER_CLASS_universe(rclass)          = REGISTER_SET_Range(REGISTER_MIN, REGISTER_MIN + register_count - 1);
  REGISTER_CLASS_allocatable(rclass)       = allocatable;
  REGISTER_CLASS_callee_saves(rclass)      = callee;
  REGISTER_CLASS_caller_saves(rclass)      = caller;
  REGISTER_CLASS_function_argument(rclass) = func_argument;
  REGISTER_CLASS_function_value(rclass)    = func_value;
  REGISTER_CLASS_shrink_wrap(rclass)       = shrink_wrap;
  REGISTER_CLASS_register_count(rclass)    = register_count;
  REGISTER_CLASS_stacked(rclass)           = stacked;
  REGISTER_CLASS_rotating(rclass)          = rotating;
  REGISTER_CLASS_can_store(rclass)
                                        = ISA_REGISTER_CLASS_INFO_Can_Store(icinfo);
  REGISTER_CLASS_multiple_save(rclass)
                                        = ISA_REGISTER_CLASS_INFO_Multiple_Save(icinfo);

  /* There are multiple integer return regs -- v0 is the lowest
   * of the set.
   */
  if ( rclass == ISA_REGISTER_CLASS_integer ) {
    Set_CLASS_REG_PAIR_reg(CLASS_REG_PAIR_v0, REGISTER_SET_Choose(func_value));
    Set_CLASS_REG_PAIR_rclass(CLASS_REG_PAIR_v0, rclass);
  }
}


mISA_REGISTER_CLASS Mtype_RegClass_Map[MTYPE_COUNT + 1];

void Init_Mtype_RegClass_Map(void)
{
  INT i;
  mISA_REGISTER_CLASS * const map = Mtype_RegClass_Map;

  for (i = 0; i <= MTYPE_COUNT; ++i) map[i] = ISA_REGISTER_CLASS_UNDEFINED;

  map[MTYPE_I1] = ISA_REGISTER_CLASS_integer;
  map[MTYPE_I2] = ISA_REGISTER_CLASS_integer;
  map[MTYPE_I4] = ISA_REGISTER_CLASS_integer;
  map[MTYPE_I8] = ISA_REGISTER_CLASS_integer;
  map[MTYPE_U1] = ISA_REGISTER_CLASS_integer;
  map[MTYPE_U2] = ISA_REGISTER_CLASS_integer;
  map[MTYPE_U4] = ISA_REGISTER_CLASS_integer;
  map[MTYPE_U8] = ISA_REGISTER_CLASS_integer;

  map[MTYPE_F4] = ISA_REGISTER_CLASS_float;
  map[MTYPE_F8] = ISA_REGISTER_CLASS_float;

//  map[MTYPE_F10] = ISA_REGISTER_CLASS_x87;
//  map[MTYPE_FQ] = ISA_REGISTER_CLASS_x87;
//  map[MTYPE_F16] = ISA_REGISTER_CLASS_UNDEFINED;
//
//  map[MTYPE_V16C4] = ISA_REGISTER_CLASS_float;
//  map[MTYPE_C8]    = ISA_REGISTER_CLASS_float;
//  map[MTYPE_V16C8] = ISA_REGISTER_CLASS_float;
//  map[MTYPE_V16F4] = ISA_REGISTER_CLASS_float;
//  map[MTYPE_V16F8] = ISA_REGISTER_CLASS_float;
//  map[MTYPE_V16I1] = ISA_REGISTER_CLASS_float;
//  map[MTYPE_V16I2] = ISA_REGISTER_CLASS_float;
//  map[MTYPE_V16I4] = ISA_REGISTER_CLASS_float;
//  map[MTYPE_V16I8] = ISA_REGISTER_CLASS_float;
//  map[MTYPE_V8I1] = ISA_REGISTER_CLASS_float;
//  map[MTYPE_V8I2] = ISA_REGISTER_CLASS_float;
//  map[MTYPE_V8I4] = ISA_REGISTER_CLASS_float;
//  map[MTYPE_V8I8] = ISA_REGISTER_CLASS_float;
//  map[MTYPE_V8F4] = ISA_REGISTER_CLASS_float;
//  map[MTYPE_M8I1] = ISA_REGISTER_CLASS_mmx;
//  map[MTYPE_M8I2] = ISA_REGISTER_CLASS_mmx;
//  map[MTYPE_M8I4] = ISA_REGISTER_CLASS_mmx;
//  map[MTYPE_M8F4] = ISA_REGISTER_CLASS_mmx;
//  /* 256-bit floating point */
//  map[MTYPE_V32C4] = ISA_REGISTER_CLASS_float;
//  map[MTYPE_V32C8] = ISA_REGISTER_CLASS_float;
//  map[MTYPE_V32I1] = ISA_REGISTER_CLASS_float;
//  map[MTYPE_V32I2] = ISA_REGISTER_CLASS_float;
//  map[MTYPE_V32I4] = ISA_REGISTER_CLASS_float;
//  map[MTYPE_V32I8] = ISA_REGISTER_CLASS_float;
//  map[MTYPE_V32F4] = ISA_REGISTER_CLASS_float;
//  map[MTYPE_V32F8] = ISA_REGISTER_CLASS_float;
}

/* ====================================================================
 *
 *  Initialize_Register_Subclasses
 *
 *  Initialize the register subclass information cache.
 *
 * ====================================================================
 */
static void
Initialize_Register_Subclasses(void)
{
  ISA_REGISTER_SUBCLASS sc;

  for (sc = (ISA_REGISTER_SUBCLASS)0; // standard iterator skips _UNDEFINED (0)
       sc <= ISA_REGISTER_SUBCLASS_MAX;
       sc = (ISA_REGISTER_SUBCLASS)(sc + 1))
  {
    INT i;
    ISA_REGISTER_CLASS rc;
    REGISTER_SET members = REGISTER_SET_EMPTY_SET;
    const ISA_REGISTER_SUBCLASS_INFO *scinfo = ISA_REGISTER_SUBCLASS_Info(sc);
    INT count = ISA_REGISTER_SUBCLASS_INFO_Count(scinfo);

    for (i = 0; i < count; ++i) {
      INT isa_reg = ISA_REGISTER_SUBCLASS_INFO_Member(scinfo, i);
      const char *reg_name = ISA_REGISTER_SUBCLASS_INFO_Reg_Name(scinfo, i);
      REGISTER reg = (REGISTER)(isa_reg + REGISTER_MIN);
      members = REGISTER_SET_Union1(members, reg);
      REGISTER_SUBCLASS_reg_name(sc, reg) = reg_name;
    }
    rc = ISA_REGISTER_SUBCLASS_INFO_Class(scinfo);
    members = REGISTER_SET_Intersection(members, REGISTER_CLASS_universe(rc));

    REGISTER_SUBCLASS_members(sc) = members;
    REGISTER_SUBCLASS_name(sc) = ISA_REGISTER_SUBCLASS_INFO_Name(scinfo);
    REGISTER_SUBCLASS_register_class(sc) = rc;
  }
}


/* ====================================================================
 *
 *  REGISTER_Begin
 *
 *  See interface description
 *
 * ====================================================================
 */
void
REGISTER_Begin(void)
{
  ISA_REGISTER_CLASS rclass;

  /*  Create the register classes for all the target registers.
   */
  FOR_ALL_ISA_REGISTER_CLASS( rclass ) {
    Initialize_Register_Class(rclass);
  }
  Initialize_Register_Subclasses();
  Init_Mtype_RegClass_Map();
}
