/*
 * IOSNativeHandle.h
 * 
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_IOS_NATIVE_HANDLE_H
#define LLGL_IOS_NATIVE_HANDLE_H


#include <UIKit/UIKit.h>


namespace LLGL
{


//! iOS native handle structure.
struct NativeHandle
{
    UIView* view;
};


} // /namespace LLGL


#endif



// ================================================================================
