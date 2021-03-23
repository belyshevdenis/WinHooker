#include <iostream>
#include <string>
#include <codecvt>
#include <vector>

#include <Windows.h>
#include <processthreadsapi.h>
#include <tlhelp32.h>

std::wstring a2w(const char* a) {
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	return converter.from_bytes(a);
}

DWORD sessionFromPid(DWORD pid) {
	DWORD sessionId = 0;
	if (!ProcessIdToSessionId(pid, &sessionId)) {
		std::cout << "ProcessIdToSessionId failed for PID " << pid << "; error code=0x" << std::hex << GetLastError() << "\n";
	}
	return sessionId;
}

// returns PIDs
void findProcesses(PCWSTR processName, std::vector<DWORD>& pids) {
	auto currentSession = sessionFromPid(::GetCurrentProcessId());
	std::cout << "Finding \"" << processName << "\" in session #" << currentSession << std::endl;

	::PROCESSENTRY32 entry;
	entry.dwSize = sizeof(::PROCESSENTRY32);

	// Make a snapshot with all running processes
	auto snapshot = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if (!snapshot) {
		throw std::runtime_error("CreateToolhelp32Snapshot failed");
	}

	// Enumerate all processes
	if (Process32First(snapshot, &entry) == TRUE) {
		while (Process32Next(snapshot, &entry) == TRUE) {
			auto targetSession = sessionFromPid(entry.th32ProcessID);
			if (targetSession == currentSession && _wcsicmp(entry.szExeFile, processName) == 0) {
				// Add the found process from the current session that matches the target name
				pids.push_back(entry.th32ProcessID);
			}
		}
	}
}

void inject(HANDLE process, const std::wstring& dllPath) {
	auto loadLibraryFn = reinterpret_cast<LPTHREAD_START_ROUTINE>(&LoadLibraryW);

	// Allocate memory on the target for injected DLL name
	const size_t remoteBufSize = (dllPath.size() + 1) * sizeof(wchar_t);
	LPVOID remoteBuf = ::VirtualAllocEx(process, nullptr, remoteBufSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	if (!remoteBuf) {
		throw std::runtime_error("VirtualAllocEx failed");
	}

	// Write the DLL name into the target memory
	if (!::WriteProcessMemory(process, remoteBuf, dllPath.c_str(), remoteBufSize, nullptr)) {
		throw std::runtime_error("WriteProcessMemory failed");
	}

	// Run LoadLibraryW function on the target
	auto remoteThread = ::CreateRemoteThread(process, nullptr, 0, loadLibraryFn, remoteBuf, 0, nullptr);
	if (!remoteThread) {
		throw std::runtime_error("CreateRemoteThread failed");
	}

	// Wait until LoadLibraryW exits for 3 seconds
	const DWORD waitingResult = ::WaitForSingleObject(remoteThread, 3000);
	if (waitingResult != WAIT_OBJECT_0) {
		::VirtualFreeEx(process, remoteBuf, 0, MEM_RELEASE);
		throw std::runtime_error("tired to wait for remote thread to finish");
	}

	// Deallocate memory on the target. Don't check the error because we can't do anything with it.
	::VirtualFreeEx(process, remoteBuf, 0, MEM_RELEASE);
}

int main(int argc, char** argv) {
	if (argc < 3) {
		std::cout << "Usage: injector.exe <dll_to_inject> <process_name|PID>\n\tNote: the tool finds all processes with <process_name> and injects <dll_to_inject> to ALL processes found.";
	}

	HANDLE process = NULL;
	try {
		std::vector<DWORD> pids;
		// Try to get the PID from 2nd parameter
		auto pid = atoi(argv[2]);
		if (pid != 0) {
			pids.push_back(pid);
		}
		else {
			// Try to find processes by name from the 2nd parameter
			findProcesses(a2w(argv[2]).c_str(), pids);
			if (pids.empty()) {
				throw std::runtime_error("Process is not found in current session");
			}
		}

		// iterate over all pids and inject DLL to all processes
		for (auto pid = pids.begin(); pid != pids.end(); ++pid) {
			const auto dllToInject = a2w(argv[1]);
			try {
				// Open target process
				process = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, *pid);
				if (!process) {
					throw std::runtime_error("Failed to open the process");
				}

				// Inject DLL from the 1st parameter to the target process
				inject(process, dllToInject);
			}
			catch (const std::exception& ex) {
				std::cout << "Failed to inject " << argv[1] << " to process with PID " << *pid << ": " << ex.what() << std::endl;
			}
			std::cout << "dll is injected successfully\n";
		}
	} catch (const std::exception& ex) {
		std::cout << "ERROR: " << ex.what();
	}

	if (process != NULL) {
		::CloseHandle(process);
	}

	system("pause");
	return 0;
}

