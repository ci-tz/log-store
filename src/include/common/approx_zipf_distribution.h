#include <cmath>
#include <random>

class ApproxZipfDistribution {
 public:
  ApproxZipfDistribution(int N, double s) : N(N), s(s) {
    // 预计算前部分的累积概率分布表
    partial_cdf.resize(N + 1, 0.0);
    for (int i = 1; i <= N; ++i) {
      partial_cdf[i] = partial_cdf[i - 1] + 1.0 / std::pow(i, s);
    }
    normalization_constant = partial_cdf[N];
    for (int i = 1; i <= N; ++i) {
      partial_cdf[i] /= normalization_constant;  // 归一化
    }
  }

  int operator()(std::mt19937 &gen) {
    std::uniform_real_distribution<> dis(0.0, 1.0);
    double U = dis(gen);

    // 使用二分查找在 CDF 中找到采样点
    int left = 1, right = N;
    while (left < right) {
      int mid = left + (right - left) / 2;
      if (partial_cdf[mid] >= U) {
        right = mid;
      } else {
        left = mid + 1;
      }
    }
    return left;
  }

 private:
  int N;                            // 分布支持范围
  double s;                         // Zipf 参数
  double normalization_constant;    // 归一化常数
  std::vector<double> partial_cdf;  // 部分累积概率分布表
};