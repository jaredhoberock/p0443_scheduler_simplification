// $ clang-10 -std=c++20 demo.cpp -lstdc++

#include <exception>
#include "execution.hpp"
#include <iostream>
#include <utility>


struct execution_context
{
  template<class F>
    requires invocable<F&>
  void execute_invocable(F f) const
  {
    std::invoke(f);
  }

  template<execution::receiver_of R>
  void submit_receiver(R&& r) const
  {
    try
    {
      execution::set_value(std::move(r));
    }
    catch(...)
    {
      execution::set_error(std::move(r), std::current_exception());
    }
  }


  struct executor_type
  {
    const execution_context& context_;

    template<class F>
      requires invocable<F&>
    void execute(F&& f) const
    {
      context_.execute_invocable(std::forward<F>(f));
    }

    friend bool operator==(const executor_type& a, const executor_type& b)
    {
      return &a.context_ == &b.context_;
    }

    friend bool operator!=(const executor_type& a, const executor_type& b)
    {
      return !(a == b);
    }
  };

  executor_type executor() const
  {
    return {*this};
  }


  struct scheduler_type
  {
    const execution_context& context_;

    struct sender_type
    {
      template<template<class...> class Tuple, template<class...> class Variant>
      using value_types = Variant<Tuple<>>;

      template<template<class...> class Variant>
      using error_types = Variant<std::exception_ptr>;

      static constexpr bool sends_done = true;

      const execution_context& context_;

      template<execution::receiver_of R> 
      struct operation
      {
        const execution_context& context_;
        remove_cvref_t<R> receiver_;

        void start() noexcept
        {
          context_.submit_receiver(std::move(receiver_));
        }
      };

      template<execution::receiver_of R>
      operation<R> connect(R&& r) const
      {
        return {context_, std::forward<R>(r)};
      }
    };

    sender_type schedule() const
    {
      return {context_};
    }

    friend bool operator==(const scheduler_type& a, const scheduler_type& b)
    {
      return &a.context_ == &b.context_;
    }

    friend bool operator!=(const scheduler_type& a, const scheduler_type& b)
    {
      return !(a == b);
    }
  };

  scheduler_type scheduler() const
  {
    return {*this};
  }
};


static_assert(execution::executor<execution_context::executor_type>);
static_assert(execution::scheduler<execution_context::executor_type>);
static_assert(execution::sender<execution_context::executor_type>);
static_assert(execution::scheduler<execution_context::scheduler_type>);
static_assert(execution::sender<execution_context::scheduler_type::sender_type>);


struct my_receiver
{
  void set_value() && noexcept
  {
    std::cout << "my_receiver::set_value" << std::endl;
  }

  template<class E>
  void set_error(E&&) && noexcept
  {
    std::cout << "my_receiver::set_error" << std::endl;
  }

  void set_done() && noexcept
  {
    std::cout << "my_receiver::set_done" << std::endl;
  }
};

static_assert(execution::is_nothrow_receiver_of_v<my_receiver>);
static_assert(execution::operation_state<execution_context::scheduler_type::sender_type::operation<my_receiver>>);


int main()
{
  execution_context ctx;

  {
    auto ex = ctx.executor();

    // execute a lambda on the executor
    execution::execute(ex, []
    {
      std::cout << "lambda" << std::endl;
    });
  }

  {
    auto sched = ctx.scheduler();

    // submit a receiver on the scheduler
    execution::submit(execution::schedule(sched), my_receiver{});
  }

  {
    auto ex = ctx.executor();

    // treat the executor like a scheduler
    execution::submit(execution::schedule(ex), my_receiver{});
  }

  {
    auto ex = ctx.executor();

    // treat the executor like a sender of void
    // XXX undesirable
    execution::submit(ex, my_receiver{});
  }

  return 0;
}

