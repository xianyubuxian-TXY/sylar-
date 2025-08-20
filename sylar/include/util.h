#pragma once
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>

namespace sylar{
pid_t getThreadId();
uint32_t getFiberId();
};