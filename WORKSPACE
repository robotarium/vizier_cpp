
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
    name = "spdlog",
    urls = ["https://github.com/gabime/spdlog/archive/v1.x.zip"],
    strip_prefix = "spdlog-1.x",
    build_file = "@//:spdlog.BUILD",
)

http_archive(
    name = "json",
    urls = ["https://github.com/nlohmann/json/archive/develop.zip"],
    strip_prefix = "json-develop",
    build_file = "@//:njson.BUILD",
)

http_archive(
    name = "gtest",
    url = "https://github.com/google/googletest/archive/release-1.7.0.zip",
    sha256 = "b58cb7547a28b2c718d1e38aee18a3659c9e3ff52440297e965f5edffe34b6d0",
    build_file = "@//:gtest.BUILD",
    strip_prefix = "googletest-release-1.7.0",
)
