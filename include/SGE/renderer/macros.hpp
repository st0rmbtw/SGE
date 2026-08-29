#pragma once

#ifndef SGE_RENDERER_MACROS_HPP_
#define SGE_RENDERER_MACROS_HPP_

#define SGE_RESOURCE_RELEASE(_PTR) if ((_PTR) != nullptr) { context->Release(*(_PTR)); (_PTR) = nullptr; }

#endif