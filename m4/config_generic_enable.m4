dnl ------------------------------------------------------------------------ dnl
dnl Copyright (c) 2012 Los Alamos National Security, LLC
dnl All rights reserved.
dnl ------------------------------------------------------------------------ dnl

dnl ------------------------------------------------------------------------ dnl
dnl CONFIG_GENERIC_ENABLE($1 = string name,
dnl							$2 = macro name)
dnl
dnl ------------------------------------------------------------------------ dnl

AC_DEFUN([CONFIG_GENERIC_ENABLE], [
	AC_ARG_VAR([$2], [Enable $1])

	AC_ARG_ENABLE([$1],
		[AC_HELP_STRING([--enable-$1],
		[Enable $1])],
		[
			if test -n "$$2" ; then
				if test "$$2" = "yes" ; then
					enable=1
				else
					enable=0
				fi
			elif test "$enableval" = "yes" ; then
				enable=1
			else
				enable=0
			fi
		],
		[
			if test -n "$$2" ; then
				if test "$$2" = "yes" ; then
					enable=1
				else
					enable=0
				fi
			else
				enable=0
			fi
		])
		
		AC_MSG_CHECKING(to see if $1 is enabled)

		if test "$enable" = "1" ; then
			AC_DEFINE(ENABLE_$2, [1], [Define if you want $2])
			AC_SUBST(enable_$1, [yes])
			AC_MSG_RESULT(yes)
		else
			AC_SUBST(enable_$1, [no])
			AC_MSG_RESULT(no)
		fi
])
