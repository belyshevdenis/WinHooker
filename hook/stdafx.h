#pragma once
#include <windows.h>

#include <string>
#include <codecvt>

#include <mhook-lib/mhook.h>
#include <plog/Init.h>
#include <plog/Log.h>
#include <plog/Converters/NativeEOLConverter.h>
#include <plog/Appenders/RollingFileAppender.h>
#include <plog/Formatters/TxtFormatter.h>