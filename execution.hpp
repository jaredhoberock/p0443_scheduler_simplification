#pragma once

#include "concepts.hpp"
#include <exception>
#include <functional>
#include <memory>
#include <optional>
#include <type_traits>
#include <utility>


template<class T>
using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>;


namespace execution
{
namespace detail
{


template<class E, class F>
concept has_execute_member_function = requires(E&& e, F&& f) { std::forward<E>(e).execute(std::forward<F>(f)); };

template<class E, class F>
concept has_execute_free_function = requires(E&& e, F&& f) { execute(std::forward<E>(e), std::forward<F>(f)); };


struct execute_t
{
  template<class E, class F>
    requires has_execute_member_function<E&&,F&&>
  constexpr auto operator()(E&& e, F&& f) const noexcept(noexcept(std::forward<E>(e).execute(std::forward<F>(f))))
  {
    return std::forward<E>(e).execute(std::forward<F>(f));
  }

  template<class E, class F>
    requires (!has_execute_member_function<E&&,F&&> and has_execute_free_function<E&&,F&&>)
  constexpr auto operator()(E&& e, F&& f) const noexcept(noexcept(execute(std::forward<E>(e), std::forward<F>(f))))
  {
    return execute(std::forward<E>(e), std::forward<F>(f));
  }
};


} // end detail


constexpr detail::execute_t execute{};


namespace detail
{


template<class E, class F>
concept executor_of_impl =
  invocable<remove_cvref_t<F>&> and
  constructible_from<remove_cvref_t<F>, F> and
  move_constructible<remove_cvref_t<F>> and
  copy_constructible<E> and
  std::is_nothrow_copy_constructible_v<E> and
  equality_comparable<E> and
  requires(const E& e, F&& f)
  {
    execution::execute(e, std::forward<F>(f));
  }
;


} // end detail


struct invocable_archetype
{
  void operator()() noexcept;
};

template<class E>
concept executor = detail::executor_of_impl<E, invocable_archetype>;

template<class E, class F>
concept executor_of = executor<E> and detail::executor_of_impl<E, F>;


namespace detail
{


template<class O>
concept has_start_member_function = requires(O&& o) { std::forward<O>(o).start(); };

template<class O>
concept has_start_free_function = requires(O&& o) { start(std::forward<O>(o)); };

struct start_t
{
  template<class O>
    requires has_start_member_function<O&&>
  constexpr auto operator()(O&& o) const noexcept(noexcept(std::forward<O>(o).start()))
  {
    return std::forward<O>(o).start();
  }

  template<class O>
    requires (!has_start_member_function<O&&> and has_start_free_function<O&&>)
  constexpr auto operator()(O&& o) const noexcept(noexcept(start(std::forward<O>(o))))
  {
    return start(std::forward<O>(o));
  }
};


} // end detail


constexpr detail::start_t start{};


template<class O>
concept operation_state =
  destructible<O> and
  std::is_object_v<O> and
  requires(O& o)
  {
    { execution::start(o) } noexcept;
  }
;


namespace detail
{


template<class R, class E>
concept has_set_error_member_function = requires(R&& r, E&& e) { std::forward<R>(r).set_error(std::forward<E>(e)); };

template<class R, class E>
concept has_set_error_free_function = requires(R&& r, E&& e) { set_error(std::forward<R>(r), std::forward<E>(e)); };

struct set_error_t
{
  template<class R, class E>
    requires has_set_error_member_function<R&&,E&&>
  constexpr auto operator()(R&& r, E&& e) const noexcept(noexcept(std::forward<R>(r).set_error(std::forward<E>(e))))
  {
    return std::forward<R>(r).set_error(std::forward<E>(e));
  }

  template<class R, class E>
    requires (!has_set_error_member_function<R&&,E&&> and has_set_error_free_function<R&&,E&&>)
  constexpr auto operator()(R&& r, E&& e) const noexcept(noexcept(set_error(std::forward<R>(r), std::forward<E>(e))))
  {
    return set_error(std::forward<R>(r), std::forward<E>(e));
  }
};


} // end detail


constexpr detail::set_error_t set_error{};


namespace detail
{


template<class R>
concept has_set_done_member_function = requires(R&& r) { std::forward<R>(r).set_done(); };

template<class R>
concept has_set_done_free_function = requires(R&& r) { set_done(std::forward<R>(r)); };

struct set_done_t
{
  template<class R>
    requires has_set_done_member_function<R&&>
  constexpr auto operator()(R&& r) const noexcept(noexcept(std::forward<R>(r).set_done()))
  {
    return std::forward<R>(r).set_done();
  }

