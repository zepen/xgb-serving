//
// Created by cher8-tech on 2023/5/19.
//
#include <xgboost/c_api.h>
#include <iostream>
#include <regex>
#include <random>
#include "../server/util.h"


int main(int argc, char** argv) {

    // 加载模型
    BoosterHandle gbdt;
    XGBoosterCreate(nullptr, 0, &gbdt);
    XGBoosterLoadModel(gbdt, "/Users/cher8-tech/PycharmProjects/offline_task/bst.model");
    std::cout << "bst.model loaded!" << std::endl;

    BoosterHandle lr;
    XGBoosterCreate(nullptr, 0, &lr);
    XGBoosterLoadModel(lr, "/Users/cher8-tech/PycharmProjects/offline_task/lr.model");
    std::cout << "lr.model loaded!" << std::endl;

    DMatrixHandle dmat;
    int nrow = 100;
    int ncol = 100;
    float data[nrow][ncol];
    for (int i = 0; i < nrow; i++){
        for (int j = 0; j < ncol; j++){
            if (i == 0) {
                data[i][j] = 1;
            } else {
                data[i][j] = 0;
            }
        }
    }

    for (int i = 0; i < nrow; i++){
        for (int j = 0; j < ncol; j++){
            std::cout << data[i][j] << ";";
        }
        std::cout << std::endl;
    }

    XGDMatrixCreateFromMat((float *) data, nrow, ncol, -1, &dmat);

    bst_ulong out_len;
    const float *out_result;
    XGBoosterPredict(gbdt, dmat, 2, 0, false, &out_len, &out_result);

    for (int i = 0; i < nrow; i++){
        for (int j = 0; j < out_len / nrow; j++){
            std::cout << j + i * (out_len / nrow) << "," << out_result[j + i * (out_len / nrow)] << ";";
        }
        std::cout << std::endl;
    }
    return 0;
}
