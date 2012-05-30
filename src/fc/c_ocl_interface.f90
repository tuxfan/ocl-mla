!------------------------------------------------------------------------------!
! Copyright (c) 2012 Los Alamos National Security, LLC
! All rights reserved.
!------------------------------------------------------------------------------!

module ocl_interface

   use, intrinsic :: ISO_C_BINDING
   use :: ocl_bindings
   use :: ocl_data

   contains

   !---------------------------------------------------------------------------!
   ! ocl_init
   !---------------------------------------------------------------------------!

   subroutine ocl_init(ierr)
      use :: ocl_data
      implicit none
      integer(int32_t) :: ierr

      cl_mem_size = ocl_mem_size_f90()

      ierr = ocl_init_f90()
   end subroutine ocl_init

   !---------------------------------------------------------------------------!
   ! ocl_finalize
   !---------------------------------------------------------------------------!

   subroutine ocl_finalize(ierr)
      implicit none
      integer(int32_t) :: ierr

      ierr = ocl_finalize_f90()
   end subroutine ocl_finalize

   !---------------------------------------------------------------------------!
   ! ocl_create_buffer
   !---------------------------------------------------------------------------!

   subroutine ocl_create_buffer(device_id, elements, flags, &
      host_ptr, buffer, ierr)
      use :: ocl_data
      implicit none
      integer(int32_t) :: device_id
      integer(c_size_t) :: elements
      integer(cl_bitfield) :: flags
      type(c_ptr) :: host_ptr
      type(ocl_allocation_t) buffer
      integer(int32_t) :: ierr

      ierr = ocl_create_buffer_f90(device_id, elements, flags, &
         host_ptr, buffer)
   end subroutine ocl_create_buffer

   !---------------------------------------------------------------------------!
   ! ocl_release_buffer
   !---------------------------------------------------------------------------!

   subroutine ocl_release_buffer(buffer, ierr)
      use :: ocl_data
      implicit none
      type(ocl_allocation_t) :: buffer
      integer(int32_t) :: ierr
      
      ierr = ocl_release_buffer_f90(buffer)
   end subroutine ocl_release_buffer

   !---------------------------------------------------------------------------!
   ! ocl_enqueue_write_buffer
   !---------------------------------------------------------------------------!

   subroutine ocl_enqueue_write_buffer(device_id, buffer, synchronous, &
      offset, cb, ptr, event, ierr)
      use :: ocl_data
      implicit none
      integer(int32_t) :: device_id
      type(ocl_allocation_t) :: buffer
      integer(int32_t) :: synchronous
      integer(c_size_t) :: offset
      integer(c_size_t) :: cb
      type(c_ptr) :: ptr
      type(ocl_allocation_t) :: event
      integer(int32_t) :: ierr

      ierr = ocl_enqueue_write_buffer_f90(device_id, buffer, synchronous, &
         offset, cb, ptr, event)
   end subroutine ocl_enqueue_write_buffer

   !---------------------------------------------------------------------------!
   ! ocl_enqueue_read_buffer
   !---------------------------------------------------------------------------!

   subroutine ocl_enqueue_read_buffer(device_id, buffer, synchronous, &
      offset, cb, ptr, event, ierr)
      use :: ocl_data
      implicit none
      integer(int32_t) :: device_id
      type(ocl_allocation_t) :: buffer
      integer(int32_t) :: synchronous
      integer(c_size_t) :: offset
      integer(c_size_t) :: cb
      type(c_ptr) :: ptr
      type(ocl_allocation_t) :: event
      integer(int32_t) :: ierr

      ierr = ocl_enqueue_read_buffer_f90(device_id, buffer, synchronous, &
         offset, cb, ptr, event)
   end subroutine ocl_enqueue_read_buffer

   !---------------------------------------------------------------------------!
   ! ocl_enqueue_map_buffer
   !---------------------------------------------------------------------------!

   subroutine ocl_enqueue_map_buffer(device_id, buffer, synchronous, &
      flags, offset, cb, ptr, event, ierr)
      use :: ocl_data
      implicit none
      integer(int32_t) :: device_id
      type(ocl_allocation_t) :: buffer
      integer(int32_t) :: synchronous
      integer(cl_bitfield), value :: flags
      integer(c_size_t) :: offset
      integer(c_size_t) :: cb
      type(c_ptr) :: ptr
      type(ocl_allocation_t) :: event
      integer(int32_t) :: ierr

      ierr = ocl_enqueue_map_buffer_f90(device_id, buffer, synchronous, &
         flags, offset, cb, ptr, event)
   end subroutine ocl_enqueue_map_buffer

   !---------------------------------------------------------------------------!
   ! ocl_enqueue_unmap_buffer
   !---------------------------------------------------------------------------!

   subroutine ocl_enqueue_unmap_buffer(device_id, buffer, ptr, event, ierr)
      use :: ocl_data
      implicit none
      integer(int32_t), value :: device_id
      type(ocl_allocation_t) :: buffer
      type(c_ptr), value :: ptr
      type(ocl_allocation_t) :: event
      integer(int32_t) :: ierr

      ierr = ocl_enqueue_unmap_buffer_f90(device_id, buffer, ptr, event)
   end subroutine ocl_enqueue_unmap_buffer

   !---------------------------------------------------------------------------!
   ! ocl_initialize_event
   !---------------------------------------------------------------------------!

   subroutine ocl_initialize_event(event, ierr)
      use :: ocl_data
      implicit none
      type(ocl_allocation_t) event
      integer(int32_t) :: ierr

      ierr = ocl_initialize_event_f90(event)
   end subroutine ocl_initialize_event

   !---------------------------------------------------------------------------!
   ! ocl_release_event
   !---------------------------------------------------------------------------!

   subroutine ocl_release_event(event, ierr)
      use :: ocl_data
      implicit none
      type(ocl_allocation_t) event
      integer(int32_t) :: ierr

      ierr = ocl_release_event_f90(event)
   end subroutine ocl_release_event

   !---------------------------------------------------------------------------!
   ! ocl_initialize_event_wait_list
   !---------------------------------------------------------------------------!

   subroutine ocl_initialize_event_wait_list(list, ierr)
      use :: ocl_data
      implicit none
      type(ocl_allocation_t) list
      integer(int32_t) :: ierr

      ierr = ocl_initialize_event_wait_list_f90(list)
   end subroutine ocl_initialize_event_wait_list

   !---------------------------------------------------------------------------!
   ! ocl_add_event_to_wait_list
   !---------------------------------------------------------------------------!

   subroutine ocl_add_event_to_wait_list(list, event, ierr)
      use :: ocl_data
      implicit none
      type(ocl_allocation_t) list
      type(ocl_allocation_t) event
      integer(int32_t) :: ierr

      ierr = ocl_add_event_to_wait_list_f90(list, event)
   end subroutine ocl_add_event_to_wait_list

   !---------------------------------------------------------------------------!
   ! ocl_set_event_list
   !---------------------------------------------------------------------------!

   subroutine ocl_set_event_list(event, list, ierr)
      use :: ocl_data
      implicit none
      type(ocl_allocation_t) event
      type(ocl_allocation_t) list
      integer(int32_t) :: ierr

      ierr = ocl_set_event_list_f90(event, list)
   end subroutine ocl_set_event_list

   !---------------------------------------------------------------------------!
   ! ocl_clear_event
   !---------------------------------------------------------------------------!

   subroutine ocl_clear_event(event, ierr)
      use :: ocl_data
      implicit none
      type(ocl_allocation_t) event
      integer(int32_t) :: ierr

      ierr = ocl_clear_event_f90(event)
   end subroutine ocl_clear_event

   !---------------------------------------------------------------------------!
   ! ocl_clear_event_wait_list
   !---------------------------------------------------------------------------!

   subroutine ocl_clear_event_wait_list(list, ierr)
      use :: ocl_data
      implicit none
      type(ocl_allocation_t) list
      integer(int32_t) :: ierr

      ierr = ocl_clear_event_wait_list_f90(list)
   end subroutine ocl_clear_event_wait_list

   !---------------------------------------------------------------------------!
   ! ocl_wait_for_events
   !---------------------------------------------------------------------------!

   subroutine ocl_wait_for_events(list, ierr)
      use :: ocl_data
      implicit none
      type(ocl_allocation_t) list
      integer(int32_t) :: ierr

      ierr = ocl_wait_for_events_f90(list)
   end subroutine ocl_wait_for_events

   !---------------------------------------------------------------------------!
   ! ocl_add_program
   !---------------------------------------------------------------------------!

   subroutine ocl_add_program(device_id, program_name, program_source, &
      compile_options, ierr)
      use :: ocl_data
      implicit none
      integer(int32_t) :: device_id
      character(kind=c_char), dimension(*) :: program_name
      character(kind=c_char), dimension(*) :: program_source
      character(kind=c_char), dimension(*) :: compile_options
      integer(int32_t) :: ierr
      
      ierr = ocl_add_program_f90(device_id, program_name, program_source, &
         compile_options)
   end subroutine ocl_add_program

   !---------------------------------------------------------------------------!
   ! ocl_add_kernel
   !---------------------------------------------------------------------------!

   subroutine ocl_add_kernel(device_id, program_name, kernel_source_name, &
      kernel_name, ierr)
      use :: ocl_data
      implicit none
      integer(int32_t) :: device_id
      character(kind=c_char), dimension(*) :: program_name
      character(kind=c_char), dimension(*) :: kernel_source_name
      character(kind=c_char), dimension(*) :: kernel_name
      integer(int32_t) :: ierr

      ierr = ocl_add_kernel_f90(device_id, program_name, kernel_source_name, &
         kernel_name)
   end subroutine ocl_add_kernel

   !---------------------------------------------------------------------------!
   ! ocl_set_kernel_arg
   !---------------------------------------------------------------------------!

   subroutine ocl_set_kernel_arg(program_name, kernel_name, &
      arg_index, arg_size, arg_value, ierr)
      use :: ocl_data
      implicit none
      character(kind=c_char), dimension(*) :: program_name
      character(kind=c_char), dimension(*) :: kernel_name
      integer(int32_t) :: arg_index
      integer(c_size_t) :: arg_size
      type(c_ptr) :: arg_value
      integer(int32_t) :: ierr

      ierr = ocl_set_kernel_arg_f90(program_name, kernel_name, &
         arg_index, arg_size, arg_value)
   end subroutine ocl_set_kernel_arg

   !---------------------------------------------------------------------------!
   ! ocl_add_kernel_arg_mem
   !---------------------------------------------------------------------------!

   subroutine ocl_set_kernel_arg_mem(program_name, kernel_name, &
      arg_index, arg_value, ierr)
      use :: ocl_data
      implicit none
      character(kind=c_char), dimension(*) :: program_name
      character(kind=c_char), dimension(*) :: kernel_name
      integer, intent(in) :: arg_index
      type(ocl_allocation_t) :: arg_value
      integer(int32_t) :: ierr

      ierr = ocl_set_kernel_arg_allocation_f90(program_name, &
         kernel_name, arg_index, cl_mem_size, arg_value)
   end subroutine ocl_set_kernel_arg_mem

   !---------------------------------------------------------------------------!
   ! ocl_add_kernel_arg_local
   !---------------------------------------------------------------------------!

   subroutine ocl_set_kernel_arg_local(program_name, kernel_name, &
      arg_index, arg_size, ierr)
      use, intrinsic :: ISO_C_BINDING
      use :: ocl_data
      implicit none
      character(kind=c_char), dimension(*) :: program_name
      character(kind=c_char), dimension(*) :: kernel_name
      integer, intent(in) :: arg_index
      integer(c_size_t) :: arg_size
      integer(int32_t) :: ierr

      ierr = ocl_set_kernel_arg_f90(program_name, &
         kernel_name, arg_index, arg_size, C_NULL_PTR)
   end subroutine ocl_set_kernel_arg_local

   !---------------------------------------------------------------------------!
   ! ocl_add_kernel_arg_int
   !---------------------------------------------------------------------------!

   subroutine ocl_set_kernel_arg_int(program_name, kernel_name, &
      arg_index, arg_value, ierr)
      use :: ocl_data
      implicit none
      character(kind=c_char), dimension(*) :: program_name
      character(kind=c_char), dimension(*) :: kernel_name
      integer, intent(in) :: arg_index
      integer(c_int), target :: arg_value
      integer(c_size_t) :: arg_size
      integer(int32_t) :: ierr

      arg_size = sizeof(arg_value)

      ierr = ocl_set_kernel_arg_f90(program_name, kernel_name, &
         arg_index, arg_size, c_loc(arg_value))
   end subroutine ocl_set_kernel_arg_int

   !---------------------------------------------------------------------------!
   ! ocl_kernel_hints
   !---------------------------------------------------------------------------!

   subroutine ocl_kernel_hints(program_name, kernel_name, hints, ierr)
      use :: ocl_data
      implicit none
      character(kind=c_char), dimension(*) :: program_name
      character(kind=c_char), dimension(*) :: kernel_name
      type(ocl_kernel_hints_t) :: hints
      integer(int32_t) :: ierr

      ierr = ocl_kernel_hints_f90(program_name, kernel_name, hints)
   end subroutine ocl_kernel_hints

   !---------------------------------------------------------------------------!
   ! ocl_ndrange_hints
   !---------------------------------------------------------------------------!

   subroutine ocl_ndrange_hints(indeces, max_work_group_size, &
      work_group_size, work_group_indeces, single_indeces)
      use :: ocl_data
      implicit none
      integer(c_size_t) :: indeces
      integer(c_size_t) :: max_work_group_size
      integer(c_size_t) :: work_group_size
      integer(c_size_t) :: work_group_indeces
      integer(c_size_t) :: single_indeces
      integer(int32_t) :: ierr

      ierr = ocl_ndrange_hints_f90(indeces, max_work_group_size, &
         work_group_size, work_group_indeces, single_indeces)
   end subroutine ocl_ndrange_hints

   !---------------------------------------------------------------------------!
   ! ocl_enqueue_kernel_ndrange
   !---------------------------------------------------------------------------!

   subroutine ocl_enqueue_kernel_ndrange(device_id, program_name, &
      kernel_name, kernel_dim, global_offset, global_size, local_size, &
      event, ierr)
      use :: ocl_data
      implicit none
      integer(int32_t) :: device_id
      character(kind=c_char), dimension(*) :: program_name
      character(kind=c_char), dimension(*) :: kernel_name
      integer(int32_t) :: kernel_dim
      integer(c_size_t), target :: global_offset
      integer(c_size_t), target :: global_size
      integer(c_size_t), target :: local_size
      type(ocl_allocation_t) :: event
      integer(int32_t) :: ierr

      ierr = ocl_enqueue_kernel_ndrange_f90(device_id, program_name, &
         kernel_name, kernel_dim, c_loc(global_offset), c_loc(global_size), &
         c_loc(local_size), event)
   end subroutine ocl_enqueue_kernel_ndrange

   !---------------------------------------------------------------------------!
   ! ocl_initialize_kernel_token
   !---------------------------------------------------------------------------!

   subroutine ocl_initialize_kernel_token(token)
      use :: ocl_data
      implicit none
      type(ocl_allocation_t) :: token
      integer(int32_t) :: ierr

      ierr = ocl_initialize_kernel_token_f90(token)
   end subroutine ocl_initialize_kernel_token

   !---------------------------------------------------------------------------!
   ! ocl_kernel_token
   !---------------------------------------------------------------------------!

   subroutine ocl_kernel_token(program_name, kernel_name, token, ierr)
      use :: ocl_data
      implicit none
      character(kind=c_char), dimension(*) :: program_name
      character(kind=c_char), dimension(*) :: kernel_name
      type(ocl_allocation_t) :: token
      integer(int32_t) :: ierr

      ierr = ocl_kernel_token_f90(program_name, kernel_name, token)
   end subroutine ocl_kernel_token

   !---------------------------------------------------------------------------!
   ! ocl_enqueue_kernel_ndrange
   !---------------------------------------------------------------------------!

   subroutine ocl_enqueue_kernel_ndrange_token(device_id, kernel, &
      kernel_dim, global_offset, global_size, local_size, event, ierr)
      use :: ocl_data
      implicit none
      integer(int32_t) :: device_id
      type(ocl_allocation_t) :: kernel
      integer(int32_t) :: kernel_dim
      integer(c_size_t), target :: global_offset
      integer(c_size_t), target :: global_size
      integer(c_size_t), target :: local_size
      type(ocl_allocation_t) :: event
      integer(int32_t) :: ierr

      ierr = ocl_enqueue_kernel_ndrange_token_f90(device_id, kernel, &
         kernel_dim, c_loc(global_offset), c_loc(global_size), &
         c_loc(local_size), event)
   end subroutine ocl_enqueue_kernel_ndrange_token

   !---------------------------------------------------------------------------!
   ! ocl_finish
   !---------------------------------------------------------------------------!

   subroutine ocl_finish(device_id)
      use :: ocl_data
      implicit none
      integer(int32_t) :: device_id
      integer(int32_t) :: ierr

      ierr = ocl_finish_f90(device_id)
   end subroutine ocl_finish

