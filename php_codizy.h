/*
  +----------------------------------------------------------------------+
  | codizy                                                              |
  +----------------------------------------------------------------------+
  | Copyright (c) 2008 The PHP Group                                     |
  +----------------------------------------------------------------------+
  | This source file is subject to the new BSD license, that is bundled  |
  | with this package in the file LICENSE.                               |
  +----------------------------------------------------------------------+
  | Author:Surf Chen <surfchen@gmail.com>                                |
  +----------------------------------------------------------------------+
*/

/* $Id: php_codizy.h 63 2010-11-08 13:37:14Z surfchen $ */

#ifndef PHP_CODIZY_H
#define PHP_CODIZY_H

extern zend_module_entry codizy_module_entry;
#define phpext_codizy_ptr &codizy_module_entry
int codizy_add_callback(
    char *function_name,
    int function_len,
    char *callback_name,
    int callback_len,
    int type TSRMLS_DC);


#ifdef PHP_WIN32
#define PHP_CODIZY_API __declspec(dllexport)
#else
#define PHP_CODIZY_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

typedef struct _codizy_function_list {
    char *name;
    zval *func;
    struct _codizy_callback_list *callback_ref;
    struct _codizy_function_list *next;
} codizy_function_list;
typedef struct _codizy_callback_list {
    char *name;
    zval *func;
    struct _codizy_callback_list *next;
} codizy_callback_list;

PHP_MINIT_FUNCTION(codizy);
PHP_MSHUTDOWN_FUNCTION(codizy);
PHP_RINIT_FUNCTION(codizy);
PHP_RSHUTDOWN_FUNCTION(codizy);
PHP_MINFO_FUNCTION(codizy);

PHP_FUNCTION(codizy_add_pre);
PHP_FUNCTION(codizy_add_post);

/* 
  	Declare any global variables you may need between the BEGIN
	and END macros here:     

*/
ZEND_BEGIN_MODULE_GLOBALS(codizy)
    codizy_function_list *codizy_pre_list;
    codizy_function_list *codizy_post_list;
    int use_callback;
ZEND_END_MODULE_GLOBALS(codizy)

#define CALLBACK_DISABLE 0
#define CALLBACK_ENABLE 1

/* In every utility function you add that needs to use variables
   in php_codizy_globals, call TSRMLS_FETCH(); after declaring other 
   variables used by that function, or better yet, pass in TSRMLS_CC
   after the last function argument and declare your utility function
   with TSRMLS_DC after the last declared argument.  Always refer to
   the globals in your function as CODIZY_G(variable).  You are 
   encouraged to rename these macros something shorter, see
   examples in any other php module directory.
*/

#ifdef ZTS
#define FCG(v) TSRMG(codizy_globals_id, zend_codizy_globals *, v)
#else
#define FCG(v) (codizy_globals.v)
#endif

#endif	/* PHP_CODIZY_H */



//#define CODIZY_DEBUG(str) fprintf(stderr,"%s",str)
#define CODIZY_DEBUG(str) ;

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
