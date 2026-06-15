
#ifndef  OCC_CG_PASS_H
#define OCC_CG_PASS_H

#include "cgir.h"

class CG_PASS {
  CG_COMPOSITE  *_cgmon = nullptr;
  const char    *_name = nullptr;
  BOOL           _enabled = FALSE;

public:
  virtual ~CG_PASS(): {}
  // 两阶段初始化（贴合你现有风格）
  virtual void   Init(CG_COMPOSITE *cgmon) { _cgmon = cgmon; }
  virtual BOOL   Run()             = FALSE;
  virtual 
  const char    *Name()                    { return _name;     }
  BOOL           Get_enabled()             { return _enabled;  }
  BOOL           Set_enabled(BOOL enb)     { _enabled = enb;  }
  // 可选：dump 本 pass 运行后的中间结果（无副作用）
  virtual void   Dump(FILE *file = stderr) {}
protected:
  // 便捷访问方法，避免每个子类都写一遍 _cgmon->Cgir()
  CG_COMPOSITE *Cgmon() const {
    AssertThat(_cgmon != nullptr, ("CG_PASS::Init() not called before Run()"));
    return _cgmon;
  }
  CGIR *Cgir() const { return Cgmon()->Cgir(); }
}

class CG_BUILD_PASS : public CG_PASS {
  public:
    CG_BUILD_PASS() { _name = "build"; }

    BOOL Run(PU_INFO *pu) override {
      // 原逻辑：Cgmon()->CG_convert_function(&pu->scope)
      File()->Scopes()->Goto_function(pu->proc_sym);
      Cgmon()->CG_convert_function(&pu->scope);
      return TRUE;
    }

    void Dump(FILE *file) override {
      Cgir()->Print(file);   // 原 CG_process_funcs 里的 Print
    }
  };

  2. LiveRange Pass

  class CG_LIVE_RANGE_PASS : public CG_PASS {
  public:
    CG_LIVE_RANGE_PASS() { _name = "live_range"; }

    BOOL Run(PU_INFO *pu) override {
      // 原逻辑：Cgmon()->Lra().Analyze_live_range(pu_info)
      Cgmon()->Lra().Analyze_live_range(pu);
      return TRUE;
    }

    void Dump(FILE *file) override {
      Cgmon()->Lra().Print(file);
    }
  };

  3. RegAlloc Pass

  class CG_REG_ALLOC_PASS : public CG_PASS {
  public:
    CG_REG_ALLOC_PASS() { _name = "reg_alloc"; }

    BOOL Run(PU_INFO *pu) override {
      // 原逻辑：Cgmon()->Reg_alloc().Register_allocate(pu_info)
      Cgmon()->Reg_alloc().Register_allocate(pu);
      return TRUE;
    }

    void Dump(FILE *file) override {
      Cgmon()->Reg_alloc().Print_freq_map(file);
      Cgmon()->Reg_alloc().Print_live_range(file);
    }
  };

  4. FrameLayout Pass

  class CG_FRAME_LAYOUT_PASS : public CG_PASS {
  public:
    CG_FRAME_LAYOUT_PASS() { _name = "frame_layout"; }

    BOOL Run(PU_INFO *pu) override {
      // 原逻辑
      Cgir()->Layout()->Calculate_stack_frame_size();
      Cgir()->Recalibrate_offset(pu);
      return TRUE;
    }

    void Dump(FILE *file) override {
      Cgir()->Layout()->Print(file);
    }
  };


#endif //OCC_CG_PASS_H
