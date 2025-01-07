#pragma once

#include <iostream>
#include <memory>

#include "placement/no_placement.h"
#include "placement/placement.h"
#include "placement/sepgc.h"

namespace logstore {

class PlacementFactory {
 public:
  static std::shared_ptr<Placement> GetPlacement(const std::string &type) {
    if (type == "NoPlacement") {
      return std::make_shared<NoPlacement>();
    } else if (type == "SepGC") {
      return std::make_shared<SepGC>();
    } else {
      std::cerr << "Unknown placement type: " << type << std::endl;
      return nullptr;
    }
  }
};

};  // namespace logstore