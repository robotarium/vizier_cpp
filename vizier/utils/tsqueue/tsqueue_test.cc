#include "vizier/utils/tsqueue/tsqueue.h"
#include <chrono>
#include <optional>
#include "gtest/gtest.h"

TEST(Enqueue, Basic) {
    ThreadSafeQueue<size_t> q;
    for (size_t i = 0; i < size_t(50); ++i) {
        q.enqueue(i);
    }

    for (size_t i = 0; i < size_t(50); ++i) {
        EXPECT_EQ(i, q.dequeue());
    }
}

TEST(Dequeue, Timeout) {
    ThreadSafeQueue<int> q;

    std::optional<int> result = q.dequeue(std::chrono::milliseconds(1000));
    EXPECT_FALSE(bool(result));
}