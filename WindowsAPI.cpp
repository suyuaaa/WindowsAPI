#include "API.h"

int main() {
    HINSTANCE hInstance = GetModuleHandle(nullptr);

    Window mainWindow;
    mainWindow.SetDraggable(true);
    if (mainWindow.Create("Win32封装示例", hInstance, 400, 300)) {
        int label = mainWindow.Add_Label("这是一个动态添加的标签");
        label = mainWindow.Add_Label("这是一个动态添加的标签");
        label = mainWindow.Add_Label("这是wwwwww签", 100, 100);
        label = mainWindow.Add_Label("这是一个动态添加的标签");
        label = mainWindow.Add_Label("这是一个动态添加的标签");

        // 使用Windows定义的修饰符常量
        mainWindow.Add_short_cut(FVIRTKEY | FCONTROL, 'S', []() {
            std::cout << "Ctrl+S 被按下 - 执行保存操作" << std::endl;
            });

        mainWindow.Add_short_cut(FVIRTKEY | FCONTROL | FSHIFT, 'O', []() {
            std::cout << "Ctrl+Shift+O 被按下 - 打开文件" << std::endl;
            });

        mainWindow.Add_short_cut(FVIRTKEY | FALT, VK_F4, []() {
            std::cout << "Alt+F4 被按下 - 关闭窗口" << std::endl;
            PostQuitMessage(0);
            });
        mainWindow.AddShortcutAuto("Ctrl+f", []() {
            std::cout << "自动解析: Ctrl+f 被按下 - 执行爆炸操作" << std::endl;
			});
        mainWindow.Show(SW_SHOW);
        mainWindow.Run();
    }
    else {
        std::cerr << "窗口创建失败!" << std::endl;
    }

    return 0;
}