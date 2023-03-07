#include <windows.h>
#include <stdint.h> 
#include <xInput.h>
#include <cstdio>

#define internal static 
#define local_persist static 
#define global_variable static 


// NOTE: This is a global for now!
struct win32_offscreen_buffer
{
    BITMAPINFO Info;
    void *Memory;
    int Height;  
    int Width;
    int Pitch;
    int BytesPerPixel = 4;
};

global_variable win32_offscreen_buffer GlobalBackBuffer;
global_variable bool isRunning;
global_variable uint32_t VKCode;

struct win32_window_dimension
{
    int Width;
    int Height;
};


// NOTE: This is madness

#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE *pState)
typedef X_INPUT_GET_STATE(x_input_get_state);
X_INPUT_GET_STATE(XInputGetStateStub)
{
    return 0;
}
global_variable x_input_get_state *XInputGetState_ = XInputGetStateStub; 
#define XInputGetState XInputGetState_


#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration)
typedef X_INPUT_SET_STATE(x_input_set_state);
X_INPUT_SET_STATE(XInputSetStateStub)
{
    return 0;
}
global_variable x_input_set_state *XInputSetState_ = XInputSetStateStub;
#define XInputSetState XInputSetState_

 // NOTE: Madness is end it here

internal void Win32LoadXInput(void)
{
    HMODULE XInputLibrary =  LoadLibraryA("xinput1_4.dll");
    if (XInputLibrary)
    {
        XInputGetState = (x_input_get_state *)GetProcAddress(XInputLibrary, "XInputGetState");
        XInputSetState = (x_input_set_state *)GetProcAddress(XInputLibrary, "XInputSetState");

    }
}


internal win32_window_dimension Win32GetWindowDimension(HWND Window)
{
    win32_window_dimension result;

    RECT ClientRect;
    GetClientRect(Window, &ClientRect);
    result.Height = ClientRect.bottom - ClientRect.top;
    result.Width = ClientRect.right - ClientRect.left;

    return result;
}

internal void RenderWeirdGradient(win32_offscreen_buffer *Buffer, int xOffset, int yOffset)
{

    int Width = Buffer->Width;     
    int Height = Buffer->Height;
    
    //int Pitch = Width * Buffer->BytesPerPixel;
    uint8_t *Row  = (uint8_t *)Buffer->Memory;

    for (int y = 0; y < Buffer->Height; y++)
    {
        uint32_t *Pixel = (uint32_t *) Row;
        for (int x = 0; x < Buffer->Width; x++)
        {
            uint8_t Blue = (x + xOffset);
            uint8_t Green = (y + yOffset);

            *Pixel++ = ((Green << 16) | Blue);
        }

        Row = (uint8_t *) Pixel;
        //Row += Buffer->Pitch;
    }
}


internal void Win32ResizeDIBSection(win32_offscreen_buffer *Buffer, int Width, int Height)
{

    // TODO: bullet proof this.

    if (Buffer->Memory)
    {
        VirtualFree(Buffer->Memory, 0, MEM_RELEASE);
    }

    Buffer->Height = Height;
    Buffer->Width = Width;

    // NOTE: Negative Height is to make bitmap top-down and left to right!
    Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info.bmiHeader);
    Buffer->Info.bmiHeader.biWidth = Buffer->Width;
    Buffer->Info.bmiHeader.biHeight = -Buffer->Height;
    Buffer->Info.bmiHeader.biPlanes = 1;
    Buffer->Info.bmiHeader.biBitCount = 32; 
    Buffer->Info.bmiHeader.biCompression = BI_RGB;
    
    // NOTE: May just allocate this ourselves?
    //BitmapHandle = CreateDIBSection(BitmapDeviceContext, &BitmapInfo, DIB_RGB_COLORS, &BitmapMemory, 0, 0);

    
    int BitmapMemorySize = (Buffer->Width * Buffer->Height) * Buffer->BytesPerPixel;
    Buffer->Memory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);

    // NOTE: Clear to black

    Buffer->Pitch = Width * Buffer->BytesPerPixel;

}

internal void Win32CopyBufferToWindow(win32_offscreen_buffer *Buffer, HDC DeviceContext, int WindowWidth, int WindowHeight)
{
    // NOTE: Aspect ratio corection
    StretchDIBits(DeviceContext, 0, 0, WindowWidth, WindowHeight, 0, 0, Buffer->Width, Buffer->Height, Buffer->Memory, &Buffer->Info, DIB_RGB_COLORS, SRCCOPY);
}

