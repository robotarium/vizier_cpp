cc_library(
    name = "mqttclient",
    hdrs = ["mqttclient_async.h"],
    linkopts = ["-pthread", "-lmosquitto"],
    deps = [
            "//vizier/vizier_node:utils",
            "//vizier/utils/tsqueue:tsqueue",
            "@spdlog//:spdlog"
    ],
    visibility = ["//visibility:public"],
)

cc_binary(
    name = "mqttclienttestasync",
    srcs = ["mqttclienttest_async.cc"],
    deps = [
        ":mqttclient",
        "@spdlog//:spdlog",
    ],
)