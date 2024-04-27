//
// Created by lugt on 24-4-27.
//

#ifndef TIMING_H
#define TIMING_H


/* Provide identifiers for the various time accumulators supplied: */
typedef enum {
	T_FE, // Front-end
	T_FE_CONV,
	T_FE_IR_GEN,

	T_BE_Comp,		/* time for whole back-end */
	T_BE_PU_CU,		/* BE PU processing -- per compilation unit */
	T_BE_PU_Comp,		/* 		       per compilation */
	T_ReadIR_CU,		/* Reading IR */
	T_ReadIR_Comp,
	T_Lower_CU,		/* Lowering WHIRL */
	T_Lower_Comp,
	T_ORI_CU,		/* Olimit Region Insertion */
	T_ORI_Comp,

	T_Preopt_CU,		/* Pre-optimizer */
	T_Preopt_Comp,
	T_Wopt_CU,		/* Global optimizer */
	T_Wopt_Comp,

	T_IPA_S_CU,		/* IPA summary phase */
	T_IPA_S_Comp,

	T_LNO_CU,		/* Loop Nest Optimization */
	T_LNO_Comp,
	T_LNOParentize_CU,	/* Parentization in LNO */
	T_LNOParentize_Comp,
	T_LNOAccess_CU,		/* Build access arrays in LNO */
	T_LNOAccess_Comp,
	T_LNOBuildDep_CU,       /* Build array dep graph in LNO */
	T_LNOBuildDep_Comp,

	T_W2C_CU,		/* Whirl to C translation */
	T_W2C_Comp,
	T_W2F_CU,		/* Whirl to Fortran translation */
	T_W2F_Comp,

	T_CodeGen_CU,		/* Code generator total */
	T_CodeGen_Comp,

	T_GLRA_CU,		/* Global register allocation */
	T_GLRA_Comp,
	T_Expand_CU,		/* Code expansion */
	T_Expand_Comp,
	T_Localize_CU,		/* Localize */
	T_Localize_Comp,
	T_SWpipe_CU,		/* Software pipelining */
	T_SWpipe_Comp,
	T_GCM_CU,		/* Global code motion */
	T_GCM_Comp,
	T_EBO_CU,		/* Extended Block Optimization */
	T_EBO_Comp,
	T_CFLOW_CU,		/* Control Flow Optimization */
	T_CFLOW_Comp,
	T_Loop_CU,		/* CG Loop */
	T_Loop_Comp,
	T_Freq_CU,		/* BB frequency */
	T_Freq_Comp,
	T_HBF_CU,		/* HyperBlock Formation */
	T_HBF_Comp,
	T_Sched_CU,		/* HyperBlock Scheduling */
	T_Sched_Comp,
	T_THR_CU,		/* Tree-Height Reduction Phase */
	T_THR_Comp,
	T_LRA_CU,		/* Local Register Allocation */
	T_LRA_Comp,
	T_GRA_CU,		/* Register allocation */
	T_GRA_Comp,
        T_GRU_CU,		/* Fuse Spills */
	T_GRU_COMP,
	T_Emit_CU,		/* Code emission */
	T_Emit_Comp,
	T_Region_Finalize_CU,	/* Region Finalize */
	T_Region_Finalize_Comp,
        T_Dispatch_Sched_CU,    /* Dispatch Scheduling */
        T_Dispatch_Sched_Comp,
	T_CalcDom_CU,		/* Calculate_Dominators
				 * (this time is included in callers time) */
	T_CalcDom_Comp,

	T_Ipfec_Profiling_CU,	/* Ipfec Profiling */
	T_Ipfec_Profiling_Comp,
	T_Ipfec_Region_CU,	/* Ipfec Region Formation */
	T_Ipfec_Region_Comp,
	T_Ipfec_If_Conv_CU,	/* Ipfec If-Conversion */
	T_Ipfec_If_Conv_Comp,
	T_Ipfec_PRDB_CU,	/* Ipfec PRDB */
	T_Ipfec_PRDB_Comp,
	T_Ipfec_GLOS_CU,	/* Ipfec global scheduling */
	T_Ipfec_GLOS_MISC1_CU,
	T_Ipfec_GLOS_MISC2_CU,
	T_Ipfec_GLOS_Comp,
	T_Ipfec_LOCS_CU,	/* Ipfec local scheduling */
	T_Ipfec_LOCS_Comp,
	T_Ipfec_Speculation_CU,	/* Ipfec Speculation */
	T_Ipfec_Speculation_Comp,
	T_Ipfec_Multi_Branch_CU,	/* Ipfec Multiple branch */
	T_Ipfec_Multi_Branch_Comp,
  T_Ipfec_DAG_CU,
  T_Ipfec_DAG_Comp,

#ifdef SPECMT_LT
        T_SPECMT_BG,
        T_SPECMT_PA,
#endif
	T_WSSA_EMIT_CU,		/* WSSA PreOpt emitter */
	T_WSSA_EMIT_Comp,
	T_AOT_PTN,		/* AOT partition */
	T_AOT_COMP,		/* AOT compile */
	T_BUILD_CHA,		/* BUILD CLASS HIERARCHY */
	T_TAG_PROP,		/* Tag propagation */

	T_LAST			/* Last index defined */
} TIMER_ID;

#endif //TIMING_H
