#ifndef VIZIER_PROMISE_H
#define VIZIER_PROMISE_h

#include <mutex>
#include <condition_variable>

template <class T> 
class Promise {

private:
  T value = NULL;
  mutable std::mutex m;
  std::condition_variable c;

public:
  Promise() = default; 
  //{
    //value = NULL;
    //m = std::mutex();
    //c = std::condition_variable();
  //}

  ~Promise(void) {}

  T wait() {
    std::unique_lock<std::mutex> lock(this->m);

    // While our condition isn't satisfied, wait on the lock.  Protects against
    // Spurious wake-ups
    while(this->value == NULL)
    {
      this->c.wait(lock);
    }

    return this->value;
  }

  bool fulfill(T val) {
    std::lock_guard<std::mutex> lock(m);

    if(val != NULL) {
      return false;
    }
    
    this->value = val;
    this->c.notify_all();

    return true;
  }
};
#endif
