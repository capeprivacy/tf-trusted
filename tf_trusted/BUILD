licenses(["notice"])  # Apache v2.0

package(
    default_visibility = ["//visibility:public"],
)

load(
    "@com_google_asylo//asylo/bazel:asylo.bzl",
    "enclave_loader",
    "sim_enclave",
    "enclave_test",
)
load(
    "@com_google_asylo//asylo/bazel:proto.bzl",
    "asylo_proto_library",
    "asylo_grpc_proto_library",
)

# The generated code for the model server (and client).
asylo_grpc_proto_library(
    name = "model_server_grpc_proto",
    srcs = ["proto/model_server.proto"],
)

cc_library(
    name = "model_server",
    srcs = ["model_server.cc"],
    hdrs = ["model_server.h"],
    deps = [
        ":model_server_grpc_proto",
        "@com_github_grpc_grpc//:grpc++",
        "@org_tensorflow//tensorflow/lite:framework",
        "@org_tensorflow//tensorflow/lite/kernels:builtin_ops",
    ],
    copts = ["-DEIGEN_NO_CPUID"],
)

# Contains extensions to enclave protos.
asylo_proto_library(
    name = "tf_trusted_config_proto",
    srcs = ["tf_trusted_config.proto"],
    deps = ["@com_google_asylo//asylo:enclave_proto"],
)

# The enclave
sim_enclave(
    name = "tf_trusted_enclave.so",
    srcs = ["tf_trusted_enclave.cc"],
    config = "@com_google_asylo//asylo/grpc/util:grpc_enclave_config",
    deps = [
        ":tf_trusted_config_proto_cc",
        ":model_server",
        "@com_github_grpc_grpc//:grpc++",
        "@com_github_grpc_grpc//:grpc++_reflection",
        "@com_google_absl//absl/memory",
        "@com_google_absl//absl/synchronization",
        "@com_google_asylo//asylo:enclave_runtime",
        "@com_google_asylo//asylo/util:status",
    ],
)

# The driver for the gRPC server enclave.
enclave_loader(
    name = "tf_trusted",
    enclaves = {"enclave": ":tf_trusted_enclave.so"},
    loader_args = ["--enclave_path='{enclave}'"],
    srcs = ["tf_trusted_driver.cc"],
    deps = [
        ":tf_trusted_config_proto_cc",
        "@com_github_gflags_gflags//:gflags_nothreads",
        "@com_google_absl//absl/synchronization",
        "@com_google_absl//absl/time",
        "@com_google_asylo//asylo:enclave_client",
        "@com_google_asylo//asylo/util:logging",
    ],
)
