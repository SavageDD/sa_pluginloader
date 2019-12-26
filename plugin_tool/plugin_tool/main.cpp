#include <windows.h>
#include "json.hpp"
#include <iostream>
#include <fstream>
#include <iomanip>

// for convenience
using json = nlohmann::json;

std::string game_sa = "C:\\Games\\gamesa\\";

std::string ExePath() {
	char buffer[MAX_PATH];
	GetModuleFileName(NULL, buffer, MAX_PATH);
	std::string::size_type pos = std::string(buffer).find_last_of("\\/");
	return std::string(buffer).substr(0, pos);
}

int Inject() {
	// Prepare to create a new process.
	PROCESS_INFORMATION ProcessInfo;
	STARTUPINFO StartupInfo;

	std::string path = ExePath();

	std::string cmd = "-module \"" + path + "\"";
	memset(&ProcessInfo, 0, sizeof(PROCESS_INFORMATION));
	memset(&StartupInfo, 0, sizeof(STARTUPINFO));
	if (CreateProcessA((game_sa + "gta_sa.exe").c_str(), (char*)cmd.c_str(), NULL, NULL, FALSE, DETACHED_PROCESS | CREATE_SUSPENDED, NULL, game_sa.c_str(), &StartupInfo, &ProcessInfo))
	{
		path += "\\module\\sainjector.dll";
		std::cout << path.c_str() << std::endl;
		HMODULE hMod = GetModuleHandle("kernel32.dll");

		void* addr = NULL;
		if (hMod) {
			addr = (void*)GetProcAddress(hMod, "LoadLibraryA");
		}
		else
		{
			MessageBoxA(NULL, "Could not find kernel32.dll", "[ERROR]", MB_ICONERROR);
			return 0;
		}

		void* arg = (void*)VirtualAllocEx(ProcessInfo.hProcess, NULL, path.length(), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
		if (arg != NULL)
		{
			WriteProcessMemory(ProcessInfo.hProcess, arg, path.c_str(), path.length(), NULL);
		}
		else
		{
			MessageBoxA(NULL, "Memory could not be allocated/", "[ERROR]", MB_ICONERROR);
			return 0;
		}

		HANDLE id = NULL;
		if (addr != NULL)
		{
			id = CreateRemoteThread(ProcessInfo.hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)addr, arg, CREATE_SUSPENDED, NULL);
		}
		else
		{
			MessageBoxA(NULL, "Could not find the address of LoadLibraryA", "[ERROR]", MB_ICONERROR);
			return 0;
		}

		if (id)
		{
			ResumeThread(id);
			WaitForSingleObject(id, INFINITE);
		}
		else
		{
			MessageBoxA(NULL, "the ID returned from CreateRemoteThread was invalid.", "[ERROR]", MB_ICONERROR);
			return 0;
		}

		VirtualFreeEx(ProcessInfo.hProcess, arg, 0, MEM_RELEASE);
		ResumeThread(ProcessInfo.hThread);
		CloseHandle(ProcessInfo.hProcess);
	}
	else {
		std::cout << "Error CreateProcess" << std::endl;
	}
	return 0;
}

int main(int argc, char* argv[])
{
	try {
		json j;
		std::ifstream i("config.json");
		if (i.good()) {
			i >> j;
			i.close();
		}
		else {
			std::ofstream o("config.json");
			j = {
			  {"gta_path", "C:\\Games\\gamesa\\"}
			};
			o << std::setw(4) << j << std::endl;
			o.close();
		}
		game_sa = j["gta_path"].get<std::string>();
		std::cout << game_sa << std::endl;
	}
	catch (...) {
		std::cout << "Json error" << std::endl;
	}
	return Inject();
}