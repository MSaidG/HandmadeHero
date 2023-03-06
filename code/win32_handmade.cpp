#include <windows.h>
#include <stdint.h>

#define internal static 
#define local_persist static 
#define global_variable static 


// NOTE: This is a global for now!
global_variable bool isRunning;
global_variable BITMAPINFO BitmapInfo;
global_variable void *BitmapMemory;
global_variable int BitmapHeight;  
global_variable int BitmapWidth;
global_variable int BytesPerPixel = 4;


internal void RenderWeirdGradient(int xOffset, int yOffset)
{

    int Width = BitmapWidth;     
    int Height = BitmapHeight;
    
    int Pitch = Width * BytesPerPixel;
    uint8_t *Row  = (uint8_t *)BitmapMemory;

    for (int y = 0; y < BitmapHeight; y++)
    {
        uint32_t *Pixel = (uint32_t *) Row;
        for (int x = 0; x < BitmapWidth; x++)
        {
            uint8_t Blue = (x + xOffset);
            uint8_t Green = (y + yOffset);

            /*
                Memory: BB GG RR XX
                Register: xx RR GG BB

                Pixel (32-bits)
            */

            *Pixel++ = ((Green << 8) | Blue);
            //*Row++ = ((Green << 8) | Blue);

        }

        //Row = (uint8_t *) Pixel;
        Row += Pitch;
    }
}


internal void ResizeDIBSection(int Width, int Height)
{

    // TODO: bullet proof this.

    if (BitmapMemory)
    {
        VirtualFree(BitmapMemory, 0, MEM_RELEASE);
    }

    BitmapHeight = Height;
    BitmapWidth = Width;

    BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader);
    BitmapInfo.bmiHeader.biWidth = BitmapWidth;
    BitmapInfo.bmiHeader.biHeight = -BitmapHeight;
    BitmapInfo.bmiHeader.biPlanes = 1;
    BitmapInfo.bmiHeader.biBitCount = 32; 
    BitmapInfo.bmiHeader.biCompression = BI_RGB;
    
    // NOTE: May just allocate this ourselves?
    //BitmapHandle = CreateDIBSection(BitmapDeviceContext, &BitmapInfo, DIB_RGB_COLORS, &BitmapMemory, 0, 0);

    
    int BitmapMemorySize = (BitmapWidth * BitmapHeight) * BytesPerPixel;
    BitmapMemory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);

    // NOTE: Clear to black

}

internal void UpdateWindow(HDC DeviceContext, RECT ClientRect , int X, int Y, int Width, int Height)
{
    int WindowWidth =  ClientRect.right - ClientRect.left;
    int WindowHeight = ClientRect.bottom - ClientRect.top;
    StretchDIBits(DeviceContext, 0, 0, BitmapWidth, BitmapHeight, 0, 0, WindowWidth, WindowHeight, BitmapMemory, &BitmapInfo, DIB_RGB_COLORS, SRCCOPY);
}

LRESULT Wndproc(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam)
{
    LRESULT result = 0;

    switch (Message)
    {
        case WM_SIZE:
        {
            RECT ClientRect;
            GetClientRect(Window, &ClientRect);
            int Height = ClientRect.bottom - ClientRect.top;
            int Width = ClientRect.right - ClientRect.left;
            ResizeDIBSection(Width, Height);

        }
            
            break;

        case WM_DESTROY:
        {
            // NOTE: Handle this with a message to the user?
            isRunning = false;
        }
            break;

        case WM_CLOSE:
        {
            // NOTE: Handle this as an error - recreate window?
            isRunning = false;
            PostQuitMessage(0);
        }
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

            RECT ClientRect;
            GetClientRect(Window, &ClientRect);

            UpdateWindow(DeviceContext, ClientRect, X, Y, Width, Height); 
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

    WindowClass.style = CS_HREDRAW | CS_VREDRAW;    
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
        HWND Window = CreateWindowEx(0, WindowClass.lpszClassName, (LPCSTR)"Handmade Hero", WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT,
                                             CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, hInstance, 0);

        if (Window)
        {
            int xOffset = 0;
            int yOffset = 0;

            isRunning = true;
            while (isRunning)
            {   
                MSG Message;
                while (PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
                {   
                    if (Message.message == WM_QUIT)
                    {
                        isRunning = false;
                    }

                    TranslateMessage(&Message);
                    DispatchMessage(&Message);
                }

                RenderWeirdGradient(xOffset, yOffset);

                HDC DeviceContext = GetDC(Window);
                RECT ClientRect;

                GetClientRect(Window, &ClientRect);
                int WindowWidth =  ClientRect.right - ClientRect.left;
                int WindowHeight = ClientRect.bottom - ClientRect.top;

                UpdateWindow(DeviceContext, ClientRect, 0, 0, WindowWidth, WindowHeight); 

                ReleaseDC(Window, DeviceContext);

                xOffset++;
                yOffset++;
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