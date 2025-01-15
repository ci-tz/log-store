#pragma once

#include <iostream>
#include <memory>

#include "placement/dac.h"
#include "placement/no_placement.h"
#include "placement/placement.h"
#include "placement/sepbit.h"
#include "placement/sepgc.h"

namespace logstore {

class PlacementFactory {
 public:
  static std::shared_ptr<Placement> GetPlacement(const std::string &type) {
    if (type == "NoPlacement") {
      return std::make_shared<NoPlacement>();
    } else if (type == "SepGC") {
      return std::make_shared<SepGC>();
    } else if (type == "SepBIT") {
      return std::make_shared<SepBIT>();
    } else if (type == "DAC") {
      return std::make_shared<DAC>(Config::GetInstance().opened_segment_num);
    } else {
      std::cerr << "Unknown placement type: " << type << std::endl;
      return nullptr;
    }
  }
};

};  // namespace logstore