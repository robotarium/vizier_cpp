cc_library(
    name = "mqttclient",
    hdrs = ["mqttclient.h", "mqttclient_async.h"],
    linkopts = ["-pthread", "-lmosquitto"],
    deps = [
            "//vizier/utils/tsqueue:tsqueue",
            "@spdlog//:spdlog"
    ],
    visibility = ["//visibility:public"],
)

cc_binary(
    name = "mqttclienttest",
    srcs = ["mqttclienttest.cc"],
    deps = [
        ":mqttclient",
        "@spdlog//:spdlog",
    ],
)

cc_binary(
    name = "mqttclienttestasync",
    srcs = ["mqttclienttest_async.cc"],
    deps = [
        ":mqttclient",
        "@spdlog//:spdlog",
    ],
)