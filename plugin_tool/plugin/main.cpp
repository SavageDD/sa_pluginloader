#define _CRT_SECURE_NO_WARNINGS // FUCK THIS SHIT
#include <Windows.h>
#include <iostream>
#include <thread>

std::thread* mainloop = nullptr;
class GameEntry {
public:
	static void Entry()
	{
		Sleep(100);
		std::cout << " Game has be plant" << std::endl;
	}
	void console() {
		AllocConsole();
		SetConsoleCP(1251);
		SetConsoleOutputCP(1251);
		freopen("CONIN$", "r", stdin);
		freopen("CONOUT$", "w", stdout);
		freopen("CONOUT$", "w", stderr);
	}
	GameEntry() {
		console();
		mainloop = new std::thread(Entry);
	}
};

GameEntry* entry = nullptr;

extern "C" __declspec(dllexport) void EntryPoint() {
	entry = new GameEntry();
}