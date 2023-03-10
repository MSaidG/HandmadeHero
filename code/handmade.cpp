#include "handmade.h"
#define Pi 3.14159265359f

static void GameOutputSound(game_sound_output_buffer *soundBuffer, int hz)
{
    static float tSine;
    int16_t audioVolume = 3000;
    int WavePeriod = soundBuffer->SamplePerSecond / hz;

    int16_t *SampleOut = soundBuffer -> Samples;
    for (int SampleIndex = 0; SampleIndex < soundBuffer->SampleCount; ++SampleIndex)
    {           
        float SineValue = sinf(tSine);
        int16_t SampleValue = (int16_t)(SineValue * audioVolume);

        *SampleOut++ = SampleValue;
        *SampleOut++ = SampleValue;

        tSine += 2.0f*Pi*1.0f / (float)WavePeriod;
    }
}

static void RenderWeirdGradient(game_offscreen_buffer *buffer, int xOffset, int yOffset)
{


    int width = buffer->Width;     
    int height = buffer->Height;
    
    //int Pitch = Width * buffer->BytesPerPixel;
    uint8_t *row  = (uint8_t *)buffer->Memory;

    for (int y = 0; y < height; y++)
    {
        uint32_t *pixel = (uint32_t *) row;
        for (int x = 0; x < width; x++)
        {
            uint8_t blue = (x + xOffset);
            uint8_t green = (y + yOffset);

            *pixel++ = ((green << 16) | blue);
        }

        row = (uint8_t *) pixel;
        //Row += buffer->Pitch;
    }
}

static void GameUpdateAndRender(game_memory *memory, game_input *input, game_offscreen_buffer *buffer, game_sound_output_buffer *soundBuffer)
{
    Assert(sizeof(game_state ) <= memory->permanentStorageSize);

    game_state *gameState = (game_state *) memory->permanentStorage;

    if (!memory->isInitialized)
    {
        char *fileName = (char *)__FILE__;

        debug_read_file_result file = DEBUGPlatformReadEntireFile(fileName);
        if (file.contents)
        {
            DEBUGPlatformWriteEntireFile((char *)"C:/HandmadeHero/data/test.out", file.contentSize, file.contents);
            DEBUGPlatformFreeFileMemory(file.contents);
        }

        gameState->hz = 256;
        memory->isInitialized = true;
    }

    
    game_controller_input *input0 = &input->controllers[0];
    if (input0->isAnalog)
    {
        gameState->blueOffset += (int) (4.0f * (input0->endX));
        gameState->hz = 256 + (int) (128.0f * (input0->endY));
    }
    else
    {
        
    }

    if (input0->down.endedDown)
    {
        gameState->greenOffset += 1;
    }

    //TODO: Allow sample offsets for more robust platform options
    GameOutputSound(soundBuffer, gameState->hz);
    RenderWeirdGradient(buffer, gameState->blueOffset, gameState->greenOffset);
}






















