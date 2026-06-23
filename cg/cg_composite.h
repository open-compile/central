
#ifndef OCC_CG_COMPOSITE_H
#define OCC_CG_COMPOSITE_H

#include "cgir.h"
#include "lra.h"
#include "target.h"
#include "target_backend.h"

#include <memory>

class CG_LIVE_RANGE;

/**
 * The Entire CG class to access.
 */
class CG_COMPOSITE {
private:
  CGIR            *_cgir = nullptr;
  CG_EMITTER       _emitter;
  CGIR_BUILDER     _builder;
  CG_LIVE_RANGE     _lra;
  CG_REG_ALLOC      _reg_alloc;
  const TARGET_INFO *_target = nullptr;
  std::unique_ptr<TARGET_BACKEND> _backend;
public:
  void Init(const TARGET_INFO &target) {
    _target = &target;
    std::string backend_error;
    _backend = Create_target_backend(target, &backend_error);
    AssertThat(_backend != nullptr, ("%s", backend_error.c_str()));
    _cgir = new CGIR();
    AssertThat(_cgir != nullptr, ("Cgir must be a valid CGIR"));
    _builder.Init(_cgir, _target);
    _emitter.Init(_cgir, &_builder, _backend.get());
    _reg_alloc.Init(_cgir, &_builder);
    _lra.Init(_cgir);
  }
  TARGET_BACKEND &Backend() const {
    AssertThat(_backend != nullptr, ("Target backend must be initialized"));
    return *_backend;
  }
  const TARGET_INFO &Target() const {
    AssertThat(_target != nullptr, ("Target must be initialized"));
    return *_target;
  }
  CGIR *Cgir() {
    AssertThat(_cgir != nullptr,
      ("Cgir is not initialized in Live range analysis"));
    return _cgir;
  }
  CG_EMITTER     &Emitter() {
    return _emitter;
  }
  CGIR_BUILDER   &Builder() {
    return _builder;
  }
  CG_REG_ALLOC   &Reg_alloc() { return _reg_alloc; }
  CG_LIVE_RANGE  &Lra()       { return _lra;       }
  void          CG_convert_function(SCOPE *scope);        // Initialize the CG stuff
};


#endif //OCC_CGIR_H
