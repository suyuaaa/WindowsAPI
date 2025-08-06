#ifndef UNICODE
#define _UNICODE
#endif 

#include <windows.h>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <codecvt>
#include <functional>

class Window {
private:
    DWORD style_ = WS_OVERLAPPEDWINDOW;
    std::string window_name;
    bool draggable_ = false;
    HWND hwnd = nullptr;
    bool running = false;
    HICON hIcon_ = nullptr;
    int line = 0;
    int liney = 70;
    std::map<int, HWND> controls;
    HACCEL hAccelTable_ = nullptr;
    std::map<int, std::function<void()>> shortcutCallbacks_;
    int nextShortcutId_ = 1000;

    static std::wstring string_to_wide(const std::string& str) {
        if (str.empty()) return L"";
        int size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
        std::wstring result(size, 0);
        MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &result[0], size);
        return result;
    }

public:
    ~Window() {
        if (hAccelTable_) {
            DestroyAcceleratorTable(hAccelTable_);
        }
    }

    // 实例消息处理函数
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) {
        switch (uMsg) {
        case WM_ACTIVATE: {
            if (LOWORD(wParam) != WA_INACTIVE) {
                SetFocus(hwnd);
            }
            return 0;
        }
        case WM_KEYDOWN: {
            if (wParam == VK_ESCAPE) {
                std::cout << "ESC pressed, closing window." << std::endl;
                DestroyWindow(hwnd);
            }
            return 0;
        }
        case WM_DESTROY: {
            running = false;
            PostQuitMessage(0);
            return 0;
        }

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));
            EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_NCHITTEST: {
            if (!draggable_)
                break;

            LRESULT hit = DefWindowProc(hwnd, uMsg, wParam, lParam);
            return (hit == HTCLIENT) ? HTCAPTION : hit;
        }
        case WM_COMMAND: {
            int cmdId = LOWORD(wParam);
            if (shortcutCallbacks_.find(cmdId) != shortcutCallbacks_.end()) {
                shortcutCallbacks_[cmdId]();
                return 0;
            }
            break;
        }
        default:
            break;
        }
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    // 静态转发器
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        Window* pThis = nullptr;

        if (uMsg == WM_NCCREATE) {
            CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
            pThis = (Window*)pCreate->lpCreateParams;
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);
            pThis->hwnd = hwnd;
        }
        else {
            pThis = (Window*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
        }

        if (pThis) {
            return pThis->HandleMessage(uMsg, wParam, lParam);
        }

        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    void SetIcon(HICON icon) {
        hIcon_ = icon;
        if (hwnd) {
            SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon_);
        }
    }

    void SetStyle(DWORD style) { style_ = style; }

    void SetDraggable(bool draggable) {
        draggable_ = draggable;
    }

    int AddShortcutAuto(const std::string& shortcut, std::function<void()> callback) {
        BYTE modifiers = 0;
        WORD key = 0;

        // 解析快捷键字符串
        size_t pos = shortcut.find('+');
        if (pos != std::string::npos) {
            std::string modStr = shortcut.substr(0, pos);
            std::string keyStr = shortcut.substr(pos + 1);

            // 处理修饰键
            if (modStr.find("Ctrl") != std::string::npos ||
                modStr.find("Control") != std::string::npos) {
                modifiers |= FCONTROL;
            }
            if (modStr.find("Alt") != std::string::npos) {
                modifiers |= FALT;
            }
            if (modStr.find("Shift") != std::string::npos) {
                modifiers |= FSHIFT;
            }

            // 处理主键
            if (keyStr.size() == 1) {
                // 字母或数字键
                key = std::toupper(keyStr[0]);
            }
            else {
                // 功能键和特殊键
                static const std::map<std::string, WORD> keyMap = {
                    {"F1", VK_F1}, {"F2", VK_F2}, {"F3", VK_F3}, {"F4", VK_F4},
                    {"F5", VK_F5}, {"F6", VK_F6}, {"F7", VK_F7}, {"F8", VK_F8},
                    {"F9", VK_F9}, {"F10", VK_F10}, {"F11", VK_F11}, {"F12", VK_F12},
                    {"Esc", VK_ESCAPE}, {"Enter", VK_RETURN}, {"Space", VK_SPACE},
                    {"Tab", VK_TAB}, {"Backspace", VK_BACK}, {"Del", VK_DELETE},
                    {"Ins", VK_INSERT}, {"Home", VK_HOME}, {"End", VK_END},
                    {"PgUp", VK_PRIOR}, {"PgDn", VK_NEXT}
                };

                auto it = keyMap.find(keyStr);
                if (it != keyMap.end()) {
                    key = it->second;
                }
                else {
                    std::cerr << "未知按键或按键组合: " << keyStr << std::endl;
                    return -1;
                }
            }
        }
        else {
            // 单键处理（如功能键）
            if (shortcut.size() > 1) {
                // 处理功能键
                static const std::map<std::string, WORD> keyMap = {
                    {"F1", VK_F1}, {"F2", VK_F2} /* 同上 */
                };
                auto it = keyMap.find(shortcut);
                if (it != keyMap.end()) {
                    key = it->second;
                }
            }
            else {
                // 单字符键
                key = std::toupper(shortcut[0]);
            }
        }

        if (key == 0) {
            std::cerr << "Invalid shortcut format: " << shortcut << std::endl;
            return -1;
        }

        // 调用现有的快捷键添加方法
        return Add_short_cut(FVIRTKEY | modifiers, key, callback);
    }

    bool Create(const std::string& name, HINSTANCE hInstance, int width = CW_USEDEFAULT,
        int height = CW_USEDEFAULT, int x = CW_USEDEFAULT, int y = CW_USEDEFAULT) {
        window_name = name;

        // 注册窗口类
        std::wstring class_name = L"WindowClass_" + string_to_wide(name);
        WNDCLASS wc = { };
        wc.lpfnWndProc = WindowProc;
        wc.hInstance = hInstance;
        wc.lpszClassName = class_name.c_str();
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wc.hIcon = hIcon_ ? hIcon_ : LoadIcon(nullptr, IDI_APPLICATION);

        if (!RegisterClass(&wc)) {
            std::cerr << "Window class registration failed!\n";
            return false;
        }

        // 创建窗口实例
        std::wstring title = string_to_wide(name);
        hwnd = CreateWindowEx(
            0,
            class_name.c_str(),
            title.c_str(),
            style_,
            x, y, width, height,
            nullptr,
            nullptr,
            hInstance,
            this
        );

        return (hwnd != nullptr);
    }

    void Show(int nCmdShow = SW_SHOW) const {
        if (hwnd) {
            ShowWindow(hwnd, nCmdShow);
            UpdateWindow(hwnd);
        }
    }

    int Add_Label(std::string text = "", int spacing = 40, int x = -1, int y = -1) {
        line++;
        int ly;
        if (x < 0) {
            x = 50;
        }
        if (y >= 0) {
            ly = y;
        }
        else {
            ly = liney;
            liney += spacing;
        }
        std::cout << "添加标签: " << text << " at (" << x << ", " << ly << ")" << std::endl;

        HWND hNewStatic = CreateWindowEx(
            0,
            L"STATIC",
            string_to_wide(text).c_str(),
            WS_CHILD | WS_VISIBLE | SS_LEFT,
            x, ly, 200, 30,
            hwnd,
            (HMENU)(INT_PTR)line,
            GetModuleHandle(NULL),
            NULL
        );

        controls[line] = hNewStatic;
        SendMessage(hNewStatic, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);
        return line;
    }

    int Add_short_cut(BYTE modifiers, WORD key, std::function<void()> callback) {
        ACCEL accel = { 0 };
        accel.fVirt = FVIRTKEY | modifiers; // 使用Windows定义的FVIRTKEY
        accel.key = key;
        accel.cmd = nextShortcutId_;

        // 创建新的加速键表
        std::vector<ACCEL> accels;
        if (hAccelTable_) {
            int count = CopyAcceleratorTable(hAccelTable_, nullptr, 0);
            accels.resize(count);
            CopyAcceleratorTable(hAccelTable_, accels.data(), count);
            DestroyAcceleratorTable(hAccelTable_);
        }
        accels.push_back(accel);

        hAccelTable_ = CreateAcceleratorTable(accels.data(), static_cast<int>(accels.size()));
        if (!hAccelTable_) {
            std::cerr << "创建加速键表失败!" << std::endl;
            return -1;
        }

        // 存储回调函数
        shortcutCallbacks_[nextShortcutId_] = callback;
        return nextShortcutId_++;
    }

    void Run() {
        MSG msg;
        running = true;

        // 设置窗口焦点
        SetFocus(hwnd);

        while (running && GetMessage(&msg, nullptr, 0, 0)) {
            if (hAccelTable_ && TranslateAccelerator(hwnd, hAccelTable_, &msg)) {
                continue;
            }

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    HWND GetHandle() const { return hwnd; }
};