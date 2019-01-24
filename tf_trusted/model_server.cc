
#include <iostream>

#include <google/protobuf/repeated_field.h>

#include "tf_trusted/model_server.h"

#include "include/grpcpp/grpcpp.h"

namespace tf_trusted {

int64_t NumElements(TfLiteTensor * t) {
  auto dims = t->dims->size;
  int64_t count = 1;
  for(int i = 0; i < dims; i++) {
    count *= t->dims->data[i];
  }

  return count;
}

void ModelRunner::forward(const google::protobuf::RepeatedField<float> input) {
    float* input_tensor = interpreter->typed_input_tensor<float>(0);

    std::copy(input.begin(), input.end(), input_tensor);

    interpreter->Invoke();
}

int64_t ModelRunner::get_output_size() {
    return NumElements(interpreter->tensor(interpreter->outputs()[0]));
}

template <typename T>
T * ModelRunner::get_output() {
    return interpreter->typed_output_tensor<T>(0);
}

ModelServer::ModelServer()
    : Service() {}

grpc::Status ModelServer::GetModelLoad(grpc::ServerContext *context,
                                       const GetModelLoadRequest *req,
                                       GetModelLoadResponse *res) {

  if(runner == nullptr) {
    runner = new ModelRunner(req->model());
  }

  return grpc::Status::OK;
}

grpc::Status ModelServer::GetModelPredict(grpc::ServerContext *context,
                                          const GetModelPredictRequest *req,
                                          GetModelPredictResponse *res) {
  runner->forward(req->input());

  int size = runner->get_output_size();

  switch(req->return_type()) {
    case ReturnType::FLOAT:
    {
        float * output = runner->get_output<float>();
        google::protobuf::RepeatedField<float> data(output, output + size);
        res->mutable_float_result()->Swap(&data);
    }
    break;
    case ReturnType::DOUBLE:
    {
        double * output = runner->get_output<double>();
        google::protobuf::RepeatedField<double> data(output, output + size);
        res->mutable_double_result()->Swap(&data);
    }
    break;
    case ReturnType::INT32:
    {
        int32_t * output = runner->get_output<int32_t>();
        google::protobuf::RepeatedField<int32_t> data(output, output + size);
        res->mutable_int32_result()->Swap(&data);
    }
    break;
    case ReturnType::INT64:
    {
        int64_t * output = runner->get_output<int64_t>();
        google::protobuf::RepeatedField<int64_t> data(output, output + size);
        res->mutable_int64_result()->Swap(&data);
    }
    break;
  }

  return grpc::Status::OK;
}

}  // namespace tf_trusted
