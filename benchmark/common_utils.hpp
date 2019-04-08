#ifndef UTILS_HPP
#define UTILS_HPP

#include <algorithm>
#include <benchmark/benchmark.h>
#include <chrono>
#include <climits>
#include <memory>
#include <string>

#include "blas_meta.h"
#include "range.hpp"
#include "cli_args.hpp"

using index_t = int;
using Blas3Range = Range5D<ValueRange<const char*>, ValueRange<const char*>,
                           SizeRange<int>, SizeRange<int>, SizeRange<int>>;

namespace blas_benchmark {

// Forward-declaring the function that will create the benchmark
void create_benchmark(Args& args);

namespace utils {

/**
 * @fn get_range
 * @brief Returns a range containing the parameters, either read from a file
 * according to the command-line args, or the default ones.
 * This function must be implemented for each blas level.
 */
template<typename range_t>
range_t get_range(Args& args);

template <>
inline Blas3Range get_range<Blas3Range>(Args& args) {
  // Matrix dimensions bounds
  constexpr const int dim_min = 2 << 5;
  constexpr const int dim_max = 2 << 10;
  // Matrix dimensions multiplier
  constexpr const int dim_mult = 2;

  Blas3Range range = nd_range(value_range({"n", "t"}), value_range({"n", "t"}),
                        size_range(dim_min, dim_max, dim_mult),
                        size_range(dim_min, dim_max, dim_mult),
                        size_range(dim_min, dim_max, dim_mult));
  return range;
}

/**
 * @fn get_type_name
 * @brief Returns a string with the given type. The C++ specification doesn't
 * guarantee that typeid(T).name is human readable so we specify the template
 * for float and double.
 */
template<typename scalar_t>
inline std::string get_type_name() {
  std::string type_name(typeid(scalar_t).name());
  return type_name;
}
template<>
inline std::string get_type_name<float>() { return "float"; }
template<>
inline std::string get_type_name<double>() { return "double"; }

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

/**
 * @fn time_event
 * @brief Times 1 event, and returns the aggregate time.
 */
template <typename event_t>
inline cl_ulong time_event(event_t&);
// Declared here, defined separately in the specific utils.hpp files

/**
 * @fn time_events
 * @brief Times n events, and returns the aggregate time.
 */
template <typename event_t>
inline cl_ulong time_events(std::vector<event_t> es) {
  cl_ulong total_time = 0;
  for (auto e : es) {
    total_time += time_event(e);
  }
  return total_time;
}

template <typename event_t, typename... other_events_t>
inline cl_ulong time_events(event_t first_event, other_events_t... next_events)
{
  return time_events<event_t>(
      blas::concatenate_vectors(first_event, next_events...));
}

/**
 * @fn timef
 * @brief Calculates the time spent executing the function func
 * (both overall and event time, returned in nanoseconds in a tuple of double)
 */
template <typename function_t, typename... args_t>
static std::tuple<double, double> timef(function_t func, args_t&&... args) {
  auto start = std::chrono::system_clock::now();
  auto event = func(std::forward<args_t>(args)...);
  auto end = std::chrono::system_clock::now();
  double overall_time = (end - start).count();

  double event_time = static_cast<double>(time_events(event));

  return std::make_tuple(overall_time, event_time);
}

// Functions to initialize and update the counters

inline void init_counters(benchmark::State& state) {
  state.counters["best_event_time"] = ULONG_MAX;
  state.counters["best_overall_time"] = ULONG_MAX;
}

inline void update_counters(benchmark::State& state,
                            std::tuple<double, double> times) {
  state.PauseTiming();
  double overall_time, event_time;
  std::tie(overall_time, event_time) = times;
  state.counters["total_event_time"] += event_time;
  state.counters["best_event_time"] =
      std::min<double>(state.counters["best_event_time"], event_time);
  state.counters["total_overall_time"] += overall_time;
  state.counters["best_overall_time"] =
      std::min<double>(state.counters["best_overall_time"], overall_time);
  state.ResumeTiming();
}

inline void calc_avg_counters(benchmark::State& state) {
  state.counters["avg_event_time"] =
      state.counters["total_event_time"] / state.iterations();
  state.counters["avg_overall_time"] =
      state.counters["total_overall_time"] / state.iterations();
}

}  // namespace utils
}  // namespace blas_benchmark

#endif
