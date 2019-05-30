#ifndef VIZIER_TSQUEUE_H
#define VIZIER_TSQUEUE_H

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <optional>
#include <queue>

template <class T>
using optional = std::optional<T>;
template <class T>
using unique_lock = std::unique_lock<T>;
using mutex = std::mutex;
using condition_variable = std::condition_variable;

/*
TODO: Doc
*/
template <class T>
class ThreadSafeQueue {
private:
    std::queue<T> q;
    mutable std::unique_ptr<mutex> m;
    std::unique_ptr<std::condition_variable> c;

public:
    /*
    TODO: Doc
    */
    ThreadSafeQueue() {
        m = std::make_unique<mutex>();
        c = std::make_unique<condition_variable>();
    }

    ThreadSafeQueue(ThreadSafeQueue&& that) = default;
    ThreadSafeQueue& operator=(ThreadSafeQueue&& that) = default;
    ~ThreadSafeQueue() = default;

    /*
    TODO: Doc
    */
    size_t size() const {
        std::lock_guard<mutex> lock(*m);
        return q.size();
    }

    /*
    TODO: Doc
    */
    void enqueue(const T& t) {
        std::lock_guard<mutex> lock(*m);
        q.push(t);
        c->notify_one();
    }

    /*
    TODO: Doc
    */
    void enqueue(T&& t) {
        std::lock_guard<mutex> lock(*m);
        q.push(std::move(t));
        c->notify_one();
    }

    /*
    TODO: Doc
    */
    T dequeue() {
        // Get a lock on the mutex
        std::unique_lock<mutex> lock(*m);

        // While our condition isn't satisfied, wait on the lock.  Protects against
        // Spurious wake-ups
        while (q.empty()) {
            c->wait(lock);
        }

        T val = std::move(q.front());
        q.pop();

        return val;
    }

    /*
    TODO: Doc
    */
    optional<T> dequeue(std::chrono::milliseconds timeout) {
        std::unique_lock<mutex> lock(*m);

        // While our condition isn't satisfied, wait on the lock.  Protects against
        // Spurious wake-ups
        while (q.empty()) {
            // TODO: Shorten timeout if spurious wakeup
            std::cv_status result = c->wait_for(lock, timeout);

            if (result == std::cv_status::timeout) {
                return std::nullopt;
            }
        }

        T val = std::move(q.front());
        q.pop();

        // Constructs to optional
        // TODO: is std::move necessary?
        return std::move(val);
    }
};

#endif
