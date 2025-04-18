load(
    "@local_config_rocm//rocm:build_defs.bzl",
    "if_rocm_is_configured",
)
load(
    "@tsl//tsl/platform/default:cuda_build_defs.bzl",
    "if_cuda_is_configured",
)
load("//xla:xla.bzl", "xla_cc_test")
load("//xla/tsl:tsl.bzl", "internal_visibility")

package(
    # copybara:uncomment default_applicable_licenses = ["//tensorflow:license"],
    default_visibility = internal_visibility([":friends"]),
    licenses = ["notice"],
)

package_group(
    name = "friends",
    includes = [
        "//xla:friends",
    ],
)

cc_library(
    name = "llvm_gpu_backend",
    srcs = [
        "gpu_backend_lib.cc",
        "utils.cc",
    ],
    hdrs = [
        "gpu_backend_lib.h",
        "utils.h",
    ],
    local_defines = if_cuda_is_configured(["GOOGLE_CUDA=1"]) + if_rocm_is_configured(["TENSORFLOW_USE_ROCM=1"]),
    deps = [
        "//xla:status",
        "//xla:status_macros",
        "//xla:statusor",
        "//xla:types",
        "//xla:util",
        "//xla:xla_proto_cc",
        "//xla/service/gpu:metrics",
        "//xla/service/llvm_ir:llvm_command_line_options",
        "//xla/service/llvm_ir:llvm_type_conversion_util",
        "//xla/stream_executor:device_description",
        "//xla/tsl/util:env_var",
        "@com_google_absl//absl/base",
        "@com_google_absl//absl/base:core_headers",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/memory",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_absl//absl/synchronization",
        "@llvm-project//llvm:Analysis",
        "@llvm-project//llvm:BitReader",
        "@llvm-project//llvm:BitWriter",
        "@llvm-project//llvm:CodeGen",
        "@llvm-project//llvm:Core",
        "@llvm-project//llvm:IPO",
        "@llvm-project//llvm:IRReader",
        "@llvm-project//llvm:Linker",
        "@llvm-project//llvm:MC",
        "@llvm-project//llvm:NVPTXCodeGen",  # buildcleaner: keep
        "@llvm-project//llvm:ObjCARC",  # buildcleaner: keep
        "@llvm-project//llvm:Passes",
        "@llvm-project//llvm:Scalar",
        "@llvm-project//llvm:Support",
        "@llvm-project//llvm:Target",
        "@llvm-project//mlir:NVVMDialect",
        "@local_config_cuda//cuda:cuda_headers",
        "@tsl//tsl/platform:cuda_libdevice_path",
        "@tsl//tsl/platform:env",
        "@tsl//tsl/platform:errors",
        "@tsl//tsl/platform:logging",
        "@tsl//tsl/platform:path",
        "@tsl//tsl/platform:random",
        "@tsl//tsl/platform:rocm_rocdl_path",
        "@tsl//tsl/platform:status",
        "@tsl//tsl/platform:statusor",
        "@tsl//tsl/profiler/lib:scoped_annotation",
        "@tsl//tsl/profiler/lib:traceme",
    ] + if_rocm_is_configured([
        "@local_config_rocm//rocm:rocm_headers",
        "@llvm-project//llvm:AMDGPUCodeGen",
        "@llvm-project//llvm:AMDGPUAsmParser",
    ]),
)

xla_cc_test(
    name = "utils_test",
    size = "small",
    srcs = ["utils_test.cc"],
    data = [
        "tests_data/saxpy.ll",
    ],
    deps = [
        ":llvm_gpu_backend",
        "//xla/tests:xla_internal_test_main",
        "@llvm-project//llvm:Core",
        "@tsl//tsl/platform:path",
        "@tsl//tsl/platform:resource_loader",
        "@tsl//tsl/platform:test",
    ],
)
