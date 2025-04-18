"""Provides the repository macro to import Gloo."""

load("//third_party:repo.bzl", "tf_http_archive", "tf_mirror_urls")

def repo():
    """Imports Gloo."""

    GLOO_COMMIT = "a38a5345a1d01f5db65e1f91d42effe07fee1e5c"
    GLOO_SHA256 = "fdd0ec992541258eb7446cce519050fed59bfde10f2adf6103a118c56ecea7a5"

    tf_http_archive(
        name = "gloo",
        sha256 = GLOO_SHA256,
        strip_prefix = "gloo-{commit}".format(commit = GLOO_COMMIT),
        urls = tf_mirror_urls("https://github.com/wenlongx/gloo/archive/{commit}.tar.gz".format(commit = GLOO_COMMIT)),
        build_file = "//third_party/gloo:gloo.BUILD",
    )
