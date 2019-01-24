#ifndef TF_TRUSTED_MODEL_SERVER_H_
#define TF_TRUSTED_MODEL_SERVER_H_

#include <string>

#include "tf_trusted/proto/model_server.grpc.pb.h"
#include "include/grpcpp/grpcpp.h"
#include "include/grpcpp/server.h"

#include "tensorflow/contrib/lite/model.h"
#include "tensorflow/contrib/lite/interpreter.h"
#include "tensorflow/contrib/lite/op_resolver.h"
#include "tensorflow/contrib/lite/kernels/register.h"
#include "tensorflow/contrib/lite/stderr_reporter.h"

namespace tf_trusted {

class ModelRunner {
  public:
    std::unique_ptr<tflite::Interpreter> interpreter;

    ModelRunner(std::string model_bytes) {
      // Need to take ownership of this buffer.
      model_bytes_ = std::move(model_bytes);

      model_ = tflite::FlatBufferModel::BuildFromBuffer(model_bytes_.c_str(), model_bytes_.length(),
                                                       tflite::DefaultErrorReporter());
      tflite::ops::builtin::BuiltinOpResolver resolver;
      tflite::InterpreterBuilder(*model_, resolver)(&interpreter);

      // TODO figure out how to resize based on input shape
      interpreter->AllocateTensors();
    }

    void forward(const google::protobuf::RepeatedField<float> input);
    int64_t get_output_size();

    template <typename T>
    T * get_output();

    void DebugTensors() {
      auto num_tensors = interpreter->tensors_size();

      for(int i = 0; i < num_tensors; i++) {
        auto tensor = interpreter->tensor(i);
        std::cout << "TYPE: " << tensor->type << std::endl;
        std::cout << "SIZE: " << tensor->bytes << std::endl;

        if(tensor->type == 1) {
          auto weights_tensor = interpreter->typed_tensor<float>(i);
          for (int i = 0; i < tensor->bytes / sizeof(float); i++) {
            std::cout << weights_tensor[i] << " ";
          }
         std::cout << std::endl;
        }
      }
    }

private:
  std::unique_ptr<tflite::FlatBufferModel> model_;
  std::string model_bytes_;
};

class ModelServer final : public Model::Service {
 public:
  ModelServer();

  ~ModelServer() {
      delete runner;
  }

 private:
  ModelRunner * runner = nullptr;

  grpc::Status GetModelLoad(grpc::ServerContext *context,
                        const GetModelLoadRequest *query,
                        GetModelLoadResponse *response) override;

  grpc::Status GetModelPredict(grpc::ServerContext *context,
                        const GetModelPredictRequest *query,
                        GetModelPredictResponse *response) override;

};

}  // namespace tf_trusted

#endif // TF_TRUSTED_MODEL_SERVER_H_
