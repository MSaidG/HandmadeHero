#include <windows.h>

extern "C" int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    int x;
    x = 10;
    float y;
    y = 10.1;
    double z;
    z = x + y;

   MessageBox(
        NULL,
        (LPCWSTR)L"This is handmade Hero.",
        (LPCWSTR)L"Handmade Hero",
        MB_OK | MB_ICONINFORMATION
    );
        
    return 0;
}