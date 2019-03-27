#include "vizier/vizier_node/vizier_node.h"
#include <iostream>


void test_create_message_id() {
    for(int i = 0; i < 10; ++i) {
        std::cout << vizier::create_message_id() << std::endl;
    }
}

int main() {
    test_create_message_id();
    return 0;
}