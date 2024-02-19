#pragma once
#define UNICODE

#include <windows.h>
#include <shellapi.h>

#define WM_TRAY_CALLBACK_MESSAGE (WM_USER + 1)
#define WC_TRAY_CLASS_NAME L"TRAY"
#define ID_TRAY_FIRST 1000
#define ICON_PATH LR"(icon.ico)"

static TCHAR buttonQuitText[] = L"Quit";

static WNDCLASSEX wc;
static NOTIFYICONDATA nid;
static HWND hwnd;
static HMENU hmenu = NULL;

static void tray_exit() {
	Shell_NotifyIcon(NIM_DELETE, &nid);
	if (nid.hIcon != 0) {
		DestroyIcon(nid.hIcon);
	}
	if (hmenu != 0) {
		DestroyMenu(hmenu);
	}
	PostQuitMessage(0);
	UnregisterClass(WC_TRAY_CLASS_NAME, GetModuleHandle(NULL));
}

static LRESULT CALLBACK _tray_wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	switch (msg) {
	case WM_CLOSE:
		DestroyWindow(hwnd);
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_TRAY_CALLBACK_MESSAGE:
		if (lparam == WM_LBUTTONUP || lparam == WM_RBUTTONUP) {
			POINT p;
			GetCursorPos(&p);
			SetForegroundWindow(hwnd);
			WORD cmd = TrackPopupMenu(
				hmenu,
				TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD | TPM_NONOTIFY,
				p.x, p.y, 0, hwnd, NULL
			);
			if (cmd != 0) {
				SendMessage(hwnd, WM_COMMAND, cmd, 0);
			}
			return 0;
		}
		break;
	case WM_COMMAND:
	{
		UINT menuItemId = (UINT)wparam;
		if (menuItemId >= ID_TRAY_FIRST) {
			tray_exit();
			return 0;
		}
	}
	break;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

static HMENU _tray_menu(UINT* id) {
	HMENU hmenu = CreatePopupMenu();
	MENUITEMINFO item;
	memset(&item, 0, sizeof(item));
	item.cbSize = sizeof(MENUITEMINFO);
	item.fMask = MIIM_ID | MIIM_TYPE | MIIM_STATE | MIIM_DATA;
	item.fType = 0;
	item.fState = 0;
	item.wID = *id;
	item.dwTypeData = buttonQuitText;
	InsertMenuItem(hmenu, *id, TRUE, &item);
	return hmenu;
}

static void tray_update() {
	HMENU prevmenu = hmenu;
	UINT id = ID_TRAY_FIRST;
	hmenu = _tray_menu(&id);
	SendMessage(hwnd, WM_INITMENUPOPUP, (WPARAM)hmenu, 0);
	HICON icon;
	ExtractIconEx(ICON_PATH, 0, NULL, &icon, 1);
	if (nid.hIcon) {
		DestroyIcon(nid.hIcon);
	}
	nid.hIcon = icon;
	Shell_NotifyIcon(NIM_MODIFY, &nid);

	if (prevmenu != NULL) {
		DestroyMenu(prevmenu);
	}
}

static int tray_init() {
	memset(&wc, 0, sizeof(wc));
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.lpfnWndProc = _tray_wnd_proc;
	wc.hInstance = GetModuleHandle(NULL);
	wc.lpszClassName = WC_TRAY_CLASS_NAME;
	if (!RegisterClassEx(&wc)) {
		return -1;
	}

	hwnd = CreateWindowEx(0, WC_TRAY_CLASS_NAME, NULL, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL);
	if (hwnd == NULL) {
		return -1;
	}
	UpdateWindow(hwnd);

	memset(&nid, 0, sizeof(nid));
	nid.cbSize = sizeof(NOTIFYICONDATA);
	nid.hWnd = hwnd;
	nid.uID = 0;
	nid.uFlags = NIF_ICON | NIF_MESSAGE;
	nid.uCallbackMessage = WM_TRAY_CALLBACK_MESSAGE;
	Shell_NotifyIcon(NIM_ADD, &nid);

	tray_update();
	return 0;
}
