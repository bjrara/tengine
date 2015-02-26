#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

static ngx_int_t ngx_http_mytest_handler(ngx_http_request_t *r);

typedef struct {
	ngx_str_t test_str;
	ngx_int_t test_num;
	ngx_flag_t test_flag;
	size_t test_size;
	ngx_array_t *test_str_array;
	ngx_array_t *test_keyval;
	off_t test_off;
	ngx_msec_t test_msec;
	time_t test_sec;
	ngx_bufs_t test_bufs;
	ngx_uint_t test_enum_seq;
	ngx_uint_t test_bitmask;
	ngx_uint_t test_access;
	ngx_path_t *test_path;
	
	ngx_str_t my_config_str;
	ngx_int_t my_config_num;
} ngx_http_mytest_conf_t;

static char *ngx_http_mytest(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
	ngx_http_core_loc_conf_t *clcf;
	clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
	clcf -> handler = ngx_http_mytest_handler;
	
	return NGX_CONF_OK;	
}

static ngx_command_t ngx_http_mytest_commands[] = {
	{
		ngx_string("mytest"),
		NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_NOARGS,
		ngx_http_mytest, // ngx_command_t setter
		NGX_HTTP_LOC_CONF_OFFSET,
		0,
		NULL
	},
	ngx_null_command
};

// request 的所有信息（方法、URI、协议版本号和头部等）抖可以在 r  中取得
static ngx_int_t ngx_http_mytest_handler(ngx_http_request_t *r)
{
	if (!(r -> method & (NGX_HTTP_GET|NGX_HTTP_HEAD))) {
		return NGX_HTTP_NOT_ALLOWED;
	}
	ngx_int_t rc = ngx_http_discard_request_body(r);
	if (rc != NGX_OK) {
		return rc;
	}
	
	ngx_str_t type = ngx_string("text/plain");
	ngx_str_t response = ngx_string("Hello world");
	r -> headers_out.status = NGX_HTTP_OK;
	r -> headers_out.content_length_n = response.len;
	r -> headers_out.content_type = type;
	
	rc = ngx_http_send_header(r);
	if (rc == NGX_ERROR || rc > NGX_OK || r -> header_only) {
		return rc;
	}
	
	ngx_buf_t *b;
	b = ngx_create_temp_buf(r -> pool, response.len);
	if (b == NULL) {
		return NGX_HTTP_INTERNAL_SERVER_ERROR;
	}
	
	ngx_memcpy(b -> pos, response.data, response.len);
	// 一定要设置好 last 指针
	b -> last = b -> pos + response.len;
	// 声明这是最后一块缓冲区
	b -> last_buf = 1;
	
	ngx_chain_t out;
	out.buf = b;
	out.next = NULL;
	
	// 最后一步为发送包体，发送结束后 HTTP 框架会调用 ngx_http_finalize_request 方法结束请求
	return ngx_http_output_filter(r, &out);
}
/*
static void *ngx_http_mytest_create_loc_conf(ngx_conf_t *cf)
{
	ngx_http_mytest_conf_t *mycf;
	mycf = (ngx_http_mytest_conf_t *)ngx_pcalloc(cf -> pool, sizeof(ngx_http_mytest_conf_t));
	if (mycf == NULL) {
		return NULL;
	}
	
	mycf -> test_flag = NGX_CONF_UNSET;
	mycf -> test_num = NGX_CONF_UNSET;
	mycf -> test_str_array = NGX_CONF_UNSET_PTR;
	mycf -> test_keyval = NULL;
	mycf -> test_off = NGX_CONF_UNSET;
	mycf -> test_msec = NGX_CONF_UNSET_MSEC;
	mycf -> test_sec = NGX_CONF_UNSET;
	mycf -> test_size = NGX_CONF_UNSET_SIZE;
	
	return mycf;
}
*/
// 如果没有什么工作是必须在 http 框架初始化时完成的，那就不必实现 ngx_http_module_t 的回调方法
static ngx_http_module_t ngx_http_mytest_module_ctx = {
	NULL,		/* preconfiguration */
	NULL,		/* postconfiguration */
	
	NULL,		/* create main configuration, http{} */
	NULL,		/* init main configuration */
	
	NULL,		/* create server configuration, http{}, server{} */
	NULL,		/* merge server configuration */
	
	NULL,		/* create location configuration, http{}, server{}, location{} */
	NULL,		/* merge location configuration */
};

ngx_module_t ngx_http_mytest_module = {
	NGX_MODULE_V1,
	&ngx_http_mytest_module_ctx,	/* module context */
	ngx_http_mytest_commands,		/* module directives */
	NGX_HTTP_MODULE,				/* module type */
	NULL,							/* init master */
	NULL,							/* init module */
	NULL,							/* init process */
	NULL,							/* init thread */
	NULL,							/* exit thread */
	NULL,							/* exit process */
	NULL,							/* exit master */
	NGX_MODULE_V1_PADDING
};