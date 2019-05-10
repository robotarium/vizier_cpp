cc_library(
    name = "tsqueue",
    hdrs = ["tsqueue.h"],
    includes = ["."],
    visibility = ["//visibility:public"],
)

cc_binary(
    name = "tsqueue_test",
    srcs = ["tsqueue_test.cc",],
    copts = ["-Iexternal/gtest/include"],
    deps = [
        "@gtest//:main",
        ":tsqueue",
    ]
)
