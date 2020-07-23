// $ clang-10 -std=c++20 demo.cpp -lstdc++

#include <iostream>
#include <exception>
#include "execution.hpp"
#include <iostream>
#include <utility>


template<class T>
concept can_set_error = requires(T&& t) { execution::set_error(std::forward<T>(t)); };


struct execution_context
{
  template<class F>
    requires invocable<F&>
  void execute_invocable(F f) const noexcept
  {
    std::invoke(f);
  }


  struct executor_type
  {
    const execution_context& context_;

    template<class F>
      requires invocable<F&>
    void execute(F&& f) const noexcept
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
};


static_assert(execution::executor<execution_context::executor_type>);


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
static_assert(execution::executor_of<execution_context::executor_type, my_receiver>);


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
    auto ex = ctx.executor();

    // NEW: execute a receiver on the executor
    execution::execute(ex, my_receiver{});
  }

  {
    auto ex = ctx.executor();

    // schedule a receiver on the executor
    auto lazy_op = execution::connect(execution::schedule(ex), my_receiver{});

    execution::start(lazy_op);
  }

  //{
  //  auto ex = ctx.executor();

  //  // ERROR: treat the executor like a sender of void
  //  execution::submit(ex, my_receiver{});
  //}

  return 0;
}

