#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <stdio.h>
#include <thread>
#include <string>
#include <iostream>
#include <vector>

void FreeProtection(DWORD addr, int size)
{
	DWORD d;
	VirtualProtect((PVOID)addr, size, PAGE_EXECUTE_READWRITE, &d);
}

static unsigned long CallHook(unsigned long from, unsigned long to)
{
	unsigned long origin = *(unsigned long*)(from + 1);
	FreeProtection(from, 5);
	unsigned long caddr = to - (from + 5);
	*(unsigned char*)(from) = 0xE8;
	*(unsigned long*)(from + 1) = caddr;

	return origin + (from + 5);
}
class InputParser {

	char* argv = nullptr;
	std::string ParseText() {
		std::string text;
		if (*argv == '"' && *(argv + 1) != '"')
			argv++;
		while (*argv && *argv != '"') {
			text.push_back(*argv);
			argv++;
		}
		argv++;
		return text;
	}
	std::string ParseArg() {
		std::string text;
		if (*argv == ' ')
			argv++;
		while (*argv && *argv != ' ') {
			text.push_back(*argv);
			argv++;
		}
		argv++;
		return text;
	}
public:
	InputParser(char *text) {
		argv = text;

		while (*argv) {
			if (*argv == '"') {
				std::string tx = ParseText();
				this->tokens.push_back(tx);
				continue;
			}
			else if (*argv != ' ') {
				std::string tx = ParseArg();
				this->tokens.push_back(tx);
				continue;
			}
			argv++;
		}

	}
	/// @author iain
	const std::string& getCmdOption(const std::string &option) const {
		std::vector<std::string>::const_iterator itr;
		itr = std::find(this->tokens.begin(), this->tokens.end(), option);
		if (itr != this->tokens.end() && ++itr != this->tokens.end()) {
			return *itr;
		}
		static const std::string empty_string("");
		return empty_string;
	}
	/// @author iain
	bool cmdOptionExists(const std::string &option) const {
		return std::find(this->tokens.begin(), this->tokens.end(), option)
			!= this->tokens.end();
	}
private:
	std::vector <std::string> tokens;
};

typedef void(*entry_)();
void LoadLibrarys(std::string path, std::vector<std::string> libs) {
	for (int i = 0; i < libs.size(); i++) {
		std::string p = path + libs[i];
		std::cout << "LoadLibrarys " << p << std::endl;
		HMODULE library = LoadLibrary(p.c_str());
		if (library) {
			entry_ entry = (entry_)GetProcAddress(library, "EntryPoint");
			if (entry) {
				entry();
			}
		}
	}
}

signed int __cdecl isAlreadyRunning() {
	AllocConsole();
	freopen("CONIN$", "r", stdin);
	freopen("CONOUT$", "w", stdout);
	freopen("CONOUT$", "w", stderr);

	PCHAR szCmdLine = GetCommandLineA();

	InputParser params(szCmdLine);
	std::string path = params.getCmdOption("-module");
	
	//Module list
	LoadLibrarys(path, std::vector<std::string>{
		"\\module\\plugin.dll"
	});
	return false;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		CallHook(0x74872D, (unsigned long)isAlreadyRunning);
		break;
	case DLL_THREAD_ATTACH:

		break;
	case DLL_THREAD_DETACH:

		break;
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}
