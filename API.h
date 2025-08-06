#pragma once
#ifndef UNICODE
#define _UNICODE
#endif 

#include <windows.h>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <codecvt>
#include <functional>// 添加Window类声明，确保main.cpp能识别Window
class Window {
public:
    Window();
    bool Create(const char* title, HINSTANCE hInstance, int width, int height);
    void SetDraggable(bool draggable);
    int Add_Label(const char* text, int x = 0, int y = 0);
    void Add_short_cut(int modifiers, int key, std::function<void()> callback);
    void AddShortcutAuto(const char* shortcut, std::function<void()> callback);
    void Show(int nCmdShow);
    void Run();
};
