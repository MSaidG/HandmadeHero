#include <windows.h>
#include <stdint.h> 

struct win32_sound_output
{
    // Audio Test
    int SamplePerSecond;
    uint32_t RunningSampleIndex;
    int BytesPerSample;
    int SecondaryBufferSize;
    float tSine;
    int LatencySampleCount;
};

struct win32_offscreen_buffer
{
    BITMAPINFO Info;
    void *Memory;
    int Height;  
    int Width;
    int Pitch;
    int BytesPerPixel = 4;
};

struct win32_window_dimension
{
    int Width;
    int Height;
};