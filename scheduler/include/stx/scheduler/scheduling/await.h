

#include <tuple>
#include <utility>

#include "stx/scheduler.h"

namespace stx {

namespace sched {
template <typename Fn, typename FirstInput, typename... OtherInputs>
auto await(stx::TaskScheduler &scheduler, Fn task, TaskPriority priority,
           TaskTraceInfo trace_info, Future<FirstInput> first_input,
           Future<OtherInputs>... other_inputs) {
  auto timepoint = std::chrono::steady_clock::now();
  TaskId task_id{scheduler.next_task_id};
  scheduler.next_task_id++;

  static_assert(std::is_invocable_v<Fn &, Future<FirstInput> &&,
                                    Future<OtherInputs> &&...>);

  using output = std::invoke_result_t<Fn &, Future<FirstInput> &&,
                                      Future<OtherInputs> &&...>;

  std::array<FutureAny, 1 + sizeof...(OtherInputs)> await_futures{
      FutureAny{first_input.share()}, FutureAny{other_inputs.share()}...};

  RcFn<TaskReady(nanoseconds)> readiness_fn =
      stx::fn::rc::make_functor(scheduler.allocator, [await_futures_ =
                                                          std::move(
                                                              await_futures)](
                                                         nanoseconds) {
        bool all_ready = std::all_of(
            await_futures_.begin(), await_futures_.end(),
            [](FutureAny const &future) { return future.is_done(); });

        return all_ready ? TaskReady::Yes : TaskReady::No;
      }).unwrap();

  std::tuple<Future<FirstInput>, Future<OtherInputs>...> args{
      std::move(first_input), std::move(other_inputs)...};

  Promise promise = stx::make_promise<output>(scheduler.allocator).unwrap();
  Future future = promise.get_future();

  PromiseAny task_promise = PromiseAny{promise.share()};

  RcFn<void()> fn =
      stx::fn::rc::make_functor(scheduler.allocator, [task_ = std::move(task),
                                                      args_ = std::move(args),
                                                      promise_ = std::move(
                                                          promise)]() mutable {
        if constexpr (!std::is_void_v<output>) {
          output result = std::apply(task_, std::move(args_));
          promise_.notify_completed(std::forward<output>(result));
        } else {
          std::apply(task_, std::move(args_));
          promise_.notify_completed();
        }
      }).unwrap();

  scheduler.entries =
      stx::vec::push(std::move(scheduler.entries),
                      Task{std::move(fn), std::move(readiness_fn), timepoint,
                           std::move(task_promise), task_id, priority,
                           std::move(trace_info)})
          .unwrap();

  return future;
}

template <typename Fn, typename FirstInput, typename... OtherInputs>
auto await_any(stx::TaskScheduler &scheduler, Fn task, TaskPriority priority,
               TaskTraceInfo trace_info, Future<FirstInput> first_input,
               Future<OtherInputs>... other_inputs) {
  auto timepoint = std::chrono::steady_clock::now();
  TaskId task_id{scheduler.next_task_id};
  scheduler.next_task_id++;

  static_assert(std::is_invocable_v<Fn &, Future<FirstInput> &&,
                                    Future<OtherInputs> &&...>);

  using output = std::invoke_result_t<Fn &, Future<FirstInput> &&,
                                      Future<OtherInputs> &&...>;

  std::array<FutureAny, 1 + sizeof...(OtherInputs)> await_futures{
      FutureAny{first_input.share()}, FutureAny{other_inputs.share()}...};

  RcFn<TaskReady(nanoseconds)> readiness_fn =
      stx::fn::rc::make_functor(scheduler.allocator, [await_futures_ =
                                                          std::move(
                                                              await_futures)](
                                                         nanoseconds) {
        bool any_ready = std::any_of(
            await_futures_.begin(), await_futures_.end(),
            [](FutureAny const &future) { return future.is_done(); });

        return any_ready ? TaskReady::Yes : TaskReady::No;
      }).unwrap();

  std::tuple<Future<FirstInput>, Future<OtherInputs>...> args{
      std::move(first_input), std::move(other_inputs)...};

  Promise promise = stx::make_promise<output>(scheduler.allocator).unwrap();
  Future future = promise.get_future();
  PromiseAny task_promise{promise.share()};

  RcFn<void()> fn =
      stx::fn::rc::make_functor(scheduler.allocator, [task_ = std::move(task),
                                                      args_ = std::move(args),
                                                      promise_ = std::move(
                                                          promise)]() mutable {
        if constexpr (!std::is_void_v<output>) {
          output result = std::apply(task_, std::move(args_));
          promise_.notify_completed(std::forward<output>(result));
        } else {
          std::apply(task_, std::move(args_));
          promise_.notify_completed();
        }
      }).unwrap();

  scheduler.entries =
      stx::vec::push(std::move(scheduler.entries),
                      Task{std::move(fn), std::move(readiness_fn), timepoint,
                           std::move(task_promise), task_id, priority,
                           std::move(trace_info)})
          .unwrap();

  return future;
}

}  // namespace sched
}  // namespace stx
