dnl ------------------------------------------------------------------------ dnl
dnl Copyright (c) 2012 Los Alamos National Security, LLC
dnl All rights reserved.
dnl
dnl $Revision: 43 $
dnl $Date: 2012-03-27 20:49:25 -0600 (Tue, 27 Mar 2012) $
dnl $Author: bergen $
dnl ------------------------------------------------------------------------ dnl

dnl ------------------------------------------------------------------------ dnl
dnl CONFIG_TEST_STRNDUP()
dnl ------------------------------------------------------------------------ dnl

AC_DEFUN([CONFIG_TEST_STRNDUP], [
	AC_ARG_VAR([HAVE_STRNDUP], [Test for strndup function])

	AC_CHECK_DECLS_ONCE([strndup])

	if test $ac_cv_have_decl_strndup = yes; then
		AC_DEFINE([HAVE_STRNDUP], 1,
			[Define if you have the strndup() function])
	fi
])
