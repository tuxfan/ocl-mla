################################################################################
# Copyright (c) 2012 Los Alamos National Security, LLC
# All rights reserved.
#
# $Revision: 43 $
# $Date: 2012-03-27 20:49:25 -0600 (Tue, 27 Mar 2012) $
# $Author: bergen $
################################################################################

bld.test('file_kernel', 'file_kernel.f90')
bld.test_input_files('test.cl')
