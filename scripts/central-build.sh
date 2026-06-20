#!/usr/bin/env bash
#
# central-build.sh — 在 opencti 容器(Ubuntu 20.04 / linux-amd64,经 Rosetta)里
# 构建 central 编译器,并 install 到 /home/cti/central-install。
#
# 这一步即「部署到 CTI」:CTI 的 CC 是 $HOME/CTI/compiler/bin/compiler ->
# central-install/bin/central-gcc(wrapper)-> central-install/bin/compiler,
# install prefix 就是 central-install,所以 install 完成 = CTI 立即用上新编译器。
#
# 用法:
#   scripts/central-build.sh              # 增量构建
#   scripts/central-build.sh --clean      # 干净构建(删 build 目录重新 configure)
#   JOBS=1 scripts/central-build.sh       # 并行度(默认 2;容器内存小,-j5 会 OOM)
#   CONTAINER=opencti scripts/central-build.sh
#
set -euo pipefail

CONTAINER="${CONTAINER:-opencti}"
JOBS="${JOBS:-2}"
SRC=/home/cti/central-src
BUILD=/home/cti/central-build          # 容器本地(非 virtiofs),更快、无权限坑
INSTALL=/home/cti/central-install
DEBCACHE="$SRC/.debcache"

CLEAN=0
[ "${1:-}" = "--clean" ] && CLEAN=1

cexec()     { container exec "$CONTAINER" su - cti -c "$1"; }
crootexec() { container exec "$CONTAINER" bash -c "$1"; }

echo "==> [build] 容器=$CONTAINER  并行=-j$JOBS  clean=$CLEAN"

container ls 2>/dev/null | grep -q "^$CONTAINER\b" \
  || { echo "!! 容器 '$CONTAINER' 没在运行,先 'container start $CONTAINER'。" >&2; exit 1; }

# 1. 确保原生 cmake + g++(容器默认没有;从 .debcache 离线装,容器重建后会丢)
if ! cexec 'command -v cmake >/dev/null && command -v g++ >/dev/null'; then
  echo "==> 容器内缺 cmake/g++,从 .debcache 离线安装…"
  crootexec "cd $DEBCACHE && dpkg -i libuv1_*.deb cmake-data_*.deb libarchive13_*.deb \
    libjsoncpp1_*.deb librhash0_*.deb cmake_*.deb libstdc++-9-dev_*.deb g++-9_*.deb g++_*.deb \
    >/dev/null 2>&1; apt-get install -f -y --no-download >/dev/null 2>&1 || true"
  cexec 'command -v cmake >/dev/null && command -v g++ >/dev/null' \
    || { echo "!! cmake/g++ 安装失败,检查 $DEBCACHE。" >&2; exit 2; }
fi

# 2. configure(干净构建 / build 目录不存在时)
if [ "$CLEAN" = 1 ] || ! cexec "test -f $BUILD/Makefile"; then
  echo "==> configure(prefix=$INSTALL)…"
  cexec "rm -rf $BUILD && mkdir -p $BUILD && cd $BUILD && $SRC/configure $INSTALL"
fi

# 3. make + install
echo "==> make -j$JOBS && make install …"
if ! cexec "cd $BUILD && make -j$JOBS && make install"; then
  echo "!! 构建失败。若日志含 'Killed ... cc1plus' 是 OOM,改用 JOBS=1 重试。" >&2
  exit 3
fi

echo "==> ✅ 构建并部署完成:"
cexec "ls -la $INSTALL/bin/compiler"
echo "   用 scripts/central-run-case.sh <文件> 快速验证,或 scripts/central-cti.sh <sched/opt> 跑回归。"
