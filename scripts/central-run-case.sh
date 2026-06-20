#!/usr/bin/env bash
#
# central-run-case.sh — 用当前 central-install 的编译器,把一个(或多个)单文件 case
# 走完整流水线:central 编译(.s) → 交叉汇编(assemble, .o)→ 交叉链接(link)→
# qemu-arm 运行,逐步打印命令和结果。开发时快速验证用。
#
# 用法:
#   scripts/central-run-case.sh testcase/foo.sy
#   scripts/central-run-case.sh --sysy bar.sy            # 链接 hard-float libsysy
#   scripts/central-run-case.sh --in input.txt prog.sy   # stdin 喂给可执行
#   scripts/central-run-case.sh -O2 prog.c               # -O* 透传给 central
#   scripts/central-run-case.sh main.sy helper.c         # 多源一起编译链接
#   scripts/central-run-case.sh prog.sy -- 1 2 3         # -- 之后是程序运行参数
#   scripts/central-run-case.sh --keep prog.sy           # 保留 /tmp 中间产物
#
# 文件用 host 路径(相对/绝对均可),会自动映射到容器内挂载路径。
#
set -euo pipefail

CONTAINER="${CONTAINER:-opencti}"
CC=/home/cti/central-install/bin/compiler
CROSS=arm-linux-gnueabihf-gcc
XARCH="-marm -march=armv7-a -mfloat-abi=hard -mfpu=vfpv3-d16"
QEMU="qemu-arm -L /usr/arm-linux-gnueabihf"
LIBSYSY="-L/home/cti/central-install/lib -lsysy"
WORK=/tmp/central-runcase
TIMEOUT_S="${TIMEOUT_S:-15}"

cexec() { container exec "$CONTAINER" su - cti -c "$1"; }

# host 路径 -> 容器路径(基于 opencti 的 virtiofs 挂载表)
to_container() {
  local p; p="$(cd "$(dirname "$1")" && pwd)/$(basename "$1")"   # realpath
  case "$p" in
    /Users/xc5/compilers/central/*)          echo "/home/cti/central-src/${p#/Users/xc5/compilers/central/}";;
    /Users/xc5/compilers/central-install/*)  echo "/home/cti/central-install/${p#/Users/xc5/compilers/central-install/}";;
    /Users/xc5/compilers/central-testware/*) echo "/home/cti/central-testware/${p#/Users/xc5/compilers/central-testware/}";;
    /Users/xc5/compilers/central-opencti/*)  echo "/home/cti/central-opencti/${p#/Users/xc5/compilers/central-opencti/}";;
    /Users/xc5/compilers/opencti-src/*)      echo "/home/cti/opencti-src/${p#/Users/xc5/compilers/opencti-src/}";;
    /Users/xc5/compilers/opencti-testware/*) echo "/home/cti/opencti-testware/${p#/Users/xc5/compilers/opencti-testware/}";;
    /Users/xc5/compilers/opencti-resource/*) echo "/home/cti/opencti-resource/${p#/Users/xc5/compilers/opencti-resource/}";;
    *) return 1;;
  esac
}

SYSY=0; KEEP=0; OPTFLAG=""; INFILE=""; ARGS=""; SRCS=()
while [ $# -gt 0 ]; do
  case "$1" in
    --sysy) SYSY=1; shift;;
    --keep) KEEP=1; shift;;
    --in)   INFILE="$2"; shift 2;;
    -O*)    OPTFLAG="$1"; shift;;
    --)     shift; ARGS="$*"; break;;
    -*)     echo "未知选项: $1" >&2; exit 2;;
    *)      SRCS+=("$1"); shift;;
  esac
done
[ "${#SRCS[@]}" -ge 1 ] || { echo "用法: $0 [opts] <源文件> [更多源...] [-- 运行参数]" >&2; exit 2; }

# 映射源文件到容器路径
RSRCS=()
for s in "${SRCS[@]}"; do
  [ -f "$s" ] || { echo "!! 找不到文件: $s" >&2; exit 3; }
  r="$(to_container "$s")" || { echo "!! $s 不在容器挂载范围内(需放在 ~/compilers/ 下)。" >&2; exit 3; }
  RSRCS+=("$r")
done
INREDIR=""
if [ -n "$INFILE" ]; then
  [ -f "$INFILE" ] || { echo "!! 找不到输入文件: $INFILE" >&2; exit 3; }
  rin="$(to_container "$INFILE")" || { echo "!! 输入文件不在挂载范围内。" >&2; exit 3; }
  INREDIR="< $rin"
fi

LIB=""; [ "$SYSY" = 1 ] && LIB="$LIBSYSY"

# 构造容器内执行脚本:编译 -> 汇编 -> 链接 -> 运行
RCMD="set -u; rm -rf $WORK; mkdir -p $WORK; cd $WORK;"
OBJS=""
i=0
for r in "${RSRCS[@]}"; do
  b="case$i"; i=$((i+1))
  RCMD+=" echo '==> [编译] $CC -S $OPTFLAG $r -o $b.s'; $CC -S $OPTFLAG '$r' -o $b.s || { echo '!! central 编译失败'; exit 11; };"
  RCMD+=" echo '==> [汇编] $CROSS $XARCH -c $b.s -o $b.o'; $CROSS $XARCH -c $b.s -o $b.o || { echo '!! 汇编失败'; exit 12; };"
  OBJS+="$b.o "
done
RCMD+=" echo '==> [链接] $CROSS $XARCH $OBJS$LIB -o a.out'; $CROSS $XARCH $OBJS$LIB -o a.out || { echo '!! 链接失败'; exit 13; };"
RCMD+=" echo '==> [运行] $QEMU ./a.out $ARGS'; timeout -s KILL $TIMEOUT_S $QEMU ./a.out $ARGS $INREDIR; rc=\$?; echo \"==> 退出码 = \$rc\";"
[ "$KEEP" = 1 ] && RCMD+=" echo '中间产物保留在容器 $WORK';" || RCMD+=" cd /; rm -rf $WORK;"

cexec "$RCMD"
