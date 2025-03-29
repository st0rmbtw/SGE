#ifndef _SGE_LOG_HPP_
#define _SGE_LOG_HPP_

#pragma once

#define SGE__LOG(level, text, ...) printf("[" level "]" " " text "\n" __VA_OPT__(,) ##__VA_ARGS__)

#if SGE_DEBUG
#define SGE_LOG_ERROR(text, ...) SGE__LOG("ERROR", text, ##__VA_ARGS__)
#define SGE_LOG_INFO(text, ...) SGE__LOG("INFO", text, ##__VA_ARGS__)
#define SGE_LOG_DEBUG(text, ...) SGE__LOG("DEBUG", text, ##__VA_ARGS__); fflush(stdout)
#else
#define SGE_LOG_ERROR(text, ...) SGE__LOG("ERROR", text, ##__VA_ARGS__)
#define SGE_LOG_INFO(text, ...) ((void)0)
#define SGE_LOG_DEBUG(text, ...) ((void)0)
#endif

#endif