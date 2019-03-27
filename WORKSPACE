
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

new_http_archive(
    name = "spdlog",
    urls = ["https://github.com/gabime/spdlog/archive/v1.x.zip"],
    #url = "http://example.com/openssl.zip",
    #sha256 = "03a58ac630e59778f328af4bcc4acb4f80208ed4",
    strip_prefix = "spdlog-1.x",
    build_file = "BUILD.spdlog",
)

new_http_archive(
    name = "json",
    urls = ["https://github.com/nlohmann/json/archive/develop.zip"],
    strip_prefix = "json-develop",
    build_file = "BUILD.njson",
)