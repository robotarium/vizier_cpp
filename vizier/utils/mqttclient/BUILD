cc_library(
    name = "mqttclient",
    hdrs = ["mqttclient.h"],
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
