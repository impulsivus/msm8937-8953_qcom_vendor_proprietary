// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary – Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <mare/internal/legacy/attr.hh>
#include <mare/internal/legacy/attrobjs.hh>
#include <mare/internal/legacy/gpukernel.hh>

#include <mare/internal/task/gputask.hh>
#include <mare/internal/util/macros.hh>
#include <mare/internal/runtime.hh>
#include <mare/internal/task/cputask.hh>
#include <mare/internal/task/group.hh>
#include <mare/internal/task/task.hh>
#include <mare/internal/util/templatemagic.hh>

#if !defined(MARE_NO_TASK_ALLOCATOR)
#include <mare/internal/memalloc/concurrentbumppool.hh>
#include <mare/internal/memalloc/taskallocator.hh>
#endif

namespace mare {
namespace internal {
namespace legacy {

MARE_GCC_IGNORE_BEGIN("-Weffc++");

template <typename Body>
struct body_wrapper_base;

template<typename Fn>
struct body_wrapper_base
{
  typedef function_traits<Fn> traits;

  MARE_CONSTEXPR  Fn& get_fn() const {
    return _fn;
  }

  MARE_CONSTEXPR  legacy::task_attrs get_attrs() const {
    return legacy::create_task_attrs();
  }

  MARE_CONSTEXPR explicit body_wrapper_base(Fn&& fn):_fn(fn){}

  task* create_task() {
    return create_task(nullptr, legacy::create_task_attrs(::mare::internal::attr::none));
  }

  task* create_task(group *g, legacy::task_attrs attrs) {
    typedef typename internal::task_type_info<Fn, true> task_type_info;
    typedef typename task_type_info::user_code user_code;

#if !defined(MARE_NO_TASK_ALLOCATOR)
    char* task_buffer = task_allocator::allocate(sizeof(cputask<task_type_info>));
    auto t = new (task_buffer) cputask<task_type_info>(g, attrs, std::forward<user_code>(_fn));
#else
    auto t = new cputask<task_type_info>(g, attrs, std::forward<user_code>(_fn));
#endif

    return t;
  }

  static
  task* create_task(Fn&& fn) {
    return create_task(std::forward<Fn>(fn), nullptr);
  }

  static
  task* create_task(Fn&& fn, group *g) {
    typedef typename internal::task_type_info<Fn, true> task_type_info;
    typedef typename task_type_info::user_code user_code;

#if !defined(MARE_NO_TASK_ALLOCATOR)
    char* task_buffer = task_allocator::allocate(sizeof(cputask<task_type_info>));
    auto t = new (task_buffer) cputask<task_type_info>(g,
                                                       legacy::create_task_attrs(::mare::internal::attr::none),
                                                       std::forward<user_code>(fn));
#else
    auto t = new cputask<task_type_info>(g,
                                         legacy::create_task_attrs(::mare::internal::attr::none),
                                         std::forward<user_code>(fn));
#endif
    return t;
  }

private:
  Fn& _fn;
};

template<typename Fn>
struct body_wrapper_base< legacy::body_with_attrs<Fn> >
{
  typedef function_traits<Fn> traits;

  MARE_CONSTEXPR  Fn& get_fn() const {
    return _b.get_body();
  }

  MARE_CONSTEXPR  legacy::task_attrs get_attrs() const {
    return _b.get_attrs();
  }

  MARE_CONSTEXPR explicit body_wrapper_base(legacy::body_with_attrs<Fn> &&b):_b(b){}

  task* create_task() {
    return create_task(nullptr, get_attrs());
  }

  task* create_task(group *g, legacy::task_attrs attrs) {

    typedef typename internal::task_type_info<Fn, true> task_type_info;
    typedef typename task_type_info::user_code user_code;

#if !defined(MARE_NO_TASK_ALLOCATOR)
    char* task_buffer = task_allocator::allocate(sizeof(cputask<task_type_info>));
    auto t = new (task_buffer) cputask<task_type_info>(g, attrs, std::forward<user_code>(get_fn()));
#else
    auto t = new cputask<task_type_info>(g, attrs, std::forward<user_code>(get_fn()));
#endif
    return t;
  }

  static
  task* create_task(legacy::body_with_attrs<Fn>&& attrd_body) {
    return create_task(std::forward<legacy::body_with_attrs<Fn>>(attrd_body), nullptr);
  }

