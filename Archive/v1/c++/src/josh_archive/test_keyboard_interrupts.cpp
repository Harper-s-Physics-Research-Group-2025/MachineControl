#include <iostream>
#include <windows.h>

#pragma comment(lib, "user32.lib")

HHOOK g_keyboardHook;
volatile bool g_keyPressed = false;
volatile bool g_running = true;

LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        KBDLLHOOKSTRUCT* pKeyInfo = (KBDLLHOOKSTRUCT*)lParam;
        if (wParam == WM_KEYDOWN) {
            // Process the key press
            g_keyPressed = true;
            std::cout << "Key Pressed: " << pKeyInfo->vkCode << std::endl;

            // Check for 'Q' or 'q' (virtual key code for both is 81)
            if (pKeyInfo->vkCode == 'Q') {
                g_running = false;
                PostQuitMessage(0);  // Triggers WM_QUIT to break the message loop
            }
        
        } else if (wParam == WM_KEYUP) {
            // Process the key release
            g_keyPressed = false;
            std::cout << "Key Released: " << pKeyInfo->vkCode << std::endl;
        }
    }
    return CallNextHookEx(g_keyboardHook, nCode, wParam, lParam);
}

int main() {
    // Install the keyboard hook
    g_keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, GetModuleHandle(NULL), 0);
    if (g_keyboardHook == NULL) {
        std::cerr << "Failed to install keyboard hook." << std::endl;
        return 1;
    }

    // Keep the program running to receive keyboard inputs
    MSG msg;
    while (g_running && GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Unload the hook
    UnhookWindowsHookEx(g_keyboardHook);
    return 0;
}