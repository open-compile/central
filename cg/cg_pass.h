
#ifndef  OCC_CG_PASS_H
#define OCC_CG_PASS_H

#include "cgir.h"

class CG_PASS {  
  CG_COMPOSITE  *_cgmon = nullptr;
  BOOL           _enabled = FALSE;

protected:
  const char    *_name = nullptr;  

public:
  // 两阶段初始化（贴合你现有风格）
  virtual BOOL   Run(PU_INFO *pu)           = 0;
  virtual void   Dump(FILE *file = stderr)  = 0;
  virtual void   Init(CG_COMPOSITE *cgmon)  { _cgmon = cgmon;   }
  void           Print(FILE *file = stderr) { Dump(file);       }
  const char    *Name()                     { return _name;     }
  BOOL           Get_enabled()              { return _enabled;  }
  void           Set_enabled(BOOL enb)      { _enabled = enb;   }
  // 可选：dump 本 pass 运行后的中间结果（无副作用）
protected:
  // 便捷访问方法，避免每个子类都写一遍 _cgmon->Cgir()
  CG_COMPOSITE *Current_cg_mon() const {
    AssertThat(_cgmon != nullptr, ("CG_PASS::Init() not called before Run()"));
    return _cgmon;
  }
  CGIR *Cgir() const { return Current_cg_mon()->Cgir(); }
};

class CG_BUILD_PASS : public CG_PASS {
public:
  CG_BUILD_PASS() { _name = "build"; }
  BOOL Run(PU_INFO *pu) override;
  void Build_pred_succ(PU_INFO *pu);
  void Build_def_use(PU_INFO *pu);
  void Dump(FILE *file) override {
    Cgir()->Print(file);   // 原 CG_process_funcs 里的 Print
  }
};


class CG_LIVE_RANGE_PASS : public CG_PASS {
public:
  CG_LIVE_RANGE_PASS() { _name = "live_range"; }
  BOOL Run(PU_INFO *pu) override;
  void Dump(FILE *file) override;
};

class CG_REG_ALLOC_PASS : public CG_PASS {
public:
  CG_REG_ALLOC_PASS() { _name = "reg_alloc"; }
  BOOL Run(PU_INFO *pu) override;
  void Dump(FILE *file) override;
};

class CG_FRAME_LAYOUT_PASS : public CG_PASS {
public:
  CG_FRAME_LAYOUT_PASS() { _name = "frame_layout"; }
  BOOL Run(PU_INFO *pu) override;
  void Dump(FILE *file) override;
};


class CG_EMITTER_PASS : public CG_PASS {
  FILE     *_output_asm_file = nullptr;
public:
  CG_EMITTER_PASS() { _name = "emit"; }
  FILE *Outfile_safe() {
    AssertThat(_output_asm_file != nullptr, ("output file is null"));
    return _output_asm_file;
  }
  void Set_outfile(FILE *f) { _output_asm_file = f;            }
  void Cleanup_outfile()    { _output_asm_file = nullptr;      }
  BOOL Run(PU_INFO *pu) override;
  void Dump(FILE *file) override {
    ST_IDX func_sym = Cgir()->Current_func_sym();
    Current_cg_mon()->Emitter().Emit_tree(func_sym, file, File());
  }
};

#endif //OCC_CG_PASS_H
