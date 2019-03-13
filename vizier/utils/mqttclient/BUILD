cc_library(
    name = "mqttclient",
    hdrs = ["mqttclient.h", "mqttclient_async.h"],
    linkopts = ["-pthread", "-lmosquitto"],
    includes = ["."],
    deps = [
            "//vizier/utils/promise:promise",
            "//vizier/utils/tsqueue:tsqueue",
    ],
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