internal LRESULT Wndproc(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam)
{
    LRESULT result = 0;

    switch (Message)
    {
        case WM_SIZE:
        {



        }
            
            break;

        case WM_DESTROY:
        {
            // NOTE: Handle this with a message to the user?
            isRunning = false;
        }
            break;

        //case WM_SYSKEYDOWN:
        //{
//
        //}
        //    break;
        //
        //case WM_SYSKEYUP:
        //{
//
        //}
        //    break;
//
        //case WM_KEYDOWN:
        //{
//
        //}   
        //    break;
        //
        case WM_KEYUP:
        {
            VKCode = WParam;
            bool wasDown = ((LParam & (1 << 30)) != 0);
            bool isDown = ((LParam & (1 << 31)) != 0);

            if (VKCode == 0x57)
            {
                OutputDebugStringA("W\n");
            }
            else if (VKCode == 0x41)
            {
                OutputDebugStringA("A\n");
            }
            else if (VKCode == 0x53)
            {
                OutputDebugStringA("S\n");
            }
            else if (VKCode == 0x44)
            {
                OutputDebugStringA("D\n");
            }
            else if (VKCode == 0x45)
            {
                OutputDebugStringA("E\n");
            }
            else if (VKCode == 0x51)
            {
                OutputDebugStringA("Q\n");
            }
            else if (VKCode == VK_ESCAPE)
            {
                OutputDebugStringA("ESC:");
                if (isDown)
                {
                    OutputDebugStringA("isDown");
                }

                if (wasDown)
                {
                    OutputDebugStringA("wasDown");
                }
                 OutputDebugStringA("\n");
            }
            else if (VKCode == VK_SPACE)
            {
                OutputDebugStringA("SPACE\n");
            }

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

            win32_window_dimension Dimension = Win32GetWindowDimension(Window);

            Win32CopyBufferToWindow(&GlobalBackBuffer, DeviceContext, Dimension.Width, Dimension.Height); 

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
    Win32LoadXInput();

    //uint8_t BigOldBlockOfMemory[2*1024*1024] = {};
    OutputDebugStringA("TEST!");

    WNDCLASS WindowClass = {};

    Win32ResizeDIBSection(&GlobalBackBuffer, 1288, 728);

    WindowClass.style = CS_HREDRAW | CS_VREDRAW;    
    WindowClass.lpfnWndProc = Wndproc;
    WindowClass.hInstance = hInstance;
    //WindowsClass.hIcon = ;
    WindowClass.lpszClassName =  (LPCSTR)"HandmadeHeroWindow";
 
        
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


                // NOTE: Can pull this more frequently
                
                DWORD controllerIndex;
                for (DWORD i=0; i < XUSER_MAX_COUNT; i++)
                {
                    controllerIndex = i;
                    XINPUT_STATE ControllerState;
                    ZeroMemory(&ControllerState, sizeof(XINPUT_STATE));

                    DWORD dwResult = XInputGetState(i, &ControllerState);

                    if(dwResult == ERROR_SUCCESS)
                    {
                        // NOTE: Controller is connected
                        XINPUT_GAMEPAD *Pad = &ControllerState.Gamepad;
                        
                        bool UpDpad = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_UP);
                        bool RightDpad = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);
                        bool LeftDpad = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
                        bool DownDpad = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
                        bool LeftShoulder = (Pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
                        bool RightShoulder = (Pad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);
                        bool AButton = (Pad->wButtons & XINPUT_GAMEPAD_A);
                        bool BButton = (Pad->wButtons & XINPUT_GAMEPAD_B);
                        bool XButton = (Pad->wButtons & XINPUT_GAMEPAD_X);
                        bool YButton = (Pad->wButtons & XINPUT_GAMEPAD_Y);

                        int16_t LThumbx = Pad->sThumbLX;
                        int16_t LThumby = Pad->sThumbLY;

                        if (AButton)
                        {
                            xOffset++;
                            yOffset++;
                        }

                        

                    }
                    else
                    {
                        // NOTE: Controller is not connected
                    }
                }
                XINPUT_VIBRATION Vibration;
                Vibration.wLeftMotorSpeed = 1000;
                Vibration.wLeftMotorSpeed = 1000;
                XInputSetState(0, &Vibration);
                

                RenderWeirdGradient(&GlobalBackBuffer, xOffset, yOffset);

                HDC DeviceContext = GetDC(Window);

                win32_window_dimension Dimension = Win32GetWindowDimension(Window);

                Win32CopyBufferToWindow(&GlobalBackBuffer, DeviceContext, Dimension.Width, Dimension.Height); 

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