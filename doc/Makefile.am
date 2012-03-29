################################################################################
# Copyright (c) 2012 Los Alamos National Security, LLC
# All rights reserved.
################################################################################

all: doc

doc:
	@(echo "@DOXYGEN@ ./doxygen.conf > ./doxygen.log 2>&1")
	@(@DOXYGEN@ ./doxygen.conf > ./doxygen.log 2>&1)

clean-local:
	@(echo "rm -rf doxygen.log html latex man")
	@(rm -rf doxygen.log html latex man)
