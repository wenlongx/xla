/* Copyright 2025 The OpenXLA Authors.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#ifndef XLA_HLO_UTILS_CONCURRENCY_CONCURRENCY_UTILS_H_
#define XLA_HLO_UTILS_CONCURRENCY_CONCURRENCY_UTILS_H_

#include <iterator>
#include <optional>
#include <type_traits>
#include <utility>
#include <vector>

#include "absl/functional/any_invocable.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "xla/hlo/utils/concurrency/tsl_task_executor.h"

namespace xla::concurrency {
// Runs an action on all elements from an iterator. A successful run collects
// all the return values from actions. The implementation guarantees that the
// order of returned values corresponds to the order of elements in the argument
// iterator [action(begin), ... action(end-1)]. Note that the action can mutate
// the objects it receives from the iterator according to their semantics.
//
// The overload below is for actions that return a value. `ActionReturnT` must
// be default constructible.
//
// Returns synchronously when all actions finish. Aborts the run on the first
// failure. If a run aborts the underlying data is likely to be corrupted or
// partially modified.
//
// For synchronization, clients should make sure that actions do not deadlock or
// corrupt any state they access. Specifically, if actions access any shared
// mutable state clients must make sure that such access is synchronized.  The
// run can deadlock in all the standard ways. Specifically, if the action locks
// a set of shared resources make sure that all locks are acquired in the same
// order.
template <typename ActionReturnT, typename ForwardItT, typename TaskExecutorT>
#if __cplusplus >= 202002L
  requires(std::forward_iterator<ForwardItT> && !std::is_void_v<ActionReturnT>)
#endif
absl::StatusOr<std::vector<ActionReturnT>> ForEach(
    ForwardItT begin, ForwardItT end,
    absl::AnyInvocable<absl::StatusOr<ActionReturnT>(
        typename std::iterator_traits<ForwardItT>::value_type)>
        action,
    TaskExecutorT& task_executor,
    std::optional<int> parallelism = std::nullopt) {
  static_assert(!std::is_same_v<ActionReturnT, bool>,
                "Cannot collect vector<bool> concurrently. If you need bool "
                "return wrap it in a struct.");
  auto result_size = std::distance(begin, end);
  std::vector<ActionReturnT> result_storage(result_size);
  std::vector<Task> tasks;
  tasks.reserve(result_size);

  auto result_iterator = result_storage.begin();
  for (auto argument_iterator = begin; argument_iterator != end;
       ++argument_iterator) {
    // If modifying this function, keep an eye on iterator capture.
    // Specifically, evaluate whether capturing the iterator is correct.
    // For example, we can capture `result_iterator` because we are using
    // `std::vector`. Should you want to change the result collection consider
    // if the capture needs to change.
    auto argument = *argument_iterator;
    tasks.push_back([result_iterator, argument, &action]() {
      auto result = action(argument);
      if (result.ok()) {
        *result_iterator = *result;
      }
      return result.status();
    });
    ++result_iterator;
  }
  auto status =
      task_executor.ExecuteIndependentTasks(std::move(tasks), parallelism);
  if (status.ok()) {
    return result_storage;
  }
  return status;
}

// Runs an action on all elements from an iterator. Note that the action must be
// side-effecting to make any sense, and specifically it can be mutating.
//
// Returns synchronously when all actions finish. Aborts the run on the first
// failure. If a run aborts the underlying data is likely to be corrupted or
// partially modified.
//
// For synchronization, clients should make sure that actions do not deadlock or
// corrupt any state they access. Specifically, if actions access any shared
// mutable state clients must make sure that such access is synchronized.  The
// run can deadlock in all the standard ways. Specifically, if the action locks
// a set of shared resources make sure that all locks are acquired in the same
// order.
template <typename ForwardItT, typename TaskExecutorT>
#if __cplusplus >= 202002L
  requires(std::forward_iterator<ForwardItT>)
#endif
absl::Status ForEach(ForwardItT begin, ForwardItT end,
                     absl::AnyInvocable<absl::Status(
                         typename std::iterator_traits<ForwardItT>::value_type)>
                         action,
                     TaskExecutorT& task_executor,
                     std::optional<int> parallelism = std::nullopt) {
  auto result_size = std::distance(begin, end);
  std::vector<Task> tasks;
  tasks.reserve(result_size);

  for (auto iterator = begin; iterator != end; ++iterator) {
    auto argument = *iterator;
    tasks.push_back([argument, &action]() { return action(argument); });
  }
  return task_executor.ExecuteIndependentTasks(std::move(tasks), parallelism);
}

}  // namespace xla::concurrency

#endif  // XLA_HLO_UTILS_CONCURRENCY_CONCURRENCY_UTILS_H_
