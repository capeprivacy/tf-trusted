#include "grpc/grpc.h"
#include "grpcpp/channel.h"
#include "grpcpp/client_context.h"
#include "grpcpp/create_channel.h"
#include "grpcpp/security/credentials.h"

#include "proto/model_server.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;

using tf_trusted::Model;
using tf_trusted::GetModelLoadRequest;
using tf_trusted::GetModelLoadResponse;
using tf_trusted::GetModelPredictRequest;
using tf_trusted::GetModelPredictResponse;
using tf_trusted::ReturnType;

template <class T>
constexpr ReturnType typeToReturnType() {
    return ReturnType::NO_TYPE;
}

template <>
constexpr ReturnType typeToReturnType<float>() {
    return ReturnType::FLOAT;
}

template <>
constexpr ReturnType typeToReturnType<double>() {
    return ReturnType::DOUBLE;

}

template <>
constexpr ReturnType typeToReturnType<int32_t>() {
    return ReturnType::INT32;
}

template <>
constexpr ReturnType typeToReturnType<int64_t>() {
    return ReturnType::INT64;
}

GetModelLoadRequest MakeModelLoadRequest(std::string model_name, std::string model_bytes) {
    GetModelLoadRequest req;

    req.set_model_name(model_name);
    req.set_model(model_bytes);

    return req;
}

GetModelPredictRequest MakeModelPredictRequest(std::string model_name,
                                              const float * inputs,
                                              int size,
                                              ReturnType return_type) {
    GetModelPredictRequest req;

    google::protobuf::RepeatedField<float> data(inputs, inputs + size);
    req.mutable_input()->Swap(&data);

    req.set_model_name(model_name);
    req.set_return_type(return_type);

    return req;
}

class ModelClient {
public:
    ModelClient() {}
    ModelClient(std::shared_ptr<Channel> channel)
         : stub_(Model::NewStub(channel)) {
    }

    bool GetModelLoad(std::string model_name, std::string model_bytes) {
        auto req = MakeModelLoadRequest(model_name, model_bytes);
        GetModelLoadResponse res;

        return GetOneModelLoad(req, &res);
    }

    template <typename T>
    bool GetModelPredict(std::string model_name, const float * input, int input_size,
                  T * output, int output_size) {
        auto return_type = typeToReturnType<T>();

        auto req = MakeModelPredictRequest(model_name, input, input_size, return_type);
        GetModelPredictResponse res;

        bool status = GetOneModelPredict(req, &res);
        if(!status) {
            return status;
        }

        switch(return_type) {
            case ReturnType::FLOAT:
                std::copy(res.float_result().begin(), res.float_result().end(), output);
                break;
            case ReturnType::DOUBLE:
                std::copy(res.double_result().begin(), res.double_result().end(), output);
                break;
            case ReturnType::INT32:
                std::copy(res.int32_result().begin(), res.int32_result().end(), output);
                break;
            case ReturnType::INT64:
                std::copy(res.int64_result().begin(), res.int64_result().end(), output);
                break;
            default:
                std::cout << "NO TYPE!" << std::endl;
                return false;
        }

        return true;
    }

private:
    bool GetOneModelLoad(const GetModelLoadRequest& req, GetModelLoadResponse* res) {
        ClientContext context;
        grpc::Status status = stub_->GetModelLoad(&context, req, res);
        if (!status.ok()) {
            std::cout << "GetModelLoad rpc failed." << std::endl;
            return false;
        }

        return true;
    }

    bool GetOneModelPredict(const GetModelPredictRequest& req, GetModelPredictResponse* res) {
        ClientContext context;
        grpc::Status status = stub_->GetModelPredict(&context, req, res);
        if (!status.ok()) {
            std::cout << "GetModelPredict rpc failed." << std::endl;
            return false;
        }

        return true;
    }

    std::unique_ptr<Model::Stub> stub_;
};
