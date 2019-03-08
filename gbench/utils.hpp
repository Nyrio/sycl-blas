#ifndef UTILS_HPP
#define UTILS_HPP

#include <CL/sycl.hpp>
#include <benchmark/benchmark.h>
#include <chrono>

#include <memory>

#include "sycl_blas.h"

// Forward declare methods that we use in `benchmark.cpp`, but define in
// `main.cpp`
typedef blas::Executor<blas::PolicyHandler<blas::codeplay_policy>>
    SyclExecutorType;
typedef std::shared_ptr<SyclExecutorType> ExecutorPtr;
ExecutorPtr getExecutor();
using index_t = int;

namespace benchmark {
namespace utils {

/**
 * @fn time_event
 * @brief Get the overall run time (start -> end) of a cl::sycl::event enqueued
 * on a queue with profiling.
 */
inline cl_ulong time_event(cl::sycl::event e) {
  // get start and ed times
  cl_ulong start_time = e.template get_profiling_info<
      cl::sycl::info::event_profiling::command_start>();

  cl_ulong end_time = e.template get_profiling_info<
      cl::sycl::info::event_profiling::command_end>();

  // return the delta
  return (end_time - start_time);
}

/**
 * @fn time_events
 * @brief Times n events, and returns the aggregate time.
 */
template <typename EventT>
inline cl_ulong time_events(std::vector<EventT> es) {
  cl_ulong total_time = 0;
  for (auto e : es) {
    total_time += time_event(e);
  }
  return total_time;
}

template <typename EventT, typename... OtherEvents>
inline cl_ulong time_events(EventT first_event, OtherEvents... next_events) {
  return time_events<EventT>(
      blas::concatenate_vectors(first_event, next_events...));
}

/**
 * @fn random_scalar
 * @brief Generates a random scalar value, using an arbitrary low quality
 * algorithm.
 */
template <typename scalar_t>
static inline scalar_t random_scalar() {
  return 1e-3 * ((rand() % 2000) - 1000);
}

/**
 * @fn random_data
 * @brief Generates a random vector of scalar values, using an arbitrary low
 * quality algorithm.
 */
template <typename scalar_t>
static inline std::vector<scalar_t> random_data(size_t size,
                                                bool initialized = true) {
  std::vector<scalar_t> v = std::vector<scalar_t>(size);
  if (initialized) {
    std::transform(v.begin(), v.end(), v.begin(), [](scalar_t x) -> scalar_t {
      return random_scalar<scalar_t>();
    });
  }
  return v;
}

/**
 * @fn const_data
 * @brief Generates a vector of constant values, of a given length.
 */
template <typename scalar_t>
static inline std::vector<scalar_t> const_data(size_t size,
                                               scalar_t const_value = 0) {
  std::vector<scalar_t> v = std::vector<scalar_t>(size);
  std::fill(v.begin(), v.end(), const_value);
  return v;
}

enum class Transposition { Normal, Transposed, Conjugate };

const std::array<Transposition, 3> possible_transpositions(
    {Transposition::Normal, Transposition::Transposed,
     Transposition::Conjugate});
/**
 * @fn to_transpose_enum
 * @brief Translates from a transposition string to an enum.
 */
static inline Transposition to_transpose_enum(const char* t) {
  if (t[0] == 't') {
    return Transposition::Transposed;
  } else if (t[0] == 'c') {
    return Transposition::Conjugate;
  } else {
    return Transposition::Normal;
  }
}
/**
 * @fn from_transpose_enum
 * @brief Translates from a transposition enum to a transposition string
 */
static inline const char* from_transpose_enum(Transposition t) {
  switch (t) {
    case Transposition::Transposed:
      return "t";
      break;
    case Transposition::Conjugate:
      return "c";
      break;
    case Transposition::Normal:
      return "n";
      break;
    default:
      return "n";
  }
}

}  // namespace utils
}  // namespace benchmark

#endif
