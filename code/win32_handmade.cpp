/*
    THIS IS NOT A FINAL PLATFORM LAYER

    - Saved game locations
    - Getting a handle to our own executable file
    - Asset laoding path
    - Threading (launch a thread)
    - Raw Input (support for multiple keyboards)
    - Sleep- timebeginperiod
    - ClipCursor() (multi monitor support)
    - Fullscreen support
    - WM_SETCURSOR (Control cursor visibility)
    - QueryCancelAutoPlay
    - WM_ACTIVATEAPP (for not in the focus)
    - Blit speed improvement
    - Hardware acclereation (OpenGL or Direct3D)
    - GetKeyboardLayout (international WASD support)

    - Just a partial list of stuff!
*/

#include <malloc.h>
#include <windows.h>
#include <stdint.h> 
#include <xInput.h>
#include <dsound.h>
#include <math.h>
#include <cstdio>

#include "handmade.cpp"
#include "win32_handmade.h"

#define internal static 
#define local_persist static 
#define global_variable static 
#define Pi 3.14159265359f


global_variable win32_offscreen_buffer GlobalBackBuffer;
global_variable bool isGameRunning;
global_variable LPDIRECTSOUNDBUFFER SecondaryAudioBuffer;
global_variable uint32_t VKCode;


// NOTE: This is madness

#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE *pState)
typedef X_INPUT_GET_STATE(x_input_get_state);
X_INPUT_GET_STATE(XInputGetStateStub)
{
    return ERROR_DEVICE_NOT_CONNECTED;
}
global_variable x_input_get_state *XInputGetState_ = XInputGetStateStub; 
#define XInputGetState XInputGetState_


#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration)
typedef X_INPUT_SET_STATE(x_input_set_state);
X_INPUT_SET_STATE(XInputSetStateStub)
{
    return ERROR_DEVICE_NOT_CONNECTED;
}
global_variable x_input_set_state *XInputSetState_ = XInputSetStateStub;
#define XInputSetState XInputSetState_

#define DIRECT_SOUND_CREATE(name) HRESULT WINAPI (name)(LPCGUID lpGUID,LPDIRECTSOUND *ppDS,LPUNKNOWN pUnkOuter)
typedef DIRECT_SOUND_CREATE(direct_sound_create);

 // NOTE: Madness is end it here

internal void Win32LoadXInputLib(void)
{
    HMODULE XInputLibrary =  LoadLibraryA("xinput1_4.dll");
    if (!XInputLibrary)
    {
        HMODULE XInputLibrary = LoadLibraryA("xinput9_1_0.dll");
    }

    if (!XInputLibrary)
    {
        HMODULE XInputLibrary =  LoadLibraryA("xinput1_3.dll");
    }

    if (XInputLibrary)
    {
        XInputGetState = (x_input_get_state *)GetProcAddress(XInputLibrary, "XInputGetState");
        if (!XInputGetState) {XInputGetState = XInputGetStateStub;}

        XInputSetState = (x_input_set_state *)GetProcAddress(XInputLibrary, "XInputSetState");
        if (!XInputGetState) {XInputGetState = XInputGetStateStub;}

        // TODO: Diagnostics
    }
    else
    {
        // TODO: Diagnostics
    }
}



