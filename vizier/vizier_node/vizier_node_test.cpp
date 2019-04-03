#include "vizier/vizier_node/vizier_node.h"
#include "gtest/gtest.h"
#include <iostream>


TEST(CreateMessageIdtest, CreatesCorrectIdLength) {
    for(int i = 0; i < 100; ++i) {
        EXPECT_EQ(vizier::create_message_id().length(), 64);
    }
}