#pragma once

#include <functional>
#include <type_traits>
#include <utility>


template<class Derived, class Base>
concept derived_from =
  std::is_base_of_v<Base, Derived> and
  std::is_convertible_v<const volatile Derived*, const volatile Base*>
;


template<class From, class To>
concept convertible_to =
  std::is_convertible_v<From, To> and
  requires(std::add_rvalue_reference_t<From> (&f)())
  {
    static_cast<To>(f());
  }
;


template<class T>
concept destructible = std::is_nothrow_destructible_v<T>;


template<class T, class... Args>
concept constructible_from = destructible<T> and std::is_constructible_v<T,Args...>;


template<class T>
concept move_constructible = constructible_from<T,T> and convertible_to<T,T>;


template<class T>
concept copy_constructible =
  move_constructible<T> and
  constructible_from<T,T&> and convertible_to<T&,T> and
  constructible_from<T,const T&> and convertible_to<const T&,T> and
  constructible_from<T,const T> and convertible_to<const T,T>
;


namespace detail
{


template<class B>
concept boolean_testable_impl = convertible_to<B,bool>;


template<class B>
concept boolean_testable =
  boolean_testable_impl<B> and
  requires(B&& b)
  {
    { !std::forward<B>(b) } -> boolean_testable_impl;
  }
;


template<class T, class U>
concept weakly_equality_comparable_with =
  requires(const std::remove_reference_t<T>& t, const std::remove_reference_t<T>& u)
  {
    { t == u } -> boolean_testable;
    { t != u } -> boolean_testable;
    { u == t } -> boolean_testable;
    { u != t } -> boolean_testable;
  }
;


} // end detail


template<class T>
concept equality_comparable = detail::weakly_equality_comparable_with<T,T>;


template<class F, class... Args>
concept invocable =
  requires(F&& f, Args&&... args)
  {
    std::invoke(std::forward<F>(f), std::forward<Args>(args)...);
  }
;

