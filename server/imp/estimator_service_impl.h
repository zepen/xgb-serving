//
// Created by cher8-tech on 2023/6/1.
//
#ifndef ESTIMATE_SERVER_ESTIMATOR_SERVICE_IMPL_H
#define ESTIMATE_SERVER_ESTIMATOR_SERVICE_IMPL_H

#include <vector>
#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>
#include <xgboost/c_api.h>
#include "estimate.grpc.pb.h"

class EstimatorServiceImpl final : public estimate::server::Estimator::Service {

    static BoosterHandle LoadModel(const char *model_file);
    grpc::Status EstimatorResp(
            grpc::ServerContext* context,
            const estimate::server::EstimateRequest* request,
            estimate::server::EstimateResponse* response) override;
    void GetTreeNodeInfo();
    void GetInputFeaturesSize();
    DMatrixHandle CreateInputMatrixFromMat(int nRow);
    std::vector<std::vector<float>> GenerateOneHot(int nRow, DMatrixHandle dmat);
    const float * Predict(std::vector<std::vector<float>>features) const;

public:
    EstimatorServiceImpl(const char *generator_file, const char *predictor_file){
        features_generator = LoadModel(generator_file);
        predictor = LoadModel(predictor_file);
        GetInputFeaturesSize();
        GetTreeNodeInfo();
    };
    BoosterHandle features_generator = nullptr;
    BoosterHandle predictor = nullptr;
private:
    const char** dump = nullptr;
    size_t generate_features_size = 0;
    bst_ulong input_features_size = 0;
    std::vector<size_t> tree_node;
};

#endif //ESTIMATE_SERVER_ESTIMATOR_SERVICE_IMPL_H