internal void Win32InitDSound(HWND Window, int32_t SamplePerSecond, int32_t BufferSize)
{
    // NOTE: Load the library
    HMODULE DSoundLibrary = LoadLibraryA("dsound.dll");

    if (DSoundLibrary)
    {
        // NOTE: Get a DirectSound object! cooperative
        direct_sound_create *DirectSoundCreate = (direct_sound_create *) GetProcAddress(DSoundLibrary, "DirectSoundCreate");

        LPDIRECTSOUND DirectSound;
        if (DirectSoundCreate && SUCCEEDED(DirectSoundCreate(0, &DirectSound, 0)))
        {
            WAVEFORMATEX WaveFormat = {};
            WaveFormat.wFormatTag = WAVE_FORMAT_PCM;
            WaveFormat.nChannels = 2;
            WaveFormat.nSamplesPerSec = SamplePerSecond;
            WaveFormat.wBitsPerSample = 16; 
            WaveFormat.nBlockAlign = (WaveFormat.nChannels * WaveFormat.wBitsPerSample) / 8;
            WaveFormat.nAvgBytesPerSec = WaveFormat.nSamplesPerSec * WaveFormat.nBlockAlign;
            WaveFormat.cbSize = 0;


            if(SUCCEEDED(DirectSound->SetCooperativeLevel(Window, DSSCL_PRIORITY)))
            {
                DSBUFFERDESC BufferDescription = {};
                BufferDescription.dwSize = sizeof(BufferDescription);
                BufferDescription.dwFlags = DSBCAPS_PRIMARYBUFFER;
                
                // NOTE: "Create" a primary buffer
                // TODO: DSBCAPS_GLOBALFOCUS    
                LPDIRECTSOUNDBUFFER PrimaryBuffer;
                if (SUCCEEDED(DirectSound->CreateSoundBuffer(&BufferDescription, &PrimaryBuffer, 0)))
                {

                        // NOTE: Set the format
                    HRESULT Error = PrimaryBuffer->SetFormat(&WaveFormat);
                    if (SUCCEEDED(Error))
                    {
                        OutputDebugStringA("PrimaryBuffer created!");
                    }

                    else
                    {
                        // TODO: Diagnostics
                    }
                }
                else
                {
                    // TODO: Diagnostics
                }
            }
            else
            {
                // TODO: Diagnostics
            }

            // TODO: DSBCAPS_GETCURRENTPOSTION2
            DSBUFFERDESC BufferDescription = {};
            BufferDescription.dwSize = sizeof(BufferDescription);
            BufferDescription.dwFlags = 0;
            BufferDescription.dwBufferBytes = BufferSize;
            BufferDescription.lpwfxFormat = &WaveFormat;
            //LPDIRECTSOUNDBUFFER SecondaryAudioBuffer;      
            HRESULT Error = DirectSound->CreateSoundBuffer(&BufferDescription, &SecondaryAudioBuffer, 0);
            if (SUCCEEDED(Error))
            {
                OutputDebugStringA("SecondaryAudioBuffer created!");
            }
        }
        else
        {
            // TODO: Diagnostics
        }
    }
    else
    {
        // TODO: Diagnostics
    }

    // NOTE: Get a DirectSound object!


    
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


internal void Win32ResizeDIBSection(win32_offscreen_buffer *buffer, int Width, int Height)
{

    // TODO: bullet proof this.

    if (buffer->Memory)
    {
        VirtualFree(buffer->Memory, 0, MEM_RELEASE);
    }

    buffer->Height = Height;
    buffer->Width = Width;

    // NOTE: Negative Height is to make bitmap top-down and left to right!
    buffer->Info.bmiHeader.biSize = sizeof(buffer->Info.bmiHeader);
    buffer->Info.bmiHeader.biWidth = buffer->Width;
    buffer->Info.bmiHeader.biHeight = -buffer->Height;
    buffer->Info.bmiHeader.biPlanes = 1;
    buffer->Info.bmiHeader.biBitCount = 32; 
    buffer->Info.bmiHeader.biCompression = BI_RGB;
    
    // NOTE: May just allocate this ourselves?
    //BitmapHandle = CreateDIBSection(BitmapDeviceContext, &BitmapInfo, DIB_RGB_COLORS, &BitmapMemory, 0, 0);

    
    int BitmapMemorySize = (buffer->Width * buffer->Height) * buffer->BytesPerPixel;
    buffer->Memory = VirtualAlloc(0, BitmapMemorySize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
    buffer->Pitch = Width * buffer->BytesPerPixel;

    // NOTE: Clear to black


}

internal void Win32CopyBufferToWindow(win32_offscreen_buffer *buffer, HDC DeviceContext, int WindowWidth, int WindowHeight)
{
    // NOTE: Aspect ratio corection
    StretchDIBits(DeviceContext, 0, 0, WindowWidth, WindowHeight, 0, 0, buffer->Width, buffer->Height, buffer->Memory, &buffer->Info, DIB_RGB_COLORS, SRCCOPY);
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
            isGameRunning = false;
        }
            break;

        case WM_SYSKEYDOWN:        
        case WM_SYSKEYUP:
        case WM_KEYDOWN:
        case WM_KEYUP:
        {
            VKCode = WParam;
            bool wasDown = ((LParam & (1 << 30)) != 0);
            bool isDown = ((LParam & (1 << 31)) == 0);

            if (wasDown != isDown)
            {
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



            bool AltKeyWasDown = ((LParam & (1 << 29)) != 0);
            if ((VKCode == VK_F4) && AltKeyWasDown)
            {
                isGameRunning = false;
            }

        }
            break;

        case WM_CLOSE:
        {
            // NOTE: Handle this as an error - recreate window?
            isGameRunning = false;
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
internal void Win32ClearBuffer(win32_sound_output *SoundOutput)
{
    VOID *Region1;
    DWORD Region1Size;
    VOID *Region2;
    DWORD Region2Size;
    int audioVolume = 3000;
    if (SUCCEEDED(SecondaryAudioBuffer->Lock(0, SoundOutput->SecondaryBufferSize,
                                             &Region1, &Region1Size, 
                                             &Region2, &Region2Size, 0)))
    {
        int8_t *DestSample = (int8_t *)Region1;
        for (DWORD ByteIndex = 0; ByteIndex < Region1Size; ++ByteIndex)
        {           
            float SineValue = sinf(SoundOutput->tSine);
            int16_t SampleValue = (int16_t)(SineValue * audioVolume);

            *DestSample++ = 0;
        }
        DestSample = (int8_t *)Region2;
        for (DWORD ByteIndex = 0; ByteIndex < Region2Size; ++ByteIndex)
        {           
            *DestSample++ = 0;
        }
        SecondaryAudioBuffer->Unlock(Region1, Region1Size, Region2, Region2Size);

    }
}

internal void Win32FillSoundBar(win32_sound_output *SoundOutput, DWORD ByteToLock, DWORD BytesToWrite, game_sound_output_buffer *SourceBuffer)
{
    VOID *Region1;
    DWORD Region1Size;
    VOID *Region2;
    DWORD Region2Size;
    int audioVolume = 3000;
    if (SUCCEEDED(SecondaryAudioBuffer->Lock(ByteToLock, BytesToWrite, 
                                             &Region1, &Region1Size, 
                                             &Region2, &Region2Size, 0)))
    {
        // TODO: assert Region1Size and 2
        DWORD Region1SampleCount = Region1Size / SoundOutput->BytesPerSample;
        int16_t *DestSample = (int16_t *)Region1;
        int16_t *SourceSample = SourceBuffer->Samples;

        for (DWORD SampleIndex = 0; SampleIndex < Region1SampleCount; ++SampleIndex)
        {           
            float SineValue = sinf(SoundOutput->tSine);
            int16_t SampleValue = (int16_t)(SineValue * audioVolume);

            *DestSample++ = *SourceSample++;
            *DestSample++ = *SourceSample++;
            SoundOutput->RunningSampleIndex++;
        }

        DWORD Region2SampleCount = Region2Size / SoundOutput->BytesPerSample;
        DestSample = (int16_t *) Region2;

        for (DWORD SampleIndex = 0; SampleIndex < Region2SampleCount; ++SampleIndex)
        {
            float SineValue = sinf(SoundOutput->tSine);
            int16_t SampleValue = (int16_t)(SineValue * audioVolume);

            *DestSample++ = *SourceSample++;
            *DestSample++ = *SourceSample++;
            SoundOutput->RunningSampleIndex++;
        }

        SecondaryAudioBuffer->Unlock(Region1, Region1Size, Region2, Region2Size);

    }
}


internal void Win32ProcessDigitalXInputButton ( DWORD xInputButtonState, game_button_state *oldState, DWORD buttonBit, game_button_state *newState)
{
    newState->endedDown  = ((xInputButtonState & buttonBit) == buttonBit);
    newState->halfTransitionCount = (oldState->endedDown != newState->endedDown) ? 1 : 0;
} 

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pCmdLine, int nCmdShow)
{
    LARGE_INTEGER PerfCountFrequencyResult;
    QueryPerformanceFrequency(&PerfCountFrequencyResult);
    int64_t PerfCountFrequency = PerfCountFrequencyResult.QuadPart;
    
    Win32LoadXInputLib();

    WNDCLASS WindowClass = {};

    Win32ResizeDIBSection(&GlobalBackBuffer, 1288, 728);

    WindowClass.style = CS_HREDRAW | CS_VREDRAW;    
    WindowClass.lpfnWndProc = Wndproc;
    WindowClass.hInstance = hInstance;
    WindowClass.lpszClassName =  (LPCSTR)"HandmadeHeroWindow";
    
        
    if (RegisterClass(&WindowClass))
    {
        HWND Window = CreateWindowEx(0, WindowClass.lpszClassName, (LPCSTR)"Handmade Hero", WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT,
                                             CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, hInstance, 0);

        if (Window)
        {
            // Graphic Test

            win32_sound_output SoundOutput = {};
            SoundOutput.SamplePerSecond;
            SoundOutput.SamplePerSecond = 48000;
            SoundOutput.RunningSampleIndex = 0;
            SoundOutput.BytesPerSample = sizeof(int16_t) * 2;
            SoundOutput.SecondaryBufferSize = SoundOutput.SamplePerSecond * SoundOutput.BytesPerSample;
            SoundOutput.LatencySampleCount = SoundOutput.SamplePerSecond / 15;
            Win32InitDSound(Window, SoundOutput.SamplePerSecond, SoundOutput.SecondaryBufferSize);
            Win32ClearBuffer(&SoundOutput);   
            SecondaryAudioBuffer->Play(0, 0, DSBPLAY_LOOPING);

            int16_t *Samples = (int16_t *)VirtualAlloc(0, SoundOutput.SecondaryBufferSize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);

#if HANDMADE_INTERNAL
    LPVOID baseAddress = (LPVOID)Terabytes((uint64_t)2);
#else
    LPVOID baseAddress = 0;
#endif


            game_memory gameMemory = {};
            gameMemory.permanentStorageSize = Megabytes(64);
            gameMemory.transientStorageSize = Gigabytes((uint64_t)4);
            
            uint64_t totalSize = gameMemory.permanentStorageSize + gameMemory.transientStorageSize;
            gameMemory.permanentStorage = VirtualAlloc(baseAddress, totalSize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
            gameMemory.transientStorage = (uint8_t *)gameMemory.permanentStorage + gameMemory.permanentStorageSize;



            if (Samples && gameMemory.permanentStorage && gameMemory.transientStorage)
            {
                game_input input[2] = {};
                game_input *newInput = &input[0];
                game_input  *oldInput = &input[1];


                LARGE_INTEGER LastCounter;
                QueryPerformanceCounter(&LastCounter);
                uint64_t LastCycleCount = _rdtsc();

                isGameRunning = true;
                while (isGameRunning)
                {   

                    MSG Message;


                    while (PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
                    {   
                        if (Message.message == WM_QUIT)
                        {
                            isGameRunning = false;
                        }

                        TranslateMessage(&Message);
                        DispatchMessage(&Message);
                    }

                    // NOTE: Can pull this more frequently                
                    DWORD controllerIndex;

                    int maxControllerCount = XUSER_MAX_COUNT;
                    if (maxControllerCount > ArrayCount(newInput->controllers))
                    {
                        maxControllerCount = ArrayCount(newInput->controllers);
                    }

                    for (DWORD controllerIndex=0; controllerIndex < maxControllerCount; controllerIndex++)
                    {

                        game_controller_input *oldController = &oldInput->controllers[controllerIndex];
                        game_controller_input *newController = &newInput->controllers[controllerIndex];

                        XINPUT_STATE ControllerState;
                        ZeroMemory(&ControllerState, sizeof(XINPUT_STATE));

                        if(XInputGetState(controllerIndex, &ControllerState) == ERROR_SUCCESS)
                        {
                            // NOTE: Controller is connected
                            XINPUT_GAMEPAD *Pad = &ControllerState.Gamepad;

                            // TODO: Dpad
                            bool up = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_UP);
                            bool right = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);
                            bool left = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
                            bool down = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN);


                            newController->isAnalog = true;

                            newController->startX = oldController->endX;
                            newController->startY = oldController->endY;

                            float x;
                            if (Pad->sThumbLX < 0)
                            {
                                x = (float)Pad->sThumbLX / 32768.0f;
                            }    
                            else
                            {
                                x = (float)Pad->sThumbLX / 32767.0f;
                            }
                            newController->minX = oldController->maxX = newController->endX = x;


                            float y;
                            if (Pad->sThumbLX < 0)
                            {
                                y = (float)Pad->sThumbLX / 32768.0f;
                            }    
                            else
                            {
                                y = (float)Pad->sThumbLX / 32767.0f;
                            }

                            // TODO: min/max macros!!!
                            newController->minY = oldController->maxY = newController->endY = y;


                            Win32ProcessDigitalXInputButton (Pad->wButtons, &oldController->down, XINPUT_GAMEPAD_A, &newController->down);     
                            Win32ProcessDigitalXInputButton (Pad->wButtons, &oldController->right, XINPUT_GAMEPAD_B, &newController->right);     
                            Win32ProcessDigitalXInputButton (Pad->wButtons, &oldController->left, XINPUT_GAMEPAD_X, &newController->left);     
                            Win32ProcessDigitalXInputButton (Pad->wButtons, &oldController->up, XINPUT_GAMEPAD_Y, &newController->up);     
                            Win32ProcessDigitalXInputButton (Pad->wButtons, &oldController->leftShoulder, XINPUT_GAMEPAD_LEFT_SHOULDER, &newController->leftShoulder);     
                            Win32ProcessDigitalXInputButton (Pad->wButtons, &oldController->rightShoulder, XINPUT_GAMEPAD_RIGHT_SHOULDER, &newController->leftShoulder);     

                            bool leftShoulder = (Pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
                            bool rightShoulder = (Pad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);
                            bool aButton = (Pad->wButtons & XINPUT_GAMEPAD_A);
                            bool bButton = (Pad->wButtons & XINPUT_GAMEPAD_B);
                            bool xButton = (Pad->wButtons & XINPUT_GAMEPAD_X);
                            bool yButton = (Pad->wButtons & XINPUT_GAMEPAD_Y);
                            bool startButton = (Pad->wButtons & XINPUT_GAMEPAD_START);
                            bool backButton = (Pad->wButtons & XINPUT_GAMEPAD_BACK);


                            XINPUT_VIBRATION Vibration;
                            if (aButton)
                            {

                                Vibration.wLeftMotorSpeed = 1000;
                                Vibration.wLeftMotorSpeed = 1000;
                                XInputSetState(0, &Vibration);
                            }
                            else
                            {
                                Vibration.wLeftMotorSpeed = 1;
                                Vibration.wLeftMotorSpeed = 1;
                                XInputSetState(0, &Vibration);
                            }


                        }
                        else    
                        {
                            // NOTE: Controller is not connected
                        }
                    }

                    // TODO: Make sure this is guarded 
                    DWORD ByteToLock = 0;
                    DWORD TargetCursor = 0;
                    DWORD BytesToWrite = 0;
                    DWORD PlayCursor = 0;
                    DWORD WriteCursor = 0;
                    bool isSoundValid = false;
                    // TODO: Tighten up sound logic 
                    if (SUCCEEDED(SecondaryAudioBuffer->GetCurrentPosition(&PlayCursor, &WriteCursor)))
                    {
                        ByteToLock = (SoundOutput.RunningSampleIndex * SoundOutput.BytesPerSample) % SoundOutput.SecondaryBufferSize;

                        TargetCursor = ((PlayCursor + (SoundOutput.LatencySampleCount * SoundOutput.BytesPerSample)) % SoundOutput.SecondaryBufferSize);

                        if (ByteToLock > TargetCursor)
                        {
                            BytesToWrite = SoundOutput.SecondaryBufferSize - ByteToLock;
                            BytesToWrite += TargetCursor; 
                        }
                        else
                        {
                            BytesToWrite = TargetCursor - ByteToLock;
                        }

                        isSoundValid = true;
                    }


                    //int16_t Samples[(48000 * 2)];
                    game_sound_output_buffer SoundBuffer = {};
                    SoundBuffer.SamplePerSecond = SoundOutput.SamplePerSecond;
                    SoundBuffer.SampleCount = BytesToWrite / SoundOutput.BytesPerSample;
                    SoundBuffer.Samples = Samples;

                    game_offscreen_buffer buffer = {};
                    buffer.Memory = GlobalBackBuffer.Memory;
                    buffer.Width = GlobalBackBuffer.Width;
                    buffer.Height = GlobalBackBuffer.Height;
                    buffer.Pitch = GlobalBackBuffer.Pitch;
                    buffer.BytesPerPixel = GlobalBackBuffer.BytesPerPixel;
                    GameUpdateAndRender(&gameMemory, newInput, &buffer, &SoundBuffer);


                    if (isSoundValid)
                    {
                        Win32FillSoundBar(&SoundOutput, ByteToLock, BytesToWrite, &SoundBuffer);   
                    }
                    HDC DeviceContext = GetDC(Window);

                    win32_window_dimension Dimension = Win32GetWindowDimension(Window);
                    Win32CopyBufferToWindow(&GlobalBackBuffer, DeviceContext, Dimension.Width, Dimension.Height); 
                    ReleaseDC(Window, DeviceContext);

                    uint64_t EndCycleCount = _rdtsc();

                    LARGE_INTEGER EndCounter;
                    QueryPerformanceCounter(&EndCounter);

                    int64_t CyclesElapsed = EndCycleCount - LastCycleCount;
                    int64_t CounterElapsed = EndCounter.QuadPart - LastCounter.QuadPart;
                    float MsPerFrame = (float)(((1000.0f*(float)CounterElapsed) / (float)PerfCountFrequency));
                    float FPS = (float)PerfCountFrequency / (float)CounterElapsed;
                    float MegaCyclePerFrame = (float)CyclesElapsed / (1000.0f * 1000.0f);

                    //printf("%fms | %ffps | %fmc \n", MsPerFrame, FPS, MegaCyclePerFrame);

                    LastCounter = EndCounter;
                    LastCycleCount = EndCycleCount;

                    //Swap()
                    game_input *temp = newInput;
                    newInput = oldInput;
                    oldInput = temp;
                    // TODO: Clear this area
                }

            }    
            else
            {
                // TODO: Logging                
            }

        }
        else
        {
            // TODO: Logging
        }

    }
    else
    {
        // TODO: Logging
        OutputDebugString((LPCSTR)"Windows register failed!");
    }

    return 0;
}
