#pragma once
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <assert.h>

#ifndef _MSC_VER
#    include <sys/time.h>
#endif // !_MSC_VER

#include <string>
#include <mutex>
#include <atomic>
#include <memory>
#include <thread>
#include <condition_variable>

#include "macros.h"
#include "platform_compatibility.h"
#include "ezlog.h"