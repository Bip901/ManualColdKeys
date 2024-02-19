#ifndef UNICODE
#define UNICODE
#endif
#include <Windows.h>
#include <iostream>
#include "Tray.cpp"

struct HotKey {
	int virtualKey; //Get key codes from https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes
	int modifiers[8]; // e.g. VK_SHIFT, VK_CONTROL, VK_MENU, VK_LWIN...
	LPWSTR fileToLaunch; //e.g. foo.exe or bar.lnk
};


static LPWSTR CopyToHeap(const TCHAR* str)
{
	size_t sizeInWordsWithNullByte = wcslen(str) + 1;
	LPWSTR result = (LPWSTR)malloc(sizeInWordsWithNullByte * sizeof(TCHAR));
	if (result != NULL)
	{
		wcscpy_s(result, sizeInWordsWithNullByte, str);
	}
	return result;
}

#define HEAP_LPWSTR(text) CopyToHeap(TEXT(text))

// =======================
// = DEFINE HOTKEYS HERE =
// =======================
// Examples:
static HotKey hotkeys [] = {
	HotKey {
		0x54, // "T" Key
		{ VK_CONTROL, VK_MENU },
		HEAP_LPWSTR(R"(C:\ProgramData\Microsoft\Windows\Start Menu\Programs\Windows Terminal\Terminal.lnk)")
	},
	HotKey {
		0x43, // "C" Key
		{ VK_LWIN },
		HEAP_LPWSTR(R"(C:\Program Files (x86)\Google\Chrome\Application\chrome.exe)")
	},
};

#define WM_HOTKEY_ACTIVATED_0 (WM_USER + 64)

static LRESULT CALLBACK HookProc(int code, WPARAM wParam, LPARAM lParam)
{
	if (code >= 0)
	{
		if (wParam == WM_KEYDOWN)
		{
			DWORD virtualKeyCode = ((KBDLLHOOKSTRUCT*)lParam)->vkCode;
			for (size_t i = 0; i < sizeof(hotkeys) / sizeof(HotKey); i++)
			{
				HotKey hotkey = hotkeys[i];
				if (hotkey.virtualKey == virtualKeyCode)
				{
					bool modifiers_matching = true;
					for (size_t i = 0; i < sizeof(hotkey.modifiers) / sizeof(int); i++)
					{
						int modifier = hotkey.modifiers[i];
						if (modifier == 0) break;
						SHORT keyState = GetKeyState(modifier);
						if (((keyState >> 15) & 0x1) != 1)
						{
							modifiers_matching = false;
							break;
						}
					}
					if (modifiers_matching && PostMessage(NULL, WM_USER - i, 0, 0) != 0)
					{
						return 1; //Success; Block key event from other processes
					}
				}
			}
			return 0;
		}
	}
	return CallNextHookEx(NULL, code, wParam, lParam);
}

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd)
{
	SetWindowsHookEx(WH_KEYBOARD_LL, HookProc, hInstance, 0);
	tray_init();
	MSG msg = { };
	while (GetMessage(&msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		if (msg.message >= WM_HOTKEY_ACTIVATED_0 && msg.message < WM_APP)
		{
			int index = msg.message - WM_HOTKEY_ACTIVATED_0;
			if (index >= 0 && index < sizeof(hotkeys) / sizeof(HotKey))
			{
				HotKey hotkey = hotkeys[index];
				ShellExecute(NULL, L"open", hotkey.fileToLaunch, NULL, NULL, SW_SHOWDEFAULT);
				// Alternative way without a shell (no .lnk file suppoprt though):
				/*
				STARTUPINFO si = {};
				si.cb = sizeof(si);
				PROCESS_INFORMATION pi = {};
				if (CreateProcess(NULL, hotkey.commandline, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
				{
					CloseHandle(pi.hProcess);
					CloseHandle(pi.hThread);
				}*/
			}
		}
	}
	return 0;
}