  template<class R>
    requires (!has_set_done_member_function<R&&> and has_set_done_free_function<R&&>)
  constexpr auto operator()(R&& r) const noexcept(noexcept(set_done(std::forward<R>(r))))
  {
    set_done(std::forward<R>(r));
  }
};


} // end detail


constexpr detail::set_done_t set_done{};


template<class T, class E = std::exception_ptr>
concept receiver = 
  move_constructible<remove_cvref_t<T>> and
  constructible_from<remove_cvref_t<T>, T> and
  requires(remove_cvref_t<T>&& t, E&& e)
  {
    { execution::set_done(std::move(t)) } noexcept;
    { execution::set_error(std::move(t), std::forward<E>(e)) } noexcept;
  }
;


namespace detail
{


template<class R, class... Args>
concept has_set_value_member_function = requires(R&& r, Args&&... args) { std::forward<R>(r).set_value(std::forward<Args>(args)...); };

template<class R, class... Args>
concept has_set_value_free_function = requires(R&& r, Args&&... args) { set_value(std::forward<R>(r), std::forward<Args>(args)...); };

struct set_value_t
{
  template<class R, class... Args>
    requires has_set_value_member_function<R&&,Args&&...>
  constexpr auto operator()(R&& r, Args&&... args) const noexcept(noexcept(std::forward<R>(r).set_value(std::forward<Args>(args)...)))
  {
    std::forward<R>(r).set_value(std::forward<Args>(args)...);
  }

  template<class R, class... Args>
    requires (!has_set_value_member_function<R&&,Args&&...> and has_set_value_free_function<R&&,Args&&...>)
  constexpr auto operator()(R&& r, Args&&... args) const noexcept(noexcept(set_value(std::forward<R>(r), std::forward<Args>(args)...)))
  {
    set_value(std::forward<R>(r), std::forward<Args>(args)...);
  }
};


} // end detail


constexpr detail::set_value_t set_value{};


template<class T, class... An>
concept receiver_of = 
  receiver<T> and
  requires(remove_cvref_t<T>&& t, An&&... an)
  {
    execution::set_value(std::move(t), std::forward<An>(an)...);
  }
;


template<class R, class... An>
inline constexpr bool is_nothrow_receiver_of_v =
  receiver_of<R,An...> and
  std::is_nothrow_invocable_v<decltype(set_value), R, An...>
;


namespace detail
{


struct sender_base {};


} // end detail


using detail::sender_base;


namespace detail
{


template<class S>
struct sender_traits_base
{
  using __unspecialized = void;
};


template<template<template<class...> class, template<class...> class> class>
struct has_value_types;

template<template<template<class...> class> class>
struct has_error_types;

template<class S>
concept has_sender_types =
  requires
  {
    typename has_value_types<S::template value_types>;
    typename has_error_types<S::template error_types>;
    typename std::bool_constant<S::sends_done>;
  }
;

template<has_sender_types S>
struct sender_traits_base<S>
{
  template<template<class...> class Tuple, template<class...> class Variant>
  using value_types = typename S::template value_types<Tuple, Variant>;

  template<template<class...> class Variant>
  using error_types = typename S::template error_types<Variant>;

  static constexpr bool sends_done = S::sends_done;
};


template<class S>
  requires derived_from<S, execution::sender_base>
struct sender_traits_base<S>
{

};


} // end detail


template<class S>
struct sender_traits : detail::sender_traits_base<S> {};


template<class S>
concept sender =
  move_constructible<std::remove_reference_t<S>> and
  !requires
  {
    typename sender_traits<remove_cvref_t<S>>::__unspecialized;
  }
;


namespace detail
{


template<class S, class R>
concept has_connect_member_function = requires(S&& s, R&& r) { std::forward<S>(s).connect(std::forward<R>(r)); };

template<class S, class R>
concept has_connect_free_function = requires(S&& s, R&& r) { connect(std::forward<S>(s), std::forward<R>(r)); };



struct connect_t
{
  template<sender S, class R>
    requires has_connect_member_function<S&&,R&&>
  constexpr operation_state auto operator()(S&& s, R&& r) const noexcept(noexcept(std::forward<S>(s).connect(std::forward<R>(r))))
  {
    return std::forward<S>(s).connect(std::forward<R>(r));
  }

  template<sender S, class R>
    requires (!has_connect_member_function<S&&,R&&> and has_connect_free_function<S&&,R&&>)
  constexpr operation_state auto operator()(S&& s, R&& r) const noexcept(noexcept(connect(std::forward<S>(s), std::forward<R>(r))))
  {
    return connect(std::forward<S>(s), std::forward<R>(r));
  }
};


} // end detail


constexpr detail::connect_t connect{};


template<class S, class R>
using connect_result_t = std::invoke_result_t<decltype(connect), S, R>;


template<class S, class R>
concept sender_to =
  sender<S> and
  receiver<R> and
  requires(S&& s, R&& r)
  {
    execution::connect(std::forward<S>(s), std::forward<R>(r));
  }
;


namespace detail
{


template<class S, class R>
concept has_submit_member_function = requires(S&& s, R&& r) { std::forward<S>(s).submit(std::forward<R>(r)); };

template<class S, class R>
concept has_submit_free_function = requires(S&& s, R&& r) { submit(std::forward<S>(s), std::forward<R>(r)); };

template<class S, class R>
struct submit_receiver
{
  struct wrap
  {
    submit_receiver* p_;

