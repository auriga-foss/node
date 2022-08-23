// Interface to KOS Execute Manager abstraction library

#if defined(__cplusplus)
#define cpp_noexcept noexcept
extern "C" {
#else
#define cpp_noexcept
#endif

// FIXME: return codes are noninformative as Execution Manager's codes are
// for now. Can be populated with more appropriate codes, or changed to
// UNIX errno codes
enum KOS_EXECMGR_ERROR {
    KOS_EXECMGR_OK,
    KOS_EXECMGR_NONEXIST_ENTITY,
    KOS_EXECMGR_UNK
};

typedef long long unsigned kos_entity_id_t;

int kos_execmgr_spawn(const char *path,
                      char * const argv[],
                      char * const env[],
                      kos_entity_id_t *pid) cpp_noexcept;

int kos_execmgr_exist(kos_entity_id_t pid) cpp_noexcept;

int kos_execmgr_kill(kos_entity_id_t pid) cpp_noexcept;

#if defined(__cplusplus)
}
#endif
