#!/usr/bin/env bash
#
# central-cti.sh — 把当前 central 部署到 CTI 并重跑回归。
# 可跑整个 sched(如 szu.sanity.sched),也可只跑单个 opt(更快,如 szu.gcc.core.opt)。
#
# 用法:
#   scripts/central-cti.sh szu.sanity.sched          # 跑整个 sched
#   scripts/central-cti.sh szu.gcc.core.opt          # 只跑一个 opt(快)
#   scripts/central-cti.sh --build szu.gcc.core.opt  # 先重建 central 再跑
#   scripts/central-cti.sh --build --clean <...>     # 干净重建后再跑
#
# 名字带 "sched" 走标准调度(run_user_dtm.sh),否则当作 opt 用 TM.pl 单跑。
#
set -euo pipefail

CONTAINER="${CONTAINER:-opencti}"
ISTOIRE=/home/cti/central-opencti/istoire
CONF=/home/cti/CTI/cti_conf
RES=/home/cti/CTI/cti_res
HERE="$(cd "$(dirname "$0")" && pwd)"
MAX_WAIT_MIN="${MAX_WAIT_MIN:-45}"

cexec() { container exec "$CONTAINER" su - cti -c "$1"; }

BUILD=0; CLEAN=""; TARGET=""
while [ $# -gt 0 ]; do
  case "$1" in
    --build) BUILD=1; shift;;
    --clean) CLEAN="--clean"; shift;;
    -*) echo "未知选项: $1" >&2; exit 2;;
    *) TARGET="$1"; shift;;
  esac
done
[ -n "$TARGET" ] || { echo "用法: $0 [--build [--clean]] <sched 或 opt 文件名>" >&2; exit 2; }

# 1. 可选重建(=部署到 CTI)
if [ "$BUILD" = 1 ]; then
  CONTAINER="$CONTAINER" "$HERE/central-build.sh" $CLEAN
fi

# 2. 部署配置(展开 SIMULATOR/LIBS 到 cti_conf;run_user_dtm 内部也会做,opt 模式需手动)
cexec "cd $ISTOIRE && ./setup_env.sh >/dev/null 2>&1"

DAY=$(cexec "date '+%a' | tr 'A-Z' 'a-z'")

summarize_opt() {  # $1=opt 文件名
  local o="$1" f="$RES/log.$DAY.$o"
  local line; line=$(cexec "grep -E 'TOTAL TESTS=' '$f' 2>/dev/null | tail -1" || true)
  local detail; detail=$(cexec "grep -E 'Total Number of (COMPILATION|LINKING|EXECUTION) FAILURES = [1-9]' '$f' 2>/dev/null | sed -E 's/# Total Number of //; s/ FAILURES = / /' | tr '\n' ',' | sed 's/,\$//'" || true)
  local p t ff
  p=$(echo "$line" | grep -oE 'PASS=[0-9]+' | cut -d= -f2)
  t=$(echo "$line" | grep -oE 'TOTAL TESTS=[0-9]+' | cut -d= -f2)
  ff=$(echo "$line" | grep -oE 'FAIL=[0-9]+' | cut -d= -f2)
  printf "%-30s %8s   %4s   %s\n" "$o" "${p:-?}/${t:-?}" "${ff:-?}" "$detail"
}

if echo "$TARGET" | grep -q "sched"; then
  # ---- sched 模式 ----
  echo "==> 提交调度: run_user_dtm.sh $TARGET"
  cexec "cd $ISTOIRE && ./run_user_dtm.sh $TARGET" 2>&1 | grep -iE "Submitted|HOME" || true
  OPTS=$(cexec "grep -oE '[A-Za-z0-9._-]+\.opt' $CONF/$TARGET | sort -u")
  echo "==> 等 DTM 完成(最多 ${MAX_WAIT_MIN} 分钟)…"
  ITERS=$(( MAX_WAIT_MIN * 60 / 15 ))
  cexec '
    for i in $(seq 1 '"$ITERS"'); do
      n=$(ps -e -o args= | grep "bin/TM.pl" | grep "'"$DAY"'" | grep -v grep | grep -v defunct | wc -l)
      if [ "$i" -gt 2 ] && [ "$n" -eq 0 ]; then echo "  done (~$((i*15))s)"; exit 0; fi
      sleep 15
    done; echo "  !! 超时"'
else
  # ---- 单 opt 模式 ----
  o="$TARGET"; [ -n "${o##*.opt}" ] && o="$o.opt"   # 补 .opt 后缀
  W="$RES/work.$DAY.$o"; L="$RES/log.$DAY.$o"
  echo "==> 单跑 opt: $o"
  cexec ". $ISTOIRE/cti_cfg.sh; /home/cti/opencti-src/bin/TM.pl -d -w '$W' -l '$L' -f '$CONF/$o' -nomail -x DTM_POOL=Default clean run >/tmp/central-cti-opt.out 2>&1; echo TM-exit=\$?"
  OPTS="$o"
fi

# 3. 汇总
echo ""
echo "############ 结果汇总 (log.$DAY.<opt>) ############"
printf "%-30s %8s   %4s   %s\n" "OPT" "PASS/TOT" "FAIL" "明细"
for o in $OPTS; do summarize_opt "$o"; done
echo ""
if echo "$TARGET" | grep -q "sched"; then
  echo "网页(本地): http://localhost:8081/CTI/cgi-bin/show-schedule.cgi?sched=$CONF/$TARGET"
else
  echo "日志: 容器内 $RES/log.$DAY.$OPTS  (失败详情 grep FAILURES)"
fi
