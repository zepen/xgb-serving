//
// Created by cher8-tech on 2023/6/1.
//
#include <iostream>
#include "estimator_service_impl.h"
#include "../util.h"

grpc::Status EstimatorServiceImpl::EstimatorResp(
        grpc::ServerContext* context,
        const estimate::server::EstimateRequest* request,
        estimate::server::EstimateResponse* response) {
    std::cout << "user_id:" << request->uf().userid() << std::endl;
    auto map_ptr = response->mutable_outputs();
    DMatrixHandle dmat = CreateInputMatrixFromMat(request->nf_size());
    std::vector<std::vector<float>> one_hot = GenerateOneHot(request->nf_size(), dmat);
    const float *pred = Predict(one_hot);
    if (pred != nullptr) {
        for (int i = 0; i < request->nf_size(); i++) {
            uint64_t noteId = request->nf().Get(i).noteid();
            (*map_ptr)[noteId] = pred[i];
        }
        std::cout << "outputs:" <<  response->outputs_size() << std::endl;
        XGDMatrixFree(dmat);
    }
    return grpc::Status::OK;
}

BoosterHandle EstimatorServiceImpl::LoadModel(const char *model_file) {
    BoosterHandle booster;
    XGBoosterCreate(nullptr, 0, &booster);
    XGBoosterLoadModel(booster, model_file);
    std::cout << "Load model success, from path: " << model_file << std::endl;
    return booster;
}

void EstimatorServiceImpl::GetInputFeaturesSize() {
    if (features_generator != nullptr) {
        XGBoosterGetNumFeature(features_generator, &input_features_size);
    }
}

void EstimatorServiceImpl::GetTreeNodeInfo() {
    bst_ulong out_len;
    if (features_generator != nullptr) {
        XGBoosterDumpModel(features_generator, "", 0, &out_len, &dump);
        for (bst_ulong i = 0; i < out_len; ++i) {
            size_t tree_leaf_count = leafCount(dump[i], "leaf");
            generate_features_size += tree_leaf_count;
            tree_node.push_back(tree_leaf_count);
        }
        std::cout << "generate features size = " << generate_features_size << std::endl;
        std::cout << "total tree size = " << tree_node.size() << std::endl;
    } else {

    }
}

DMatrixHandle EstimatorServiceImpl::CreateInputMatrixFromMat(int nRow) {
    DMatrixHandle dmat;
    float data[nRow][input_features_size];
    for (int i = 0; i < nRow; i++){
        for (int j = 0; j < input_features_size; j++){
            data[i][j] = float(binomal(1, 0.4));
        }
    }
    XGDMatrixCreateFromMat((float *) data, nRow, input_features_size, -1, &dmat);
    return dmat;
}

std::vector<std::vector<float>> EstimatorServiceImpl::GenerateOneHot(int nRow, DMatrixHandle dmat) {
    bst_ulong out_len;
    const float *out_result;
    std::vector<std::vector<float>> one_hot_features(nRow, std::vector<float>(generate_features_size, 0));
    if (features_generator != nullptr) {
        // 生成one-hot特征
        XGBoosterPredict(features_generator, dmat, 2, 0, false, &out_len, &out_result);
        for (int i = 0; i < nRow; i++) {
            size_t init_index = 0;
            for (int j = 0; j < out_len / nRow; j++) {
                auto leaf_index = static_cast<std::size_t>(std::round(out_result[j + i * (out_len / nRow)]));
                size_t tree_leaf_total = tree_node[j];
                size_t index = leaf_index - tree_leaf_total + 1;
                one_hot_features[i][index + init_index] = 1;
                init_index += tree_node[j];
            }
        }
    }
    return one_hot_features;
}

const float * EstimatorServiceImpl::Predict(std::vector<std::vector<float>> features) const {
    float data[features.size()][generate_features_size];
    for (int i = 0; i < features.size(); ++i) {
        for (int j = 0; j < features[i].size(); ++j) {
            data[i][j] = features[i][j];
        }
    }
    DMatrixHandle dmat_features;
    XGDMatrixCreateFromMat((float *)data, features.size(), generate_features_size, -1, &dmat_features);
    // lr预测
    bst_ulong out_len;
    const float *pred_result;
    if (predictor != nullptr) {
        XGBoosterPredict(predictor, dmat_features, 0, 0, false, &out_len, &pred_result);
        XGDMatrixFree(dmat_features);
        return pred_result;
    }
    return nullptr;
}
