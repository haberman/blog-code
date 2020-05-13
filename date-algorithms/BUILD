
cc_binary(
    name = "benchmark",
    testonly = 1,
    srcs = [
        "algorithms.cc",
        "main.cc",
    ],
    deps = [
        "@com_github_google_benchmark//:benchmark_main",
    ],
    copts = ["-std=c++14"],  # Hinnant algorithm requires this.
)
