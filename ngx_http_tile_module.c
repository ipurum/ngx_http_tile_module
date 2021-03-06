
/*
 * 2012 (C) Sergey Maslov <sergey@maslov.biz>
 */


#include    <ndk.h>


static ngx_int_t    ngx_http_tile_xyz_to_path    (ngx_http_request_t *r, ngx_str_t *val, ngx_http_variable_value_t *v);

static  ndk_set_var_t      ngx_http_tile_xyz_to_path_var = {
    NDK_SET_VAR_MULTI_VALUE,
    ngx_http_tile_xyz_to_path,
    3,
    NULL
};


static ngx_command_t  ngx_http_tile_commands[] = {
    {
        ngx_string ("xyz_to_path"),
        NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_SIF_CONF|NGX_HTTP_LOC_CONF|NGX_HTTP_LIF_CONF|NGX_CONF_TAKE4,
        ndk_set_var_multi_value,
        0,
        0,
        &ngx_http_tile_xyz_to_path_var
    },
    ngx_null_command
};


ngx_http_module_t     ngx_http_tile_module_ctx = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
ngx_module_t          ngx_http_tile_module = {

    NGX_MODULE_V1,
    &ngx_http_tile_module_ctx,  // module context
    ngx_http_tile_commands,     // module directives
    NGX_HTTP_MODULE,            // module type
    NULL,                       // init master
    NULL,                       // init module
    NULL,                       // init process
    NULL,                       // init thread
    NULL,                       // exit thread
    NULL,                       // exit process
    NULL,                       // exit master
    NGX_MODULE_V1_PADDING
};


static ngx_int_t
ngx_http_tile_xyz_to_path (ngx_http_request_t *r, ngx_str_t *val, ngx_http_variable_value_t *v)
{
    size_t                      len;
    ngx_int_t                   x, y, i, hash[5];
    u_char                     *p;

    x = ngx_atoi(v->data, v->len);
    if (x == NGX_ERROR) {
        ngx_log_debug1(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, "xyz_to_path bad value x: \"%v\"", v);
        r->err_status = NGX_HTTP_BAD_REQUEST;
        return NGX_ERROR;
    }
    ngx_log_debug1(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, "xyz_to_path value x: \"%ud\"", x);

    v ++;
    y = ngx_atoi(v->data, v->len);
    if (y == NGX_ERROR) {
        ngx_log_debug1(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, "xyz_to_path bad value y: \"%v\"", v);
        r->err_status = NGX_HTTP_BAD_REQUEST;
        return NGX_ERROR;
    }
    ngx_log_debug1(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, "xyz_to_path value y: \"%ud\"", y);

    v ++;
    if (ngx_atoi(v->data, v->len) == NGX_ERROR) {
        ngx_log_debug1(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, "xyz_to_path bad value z: \"%v\"", v);
        r->err_status = NGX_HTTP_BAD_REQUEST;
        return NGX_ERROR;
    }
    ngx_log_debug1(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, "xyz_to_path value z: \"%v\"", v);

    // We attempt to cluster the tiles so that a 16x16 square of tiles will be in a single directory
    // Hash stores our 40 bit result of mixing the 20 bits of the x & y co-ordinates
    // 4 bits of x & y are used per byte of output

    for (i=0; i<5; i++) {
        hash[i] = ((x & 0x0f) << 4) | (y & 0x0f);
        x >>= 4;
        y >>= 4;
    }

    len = v->len + sizeof("/256/256/256/256/256") - 1;

    /*
     * NDK provided abbreviation for the following code:
     *
     * p = ngx_palloc (r->pool, len);
     * if (p == NULL)
     * 		return  NGX_ERROR;
     *
     *
     */
    ndk_palloc_re(p, r->pool, len);

    ngx_memzero (p, len);

    (void) ngx_snprintf(p, len, "%v/%ud/%ud/%ud/%ud/%ud", v, hash[4], hash[3], hash[2], hash[1], hash[0]);

    val->data = p;
    val->len = ngx_strlen(p);

    return  NGX_OK;
}