  static
  task* create_task(legacy::body_with_attrs<Fn>&& attrd_body, group *g) {

    auto attrs = attrd_body.get_attrs();
    typedef typename internal::task_type_info<Fn, true> task_type_info;
    typedef typename task_type_info::user_code user_code;

#if !defined(MARE_NO_TASK_ALLOCATOR)
    char* task_buffer = task_allocator::allocate(sizeof(cputask<task_type_info>));
    auto t = new (task_buffer) cputask<task_type_info>(g,
                                                       attrs,
                                                       std::forward<user_code>(attrd_body.get_body()));

#else
    auto t = new cputask<task_type_info>(g,
                                         attrs,
                                         std::forward<user_code>(attrd_body.get_body()));
#endif
    return t;
  }

private:
  legacy::body_with_attrs<Fn>& _b;
};

#ifdef MARE_HAVE_GPU

template<typename Fn, typename Kernel, typename ...Args>
struct body_wrapper_base< legacy::body_with_attrs_gpu<Fn, Kernel, Args...> >
{
  typedef function_traits<Fn> traits;
  typedef typename legacy::body_with_attrs_gpu<Fn, Kernel, Args...>::kernel_parameters
                   kparams;
  typedef typename legacy::body_with_attrs_gpu<Fn, Kernel, Args...>::kernel_arguments
                   kargs;

  MARE_CONSTEXPR Fn& get_fn() const {
    return _b.get_body();
  }

  MARE_CONSTEXPR legacy::task_attrs get_attrs() const {
    return _b.get_attrs();
  }

  MARE_CONSTEXPR explicit
  body_wrapper_base(const legacy::body_with_attrs_gpu<Fn, Kernel, Args...> &&b):_b(b){}

  template<size_t Dims>
  static
  task* create_task(legacy::device_ptr const& device,
                    const ::mare::range<Dims>& r,
                    const ::mare::range<Dims>& l,
                    legacy::body_with_attrs_gpu<Fn, Kernel, Args...>&& attrd_body) {
    auto attrs = attrd_body.get_attrs();
    return new gputask<Dims, Fn, kparams, kargs>(
                                           device,
                                           r,
                                           l,
                                           attrd_body.get_body(),
                                           attrs,
                                           attrd_body.get_gpu_kernel(),
                                           attrd_body.get_cl_kernel_args());
  }

private:
  const legacy::body_with_attrs_gpu<Fn, Kernel, Args...>& _b;
};
#endif

template<typename Body>
struct body_wrapper : public body_wrapper_base<Body>
{
  typedef typename body_wrapper_base<Body>::traits body_traits;
  typedef typename body_traits::return_type return_type;

  static const size_t s_arity = body_traits::arity::value;

  MARE_CONSTEXPR  static
  size_t get_arity() {
    return s_arity;
  }

  template<typename ...Args>
  MARE_CONSTEXPR  return_type operator()(Args... args) const {
    return (body_wrapper_base<Body>::get_fn())(args...);
  }

  MARE_CONSTEXPR explicit body_wrapper(Body&& b):
    body_wrapper_base<Body>(std::forward<Body>(b))
  {}
};

template<typename B>
static body_wrapper<B> get_body_wrapper(B&& b)
{
  return body_wrapper<B>(std::forward<B>(b));
}

template<typename T>
struct remove_cv_and_reference{
  typedef typename
  std::remove_cv<typename std::remove_reference<T>::type>::type type;
};

template<typename T>
struct is_task_ptr : std::integral_constant<
  bool,
  std::is_same<::mare::internal::task_shared_ptr, typename remove_cv_and_reference<T>::type>::value> {};

template< class T >
struct is_mare_task_ptr : std::integral_constant<
  bool,
  is_task_ptr<T>::value> {};

template<typename T, typename =
         typename std::enable_if<is_mare_task_ptr<T>::value>::type>
inline void launch_dispatch(::mare::internal::group_shared_ptr const& a_group, T const& a_task,
                            bool notify = true)
{
  (void) notify;
  auto t_ptr = c_ptr(a_task);
  auto g_ptr = c_ptr(a_group);

  MARE_API_ASSERT(t_ptr, "null task_ptr");
  t_ptr->launch(g_ptr, nullptr);
}

template<typename Body,
         typename =
         typename std::enable_if<!is_mare_task_ptr<Body>::value>::type>
inline void launch_dispatch(::mare::internal::group_shared_ptr const& a_group, Body&& body,
                            bool notify = true)
{
  auto g_ptr = c_ptr(a_group);
  MARE_API_ASSERT(g_ptr, "null group_ptr");

  ::mare::internal::legacy::body_wrapper<Body> wrapper(std::forward<Body>(body));
  auto attrs = wrapper.get_attrs();

  if (!has_attr(attrs, ::mare::internal::legacy::attr::blocking)) {
    attrs = legacy::add_attr(attrs, ::mare::internal::attr::anonymous);
    task* t =  wrapper.create_task(g_ptr, attrs);
    g_ptr->inc_task_counter();
    send_to_runtime(t, nullptr, notify);
  } else {
    task* t =  wrapper.create_task(g_ptr, attrs);
    t->launch(g_ptr, nullptr);
  }
}

template<typename T, typename =
         typename std::enable_if<is_mare_task_ptr<T>::value>::type>
inline void launch_dispatch(T const& task)
{
  static ::mare::internal::group_shared_ptr null_group_ptr;
  ::mare::internal::legacy::launch_dispatch(null_group_ptr, task);
}

MARE_GCC_IGNORE_END("-Weffc++");
};
};
};
