cc_library(
    name = "mqttclient",
    hdrs = ["mqttclient.h"],
    linkopts = ["-pthread", "-lmosquitto"],
    includes = ["."],
    deps = [
            "//tsqueue:tsqueue",
            "//promise:promise"
    ],
)

cc_binary(
    name = "mqttclienttest",
    srcs = ["mqttclienttest.cc"],
    deps = [
        ":mqttclient"
    ],
)
