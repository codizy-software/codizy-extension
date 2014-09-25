dnl $Id$
dnl config.m4 for extension codizy

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

dnl PHP_ARG_WITH(codizy, for codizy support,
dnl Make sure that the comment is aligned:
dnl [  --with-codizy             Include codizy support])

dnl Otherwise use enable:

PHP_ARG_ENABLE(codizy, whether to enable codizy support,
[  --enable-codizy           Enable codizy support])

if test "$PHP_CODIZY" != "no"; then
  dnl Write more examples of tests here...

  dnl # --with-codizy -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/codizy.h"  # you most likely want to change this
  dnl if test -r $PHP_CODIZY/$SEARCH_FOR; then # path given as parameter
  dnl   CODIZY_DIR=$PHP_CODIZY
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for codizy files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       CODIZY_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$CODIZY_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the codizy distribution])
  dnl fi

  dnl # --with-codizy -> add include path
  dnl PHP_ADD_INCLUDE($CODIZY_DIR/include)

  dnl # --with-codizy -> check for lib and symbol presence
  dnl LIBNAME=codizy # you may want to change this
  dnl LIBSYMBOL=codizy # you most likely want to change this

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $CODIZY_DIR/lib, CODIZY_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_CODIZYLIB,1,[ ])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong codizy lib version or lib not found])
  dnl ],[
  dnl   -L$CODIZY_DIR/lib -lm -ldl
  dnl ])
  dnl
  dnl PHP_SUBST(CODIZY_SHARED_LIBADD)

  PHP_NEW_EXTENSION(codizy, codizy.c, $ext_shared)
fi
