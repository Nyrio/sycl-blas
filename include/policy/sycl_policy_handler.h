/***************************************************************************
 *  @license
 *  Copyright (C) Codeplay Software Limited
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  For your convenience, a copy of the License has been included in this
 *  repository.
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  SYCL-BLAS: BLAS implementation using SYCL
 *
 *  @filename sycl_policy_handler.h
 *
 **************************************************************************/

#ifndef SYCL_BLAS_SYCL_POLICY_HANDLER_H
#define SYCL_BLAS_SYCL_POLICY_HANDLER_H

#include "blas_meta.h"
#include "container/sycl_iterator.h"
#include "policy/default_policy_handler.h"
#include "policy/sycl_policy.h"
#include <CL/sycl.hpp>
#include <stdexcept>
#include <vptr/virtual_ptr.hpp>

namespace blas {

template <>
class Policy_Handler<BLAS_SYCL_Policy> {
 public:
  using Policy = BLAS_SYCL_Policy;

  explicit Policy_Handler(cl::sycl::queue q);

  template <typename T>
  T *allocate(size_t num_elements) const;

  template <typename T>
  void deallocate(T *p) const;
  /*
  @brief this class is to return the dedicated buffer to the user
  @ tparam T is the type of the pointer
  @tparam bufferT<T> is the type of the buffer points to the data. on the host
  side buffer<T> and T are the same
  */

  template <typename T>
  buffer_iterator<T, Policy> get_buffer(T *ptr) const;
  /*
  @brief this class is to return the dedicated buffer to the user
  @ tparam T is the type of the buffer
  @tparam buffer_iterator<T, Policy> is the type of the buffer that
  user can apply arithmetic operation on the host side
  */

  template <typename T>
  buffer_iterator<T, Policy> get_buffer(buffer_iterator<T, Policy> buff) const;

  /*  @brief Getting range accessor from the buffer created by virtual pointer
      @tparam T is the type of the data
      @tparam AcM is the access mode
      @param container is the  data we want to get range accessor
  */

  template <typename Policy::access_mode_type AcM, typename T>
  typename Policy::access_type<typename scalar_type<T>::type, AcM>
  get_range_access(T *vptr);

  /*  @brief Getting range accessor from the buffer created by buffer iterator
      @tparam T is the type of the data
      @tparam AcM is the access mode
      @param container is the  data we want to get range accessor
  */

  template <typename T, typename Policy::access_mode_type AcM>
  typename Policy::access_type<typename scalar_type<T>::type, AcM>
  get_range_access(buffer_iterator<T, Policy> buff);

  /*
  @brief this function is to get the offset from the actual pointer
  @tparam T is the type of the pointer
  */

  template <typename T>
  ptrdiff_t get_offset(const T *ptr) const;
  /*
  @brief this function is to get the offset from the actual pointer
  @tparam T is the type of the buffer_iterator<T, Policy>
  */

  template <typename T>
  ptrdiff_t get_offset(buffer_iterator<T, Policy> buff) const;

  /*  @brief Copying the data back to device
      @tparam T is the type of the data
      @param src is the host pointer we want to copy from.
      @param dst is the device pointer we want to copy to.
      @param size is the number of elements to be copied
  */

  template <typename T>
  typename Policy::event_type copy_to_device(const T *src, T *dst, size_t size);
  /*  @brief Copying the data back to device
    @tparam T is the type of the data
    @param src is the host pointer we want to copy from.
    @param dst is the buffer_iterator we want to copy to.
    @param size is the number of elements to be copied
  */

  template <typename T>
  typename Policy::event_type copy_to_device(const T *src,
                                             buffer_iterator<T, Policy> dst,
                                             size_t);
  /*  @brief Copying the data back to device
      @tparam T is the type of the data
      @param src is the device pointer we want to copy from.
      @param dst is the host pointer we want to copy to.
      @param size is the number of elements to be copied
  */

  template <typename T>
  typename Policy::event_type copy_to_host(T *src, T *dst, size_t size);

  template <typename T>
  typename Policy::event_type copy_to_host(buffer_iterator<T, Policy> src,
                                           T *dst, size_t);

  inline const Policy::device_type get_device_type() const {
    return selectedDeviceType_;
  };
  inline bool has_local_memory() const { return localMemorySupport_; }
  typename Policy::queue_type get_queue() const { return q_; }

  inline size_t get_work_group_size() const { return workGroupSize_; }

  inline size_t get_num_compute_units() const { return computeUnits_; }

  inline void wait() { q_.wait(); }

  inline void wait(Policy::event_type evs) { cl::sycl::event::wait(evs); }

  /*  @brief waiting for a list of sycl events
 @param first_event  and next_events are instances of sycl::sycl::event
*/
  // this must be in header as the number of event is controlled by user and we
  // dont know howmany permutation can be used by a user
  template <typename first_event_t, typename... next_event_t>
  void inline wait(first_event_t first_event, next_event_t... next_events) {
    cl::sycl::event::wait(concatenate_vectors(first_event, next_events...));
  }

 private:
  typename Policy::queue_type q_;
  std::shared_ptr<cl::sycl::codeplay::PointerMapper> pointerMapperPtr_;
  const size_t workGroupSize_;
  const Policy::device_type selectedDeviceType_;
  const bool localMemorySupport_;
  const size_t computeUnits_;
};

}  // namespace blas
#endif  // QUEUE_SYCL_HPP
