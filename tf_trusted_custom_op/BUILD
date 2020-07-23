package(
    default_visibility = ["//visibility:public"],
)

load("@build_stack_rules_proto//cpp:cpp_grpc_library.bzl", "cpp_grpc_library")

proto_library(
    name = "model_proto",
    srcs = ["proto/model_server.proto"],
)

cpp_grpc_library(
    name = "cpp_model_proto",
    deps = ["model_proto"],
)

cc_binary(
    name = 'model_enclave_op.so',
    srcs = [
        "model_enclave_op.cc",
        "model_enclave_grpc.h",
    ],
    linkshared = 1,
    deps = [
        "@local_config_tf//:libtensorflow_framework",
        "@local_config_tf//:tf_header_lib",
        "@com_github_grpc_grpc//:grpc++",
        "@com_github_grpc_grpc//:grpc++_reflection",
        ":cpp_model_proto"
    ],
    copts = ["-pthread", "-std=c++11", "-D_GLIBCXX_USE_CXX11_ABI=0", "-DNDEBUG"]
)
