#include <windows.h>
/*
typedef struct {
  UINT      style;
  WNDPROC   lpfnWndProc;
  int       cbClsExtra;
  int       cbWndExtra;
  HINSTANCE hInstance;
  HICON     hIcon;
  HCURSOR   hCursor;
  HBRUSH    hbrBackground;
  LPCSTR    lpszMenuName;
  LPCSTR    lpszClassName;
} WNDCLASSA, *PWNDCLASSA, *NPWNDCLASSA, *LPWNDCLASSA;
*/

LRESULT Wndproc(
                HWND Window,
                UINT Message,
                WPARAM WParam,
                LPARAM LParam)
{
    LRESULT result = 0;

    switch (Message)
    {
        case WM_SIZE:
            OutputDebugStringA("WM_SIZE");

            break;

        case WM_DESTROY:
            OutputDebugStringA("WM_DESTROY");

            break;

        case WM_CLOSE:
            OutputDebugStringA("WM_CLOSE");

            break;

        case WM_ACTIVATEAPP:
            OutputDebugStringA("WM_ACTIVATEAPP");
        
            break;

        case WM_PAINT:
        {
            PAINTSTRUCT Paint;
            HDC DeviceContext = BeginPaint(Window, &Paint);
            int X = Paint.rcPaint.left; 
            int Y = Paint.rcPaint.top;
            int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;
            int Width = Paint.rcPaint.right - Paint.rcPaint.left;
            PatBlt(DeviceContext, X, Y, Width, Height, WHITENESS);
            SetPixel(DeviceContext, 100, 100, RGB(255, 0, 0));
            EndPaint(Window, &Paint);
        }    
            break;

        default:
            //OutputDebugStringA("WM_DEFAULT");
            result = DefWindowProc(Window, Message, WParam, LParam);
            break;
    }

    return result;

}

extern "C" int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pCmdLine, int nCmdShow)
{
    int x;
    x = 10;
    float y;
    y = 10.1;
    double z;
    z = x + y;

    WNDCLASS WindowClass = {};

    //WindowClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW; // Check other cs's
    WindowClass.lpfnWndProc = Wndproc;
    WindowClass.hInstance = hInstance;
    //WindowsClass.hIcon = ;
    WindowClass.lpszClassName =  (LPCSTR)"HandmadeHeroWindow";


    //MessageBox(
    //    NULL,
    //    (LPCWSTR)L"This is handmade Hero.",
    //    (LPCWSTR)L"Handmade Hero",
    //    MB_OK | MB_ICONINFORMATION
    //);
 
        
    if (RegisterClass(&WindowClass))
    {
        HWND WindowHandle = CreateWindowEx(0, WindowClass.lpszClassName, (LPCSTR)"Handmade Hero", WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT,
                                             CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, hInstance, 0);

        if (WindowHandle)
        {
            for (;;)
            {
                MSG Message;   
                BOOL MessageResult = (GetMessage(&Message, 0, 0 , 0));
                
                if (MessageResult > 0)
                {
                    TranslateMessage(&Message);
                    DispatchMessage(&Message);
                }
                else
                {
                    break;
                }
            }    

        }
        else
        {

        }


    }
    else
    {
        OutputDebugString((LPCSTR)"Windows register failed!");
    }

    return 0;
}