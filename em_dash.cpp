/*
 * Low-level keyboard hook for Em-dash alt+gr shortcut in Windows
 * Copyright (c) 2025 Tiamo Laitakari
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <windows.h>
#include <iostream>


void SendEmDash() {
    wchar_t emDash = L'—';

    // INPUT structure for the em-dash key down event
    INPUT inputDown = { 0 };
    inputDown.type = INPUT_KEYBOARD;
    inputDown.ki.wScan = emDash; // Unicode character
    inputDown.ki.dwFlags = KEYEVENTF_UNICODE;

    // INPUT structure for the em-dash key up event
    INPUT inputUp = { 0 };
    inputUp.type = INPUT_KEYBOARD;
    inputUp.ki.wScan = emDash; // Unicode character
    inputUp.ki.dwFlags = KEYEVENTF_UNICODE | KEYEVENTF_KEYUP;

    // Send the key down and key up events
    INPUT inputs[2] = { inputDown, inputUp };
    UINT sent = SendInput(2, inputs, sizeof(INPUT));
    if (sent != 2) {
        std::cerr << "Failed to send em-dash input." << std::endl;
    }
}

// Low-level keyboard hook callback
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        KBDLLHOOKSTRUCT* pKB = (KBDLLHOOKSTRUCT*)lParam;

        switch (wParam) {
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:

            // Check if 'M' is pressed while Ctrl and Alt are held (Alt+GR)
            if (pKB->vkCode == 'M') {
                bool isCtrlPressed = (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0;
                bool isAltPressed = (GetAsyncKeyState(VK_MENU) & 0x8000) != 0;
                    if (isCtrlPressed && isAltPressed) {
                    // Send the em-dash
                    SendEmDash();

                    // Suppress the original keypress
                    return 1; // Non-zero to prevent further processing
                }
            }
            break;

        case WM_KEYUP:
        case WM_SYSKEYUP:
            break;
        }
    }

    // Call the next hook in the chain
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow) {
    // Set the low-level keyboard hook
    HHOOK hook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, NULL, 0);
    if (hook == NULL) {
        std::cerr << "Failed to install hook! Error: " << GetLastError() << std::endl;
        return 1;
    }

    // Message loop to keep the hook active
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Unhook the keyboard hook before exiting
    UnhookWindowsHookEx(hook);
    return 0;
}
