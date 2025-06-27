#ifndef _SGE_LOG_HPP_
#define _SGE_LOG_HPP_

#pragma once

#include <fmt/core.h>
#include <fmt/ostream.h>

#define SGE__LOG(f, level, text, ...) fmt::println(f, "[" level "]" " " text __VA_OPT__(,) ##__VA_ARGS__)

#define SGE_LOG_ERROR(text, ...) SGE__LOG(stderr, "ERROR", text, ##__VA_ARGS__); fflush(stderr)

#if SGE_DEBUG
#define SGE_LOG_INFO(text, ...) SGE__LOG(stdout, "INFO", text, ##__VA_ARGS__)
#define SGE_LOG_DEBUG(text, ...) SGE__LOG(stdout, "DEBUG", text, ##__VA_ARGS__); fflush(stdout)
#else
#define SGE_LOG_INFO(text, ...) ((void)0)
#define SGE_LOG_DEBUG(text, ...) ((void)0)
#endif

#endif