################################################################################
# Copyright (c) 2012 Los Alamos National Security, LLC
# All rights reserved.
#
# $Revision: 44 $
# $Date: 2012-03-28 09:38:36 -0600 (Wed, 28 Mar 2012) $
# $Author: bergen $
################################################################################

#------------------------------------------------------------------------------#
# Recurse subdirs
#------------------------------------------------------------------------------#

SUBDIRS = lib examples

#------------------------------------------------------------------------------#
# Create links
#------------------------------------------------------------------------------#

links:
	@(cd ${top_srcdir}/src; [ -d include ] && rm -rf include; \
		mkdir include && cd include; \
		for file in `find .. -type f -regex '.*\.h*'` ; do \
			link=`echo $$file`; \
			ln -s $$link .; \
		done)

#------------------------------------------------------------------------------#
# Configure message
#------------------------------------------------------------------------------#

reminder: FORCE
	@(echo ; \
		echo "/*----------------------------------------------------------------------------*\\"; \
		echo " * Configuration Complete!"; \
		echo " * To run the examples, you will need to set the OCL_KERNEL_PATH environment"; \
		echo " * variable to ${top_srcdir}/examples/kernels"; \
		echo "\*----------------------------------------------------------------------------*/")

#------------------------------------------------------------------------------#
# Build documentation
#------------------------------------------------------------------------------#

doc: FORCE
	@([ -d @DOC@ ] && cd @DOC@ && make doc > doxygen.log 2>&1)

#------------------------------------------------------------------------------#
# Extra install exec
#------------------------------------------------------------------------------#

install-exec-hook:
	@(cp ${top_builddir}/lib/*.mod ${libdir}/; \
		cp ${top_builddir}/local/ocl_local.h ${includedir})
	@([ -d ${bindir}/kernels ] || mkdir ${bindir}/kernels && \
		cp ${top_srcdir}/examples/kernels/*.cl ${bindir}/kernels)

#------------------------------------------------------------------------------#
# Extra install data
#------------------------------------------------------------------------------#

install-data-hook:
	@([ -d ${datarootdir} ] || mkdir -p ${datarootdir})
	@([ -d ${mandir} ] || mkdir -p ${mandir})
	@([ -d ${datarootdir}/html ] || mkdir -p ${datarootdir}/html)
	@(cp -r ${top_builddir}/doc/man/* ${mandir})
	@(cp -r ${top_builddir}/doc/html/* ${datarootdir}/html)

#------------------------------------------------------------------------------#
# Unit tests
#------------------------------------------------------------------------------#

test: all
	@(cd test; ${MAKE})

check-local: test
	@(cd test; ${MAKE} check)

#------------------------------------------------------------------------------#
# Extra clean
#------------------------------------------------------------------------------#

clean-local:
	@(echo "rm -f *.mod"; rm -f ${top_builddir}/lib/*.mod)
	@(cd test; ${MAKE} clean)

#------------------------------------------------------------------------------#
# Extra distribution
#------------------------------------------------------------------------------#

dist-hook:
	@(mkdir -p ${distdir}/machine/templates && \
		cp ${top_srcdir}/machine/README ${distdir}/machine && \
		cp ${top_srcdir}/machine/templates/* ${distdir}/machine/templates)
	@(mkdir ${distdir}/examples/kernels && \
		cp ${top_srcdir}/examples/kernels/*.cl ${distdir}/examples/kernels)
	@(mkdir ${distdir}/doc && \
		cp ${top_srcdir}/doc/Makefile.* ${distdir}/doc && \
		cp ${top_srcdir}/doc/doxygen.conf.in ${distdir}/doc)

#------------------------------------------------------------------------------#
# Dummy rule
#------------------------------------------------------------------------------#

FORCE:
