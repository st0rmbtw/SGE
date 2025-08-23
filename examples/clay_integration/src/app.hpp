#ifndef APP_HPP_
#define APP_HPP_

#pragma once

#include <SGE/types/backend.hpp>

#include "../../common.hpp"

namespace App {
    bool Init(const ExampleConfig& config);
    void Run();
    void Destroy();
};

#endif