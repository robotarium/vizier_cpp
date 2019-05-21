cc_library(
    name = "utils",
    hdrs = ["utils.h"],
    deps = [
        "//vizier/utils/mqttclient:mqttclient",
        "@spdlog//:spdlog",
    ],
)

cc_library(
    name = "vizier_node",
    hdrs = ["vizier_node.h"],
    deps = [
        ":utils",
        "//vizier/utils/mqttclient:mqttclient",
        "@json//:json",
        "@spdlog//:spdlog",
    ],
    visibility = ["//visibility:public"],
)

cc_binary(
    name = "utils_test",
    srcs = ["utils_test.cc"],
    copts = ["-Iexternal/gtest/include"],
    deps = [
        ":utils",
        "@json//:json",
        "@gtest//:main",
    ],
)

cc_binary(
    name = "vizier_node_test",
    srcs = ["vizier_node_test.cc"],
    copts = ["-Iexternal/gtest/include"],
    deps = [
        ":vizier_node",
        "@json//:json",
        "@gtest//:main",
    ],
)
