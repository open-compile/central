#include "timing.h"

#include <chrono>
#include <sys/resource.h>
#include <unistd.h>

static double Timeval_to_sec(const timeval &tv) {
  return static_cast<double>(tv.tv_sec) +
         static_cast<double>(tv.tv_usec) / 1000000.0;
}

static long Current_rss_kb() {
  FILE *fp = fopen("/proc/self/statm", "r");
  if (fp == nullptr) return 0;

  long size_pages = 0;
  long resident_pages = 0;
  int scanned = fscanf(fp, "%ld %ld", &size_pages, &resident_pages);
  fclose(fp);
  if (scanned != 2) return 0;

  long page_size = sysconf(_SC_PAGESIZE);
  if (page_size <= 0) return 0;
  return resident_pages * page_size / 1024;
}

TIMING_SNAPSHOT Timing_snapshot() {
  using clock = std::chrono::steady_clock;
  static const clock::time_point process_start = clock::now();

  struct rusage usage;
  getrusage(RUSAGE_SELF, &usage);

  TIMING_SNAPSHOT snap;
  snap.wall_sec =
    std::chrono::duration<double>(clock::now() - process_start).count();
  snap.user_sec = Timeval_to_sec(usage.ru_utime);
  snap.sys_sec = Timeval_to_sec(usage.ru_stime);
  snap.rss_kb = Current_rss_kb();
  snap.peak_rss_kb = usage.ru_maxrss;
  return snap;
}

void Timing_trace_stage(FILE *file, const char *stage,
                        const TIMING_SNAPSHOT &start,
                        const TIMING_SNAPSHOT &end) {
  if (file == nullptr) file = stderr;
  fprintf(file,
          "[TIMING] %-16s wall=%.6fs user=%.6fs sys=%.6fs "
          "rss=%ldKB delta_rss=%+ldKB peak_rss=%ldKB\n",
          stage,
          end.wall_sec - start.wall_sec,
          end.user_sec - start.user_sec,
          end.sys_sec - start.sys_sec,
          end.rss_kb,
          end.rss_kb - start.rss_kb,
          end.peak_rss_kb);
}
