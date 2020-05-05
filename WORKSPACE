
workspace(name = "blog_code")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")

http_archive(
    name = "com_github_google_benchmark",
    urls = ["https://github.com/google/benchmark/archive/16703ff83c1ae6d53e5155df3bb3ab0bc96083be.zip"],
    strip_prefix = "benchmark-16703ff83c1ae6d53e5155df3bb3ab0bc96083be",
    sha256 = "59f918c8ccd4d74b6ac43484467b500f1d64b40cc1010daa055375b322a43ba3",
)

git_repository(
    name = "com_google_absl",
    commit = "df3ea785d8c30a9503321a3d35ee7d35808f190d",  # LTS 2020-02-25
    remote = "https://github.com/abseil/abseil-cpp.git",
    shallow_since = "1583355457 -0500"
)
