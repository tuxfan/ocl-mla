!------------------------------------------------------------------------------!
! Copyright (c) 2012 Los Alamos National Security, LLC
! All rights reserved.
!------------------------------------------------------------------------------!

module ocl_bindings

interface

   !---------------------------------------------------------------------------!
   ! ocl_init_f90
   !---------------------------------------------------------------------------!

   function ocl_init_f90() &
      result(ierr) bind(C, name="ocl_init_f90")
      use, intrinsic :: ISO_C_BINDING
      use :: ocl_data
      implicit none
      integer(int32_t) :: ierr
   end function ocl_init_f90

   !---------------------------------------------------------------------------!
   ! ocl_finalize_f90
   !---------------------------------------------------------------------------!

   function ocl_finalize_f90() &
      result(ierr) bind(C, name="ocl_finalize_f90")
      use, intrinsic :: ISO_C_BINDING
      use :: ocl_data
      implicit none
      integer(int32_t) :: ierr
   end function ocl_finalize_f90

   !---------------------------------------------------------------------------!
   ! ocl_mem_size_f90
   !---------------------------------------------------------------------------!

   function ocl_mem_size_f90() &
      result(mem_size) bind(C, name="ocl_mem_size_f90")
      use, intrinsic :: ISO_C_BINDING
      implicit none
      integer(c_size_t) :: mem_size
   end function ocl_mem_size_f90

   !---------------------------------------------------------------------------!
   ! ocl_create_buffer_f90
   !---------------------------------------------------------------------------!

   function ocl_create_buffer_f90(device_id, elements, flags, &
      host_ptr, buffer) &
      result(ierr) bind(C, name="ocl_create_buffer_f90")
      use, intrinsic :: ISO_C_BINDING
      use :: ocl_data
      implicit none
      integer(int32_t), value :: device_id
      integer(c_size_t), value :: elements
      integer(cl_bitfield), value :: flags
      type(c_ptr), value :: host_ptr
      type(ocl_allocation_t) :: buffer
      integer(int32_t) :: ierr
   end function ocl_create_buffer_f90

   !---------------------------------------------------------------------------!
   ! ocl_release_buffer_f90
   !---------------------------------------------------------------------------!

   function ocl_release_buffer_f90(buffer) &
      result(ierr) bind(C, name="ocl_release_buffer_f90")
      use, intrinsic :: ISO_C_BINDING
      use :: ocl_data
      implicit none
      type(ocl_allocation_t) :: buffer
      integer(int32_t) :: ierr
   end function ocl_release_buffer_f90

   !---------------------------------------------------------------------------!
   ! ocl_enqueue_write_buffer_f90
   !---------------------------------------------------------------------------!

   function ocl_enqueue_write_buffer_f90(device_id, buffer, synchronous, &
      offset, cb, ptr, event) &
      result(ierr) bind(C, name="ocl_enqueue_write_buffer_f90")
      use, intrinsic :: ISO_C_BINDING
      use :: ocl_data
      implicit none
      integer(int32_t), value :: device_id
      type(ocl_allocation_t) :: buffer
      integer(int32_t), value :: synchronous
      integer(c_size_t), value :: offset
      integer(c_size_t), value :: cb
      type(c_ptr), value :: ptr
      type(ocl_allocation_t) :: event
      integer(int32_t) :: ierr
   end function ocl_enqueue_write_buffer_f90

   !---------------------------------------------------------------------------!
   ! ocl_enqueue_read_buffer_f90
   !---------------------------------------------------------------------------!

   function ocl_enqueue_read_buffer_f90(device_id, buffer, synchronous, &
      offset, cb, ptr, event) &
      result(ierr) bind(C, name="ocl_enqueue_read_buffer_f90")
      use, intrinsic :: ISO_C_BINDING
      use :: ocl_data
      implicit none
      integer(int32_t), value :: device_id
      type(ocl_allocation_t) :: buffer
      integer(int32_t), value :: synchronous
      integer(c_size_t), value :: offset
      integer(c_size_t), value :: cb
      type(c_ptr), value :: ptr
      type(ocl_allocation_t) :: event
      integer(int32_t) :: ierr
   end function ocl_enqueue_read_buffer_f90

   !---------------------------------------------------------------------------!
   ! ocl_initialize_event_f90
   !---------------------------------------------------------------------------!

   function ocl_initialize_event_f90(event) &
      result(ierr) bind(C, name="ocl_initialize_event_f90")
      use, intrinsic :: ISO_C_BINDING
      use :: ocl_data
      implicit none
      type(ocl_allocation_t) :: event
      integer(int32_t) :: ierr
   end function ocl_initialize_event_f90

   !---------------------------------------------------------------------------!
   ! ocl_release_event_f90
   !---------------------------------------------------------------------------!

   function ocl_release_event_f90(event) &
      result(ierr) bind(C, name="ocl_release_event_f90")
      use, intrinsic :: ISO_C_BINDING
      use :: ocl_data
      implicit none
      type(ocl_allocation_t) :: event
      integer(int32_t) :: ierr
   end function ocl_release_event_f90

   !---------------------------------------------------------------------------!
   ! ocl_initialize_event_wait_list_f90
   !---------------------------------------------------------------------------!

   function ocl_initialize_event_wait_list_f90(list) &
      result(ierr) bind(C, name="ocl_initialize_event_wait_list_f90")
      use, intrinsic :: ISO_C_BINDING
      use :: ocl_data
      implicit none
      type(ocl_allocation_t) :: list
      integer(int32_t) :: ierr
   end function ocl_initialize_event_wait_list_f90

   !---------------------------------------------------------------------------!
   ! ocl_add_event_to_wait_list_f90
   !---------------------------------------------------------------------------!

   function ocl_add_event_to_wait_list_f90(list, event) &
      result(ierr) bind(C, name="ocl_add_event_to_wait_list_f90")
      use, intrinsic :: ISO_C_BINDING
      use :: ocl_data
      implicit none
      type(ocl_allocation_t) :: list
      type(ocl_allocation_t) :: event
      integer(int32_t) :: ierr
   end function ocl_add_event_to_wait_list_f90

   !---------------------------------------------------------------------------!
   ! ocl_set_event_list_f90
   !---------------------------------------------------------------------------!

   function ocl_set_event_list_f90(event, list) &
      result(ierr) bind(C, name="ocl_set_event_list_f90")
      use, intrinsic :: ISO_C_BINDING
      use :: ocl_data
      implicit none
      type(ocl_allocation_t) :: event
      type(ocl_allocation_t) :: list
      integer(int32_t) :: ierr
   end function ocl_set_event_list_f90

   !---------------------------------------------------------------------------!
   ! ocl_clear_event_f90
   !---------------------------------------------------------------------------!

   function ocl_clear_event_f90(event) &
      result(ierr) bind(C, name="ocl_clear_event_f90")
      use, intrinsic :: ISO_C_BINDING
      use :: ocl_data
      implicit none
      type(ocl_allocation_t) :: event
      integer(int32_t) :: ierr
   end function ocl_clear_event_f90

   !---------------------------------------------------------------------------!
   ! ocl_clear_event_wait_list_f90
   !---------------------------------------------------------------------------!

   function ocl_clear_event_wait_list_f90(list) &
      result(ierr) bind(C, name="ocl_clear_event_wait_list_f90")
      use, intrinsic :: ISO_C_BINDING
      use :: ocl_data
      implicit none
      type(ocl_allocation_t) :: list
      integer(int32_t) :: ierr
   end function ocl_clear_event_wait_list_f90

   !---------------------------------------------------------------------------!
   ! ocl_wait_for_events_f90
   !---------------------------------------------------------------------------!

   function ocl_wait_for_events_f90(list) &
      result(ierr) bind(C, name="ocl_wait_for_events_f90")
      use, intrinsic :: ISO_C_BINDING
      use :: ocl_data
      implicit none
      type(ocl_allocation_t) :: list
      integer(int32_t) :: ierr
   end function ocl_wait_for_events_f90

   !---------------------------------------------------------------------------!
   ! ocl_add_program_f90
   !---------------------------------------------------------------------------!

   function ocl_add_program_f90(device_id, program_name, program_source, &
      compile_options) &
      result(ierr) bind(C, name="ocl_add_program_f90")
      use, intrinsic :: ISO_C_BINDING
      use :: ocl_data
      implicit none
      integer(int32_t), value :: device_id
      character(kind=c_char), dimension(*) :: program_name
      character(kind=c_char), dimension(*) :: program_source
      character(kind=c_char), dimension(*) :: compile_options
      integer(int32_t) :: ierr
   end function ocl_add_program_f90

   !---------------------------------------------------------------------------!
   ! ocl_add_kernel_f90
   !---------------------------------------------------------------------------!

   function ocl_add_kernel_f90(device_id, program_name, kernel_source_name, &
      kernel_name) &
      result(ierr) bind(C, name="ocl_add_kernel_f90")
      use, intrinsic :: ISO_C_BINDING
      use :: ocl_data
      implicit none
      integer(int32_t), value :: device_id
      character(kind=c_char), dimension(*) :: program_name
      character(kind=c_char), dimension(*) :: kernel_source_name
      character(kind=c_char), dimension(*) :: kernel_name
      integer(int32_t) :: ierr
   end function ocl_add_kernel_f90

   !---------------------------------------------------------------------------!
   ! ocl_add_kernel_arg_f90
   !---------------------------------------------------------------------------!

   function ocl_set_kernel_arg_f90(program_name, kernel_name, &
      arg_index, arg_size, arg_value) &
      result(ierr) bind(C, name="ocl_set_kernel_arg_f90")
      use, intrinsic :: ISO_C_BINDING
      use :: ocl_data
      implicit none
      character(kind=c_char), dimension(*) :: program_name
      character(kind=c_char), dimension(*) :: kernel_name
      integer(int32_t), value :: arg_index
      integer(c_size_t), value :: arg_size
      type(c_ptr), value :: arg_value
      integer(int32_t) :: ierr
   end function ocl_set_kernel_arg_f90

   !---------------------------------------------------------------------------!
   ! ocl_add_kernel_arg_allocation_f90
   !---------------------------------------------------------------------------!

   function ocl_set_kernel_arg_allocation_f90(program_name, &
      kernel_name, arg_index, arg_size, arg_value) &
      result(ierr) bind(C, name="ocl_set_kernel_arg_allocation_f90")
      use, intrinsic :: ISO_C_BINDING
      use :: ocl_data
      implicit none
      character(kind=c_char), dimension(*) :: program_name
      character(kind=c_char), dimension(*) :: kernel_name
      integer(int32_t), value :: arg_index
      integer(c_size_t), value :: arg_size
      type(ocl_allocation_t) :: arg_value
      integer(int32_t) :: ierr
   end function ocl_set_kernel_arg_allocation_f90

   !---------------------------------------------------------------------------!
   ! ocl_kernel_hint_f90
   !---------------------------------------------------------------------------!

   function ocl_kernel_hint_f90(program_name, kernel_name, hint) &
      result(ierr) bind(C, name="ocl_kernel_hint_f90")
      use, intrinsic :: ISO_C_BINDING
      use :: ocl_data
      implicit none
      character(kind=c_char), dimension(*) :: program_name
      character(kind=c_char), dimension(*) :: kernel_name
      integer(c_size_t) :: hint
      integer(int32_t) :: ierr
   end function ocl_kernel_hint_f90

   !---------------------------------------------------------------------------!
   ! ocl_ndrange_hints_f90
   !---------------------------------------------------------------------------!

   function ocl_ndrange_hints_f90(indeces, max_work_group_size, &
      work_group_size, work_group_indeces, single_indeces) &
      result(ierr) bind(C, name="ocl_ndrange_hints_f90")
      use, intrinsic :: ISO_C_BINDING
      use :: ocl_data
      implicit none
      integer(c_size_t) :: indeces
      integer(c_size_t) :: max_work_group_size
      integer(c_size_t) :: work_group_size
      integer(c_size_t) :: work_group_indeces
      integer(c_size_t) :: single_indeces
      integer(int32_t) :: ierr
   end function ocl_ndrange_hints_f90

   !---------------------------------------------------------------------------!
   ! ocl_enqueue_kernel_ndrange_f90
   !---------------------------------------------------------------------------!

   function ocl_enqueue_kernel_ndrange_f90(device_id, program_name, &
      kernel_name, kernel_dim, global_offset, global_size, &
      local_size, event) &
      result(ierr) bind(C, name="ocl_enqueue_kernel_ndrange_f90")
      use, intrinsic :: ISO_C_BINDING
      use :: ocl_data
      implicit none
      integer(int32_t), value :: device_id
      character(kind=c_char), dimension(*) :: program_name
      character(kind=c_char), dimension(*) :: kernel_name
      integer(int32_t), value :: kernel_dim
      type(c_ptr), value :: global_offset
      type(c_ptr), value :: global_size
      type(c_ptr), value :: local_size
      type(ocl_allocation_t) :: event
      integer(int32_t) :: ierr
   end function ocl_enqueue_kernel_ndrange_f90

   !---------------------------------------------------------------------------!
   ! ocl_finish_f90
   !---------------------------------------------------------------------------!

   function ocl_finish_f90(device_id) &
      result(ierr) bind(C, name="ocl_finish_f90")
      use, intrinsic :: ISO_C_BINDING
      use :: ocl_data
      implicit none
      integer(int32_t), value :: device_id
      integer(int32_t) :: ierr
   end function ocl_finish_f90

