!------------------------------------------------------------------------------!
! Copyright (c) 2012 Los Alamos National Security, LLC
! All rights reserved.
!------------------------------------------------------------------------------!

program main
   use ocl
   use kernel_strings
   implicit none

   integer :: i
   integer(int32_t) :: ierr

   integer(c_int), parameter :: ELEMENTS = 8
   integer(c_size_t), parameter :: global_offset = 0
   integer(c_size_t), parameter :: global_size = ELEMENTS
   integer(c_size_t), parameter :: local_size = 1
   integer(c_size_t), parameter :: offset = 0
   integer(c_size_t) :: work_group_indeces
   integer(c_size_t) :: single_indeces
   integer(c_size_t) :: work_group_size
   integer(c_size_t) :: bytes
   integer(c_size_t) :: hint

   real(c_float), target, dimension(ELEMENTS) :: h_array

   type(ocl_allocation_t) :: d_array
   type(ocl_allocation_t) :: event
   type(ocl_allocation_t) :: wait_list

   ! step (1)
   ! data size for buffer copying
   bytes = ELEMENTS*sizeof(h_array(1))

   ! step (2)
   ! initialize array
   h_array = 0

   ! step (3)
   ! initialize the OpenCL layer
   call ocl_init(ierr)

   ! step (4)
   ! create a device-side buffer
   call ocl_create_buffer(OCL_PERFORMANCE_DEVICE, bytes, &
      CL_MEM_READ_ONLY + CL_MEM_COPY_HOST_PTR, c_loc(h_array), d_array, ierr) 

   print *, test_PPSTR

   ! step (5)
   ! add program and build
   call ocl_add_program(OCL_PERFORMANCE_DEVICE, 'program' // C_NULL_CHAR, &
      test_PPSTR // C_NULL_CHAR, '' // C_NULL_CHAR, ierr)
   !call ocl_add_program(OCL_PERFORMANCE_DEVICE, 'program' // C_NULL_CHAR, &
   !   'test.cl' // C_NULL_CHAR, '' // C_NULL_CHAR, ierr)

   ! step (6)
   ! add kernel
   call ocl_add_kernel(OCL_PERFORMANCE_DEVICE, 'program' // C_NULL_CHAR, &
      'test' // C_NULL_CHAR, 'my test' // C_NULL_CHAR, ierr)
 
   call ocl_kernel_hint('program' // C_NULL_CHAR, &
      'my test' // C_NULL_CHAR, hint, ierr)

   work_group_size = 0
   work_group_indeces = 0
   single_indeces = 0

   call ocl_ndrange_hints(global_size, hint, work_group_size, &
      work_group_indeces, single_indeces)

   ! step (7)
   ! Set kernel argument
   call ocl_set_kernel_arg_mem('program' // C_NULL_CHAR, &
      'my test' // C_NULL_CHAR, 0, d_array, ierr)

   ! step (8)
   ! initialize event for timings
   call ocl_initialize_event(event, ierr)
   call ocl_initialize_event_wait_list(wait_list, ierr)

   ! step (9)
   ! invoke kernel
   call ocl_enqueue_kernel_ndrange(OCL_PERFORMANCE_DEVICE, &
      'program' // C_NULL_CHAR, 'my test' // C_NULL_CHAR, &
      1, global_offset, global_size, local_size, event, ierr)

   ! step (10)
   ! block on kernel completion
   call ocl_finish(OCL_PERFORMANCE_DEVICE)

   ! step (11)
   ! add a timer timer event for the kernel invocation
   call ocl_add_timer('kernel' // C_NULL_CHAR, event)

   ! step (12)
   ! read data from device
   call ocl_enqueue_read_buffer(OCL_PERFORMANCE_DEVICE, d_array, 1, offset, &
      bytes, c_loc(h_array), event, ierr)

   ! step (13)
   ! block for read completion
   call ocl_finish(OCL_PERFORMANCE_DEVICE)

   ! step (14)
   ! add a timer event for the buffer read
   call ocl_add_timer('readbuffer' // C_NULL_CHAR, event)

   ! step (15)
   ! print results
   do i=1, ELEMENTS
      print *, h_array(i)
   end do
   print *

   ! step (16)
   ! print timer results
   call ocl_report_timer('kernel' // C_NULL_CHAR)
   call ocl_report_timer('readbuffer' // C_NULL_CHAR)

   ! step (17)
   ! shutdown the OpenCL layer
   call ocl_finalize(ierr)
end
