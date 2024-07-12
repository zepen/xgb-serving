//
// Created by cher8-tech on 2023/6/2.
//
#include <regex>
#include <random>

size_t leafCount(const std::string& target_str, const std::string & search_str){
    size_t count = 0;
    size_t pos = 0;
    while ((pos = target_str.find(search_str, pos)) != std::string::npos) {
        ++count;
        pos += search_str.length();
    }
    return count;
}

int binomal(int n, float prob){
    std::random_device rd;
    std::mt19937 gen(rd());
    std::binomial_distribution<> dis(n, prob);
    return dis(gen);
}
