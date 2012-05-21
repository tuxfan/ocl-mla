################################################################################
# Copyright (c) 2012 Los Alamos National Security, LLC
# All rights reserved.
################################################################################

#------------------------------------------------------------------------------#
# Recurse subdirs
#------------------------------------------------------------------------------#

SUBDIRS = lib utils examples test

#------------------------------------------------------------------------------#
# Utils file
#------------------------------------------------------------------------------#

EXTRA_DIST = utils/stringify_kernel_files

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
		cp ${top_builddir}/local/ocl_config.h ${includedir}; \
		cp ${top_builddir}/local/ocl_local.h ${includedir};)
	@([ -d ${bindir}/kernels ] || mkdir ${bindir}/kernels && \
		cp ${top_srcdir}/examples/kernels/*.cl ${bindir}/kernels)

#------------------------------------------------------------------------------#
# Extra install data
#------------------------------------------------------------------------------#

install-data-hook:
	@([ -d ${top_builddir}/@DOC@ ] && \
		([ -d ${datarootdir} ] || mkdir -p ${datarootdir}) && \
		([ -d ${mandir} ] || mkdir -p ${mandir}) && \
		([ -d ${datarootdir}/html ] || mkdir -p ${datarootdir}/html) && \
		([ -d ${top_builddir}/doc/man ] && \
			cp -r ${top_builddir}/doc/man/* ${mandir}) && \
		([ -d ${top_builddir}/doc/html ] && \
			cp -r ${top_builddir}/doc/html/* ${datarootdir}/html))

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
