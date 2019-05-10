#ifndef VIZIER_TSQUEUE_H
#define VIZIER_TSQUEUE_H

#include <queue>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <optional>

#include <iostream>

template<class T> using optional = std::optional<T>;
template<class T> using unique_lock = std::unique_lock<T>;
using mutex = std::mutex;

template <class T> 
class ThreadSafeQueue {

private:
  std::queue<T> q;
	mutex m;
	std::condition_variable c;

public:
  ThreadSafeQueue() = default;
  ~ThreadSafeQueue() = default;

  int size() {
    std::lock_guard<mutex> lock(m);
    return q.size();
  }

  void enqueue(const T& t) {
    std::lock_guard<mutex> lock(m);
    q.push(t);
    c.notify_one();
  }

  void enqueue(T&& t) {
    std::lock_guard<mutex> lock(m);
    q.push(std::move(t));
    c.notify_one();
  }

  T dequeue() {

    //Get a lock on the mutex
    std::unique_lock<mutex> lock(m);

    // While our condition isn't satisfied, wait on the lock.  Protects against
    // Spurious wake-ups
    while(q.empty()) {
      c.wait(lock);
    }

    T val = std::move(q.front());
    q.pop();

    return val;
  }

  optional<T> dequeue(std::chrono::milliseconds timeout) {
    std::unique_lock<mutex> lock(m);

    // While our condition isn't satisfied, wait on the lock.  Protects against
    // Spurious wake-ups
    std::cv_status result;
    while(q.empty()) {
      // TODO: Shorten timeout if spurious wakeup
      result = c.wait_for(lock, timeout);

      if(result == std::cv_status::timeout) {
        return std::nullopt;
      }
    }

    T val = std::move(q.front());
    q.pop();

    // Constructs to optional
    return val;
  }
};

#endif
