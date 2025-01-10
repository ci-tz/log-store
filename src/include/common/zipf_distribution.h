#pragma once

#include <cmath>
#include <iostream>
#include <random>
#include <vector>

class ZipfDistribution {
 public:
  ZipfDistribution(int N, double s) : N(N), s(s) {
    // 预计算归一化常数
    normalization_constant = 0.0;
    for (int i = 1; i <= N; ++i) {
      normalization_constant += 1.0 / std::pow(i, s);
    }
  }

  int operator()(std::mt19937 &gen) {
    std::uniform_real_distribution<> dis(0.0, 1.0);
    double rand_val = dis(gen) * normalization_constant;

    double sum = 0.0;
    for (int i = 1; i <= N; ++i) {
      sum += 1.0 / std::pow(i, s);
      if (sum >= rand_val) {
        return i;
      }
    }
    return N;
  }

 private:
  int N;
  double s;
  double normalization_constant;
};