    template<class... As>
      requires receiver_of<R, As...>
    void set_value(As&&... as) && noexcept(is_nothrow_receiver_of_v<R, As...>)
    {
      execution::set_value(std::move(p_->r_), std::forward<As>(as)...);
      delete p_;
    }

    template<class E>
      requires receiver<R,E>
    void set_error(E&& e) && noexcept
    {
      execution::set_error(std::move(p_->r_), std::forward<E>(e));
      delete p_;
    }

    void set_done() && noexcept
    {
      execution::set_done(std::move(p_->r_));
      delete p_;
    }
  };

  remove_cvref_t<R> r_;
  connect_result_t<S, wrap> state_;

  submit_receiver(S&& s, R&& r)
    : r_(std::forward<R&&>(r)),
      state_(execution::connect(std::forward<S>(s), wrap{this}))
  {}
};


struct submit_t
{
  template<class S, class R>
    requires sender_to<S,R> and has_submit_member_function<S&&,R&&>
  constexpr auto operator()(S&& s, R&& r) const noexcept(noexcept(std::forward<S>(s).submit(std::forward<R>(r))))
  {
    return std::forward<S>(s).submit(std::forward<R>(r));
  }

  template<class S, class R>
    requires sender_to<S,R> and (!has_submit_member_function<S&&,R&&> and has_submit_free_function<S&&,R&&>)
  constexpr auto operator()(S&& s, R&& r) const noexcept(noexcept(submit(std::forward<S>(s), std::forward<R>(r))))
  {
    return submit(std::forward<S>(s), std::forward<R>(r));
  }

  template<class S, class R>
    requires sender_to<S,R> and (!has_submit_member_function<S&&,R&&> and !has_submit_member_function<S&&,R&&>)
  constexpr void operator()(S&& s, R&& r) const
  {
    execution::start((new submit_receiver<S, R>(std::forward<S>(s), std::forward<R>(r)))->state_);
  }
};

} // end detail


constexpr detail::submit_t submit{};


namespace detail
{


template<class S>
concept has_schedule_member_function = requires(S&& s) { std::forward<S>(s).schedule(); };

template<class S>
concept has_schedule_free_function = requires(S&& s) { schedule(std::forward<S>(s)); };

template<executor Ex>
class as_sender
{
  private:
    Ex ex_;

  public:
    template<template<class...> class Tuple, template<class...> class Variant>
    using value_types = Variant<Tuple<>>;

    template<template<class...> class Variant>
    using error_types = Variant<std::exception_ptr>;

    static constexpr bool sends_done = true;

    explicit as_sender(Ex ex) noexcept
      : ex_(ex)
    {}

    template<receiver_of R>
    struct operation
    {
      using receiver_type = remove_cvref_t<R>;

      Ex ex_;
      receiver_type r_;

      struct as_invocable
      {
        std::optional<receiver_type> r_;

        as_invocable(receiver_type&& r)
          : r_{std::move(r)}
        {}

        as_invocable(as_invocable&& other) noexcept
          : r_{std::move(other.r_)}
        {
          other.r_.reset();
        }

        ~as_invocable()
        {
          if(r_)
          {
            execution::set_done(std::move(*r_));
          }
        }

        template<class E>
        void set_error(E&& e) & noexcept
        {
          execution::set_error(std::move(*r_), std::forward<E>(e));
          r_.reset();
        }

        void operator()() & noexcept
        {
          try
          {
            execution::set_value(std::move(*r_));
            r_.reset();
          }
          catch(...)
          {
            set_error(std::current_exception());
          }
        }
      };

      void start() noexcept
      {
        try
        {
          execution::execute(ex_, as_invocable{std::move(r_)});
        }
        catch(...)
        {
          execution::set_error(std::move(r_), std::current_exception());
        }
      }
    };

    template<receiver_of R>
    operation<R> connect(R&& r) &&
    {
      return {ex_, std::forward<R>(r)};
    }

    template<receiver_of R>
    connect_result_t<const Ex&, R> connect(R&& r) const &
    {
      return execution::connect(ex_, std::forward<R>(r));
    }
};


struct schedule_t
{
  template<class S>
    requires has_schedule_member_function<S&&>
  constexpr sender auto operator()(S&& s) const noexcept(noexcept(std::forward<S>(s).schedule()))
  {
    return std::forward<S>(s).schedule();
  }

  template<class S>
    requires (!has_schedule_member_function<S&&> and has_schedule_free_function<S&&>)
  constexpr sender auto operator()(S&& s) const noexcept(noexcept(schedule(std::forward<S>(s))))
  {
    return schedule(std::forward<S>(s));
  }

  template<executor S>
    requires (!has_schedule_free_function<S&&> and !has_schedule_free_function<S&&>)
  constexpr sender auto operator()(S&& s) const
  {
    return as_sender<remove_cvref_t<S>>{std::forward<S>(s)};
  }
};


} // end detail


constexpr detail::schedule_t schedule{};


} // end execution

