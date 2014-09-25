/*
  +----------------------------------------------------------------------+
  | codizy                                                              |
  +----------------------------------------------------------------------+
  | Copyright (c) 2008-2010  Chen Ze                                     |
  | Copyright (c) 2013-2014  Codizy                                      |
  +----------------------------------------------------------------------+
  | This source file is subject to the new BSD license, that is bundled  |
  | with this package in the file LICENSE.                               |
  +----------------------------------------------------------------------+
  | Author:Chen Ze <surfchen@gmail.com>                                  |
  |        Codizy  <technology@codizy.com>                               |
  +----------------------------------------------------------------------+
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_codizy.h"
#include "Zend/zend_execute.h"
#include "ext/standard/php_var.h"

ZEND_DECLARE_MODULE_GLOBALS(codizy)

/* True global resources - no need for thread safety here */
int codizy_pre_inc(ZEND_OPCODE_HANDLER_ARGS);
int codizy_post_inc(ZEND_OPCODE_HANDLER_ARGS);

#if ZEND_MODULE_API_NO < 20121113
ZEND_DLEXPORT void codizy_execute_internal(zend_execute_data *execute_data, int return_value_used TSRMLS_DC);
ZEND_DLEXPORT void (*codizy_zend_execute_internal)(zend_execute_data *execute_data, int return_value_used TSRMLS_DC);
#else
ZEND_DLEXPORT void codizy_execute_internal(zend_execute_data *execute_data, struct _zend_fcall_info *fci, int return_value_used TSRMLS_DC);
ZEND_DLEXPORT void (*codizy_zend_execute_internal)(zend_execute_data *execute_data, struct _zend_fcall_info *fci, int return_value_used TSRMLS_DC);
#endif
#define NEW_FN_LIST(li,fname,len) li=emalloc(sizeof(codizy_function_list));                   \
    li->name=emalloc(len+1);\
    strcpy(li->name,fname);\
    MAKE_STD_ZVAL(li->func);\
    ZVAL_STRING(li->func,li->name,0);\
    li->next=NULL

#define NEW_CB_LIST(li,fname,len) li=emalloc(sizeof(codizy_callback_list));                   \
    li->name=emalloc(len+1);\
    strcpy(li->name,fname);\
    MAKE_STD_ZVAL(li->func);\
    ZVAL_STRING(li->func,li->name,0);\
    li->next=NULL

#define MICRO_IN_SEC 1000000.00

/* {{{ codizy_functions[]
 *
 * Every user visible function must have an entry in codizy_functions[].
 */
zend_function_entry codizy_functions[] = {
	PHP_FE(codizy_add_pre,	NULL)
	PHP_FE(codizy_add_post,	NULL)
	#if ZEND_MODULE_API_NO >= 20090626
	PHP_FE_END
	#else
	{NULL, NULL, NULL}	/* Must be the last line in codizy_functions[] */
	#endif
};
/* }}} */

/* {{{ codizy_module_entry
 */
