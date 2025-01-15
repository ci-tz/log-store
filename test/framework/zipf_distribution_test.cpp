#include "common/approx_zipf_distribution.h"
#include "gtest/gtest.h"

namespace logstore {

TEST(ApproxZipfDistribution, test) {
  // 参数设置
  int N = 1000;    // 最大支持值
  double s = 1.2;  // Zipf 分布参数（越大分布越陡峭）

  // 创建随机数生成器和 Zipf 分布生成器
  std::random_device rd;
  std::mt19937 gen(rd());
  ApproxZipfDistribution zipf(N, s);

  // 采样次数
  int sample_count = 100000;
  std::map<int, int> frequency;  // 用于统计每个值出现的频率

  // 生成样本
  for (int i = 0; i < sample_count; ++i) {
    int sample = zipf(gen);
    frequency[sample]++;
  }

  // 输出结果
  std::cout << "Value\tFrequency\n";
  for (const auto &entry : frequency) {
    std::cout << entry.first << "\t" << entry.second << "\n";
  }
}

};  // namespace logstore