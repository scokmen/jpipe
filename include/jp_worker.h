#ifndef JPIPE_JP_WORKER_H
#define JPIPE_JP_WORKER_H

JP_ATTR_NONNULL(2)
JP_ATTR_READ_ONLY_N(2, 1)
jp_errno_t jp_wrk_exec(int argc, char* argv[]);

#endif  // JPIPE_JP_WORKER_H
