#include "symtab.h"
#include "basic.h"

// External use
static PU_IDX _Current_pu_idx = 0;

/**
 *  Utility Functions, 
 *  for accessing the above data structures
*/
ST* OIR::Get_symbol(ST_IDX st_idx) {
  ST *res = Global_Sym_Tab[st_idx];
  AssertThat(res != NULL, ("Cannot get the symbol by st_idx, %0#x", st_idx));
  return res;
}

TY* OIR::Get_type(TY_IDX ty_idx) {
  TY *res = Global_Type_Tab[ty_idx];
  AssertThat(res != NULL, ("Cannot get the type by ty_idx, %0#x", ty_idx));
  return res;
}

PU_IDX Get_current_pu_idx() {
  AssertThat(_Current_pu_idx > 0, ("No pu currently visiting"));
  return _Current_pu_idx;
}

template<class T>
void SECOND_LEVEL_GROWING_TABLE<T>::add (UINT8 scope_level, T *obj) {
  if (scope_level == 0) {
    global_tab.insert(obj);
  } else {
    std::map<UINT32, T*> &next_level = two_level_tab.at(this->Get_current_pu_idx());
    next_level.insert(obj);
  }
}

template<class T>
T* SECOND_LEVEL_GROWING_TABLE<T>::operator[] (UINT32 idx) 
{
  UINT8 first_level = idx & 0xff;
  UINT32 second = (idx >> 8) & 0xffffff;
  if (first_level == 0) {
    return global_tab[second];
  } else {
    return two_level_tab[Get_current_pu_idx()][second];
  }
}