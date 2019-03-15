cc_library(
    name = "vizier_node",
    hdrs = ["vizier_node.h"],
    #linkopts = ["-pthread", "-lmosquitto"],
    includes = ["."],
    deps = [
            "//vizier/utils/mqttclient:mqttclient",
    ],
)

cc_binary(
    name = "vizier_node_test",
    srcs = ["vizier_node.cpp"],
    deps = [
        ":mqttclient",
        "@json//:json",
    ],
)