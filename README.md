# p0443_scheduler_simplification

The purpose of this repository is to illustrate a proposal to simplify the executors programming model of P0443R13 by eliminating the named concept `scheduler` while retaining all of P0443's existing functionality.

The proposed change is similar to a previous simplification which eliminated the concept `bulk_executor` while retaining the functionality of `bulk_execute`.

Currently, concepts `executor` and `scheduler` are "peers". Both are usable with `execution::execute` and `execution::schedule`. This results in dependency cycles between `execute` and `schedule`, complicating the design and implementation of P0443.

Proposal Summary:
* Allow `executor`s to optionally `execute` `receiver`s of `void` directly, in addition to nullary `invocable`s. Executors which natively `execute` a `receiver` must fulfill the `receiver` contract.
* If an `executor` cannot natively `execute` a `receiver`, `execution::execute(ex, receiver)` always performs a fallback adaptation. This makes all `executor`s of `invocable`s also able to execute `receiver`s of `void`.
* When executing an `invocable`, normatively encourage `executor`s to report errors and cancellation to the invocable via `execution::set_error` and `execution::set_done`, thus providing optional error reporting channels executors currently lack.
* Eliminate `scheduler`. `execution::scheduler` works exactly as before, but accepts an `executor`.

~~~~

Before:

```
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
    // XXX some reviewers perceive this automatic adaptation as undesirable
    execution::submit(ex, my_receiver{});
  }

  return 0;
}
```

```
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
  //  // this is no longer allowed
  //  execution::submit(ex, my_receiver{});
  //}

  return 0;
}
```

