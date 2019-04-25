#include "tensorflow/core/framework/op.h"
#include "tensorflow/core/kernels/bounds_check.h"
#include "tensorflow/core/framework/shape_inference.h"
#include "tensorflow/core/framework/op_kernel.h"
#include "tensorflow/core/framework/resource_mgr.h"
#include "tensorflow/core/lib/core/refcount.h"

#include "grpc/grpc.h"
#include "grpcpp/channel.h"
#include "grpcpp/client_context.h"
#include "grpcpp/create_channel.h"
#include "grpcpp/security/credentials.h"

#include "proto/model_server.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientWriter;

using tf_trusted::Model;
using tf_trusted::GetModelLoadRequest;
using tf_trusted::GetModelLoadResponse;
using tf_trusted::GetModelPredictRequest;
using tf_trusted::GetModelPredictResponse;
using tf_trusted::ReturnType;
using tensorflow::int32;
using tensorflow::int64;

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
constexpr ReturnType typeToReturnType<int32>() {
    return ReturnType::INT32;
}

template <>
constexpr ReturnType typeToReturnType<int64>() {
    return ReturnType::INT64;
}

GetModelLoadRequest MakeModelLoadRequest(std::string model_name, std::string model_bytes) {
    GetModelLoadRequest req;

    req.set_model_name(model_name);
    req.set_model(model_bytes);

    return req;
}

GetModelPredictRequest MakeModelPredictRequest(std::string model_name,
                                              int size,
                                              ReturnType return_type) {
    GetModelPredictRequest req;

    req.set_model_name(model_name);
    req.set_return_type(return_type);
    req.set_total_size(size);

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

        auto req = MakeModelPredictRequest(model_name, input_size, return_type);
        GetModelPredictResponse res;

        bool status = StreamModelPredict(req, &res, input);
        if(!status) {
            return status;
        }

        switch(return_type) {
            case ReturnType::FLOAT:
                res.mutable_float_result()->ExtractSubrange(0, res.float_result_size(), (float*)output);
                break;
            case ReturnType::DOUBLE:
                res.mutable_double_result()->ExtractSubrange(0, res.float_result_size(), (double*)output);
                break;
            case ReturnType::INT32:
                res.mutable_int32_result()->ExtractSubrange(0, res.float_result_size(), (int32*)output);
                break;
            case ReturnType::INT64:
                res.mutable_int64_result()->ExtractSubrange(0, res.float_result_size(), (int64*)output);
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
            std::cout << "GetModelLoad rpc failed. " << std::endl;
            std::cout << status.error_message() << std::endl;
            return false;
        }

        return true;
    }

    bool StreamModelPredict(GetModelPredictRequest req, GetModelPredictResponse* res, const float * inputs) {
        ClientContext context;

        std::unique_ptr<ClientWriter<GetModelPredictRequest>> writer(stub_->GetModelPredict(&context, res));

        int64 size = 65536 / sizeof(float); // 4 kib, optimal size
        for (int i = 0; i < req.total_size(); i += size) {
            if(i + size > req.total_size()) {
                google::protobuf::RepeatedField<float> data(inputs + i, inputs + req.total_size());
                req.mutable_input()->Swap(&data);
            } else {
                google::protobuf::RepeatedField<float> data(inputs + i, inputs + i + size);
                req.mutable_input()->Swap(&data);
            }

            if (!writer->Write(req)) {
                // Broken stream.
                break;
            }
        }

        writer->WritesDone();
        grpc::Status status = writer->Finish();
        if (status.ok()) {
            return true;
        } else {
            return false;
        }
    }

    std::unique_ptr<Model::Stub> stub_;
};
