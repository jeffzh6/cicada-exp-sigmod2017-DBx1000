#pragma once

#include "stdint.h"
#include <unistd.h>
#include <cstddef>
#include <cstdlib>
#include <cassert>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string.h>
#include <typeinfo>
#include <list>
#include <mm_malloc.h>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <sstream>
#include <time.h>
#include <sys/time.h>
#include <math.h>

#include "pthread.h"
#include "config.h"
#include "stats.h"
#include "dl_detect.h"
#ifndef NOGRAPHITE
#include "carbon_user.h"
#endif

#if CC_ALG == MICA
#define NDEBUG
#include "mica/transaction/db.h"
#undef NDEBUG
struct DBConfig : public ::mica::transaction::BasicDBConfig {
  // static constexpr bool kVerbose = true;

  // static constexpr bool kPreValidation = false;
  // static constexpr bool kAbortByGetRow = false;
  // static constexpr bool kAbortByDeferredVersionInsert = false;
  // static constexpr bool kDeferredVersionInsert = false;
  // static constexpr bool kInsertNewestVersionOnly = false;
  // static constexpr bool kSortWriteSetByContention = false;
  // static constexpr bool kStragglerAvoidance = false;
  // static constexpr bool kInlinedRowVersion = true;
  // static constexpr bool kNoWaitForPending = true;

  // static constexpr bool kBackoff = false;
  // static constexpr bool kPrintBackoff = true;
  // static constexpr bool kPairwiseSleeping = true;

  // typedef ::mica::transaction::WideTimestamp Timestamp;
  // typedef ::mica::transaction::WideConcurrentTimestamp ConcurrentTimestamp;
  // typedef ::mica::transaction::WideTimestampConst TimestampConst;
  // typedef ::mica::transaction::CentralizedTimestamp Timestamp;
  // typedef ::mica::transaction::CentralizedConcurrentTimestamp
  // ConcurrentTimestamp;
  // typedef ::mica::transaction::CentralizedTimestampConst TimestampConst;

  // static constexpr bool kCollectCommitStats = false;
  // static constexpr bool kCollectProcessingStats = true;
  // typedef ::mica::transaction::ActiveTiming Timing;

  typedef ::mica::transaction::NullLogger<DBConfig> Logger;
};
typedef DBConfig::Alloc MICAAlloc;
typedef DBConfig::Logger MICALogger;
typedef DBConfig::TimestampConst TimestampConst;
typedef DBConfig::Timing MICATiming;
typedef ::mica::transaction::PagePool<DBConfig> MICAPagePool;
typedef ::mica::transaction::DB<DBConfig> MICADB;
typedef ::mica::transaction::Table<DBConfig> MICATable;
typedef ::mica::transaction::HashIndex<DBConfig> MICAIndex;
typedef ::mica::transaction::RowVersion<DBConfig> MICARowVersion;
typedef ::mica::transaction::RowAccessHandle<DBConfig> MICARowAccessHandle;
typedef ::mica::transaction::Transaction<DBConfig> MICATransaction;
typedef ::mica::transaction::Result MICAResult;
#else
#include "mica/util/latency.h"
#endif

using namespace std;

class mem_alloc;
class Stats;
class DL_detect;
class Manager;
class Query_queue;
class Plock;
class OptCC;
class VLLMan;

typedef uint32_t UInt32;
typedef int32_t SInt32;
typedef uint64_t UInt64;
typedef int64_t SInt64;

typedef uint64_t ts_t; // time stamp type

/******************************************/
// Global Data Structure
/******************************************/
extern mem_alloc mem_allocator;
extern Stats stats;
extern DL_detect dl_detector;
extern Manager * glob_manager;
extern Query_queue * query_queue;
extern Plock part_lock_man;
extern OptCC occ_man;
#if CC_ALG == VLL
extern VLLMan vll_man;
#endif

extern bool volatile warmup_finish;
extern bool volatile enable_thread_mem_pool;
extern pthread_barrier_t warmup_bar;
#ifndef NOGRAPHITE
extern carbon_barrier_t enable_barrier;
#endif

/******************************************/
// Global Parameter
/******************************************/
extern bool g_part_alloc;
extern bool g_mem_pad;
extern bool g_prt_lat_distr;
extern UInt32 g_part_cnt;
extern UInt32 g_virtual_part_cnt;
extern UInt32 g_thread_cnt;
extern ts_t g_abort_penalty;
extern bool g_central_man;
extern UInt32 g_ts_alloc;
extern bool g_key_order;
extern bool g_no_dl;
extern ts_t g_timeout;
extern ts_t g_dl_loop_detect;
extern bool g_ts_batch_alloc;
extern UInt32 g_ts_batch_num;

extern map<string, string> g_params;

// YCSB
extern UInt32 g_cc_alg;
extern ts_t g_query_intvl;
extern UInt32 g_part_per_txn;
extern double g_perc_multi_part;
extern double g_read_perc;
extern double g_write_perc;
extern double g_zipf_theta;
extern UInt64 g_synth_table_size;
extern UInt32 g_req_per_query;
extern UInt32 g_field_per_tuple;
extern UInt32 g_init_parallelism;

// TPCC
extern UInt32 g_num_wh;
extern double g_perc_payment;
extern bool g_wh_update;
extern char * output_file;
extern UInt32 g_max_items;
extern UInt32 g_cust_per_dist;

enum RC { RCOK, Commit, Abort, WAIT, ERROR, FINISH};

/* Thread */
typedef uint64_t txnid_t;

/* Txn */
typedef uint64_t txn_t;

/* Table and Row */
typedef uint64_t rid_t; // row id
typedef uint64_t pgid_t; // page id



/* INDEX */
enum latch_t {LATCH_EX, LATCH_SH, LATCH_NONE};
// accessing type determines the latch type on nodes
enum idx_acc_t {INDEX_INSERT, INDEX_READ, INDEX_NONE};
typedef uint64_t idx_key_t; // key id for index
typedef uint64_t (*func_ptr)(idx_key_t);	// part_id func_ptr(index_key);

/* general concurrency control */
enum access_t {RD, WR, XP, SCAN};
/* LOCK */
enum lock_t {LOCK_EX, LOCK_SH, LOCK_NONE };
/* TIMESTAMP */
enum TsType {R_REQ, W_REQ, P_REQ, XP_REQ};


#define MSG(str, args...) { \
	printf("[%s : %d] " str, __FILE__, __LINE__, args); } \
//	printf(args); }

// principal index structure. The workload may decide to use a different
// index structure for specific purposes. (e.g. non-primary key access should use hash)
#if (INDEX_STRUCT == IDX_BTREE)
#define INDEX		index_btree
#elif (INDEX_STRUCT == IDX_MICA)
#define INDEX		IndexMICA
#else  // IDX_HASH
#define INDEX		IndexHash
#endif

/************************************************/
// constants
/************************************************/
#ifndef UINT64_MAX
#define UINT64_MAX 		18446744073709551615UL
#endif // UINT64_MAX

