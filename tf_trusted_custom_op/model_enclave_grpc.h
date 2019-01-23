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

GetModelLoadRequest MakeModelLoadRequest(std::string model_name, std::string model_bytes) {
    GetModelLoadRequest req;

    req.set_model_name(model_name);
    req.set_model(model_bytes);

    return req;
}

GetModelPredictRequest MakeModelPredictRequest(std::string model_name, const float * inputs, int size) {
    GetModelPredictRequest req;

    google::protobuf::RepeatedField<float> data(inputs, inputs + size);
    req.mutable_input()->Swap(&data);

    req.set_model_name(model_name);

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


    bool GetModelPredict(std::string model_name, const float * input, int input_size,
                  float * output, int output_size) {
        auto req = MakeModelPredictRequest(model_name, input, input_size);
        GetModelPredictResponse res;

        bool status = GetOneModelPredict(req, &res);
        if(!status) {
            return status;
        }

        if(res.result_size() > output_size) {
            std::cout << "Output size doesn't match the returned size!" << std::endl;
            return false;
        }

        std::copy(res.result().begin(), res.result().end(), output);

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
