#include "tensorflow/core/framework/op.h"
#include "tensorflow/core/kernels/bounds_check.h"
#include "tensorflow/core/framework/shape_inference.h"
#include "tensorflow/core/framework/op_kernel.h"
#include "tensorflow/core/framework/resource_mgr.h"
#include "tensorflow/core/lib/core/refcount.h"

#include "model_enclave_grpc.h"

using namespace tensorflow;

REGISTER_OP("ModelLoadEnclave")
    .Input("model_name: string")
    .Input("model: string");

REGISTER_OP("ModelPredictEnclave")
    .Input("model_name: string")
    .Input("input: float")
    .Input("output_shape: shapeT")
    .Input("close_conn: bool")
    .Output("output: float")
    .Attr("shapeT: {int32, int64} = DT_INT32");

struct ClientResource : public ResourceBase {
  ModelClient client;

  std::string DebugString() {
    return std::string("i'm a client");
  }
};

std::string res_name("client");

class ModelLoadOp : public OpKernel {
public:
  explicit ModelLoadOp(OpKernelConstruction* context) : OpKernel(context) {}

  void Compute(OpKernelContext* context) override {
    const Tensor& model_name_tensor = context->input(0);
    const Tensor& model_tensor = context->input(1);

    auto res = new ClientResource;
    res->client = ModelClient(grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials()));
    auto resource_mgr = context->resource_manager();

    Status s = resource_mgr->Create(resource_mgr->default_container(), res_name, res);
    OP_REQUIRES_OK(context, s);

    auto model_name = model_name_tensor.flat<string>().data();
    auto model = model_tensor.flat<string>().data();

    ClientResource * res2 = nullptr;
    Status s2 = resource_mgr->Lookup(resource_mgr->default_container(), res_name, &res2);
    OP_REQUIRES_OK(context, s2);

    bool status = res2->client.GetModelLoad(*model_name, *model);
    OP_REQUIRES(context, status, errors::Internal("Issue with grpc connection"));
    res2->Unref();
  }
};

class ModelPredictOp : public OpKernel {
public:
  explicit ModelPredictOp(OpKernelConstruction* context) : OpKernel(context) {}

  void Compute(OpKernelContext* context) override {
    const Tensor& model_name_tensor = context->input(0);
    const Tensor& input_tensor = context->input(1);
    const Tensor& shape_tensor = context->input(2);
    const Tensor& close_tensor = context->input(3);
    TensorShape output_shape;
    OP_REQUIRES_OK(context, MakeShape(shape_tensor, &output_shape));

    auto resource_mgr = context->resource_manager();

    ClientResource * res = nullptr;
    Status s = resource_mgr->Lookup(resource_mgr->default_container(), res_name, &res);
    OP_REQUIRES_OK(context, s);

    // Allocate output
    Tensor* output;
    OP_REQUIRES_OK(context, context->allocate_output(0, output_shape, &output));
    auto model_name = model_name_tensor.flat<string>().data();
    auto close = close_tensor.flat<bool>().data();

    bool status = res->client.GetModelPredict(*model_name,
                                               input_tensor.flat<float>().data(),
                                               input_tensor.NumElements(),
                                               output->flat<float>().data(),
                                               output_shape.num_elements());
    OP_REQUIRES(context, status, errors::Internal("Issue with grpc connection"));

    // TODO for some reason if you unref res here the python script crashes when
    // the session exits, don't really understand why that is
  }
};

REGISTER_KERNEL_BUILDER(
  Name("ModelLoadEnclave")
  .Device(DEVICE_CPU),
  ModelLoadOp);

REGISTER_KERNEL_BUILDER(
  Name("ModelPredictEnclave")
  .Device(DEVICE_CPU),
  ModelPredictOp);