!##############################################################################!
! Utility interface
!##############################################################################!

   !---------------------------------------------------------------------------!
   ! ocl_host_initialize_timer
   !---------------------------------------------------------------------------!

   subroutine ocl_host_initialize_timer(label)
      use, intrinsic :: ISO_C_BINDING
      implicit none
      character(kind=c_char), dimension(*) :: label
      integer(int32_t) :: ierr

      ierr = ocl_host_initialize_timer_f90(label)
   end subroutine ocl_host_initialize_timer

   !---------------------------------------------------------------------------!
   ! ocl_host_clear_timer
   !---------------------------------------------------------------------------!

   subroutine ocl_host_clear_timer(label)
      use, intrinsic :: ISO_C_BINDING
      implicit none
      character(kind=c_char), dimension(*) :: label
      integer(int32_t) :: ierr

      ierr = ocl_host_clear_timer_f90(label)
   end subroutine ocl_host_clear_timer

   !---------------------------------------------------------------------------!
   ! ocl_host_start_timer
   !---------------------------------------------------------------------------!

   subroutine ocl_host_start_timer(label)
      use, intrinsic :: ISO_C_BINDING
      implicit none
      character(kind=c_char), dimension(*) :: label
      integer(int32_t) :: ierr

      ierr = ocl_host_start_timer_f90(label)
   end subroutine ocl_host_start_timer

   !---------------------------------------------------------------------------!
   ! ocl_host_stop_timer
   !---------------------------------------------------------------------------!

   subroutine ocl_host_stop_timer(label)
      use, intrinsic :: ISO_C_BINDING
      implicit none
      character(kind=c_char), dimension(*) :: label
      integer(int32_t) :: ierr

      ierr = ocl_host_stop_timer_f90(label)
   end subroutine ocl_host_stop_timer

   !---------------------------------------------------------------------------!
   ! ocl_host_report_timer
   !---------------------------------------------------------------------------!

   subroutine ocl_host_report_timer(label)
      use, intrinsic :: ISO_C_BINDING
      implicit none
      character(kind=c_char), dimension(*) :: label
      integer(int32_t) :: ierr

      ierr = ocl_host_report_timer_f90(label)
   end subroutine ocl_host_report_timer

   !---------------------------------------------------------------------------!
   ! ocl_host_report_timer
   !---------------------------------------------------------------------------!

   subroutine ocl_host_read_timer(label, val)
      use, intrinsic :: ISO_C_BINDING
      implicit none
      character(kind=c_char), dimension(*) :: label
      real(c_double) :: val
      integer(int32_t) :: ierr

      ierr = ocl_host_read_timer_f90(label, val)
   end subroutine ocl_host_read_timer

   !---------------------------------------------------------------------------!
   ! ocl_add_timer
   !---------------------------------------------------------------------------!

   subroutine ocl_add_timer(label, event)
      use, intrinsic :: ISO_C_BINDING
      implicit none
      character(kind=c_char), dimension(*) :: label
      type(ocl_allocation_t) :: event
      integer(int32_t) :: ierr

      ierr = ocl_add_timer_f90(label, event)
   end subroutine ocl_add_timer

   !---------------------------------------------------------------------------!
   ! ocl_clear_timer
   !---------------------------------------------------------------------------!

   subroutine ocl_clear_timer(label)
      use, intrinsic :: ISO_C_BINDING
      implicit none
      character(kind=c_char), dimension(*) :: label
      integer(int32_t) :: ierr

      ierr = ocl_clear_timer_f90(label)
   end subroutine ocl_clear_timer

   !---------------------------------------------------------------------------!
   ! ocl_report_timer
   !---------------------------------------------------------------------------!

   subroutine ocl_report_timer(label)
      use, intrinsic :: ISO_C_BINDING
      implicit none
      character(kind=c_char), dimension(*) :: label
      integer(int32_t) :: ierr

      ierr = ocl_report_timer_f90(label)
   end subroutine ocl_report_timer

end module ocl_interface
