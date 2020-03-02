/*
* Copyright (C) Mellanox Technologies Ltd. 2001-2020.  ALL RIGHTS RESERVED.
* See file LICENSE for terms.
*/
#ifndef XCCL_TEAM_LIB_H_
#define XCCL_TEAM_LIB_H_
#include "api/xccl.h"
#include <assert.h>
#include <string.h>

typedef struct xccl_tl_context xccl_tl_context_t;
typedef struct xccl_tl_team    xccl_tl_team_t;
typedef struct xccl_team_lib {
    char*                               name;
    int                                 priority;
    xccl_params_t                       params;
    xccl_team_lib_context_create_mode_t ctx_create_mode;
    void*                               dl_handle;
    xccl_status_t                       (*create_team_context)(xccl_team_lib_h lib,
                                                               xccl_context_config_t *config,
                                                               xccl_tl_context_t **team_context);
    xccl_status_t                       (*destroy_team_context)(xccl_tl_context_t *team_context);
    xccl_status_t                       (*progress)(xccl_tl_context_t *team_context);
    xccl_status_t                       (*team_create_post)(xccl_tl_context_t *team_ctx,
                                                            xccl_team_config_h config,
                                                            xccl_oob_collectives_t oob, xccl_tl_team_t **team);
    xccl_status_t                       (*team_destroy)(xccl_tl_team_t *team);
    xccl_status_t                       (*collective_init)(xccl_coll_op_args_t *coll_args,
                                                          xccl_coll_req_h *request, xccl_tl_team_t *team);
    xccl_status_t                       (*collective_post)(xccl_coll_req_h request);
    xccl_status_t                       (*collective_wait)(xccl_coll_req_h request);
    xccl_status_t                       (*collective_test)(xccl_coll_req_h request);
    xccl_status_t                       (*collective_finalize)(xccl_coll_req_h request);
} xccl_team_lib_t;

typedef struct xccl_lib {
    int n_libs_opened;
    int libs_array_size;
    char *lib_path;
    xccl_team_lib_t **libs;
} xccl_lib_t;

typedef struct xccl_tl_context {
    xccl_team_lib_t       *lib;
    xccl_context_config_t *cfg;
} xccl_tl_context_t;

typedef struct xccl_context {
    xccl_lib_t            *lib;
    xccl_context_config_t  cfg;
    xccl_tl_context_t    **tl_ctx;
    int                    n_tl_ctx;
} xccl_context_t;

typedef struct xccl_tl_team {
    xccl_tl_context_t     *ctx;
    xccl_team_config_t     cfg;
    xccl_oob_collectives_t oob;
} xccl_tl_team_t;

typedef struct xccl_team {
    xccl_context_t *ctx;
    int coll_team_id[XCCL_COLL_LAST];
    int n_teams;
    xccl_tl_team_t *tl_teams[1];
} xccl_team_t;

static inline void
xccl_oob_allgather(void *sbuf, void* rbuf, size_t len, xccl_oob_collectives_t *oob)
{
    xccl_ep_range_t r = {
        .type = XCCL_EP_RANGE_UNDEFINED,
    };
    oob->allgather(sbuf, rbuf, len, 0, r, oob->coll_context);
}

xccl_status_t xccl_create_context(xccl_lib_t *lib,
                                  const xccl_config_t *config,
                                  xccl_context_t **team_ctx);

typedef struct xccl_local_proc_info {
    unsigned long node_hash;
    int socketid; //if process is bound to a socket
} xccl_local_proc_info_t;

xccl_local_proc_info_t* xccl_local_process_info();

#define XCCL_TEAM_SUPER_INIT(_team, _ctx, _config, _oob) do {           \
        (_team).oob = (_oob);                                           \
        (_team).ctx = (_ctx);                                           \
        memcpy(&((_team).cfg), (_config), sizeof(xccl_team_config_t));  \
    }while(0)

#define XCCL_CONTEXT_SUPER_INIT(_ctx, _lib, _config) do {   \
        (_ctx).lib = (_lib);                                \
        (_ctx).cfg = (_config);                             \
    }while(0)

#define XCCL_STATIC_ASSERT(_cond) \
    switch(0) {case 0:case (_cond):;}

/**
 * @return Offset of _member in _type. _type is a structure type.
 */
#define xccl_offsetof(_type, _member) \
    ((unsigned long)&( ((_type*)0)->_member ))

/**
 * Get a pointer to a struct containing a member.
 *
 * @param __ptr   Pointer to the member.
 * @param type    Container type.
 * @param member  Element member inside the container.
 * @return Address of the container structure.
 */
#define xccl_container_of(_ptr, _type, _member) \
    ( (_type*)( (char*)(void*)(_ptr) - xccl_offsetof(_type, _member) ) )

#define xccl_derived_of(_ptr, _type) \
    ({\
        XCCL_STATIC_ASSERT(offsetof(_type, super) == 0) \
            xccl_container_of(_ptr, _type, super); \
    })

#endif