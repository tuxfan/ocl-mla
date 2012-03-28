!------------------------------------------------------------------------------!
! Copyright (c) 2012 Los Alamos National Security, LLC
! All rights reserved.
!
! $Revision: 43 $
! $Date: 2012-03-27 20:49:25 -0600 (Tue, 27 Mar 2012) $
! $Author: bergen $
!------------------------------------------------------------------------------!

module ocl_data

   use, intrinsic :: ISO_C_BINDING

   ! Interoperability types
   integer, parameter :: int32_t = C_INT32_T
   integer, parameter :: int64_t = C_INT64_T
   integer, parameter :: cl_bitfield = C_INT64_T

   ! Buffer creation flags
   integer(cl_bitfield), parameter :: CL_MEM_READ_WRITE     = 2**0 ! (1 << 0)
   integer(cl_bitfield), parameter :: CL_MEM_WRITE_ONLY     = 2**1 ! (1 << 1)
   integer(cl_bitfield), parameter :: CL_MEM_READ_ONLY      = 2**2 ! (1 << 2)
   integer(cl_bitfield), parameter :: CL_MEM_USE_HOST_PTR   = 2**3 ! (1 << 3)
   integer(cl_bitfield), parameter :: CL_MEM_ALLOC_HOST_PTR = 2**4 ! (1 << 4)
   integer(cl_bitfield), parameter :: CL_MEM_COPY_HOST_PTR  = 2**5 ! (1 << 5)

   ! Booleans
   integer(int32_t), parameter :: CL_FALSE = 0
   integer(int32_t), parameter :: CL_TRUE = 1

   ! Dynamically initialized variables
   integer(c_size_t) :: cl_mem_size

   ! Reference type for C pointers
   type, bind(C) :: ocl_reference_t
      type(c_ptr) :: void
   end type ocl_reference_t

   ! OpenCL logical devices
   integer(int32_t), parameter :: OCL_PERFORMANCE_DEVICE = 0
   integer(int32_t), parameter :: OCL_AUXILIARY_DEVICE = 1

   ! Type for C allocations
   type, bind(C) :: ocl_allocation_t
      integer(int32_t) :: idx
      type(c_ptr) :: void
   end type ocl_allocation_t

end module ocl_data