zend_module_entry codizy_module_entry = {
	STANDARD_MODULE_HEADER,
	"codizy",
	codizy_functions,
	PHP_MINIT(codizy),
	PHP_MSHUTDOWN(codizy),
	PHP_RINIT(codizy),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(codizy),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(codizy),
	"1.4", /* Replace with version number for your extension */
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_CODIZY
ZEND_GET_MODULE(codizy)
#endif

PHP_INI_BEGIN()
	PHP_INI_ENTRY("codizy.web_server_url", "", PHP_INI_ALL, NULL)
PHP_INI_END()


/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(codizy)
{
	CODIZY_DEBUG("MINIT begin\n");
#ifdef ZTS
    ZEND_INIT_MODULE_GLOBALS(codizy,NULL,NULL);
#endif
    codizy_zend_execute_internal=zend_execute_internal;
    zend_execute_internal=codizy_execute_internal;

	REGISTER_INI_ENTRIES();

	CODIZY_DEBUG("MINIT end\n");
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(codizy)
{
    zend_execute_internal=codizy_zend_execute_internal;

	UNREGISTER_INI_ENTRIES();

    //FREE_ZVAL(FCG(codizy_null_zval));
	CODIZY_DEBUG("MSHUTDOWN\n");
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(codizy)
{
	CODIZY_DEBUG("RINIT begin\n");

    FCG(use_callback)=CALLBACK_DISABLE;
    FCG(codizy_pre_list)=NULL;
    FCG(codizy_post_list)=NULL;
	CODIZY_DEBUG("RINIT end\n");
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(codizy)
{

	CODIZY_DEBUG("RSHUTDOWN begin\n");

	//php_var_dump(FCG(codizy_null_zval),1 TSRMLS_CC);
    codizy_function_list *f_list,*tmp_list;
    codizy_callback_list *cb,*tmp_cb;

    f_list=FCG(codizy_pre_list);
    while (f_list) {
        tmp_list=f_list->next;
        cb=f_list->callback_ref;
        while (cb) {
            tmp_cb=cb->next;
            FREE_ZVAL(cb->func);
            efree(cb->name);
            efree(cb);
            cb=tmp_cb;
        }
        FREE_ZVAL(f_list->func);
        efree(f_list->name);
        efree(f_list);
        f_list=tmp_list;
    }
    

    f_list=FCG(codizy_post_list);
    while (f_list) {
        tmp_list=f_list->next;
        cb=f_list->callback_ref;
        while (cb) {
            tmp_cb=cb->next;
            FREE_ZVAL(cb->func);
            efree(cb->name);
            efree(cb);
            cb=tmp_cb;
        }
        FREE_ZVAL(f_list->func);
        efree(f_list->name);
        efree(f_list);
        f_list=tmp_list;
    }
    FCG(use_callback)=CALLBACK_DISABLE;
    FCG(codizy_pre_list)=NULL;
    FCG(codizy_post_list)=NULL;
	CODIZY_DEBUG("RSHUTDOWN end\n");
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(codizy)
{
	php_info_print_table_start();
	php_info_print_table_row(2, "Codizy version", "1.4");
	php_info_print_table_end();

	DISPLAY_INI_ENTRIES();
}
/* }}} */


/* {{{ proto boolean codizy_add_pre(string function,string callback)
    Add a pre-callback. Return true if successfully */
PHP_FUNCTION(codizy_add_pre)
{
	CODIZY_DEBUG("codizy_add_pre() begin\n");
    char *function_name;
    char *callback_name;
    int function_len, callback_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss", &function_name, &function_len, &callback_name, &callback_len) == FAILURE) {
		return;
	}
    codizy_add_callback(function_name,function_len,callback_name,callback_len,0 TSRMLS_CC);
	CODIZY_DEBUG("codizy_add_pre() end\n");

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto boolean codizy_add_post(string function,string callback)
   Add a post-callback.Return true if successfully  */
PHP_FUNCTION(codizy_add_post)
{
    char *function_name;
    char *callback_name;
    int function_len, callback_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss", &function_name, &function_len, &callback_name, &callback_len) == FAILURE) {
		return;
	}
    codizy_add_callback(function_name,function_len,callback_name,callback_len,1 TSRMLS_CC);
	RETURN_TRUE;
}
/* }}} */

static double microtime(TSRMLS_D) {
	double dt;
    struct timeval tp = {0};

    if (gettimeofday(&tp, NULL)) {
        return 0;
    }

    return (double)(tp.tv_sec + tp.tv_usec / MICRO_IN_SEC);
}


static char *get_current_function_name(TSRMLS_D) 
{
    char *current_function;
    char *space;
    char *class_name;
    char *fname;
    class_name=get_active_class_name(&space TSRMLS_CC);

    if (strlen(space)==2) {
        fname = get_active_function_name(TSRMLS_C);
        current_function=emalloc(strlen(class_name)+3+strlen(fname));
        memset(current_function,0,strlen(class_name)+3+strlen(fname));
        strcpy(current_function,class_name);
        strcat(current_function,"::");
        strcat(current_function,fname);
    } else {
        current_function = get_active_function_name(TSRMLS_C);
    }
    if (!current_function) {
        current_function="main";
    }
    return current_function;
}


static ulong get_current_function_args(zval **args[], int get_prev TSRMLS_DC) {
    args[0] = (zval**)emalloc(sizeof(zval**));
    MAKE_STD_ZVAL(*args[0]);
    array_init(*args[0]);

	int i;
	/*These get-args-code is borrowed from ZEND_FUNCTION(func_get_args)*/
#if ZEND_MODULE_API_NO >= 20071006
	zend_execute_data *ex;
	#if PHP_VERSION_ID < 50500
		ex = EG(current_execute_data);
	#else
		if (get_prev) {
			ex = EG(current_execute_data)->prev_execute_data;
		} else {
			ex = EG(current_execute_data);
		}
	#endif
	if (!ex || !ex->function_state.arguments) {
		return 0;
	}
	void **p = ex->function_state.arguments;
	int arg_count = (int)(zend_uintptr_t) *p;
#else
	void **p = EG(argument_stack).top_element-2;
	int arg_count = (int)(zend_ulong) *p;
#endif
	for (i=0; i<arg_count; i++) {
		zval *element;
		ALLOC_ZVAL(element);
		*element = **((zval **) (p-(arg_count-i)));
		zval_copy_ctor(element);
		INIT_PZVAL(element);
		zend_hash_next_index_insert((*args[0])->value.ht, &element, sizeof(element), NULL);
	}
    return zend_hash_next_free_element((*args[0])->value.ht);
}

static int extract_current_object(zval* object, zval **args[], ulong nextid TSRMLS_DC) {
    if (object) {
		zval *element;
		ALLOC_ZVAL(element);
		*element = *object;
		zval_copy_ctor(element);
		INIT_PZVAL(element);
		zend_hash_index_update((*args[0])->value.ht, nextid, &element, sizeof(element), NULL);
		return 1;
	} else {
		return 0;
	}
}


static int callback_exist(char *func_name TSRMLS_DC) {
    codizy_function_list *pre_list,*post_list;
    pre_list=FCG(codizy_pre_list);
    post_list=FCG(codizy_post_list);

    while (pre_list) {
        if (!strcmp(pre_list->name,func_name)) {
            return 1;
        }
        pre_list=pre_list->next;
    }
    while (post_list) {
        if (!strcmp(post_list->name,func_name)) {
            return 1;
        }
        post_list=post_list->next;
    }
    return 0;
}

int codizy_add_callback(
    char *function_name,
    int function_len,
    char *callback_name,
    int callback_len,
    int type TSRMLS_DC)
{
	CODIZY_DEBUG("codizy_add_callback() begin\n");
    codizy_function_list *tmp_gfl,*gfl,*new_gfl;
    codizy_callback_list *cl=NULL,*new_cl;
    if (type==0) {
        tmp_gfl=FCG(codizy_pre_list);
    } else {
        tmp_gfl=FCG(codizy_post_list);
    }
    if (!tmp_gfl) {
		CODIZY_DEBUG("no tmp_gfl\n");
        if (type==0) {
            NEW_FN_LIST(FCG(codizy_pre_list),function_name,function_len);
            gfl=FCG(codizy_pre_list);
        } else {
            NEW_FN_LIST(FCG(codizy_post_list),function_name,function_len);
            gfl=FCG(codizy_post_list);
        }
    } else {
		CODIZY_DEBUG("yes tmp_gfl\n");
        if (type==0) {
            gfl=FCG(codizy_pre_list);
        } else {
            gfl=FCG(codizy_post_list);
        }
         while (1) {
            if (!strcmp(gfl->name,function_name)) {
                cl=gfl->callback_ref;
                break;
            }
            if (!gfl->next) {
                NEW_FN_LIST(new_gfl,function_name,function_len);
                gfl->next=new_gfl;
                gfl=new_gfl;
                break;
            }
            gfl=gfl->next;
        }
    }

    if (!cl) {
        NEW_CB_LIST(cl,callback_name,callback_len);
        gfl->callback_ref=cl;
    } else {
        while (1) {
            if (!strcmp(cl->name,callback_name)) {
                return 0;
            }
            if (!cl->next) {
                NEW_CB_LIST(new_cl,callback_name,callback_len);
                cl->next=new_cl;
                break;
            }
            cl=cl->next;
        }
    }
    FCG(use_callback)=CALLBACK_ENABLE;
	return 1;
}

static void codizy_do_callback(char *current_function,zval *** args,int type TSRMLS_DC) {
	CODIZY_DEBUG("codizy_do_callback begin\n");
    codizy_function_list *codizy_list;
    codizy_callback_list *cl;
    int arg_count;
    if (type==0) {
        arg_count=1;
        codizy_list=FCG(codizy_pre_list);
    } else {
        arg_count=3;
        codizy_list=FCG(codizy_post_list);
    }
    zval *retval=NULL;

    while (codizy_list) {
        if (!strcmp(codizy_list->name,current_function)) {
            cl=codizy_list->callback_ref;
            while (cl) {
				CODIZY_DEBUG("calling func begin\n");
                FCG(use_callback)=CALLBACK_DISABLE;
                call_user_function_ex(EG(function_table), NULL, cl->func, &retval, arg_count, args, 0,NULL TSRMLS_CC);
				CODIZY_DEBUG("calling func end\n");
                if (retval) {
                    FREE_ZVAL(retval);
                }
                FCG(use_callback)=CALLBACK_ENABLE;
                cl=cl->next;
            }
            break;
        }
        codizy_list=codizy_list->next;
    }
 
	CODIZY_DEBUG("codizy_do_callback end\n");
}

#if ZEND_MODULE_API_NO < 20121113
ZEND_API void codizy_execute_internal(zend_execute_data *execute_data_ptr, int return_value_used TSRMLS_DC)
#else
ZEND_API void codizy_execute_internal(zend_execute_data *execute_data_ptr, struct _zend_fcall_info *fci, int return_value_used TSRMLS_DC)
#endif
{
	CODIZY_DEBUG("codizy_execute_internal begin\n");
	/*
	zend_execute_data *exec_data = EG(current_execute_data);
	CODIZY_DEBUG("exec_data got\n");
    if (exec_data->opline) {
		CODIZY_DEBUG("we have opline\n");
        zend_op *opline  = exec_data->opline;
		fprintf(stderr,"xx99:%d\n",opline->lineno);
	}*/
    if (FCG(use_callback)==CALLBACK_DISABLE) {
		CODIZY_DEBUG("CALLBACK_DISABLE\n");
		if (codizy_zend_execute_internal) {
#if ZEND_MODULE_API_NO < 20121113
			codizy_zend_execute_internal(execute_data_ptr, return_value_used TSRMLS_CC);
#else
			codizy_zend_execute_internal(execute_data_ptr, fci, return_value_used TSRMLS_CC);
#endif
		} else {
#if ZEND_MODULE_API_NO < 20121113
			execute_internal(execute_data_ptr, return_value_used TSRMLS_CC);
#else
			execute_internal(execute_data_ptr, fci, return_value_used TSRMLS_CC);
#endif
		}
        return;
    }
    char *current_function;
    current_function=get_current_function_name(TSRMLS_C);
    if (callback_exist(current_function TSRMLS_CC)==0) {
		if (codizy_zend_execute_internal) {
#if ZEND_MODULE_API_NO < 20121113
			codizy_zend_execute_internal(execute_data_ptr, return_value_used TSRMLS_CC);
#else
			codizy_zend_execute_internal(execute_data_ptr, fci, return_value_used TSRMLS_CC);
#endif
		} else {
#if ZEND_MODULE_API_NO < 20121113
			execute_internal(execute_data_ptr, return_value_used TSRMLS_CC);
#else
			execute_internal(execute_data_ptr, fci, return_value_used TSRMLS_CC);
#endif
		}
    } else {
        zval ***args=NULL;
        args = (zval ***)safe_emalloc(3,sizeof(zval **), 0);
        zval *t;
        zend_execute_data *ptr;
        zval **return_value_ptr;

        ptr = EG(current_execute_data);
        ulong nextid = get_current_function_args(args, 0 TSRMLS_CC);
        int in_object = 0;
	    char *space;
	    char *class_name;
	    char *fname;
	    class_name = get_active_class_name(&space TSRMLS_CC);
        fname = get_active_function_name(TSRMLS_C);

        if (strcmp(fname, "__construct") != 0 && strcmp(class_name, fname) != 0) {
	        in_object = extract_current_object(ptr->object, args, nextid TSRMLS_CC);
	    }
        codizy_do_callback(current_function,args,0 TSRMLS_CC);

		if (in_object) {
			if (FCG(use_callback) == CALLBACK_ENABLE) {
				FCG(use_callback) = CALLBACK_DISABLE;
				zend_hash_index_del((*args[0])->value.ht, nextid);
				FCG(use_callback) = CALLBACK_ENABLE;
			} else {
	            zend_hash_index_del((*args[0])->value.ht, nextid);
	        }
	    }

        double start_time=microtime(TSRMLS_C);
		if (codizy_zend_execute_internal) {
#if ZEND_MODULE_API_NO < 20121113
			codizy_zend_execute_internal(execute_data_ptr, return_value_used TSRMLS_CC);
#else
			codizy_zend_execute_internal(execute_data_ptr, fci, return_value_used TSRMLS_CC);
#endif
		} else {
#if ZEND_MODULE_API_NO < 20121113
			execute_internal(execute_data_ptr, return_value_used TSRMLS_CC);
#else
			execute_internal(execute_data_ptr, fci, return_value_used TSRMLS_CC);
#endif
		}
        double process_time=microtime(TSRMLS_C)-start_time;

        //zend_printf("|%ld++\n",process_time);
        MAKE_STD_ZVAL(t);
        ZVAL_DOUBLE(t,process_time);
        args[2] = &t;

#if ZEND_MODULE_API_NO >= 20121113
        return_value_ptr = EX_TMP_VAR(ptr, ptr->opline->result.var)->var.ptr_ptr;
#elif ZEND_MODULE_API_NO >= 20100525
        return_value_ptr = &(*(temp_variable *)((char *) ptr->Ts + ptr->opline->result.var)).var.ptr;
#else
        return_value_ptr = &(*(temp_variable *)((char *) ptr->Ts + ptr->opline->result.u.var)).var.ptr;
#endif
        args[1] = return_value_ptr;

		extract_current_object(ptr->object, args, nextid TSRMLS_CC);
        codizy_do_callback(current_function,args,1 TSRMLS_CC);

		if (FCG(use_callback) == CALLBACK_ENABLE) {
			FCG(use_callback) = CALLBACK_DISABLE;
			zend_hash_destroy((*args[0])->value.ht);
			FCG(use_callback) = CALLBACK_ENABLE;
		} else {
            zend_hash_destroy((*args[0])->value.ht);
        }

        FREE_HASHTABLE((*args[0])->value.ht);
        FREE_ZVAL(*args[0]);
        efree(args[0]);
        efree(args);
        FREE_ZVAL(t);
    }
    if (strchr(current_function,':')!=NULL) {
        efree(current_function);
    }
	CODIZY_DEBUG("codizy_execute_internal end\n");
}




/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
