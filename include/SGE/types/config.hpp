#ifndef _SGE_TYPES_CONFIG_HPP_
#define _SGE_TYPES_CONFIG_HPP_

#pragma once

#include "../defines.hpp"

_SGE_BEGIN

namespace types {

struct AppConfig {
    bool vsync = false;
    bool fullscreen = false;
};

}

_SGE_END

#endif