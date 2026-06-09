#include "opt_stab.h"

AUX_ID OPT_STAB::Add_var(ST_IDX st, MTYPE_ID mtype) {
  auto it = _st_to_aux.find(st);
  if (it != _st_to_aux.end()) return it->second;
  AUX_ID aux = (AUX_ID)_vars.size();
  _vars.push_back(OPT_VAR());
  _vars[aux].st_idx = st;
  _vars[aux].aux_id = aux;
  _vars[aux].mtype  = mtype;
  _vars[aux].ver_count = 0;
  _ver_tab.push_back(std::vector<VER_ENTRY>());
  _st_to_aux[st] = aux;
  return aux;
}

AUX_ID OPT_STAB::Lookup(ST_IDX st) const {
  auto it = _st_to_aux.find(st);
  if (it == _st_to_aux.end()) return 0;
  return it->second;
}

UINT32 OPT_STAB::New_version(AUX_ID aux) {
  AssertThat(aux < _vars.size(), ("bad aux"));
  UINT32 v = ++_vars[aux].ver_count;
  _ver_tab[aux].push_back(VER_ENTRY());
  _ver_tab[aux].back().aux_id  = aux;
  _ver_tab[aux].back().version = v;
  _ver_tab[aux].back().def_stmt = nullptr;
  _ver_tab[aux].back().def_phi  = nullptr;
  _ver_tab[aux].back().def_bb   = nullptr;
  return v;
}

OPT_VAR &OPT_STAB::Var(AUX_ID aux) {
  AssertThat(aux < _vars.size(), ("bad aux"));
  return _vars[aux];
}

const OPT_VAR &OPT_STAB::Var(AUX_ID aux) const {
  AssertThat(aux < _vars.size(), ("bad aux"));
  return _vars[aux];
}

VER_ENTRY &OPT_STAB::Ver(AUX_ID aux, UINT32 ver) {
  AssertThat(aux < _ver_tab.size() && ver > 0 && ver <= _ver_tab[aux].size(),
             ("bad ver aux=%d ver=%d", aux, ver));
  return _ver_tab[aux][ver - 1];
}

const VER_ENTRY &OPT_STAB::Ver(AUX_ID aux, UINT32 ver) const {
  AssertThat(aux < _ver_tab.size() && ver > 0 && ver <= _ver_tab[aux].size(),
             ("bad ver aux=%d ver=%d", aux, ver));
  return _ver_tab[aux][ver - 1];
}

UINT32 OPT_STAB::Num_versions(AUX_ID aux) const {
  AssertThat(aux < _ver_tab.size(), ("bad aux"));
  return (UINT32)_ver_tab[aux].size();
}

BOOL OPT_STAB::Ver_exists(AUX_ID aux, UINT32 ver) const {
  return aux < _ver_tab.size() && ver > 0 && ver <= _ver_tab[aux].size();
}

void OPT_STAB::Reset() {
  _vars.clear();
  _st_to_aux.clear();
  _ver_tab.clear();
  _next_aux = 1;
}

void OPT_STAB::Print(FILE *f) const {
  fprintf(f, "===== OPT_STAB: %lu vars =====\n", _vars.size());
  for (UINT32 i = 0; i < _vars.size(); ++i) {
    fprintf(f, "  aux=%u st=%d ver_count=%u\n",
            i, (int)_vars[i].st_idx, _vars[i].ver_count);
  }
}
