#ifndef _SGE_LOG_HPP_
#define _SGE_LOG_HPP_

#pragma once

#include <print>

#define SGE__LOG(f, level, text, ...) std::println(f, "[" level "]" " " text __VA_OPT__(,) ##__VA_ARGS__)

#define SGE_LOG_ERROR(text, ...) SGE__LOG(stderr, "ERROR", text, ##__VA_ARGS__); (void)fflush(stderr)
#define SGE_LOG_INFO(text, ...) SGE__LOG(stdout, "INFO", text, ##__VA_ARGS__); (void)fflush(stdout)

#if SGE_DEBUG
#define SGE_LOG_DEBUG(text, ...) SGE__LOG(stdout, "DEBUG", text, ##__VA_ARGS__); (void)fflush(stdout)
#else
#define SGE_LOG_DEBUG(text, ...) ((void)0)
#endif

#endif
