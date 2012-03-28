dnl ------------------------------------------------------------------------ dnl
dnl Copyright (c) 2012 Los Alamos National Security, LLC
dnl All rights reserved.
dnl
dnl $Revision: 43 $
dnl $Date: 2012-03-27 20:49:25 -0600 (Tue, 27 Mar 2012) $
dnl $Author: bergen $
dnl ------------------------------------------------------------------------ dnl

dnl ------------------------------------------------------------------------ dnl
dnl
dnl ------------------------------------------------------------------------ dnl

AC_DEFUN([CONFIG_PROG_DOXYGEN], [
    AC_ARG_VAR([DOXYGEN],
        [User specified path to the doxygen executable])

    AC_PATH_PROG(DOXYGEN, doxygen)

    if test -n "$DOXYGEN" ; then
        AC_SUBST(HAS_DOXYGEN, [yes])
    else
        AC_SUBST(HAS_DOXYGEN, [no])
    fi
])
