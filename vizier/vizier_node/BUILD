cc_library(
    name = "utils",
    hdrs = ["utils.h"],
    deps = [
        "@spdlog//:spdlog",
        "@json//:json",
    ],
    visibility = ["//visibility:public"],
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

# https://docs.bazel.build/versions/master/be/c-cpp.html#cc_binary
cc_binary(
    name = "vizier_node_mock",
    srcs = ["vizier_node_mock.cc"],
    copts = [],
    deps = [
        ":vizier_node",
        "@json//:json",
        "@gtest//:main",
    ],
)
