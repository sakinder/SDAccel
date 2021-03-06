#ifndef xcl_core_task_h_
#define xcl_core_task_h_

#include <future>
#include <functional>
#include <chrono>
#include <vector>
#include <mutex>
#include <condition_variable>

#include <iostream>

namespace xrt { namespace task {

/**
 * Type erased std::packaged_task<RT()>
 *
 * Wraps a std::packaged_task of any return type, such that the task's
 * return value can be captured in a future.
 * 
 * Objects of this task class can be stored in any STL container even
 * when the underlying std::packaged_tasks are of different types.
 */
class task
{
  struct task_iholder 
  {
    virtual ~task_iholder() {};
    virtual void execute() = 0;
  };

  template <typename RT>
  struct task_holder : public task_iholder
  {
    std::packaged_task<RT()> held;
    task_holder(std::packaged_task<RT()>&& t) : held(std::move(t)) {}
    void execute() { held(); }
  };

  std::unique_ptr<task_iholder> content;

public:
  task() 
    : content(nullptr) 
  {}

  task(task&& rhs) 
    : content(std::move(rhs.content)) 
  {
    // Invalidate rhs to avoid double delete
    rhs.content = nullptr;
  }

  template <typename RT>
  task(std::packaged_task<RT()>&& t) 
    : content(new task_holder<RT>(std::move(t))) 
  {}

  task&
  operator=(task&& rhs)
  {
    content = std::move(rhs.content);
    return *this;
  }

  bool
  valid() const
  {
    return content!=nullptr;
  }

  void
  execute()
  {
    content->execute();
  }

  void 
  operator() ()
  {
    execute();
  }
};

/**
 * Multiple producer / multiple consumer queue of task objects
 */
class queue
{
  std::vector<task> m_tasks;
  std::mutex m_mutex;
  std::condition_variable m_work;
  bool m_stop;
public:
  queue() : m_stop(false) {}

  void 
  addWork(task&& t)
  {
    std::lock_guard<std::mutex> lk(m_mutex);
    m_tasks.push_back(std::move(t));
    m_work.notify_one();
  }

  task
  getWork()
  {
    std::unique_lock<std::mutex> lk(m_mutex);
    while (!m_stop && m_tasks.empty()) {
      m_work.wait(lk);
    }

    task task;
    if (!m_stop) {
      task = std::move(m_tasks.back());
      m_tasks.pop_back();
    }
    return std::move(task);
  }

  void
  stop()
  {
    std::lock_guard<std::mutex> lk(m_mutex);
    m_stop=true;
    m_work.notify_all();
  }
};

/**
 * event class wraps std::future<RT>
 *
 * Adds a ready() function that can be used to poll if event is ready.
 * Otherwise, currently adds no value compared to bare std::future
 */
template <typename RT>
class event
{
  mutable std::future<RT> m_future;
public:
  typedef RT value_type;

public:
  event() = delete;
  event(const event& rhs) = delete;

  event(const event&& rhs)
    : m_future(std::move(rhs.m_future))
  {}

  event(std::future<RT>&& f) 
    : m_future(std::move(f)) 
  {}

  event&
  operator=(event&& rhs)
  {
    m_future = std::move(rhs.m_future);
    return *this;
  }

  RT
  wait() const
  {
    return m_future.get();
  }

  RT
  get() const
  {
    return m_future.get();
  }

  bool
  ready() const
  {
    return (m_future.wait_for(std::chrono::seconds(0)) == std::future_status::ready);
  }
};

/**
 * Functions for adding work (tasks) to a task queue.
 * 
 * All functions return a task::event that encapsulates the 
 * return type of the task.  
 *
 * Variants of the functions supports adding both free functions
 * and member functions associated with some class object.
 */
// Free function, lambda, functor
template <typename Q,typename F, typename ...Args>
auto 
createF(Q& q, F&& f, Args&&... args) 
  -> event<decltype(f(std::forward<Args>(args)...))>
{
  typedef decltype(f(std::forward<Args>(args)...)) value_type;
  typedef std::packaged_task<value_type()> task_type;
  task_type t(std::bind(std::forward<F>(f),std::forward<Args>(args)...));
  event<value_type> e(t.get_future());
  q.addWork(std::move(t));
  return std::move(e);
}

// Member function.  gcc4.8.4+ does not need the std::bind hints in decltype
// The return type can be deduced the same as the non member function version.
template <typename Q,typename F, typename C, typename ...Args>
auto 
createM(Q& q, F&& f, C& c, Args&&... args) 
  -> event<decltype(std::bind(std::forward<F>(f),std::ref(c),std::forward<Args>(args)...)())>
{
  typedef decltype(std::bind(std::forward<F>(f),std::ref(c),std::forward<Args>(args)...)()) value_type;
  typedef std::packaged_task<value_type()> task_type;
  task_type t(std::bind(std::forward<F>(f),std::ref(c),std::forward<Args>(args)...));
  event<value_type> e(t.get_future());
  q.addWork(std::move(t));
  return std::move(e);
}

// A task worker is a thread function getting work off a task queue.
// The worker runs until the queue is stopped.
inline void
worker(queue& q)
{
  while (true) {
    auto t = q.getWork();
    if (!t.valid())
      break;
    t();
  }
}

}} // task,xrt

#endif
