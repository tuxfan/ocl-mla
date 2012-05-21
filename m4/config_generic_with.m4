dnl ------------------------------------------------------------------------ dnl
dnl Copyright (c) 2012 Los Alamos National Security, LLC
dnl All rights reserved.
dnl ------------------------------------------------------------------------ dnl

dnl ------------------------------------------------------------------------ dnl
dnl CONFIG_GENERIC_WITH($1 = string name,
dnl							$2 = define name,
dnl							$3 = default value)
dnl
dnl The convention is to use all lower-case for the sting name and
dnl all upper-case for the define.
dnl ------------------------------------------------------------------------ dnl

AC_DEFUN([CONFIG_GENERIC_WITH], [
	AC_ARG_VAR([$2], [Specify $1 (default $3)])

	AC_ARG_WITH([$1],
		[AC_HELP_STRING([--with-$1],
		[Specify value for $1])],
		[
			if test -z "$$2" ; then
				define="$withval"
			else
				define="$$2"
			fi
		],
		[
			if test -z "$$2" ; then
				define="$3"
			else
				define="$$2"
			fi
		])

		AC_MSG_CHECKING(specification for $1)
		AC_DEFINE_UNQUOTED($2, $define, [Specify value for $1])
		AC_SUBST($2, [$define])
		AC_MSG_RESULT($define)
])