!##############################################################################!
! Utility bindings
!##############################################################################!

   !---------------------------------------------------------------------------!
   ! ocl_host_initialize_timer_f90
   !---------------------------------------------------------------------------!

   function ocl_host_initialize_timer_f90(label) &
      result(ierr) bind(C, name="ocl_host_initialize_timer_f90")
      use, intrinsic :: ISO_C_BINDING
      use :: ocl_data
      implicit none
      character(kind=c_char), dimension(*) :: label
      integer(int32_t) :: ierr
   end function ocl_host_initialize_timer_f90

   !---------------------------------------------------------------------------!
   ! ocl_host_clear_timer_f90
   !---------------------------------------------------------------------------!

   function ocl_host_clear_timer_f90(label) &
      result(ierr) bind(C, name="ocl_host_clear_timer_f90")
      use, intrinsic :: ISO_C_BINDING
      use :: ocl_data
      implicit none
      character(kind=c_char), dimension(*) :: label
      integer(int32_t) :: ierr
   end function ocl_host_clear_timer_f90

   !---------------------------------------------------------------------------!
   ! ocl_host_start_timer_f90
   !---------------------------------------------------------------------------!

   function ocl_host_start_timer_f90(label) &
      result(ierr) bind(C, name="ocl_host_start_timer_f90")
      use, intrinsic :: ISO_C_BINDING
      use :: ocl_data
      implicit none
      character(kind=c_char), dimension(*) :: label
      integer(int32_t) :: ierr
   end function ocl_host_start_timer_f90

   !---------------------------------------------------------------------------!
   ! ocl_host_stop_timer_f90
   !---------------------------------------------------------------------------!

   function ocl_host_stop_timer_f90(label) &
      result(ierr) bind(C, name="ocl_host_stop_timer_f90")
      use, intrinsic :: ISO_C_BINDING
      use :: ocl_data
      implicit none
      character(kind=c_char), dimension(*) :: label
      integer(int32_t) :: ierr
   end function ocl_host_stop_timer_f90

   !---------------------------------------------------------------------------!
   ! ocl_host_report_timer_f90
   !---------------------------------------------------------------------------!

   function ocl_host_report_timer_f90(label) &
      result(ierr) bind(C, name="ocl_host_report_timer_f90")
      use, intrinsic :: ISO_C_BINDING
      use :: ocl_data
      implicit none
      character(kind=c_char), dimension(*) :: label
      integer(int32_t) :: ierr
   end function ocl_host_report_timer_f90

   !---------------------------------------------------------------------------!
   ! ocl_host_read_timer_f90
   !---------------------------------------------------------------------------!

   function ocl_host_read_timer_f90(label, val) &
      result(ierr) bind(C, name="ocl_host_read_timer_f90")
      use, intrinsic :: ISO_C_BINDING
      use :: ocl_data
      implicit none
      character(kind=c_char), dimension(*) :: label
      real(c_double) :: val
      integer(int32_t) :: ierr
   end function ocl_host_read_timer_f90

   !---------------------------------------------------------------------------!
   ! ocl_add_timer_f90
   !---------------------------------------------------------------------------!

   function ocl_add_timer_f90(label, event) &
      result(ierr) bind(C, name="ocl_add_timer_f90")
      use, intrinsic :: ISO_C_BINDING
      use :: ocl_data
      implicit none
      character(kind=c_char), dimension(*) :: label
      type(ocl_allocation_t) :: event
      integer(int32_t) :: ierr
   end function ocl_add_timer_f90

   !---------------------------------------------------------------------------!
   ! ocl_clear_timer_f90
   !---------------------------------------------------------------------------!

   function ocl_clear_timer_f90(label) &
      result(ierr) bind(C, name="ocl_clear_timer_f90")
      use, intrinsic :: ISO_C_BINDING
      use :: ocl_data
      implicit none
      character(kind=c_char), dimension(*) :: label
      integer(int32_t) :: ierr
   end function ocl_clear_timer_f90

   !---------------------------------------------------------------------------!
   ! ocl_report_timer_f90
   !---------------------------------------------------------------------------!

   function ocl_report_timer_f90(label) &
      result(ierr) bind(C, name="ocl_report_timer_f90")
      use, intrinsic :: ISO_C_BINDING
      use :: ocl_data
      implicit none
      character(kind=c_char), dimension(*) :: label
      integer(int32_t) :: ierr
   end function ocl_report_timer_f90

end interface
end module
