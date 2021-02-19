#include "stdafx.h"
#include "Hook.h"

std::string getLogDir() {
    char* buf = nullptr;
    size_t sz = 0;
    if (_dupenv_s(&buf, &sz, "LOG_DIR") == 0 && buf != nullptr)
    {
        std::string val(buf);
        free(buf);
        return val;
    }
    return "";
}

// Forms a string "<module_name>.log"
std::wstring getLogFileName() {
    std::wstring currentPath(MAX_PATH, '\0');
    DWORD len = ::GetModuleFileName(NULL, &currentPath.at(0), MAX_PATH);
    currentPath.resize(len);

    const size_t nameSeparator = currentPath.find_last_of(L"\\/");
    if (nameSeparator != std::wstring::npos && nameSeparator < currentPath.size() - 1)
    {
        return std::wstring(currentPath, nameSeparator + 1) + L".log";
    }
    return L"";
}

void initLog()
{
    static bool done = false;
    auto logDir = getLogDir();

    if (!logDir.empty() && !done) {
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
        auto logDirWide = converter.from_bytes(logDir.c_str());

        const std::wstring logFileName = getLogFileName();
        const std::wstring logPath = logDirWide + logFileName;
        static plog::RollingFileAppender<plog::TxtFormatter, plog::NativeEOLConverter<> > fileAppender(logPath.c_str(), 10 * 1024 * 1024, 5);
        plog::init(plog::verbose, &fileAppender);
        done = true;

        LOG_INFO << "Started";
    }
}

BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID reserved)
{
    if (reason == DLL_PROCESS_ATTACH)
    {
        initLog();
        HookAll();
    }
    return TRUE;
}