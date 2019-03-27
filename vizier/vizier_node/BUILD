cc_library(
    name = "vizier_node",
    hdrs = ["vizier_node.h"],
    deps = [
            "//vizier/utils/mqttclient:mqttclient",
    ],
)

cc_binary(
    name = "vizier_node_test",
    srcs = ["vizier_node_test.cpp"],
    deps = [
        ":vizier_node",
        "@json//:json",
    ],
)