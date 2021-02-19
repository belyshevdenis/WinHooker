#pragma once
#include <mhook-lib/mhook.h>
#include <plog/Log.h>
#include <string>

template <typename FnType>
FnType Hook(const std::wstring& dllName, const std::string& fnName, FnType hook) {
	LOG_VERBOSE;

	HMODULE module = ::GetModuleHandle(dllName.c_str());
	if (module == NULL) {
		LOG_ERROR << " module " << dllName << "is not found in process";
		return nullptr;
	}

	auto procAddr = ::GetProcAddress(module, fnName.c_str());
	FnType original = reinterpret_cast<FnType>(procAddr);
	if (original == nullptr) {
		LOG_ERROR << "Function " << fnName << " is not found in process";
		return nullptr;
	}

	if (Mhook_SetHook(reinterpret_cast<PVOID*>(&original), hook) != TRUE) {
		LOG_ERROR << fnName << " is not hooked";
	}
	else {
		LOG_INFO << fnName << " is hooked";
	}

	return original;
}

void HookAll();