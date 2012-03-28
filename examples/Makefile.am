################################################################################
# Copyright (c) 2012 Los Alamos National Security, LLC
# All rights reserved.
#
# $Revision:$
# $Date:$
# $Author:$
################################################################################

bin_PROGRAMS = \
	c_example \
	f90_example \
	reduce

INCLUDES = -I${top_srcdir}/src/include \
	-I${top_builddir}/local \
	@EXTRA_CPPFLAGS@

AM_FCFLAGS = @FC_MODULE_PATH_FLAG@${top_builddir}/lib

c_example_SOURCES = @top_srcdir@/examples/c_example.c
c_example_LDFLAGS = @EXTRA_LDFLAGS@
c_example_LDADD = @EXTRA_LIBS@ ${top_builddir}/lib/libocl.la

f90_example_SOURCES = @top_srcdir@/examples/f90_example.f90
f90_example_LDFLAGS = @EXTRA_LDFLAGS@
f90_example_LDADD = @EXTRA_LIBS@ ${top_builddir}/lib/libocl.la

reduce_SOURCES = @top_srcdir@/examples/reduce.c
reduce_LDFLAGS = @EXTRA_LDFLAGS@
reduce_LDADD = @EXTRA_LIBS@ ${top_builddir}/lib/libocl.la
