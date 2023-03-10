// NOTE: Services that the game provides to the platfrom layer.
#define HANDMADE_SLOW 1
#define HANDMADE_INTERNAL 1
/*
    // NOTE: Build Types

    HANDMADE_INTERNAL:
    0 - Build for public release
    1 - Build for developer only

    HANDMADE_SLOW:
    0 - Slow code not allowed!
    1 - Slow code allowed!


*/

#define ArrayCount(array) (sizeof(array) / sizeof((array)[0]))
#define Kilobytes(value) ((value)*1024)
#define Megabytes(value) (Kilobytes(value)*1024)
#define Gigabytes(value) (Megabytes(value)*1024)
#define Terabytes(value) (Gigabytes(value)*1024)

#if (HANDMADE_SLOW)
#define Assert(expression) \
    if (!(expression)) {*(int *)0 = 0;}
#else
#define Assert(expression)
#endif

// NOTE: Services that the platform layer provides to the game 
// might expand in thew future (multi threading)


// timing, controller/keyboard input, bitmap buffer, sound buffer

// TODO: In the future rendering _specifically_ will become a three tiered abstraction!!!
struct game_offscreen_buffer
{
    void *Memory;
    int Height;  
    int Width;
    int Pitch;
    int BytesPerPixel = 4;
};

struct game_sound_output_buffer
{
    int SamplePerSecond;
    int SampleCount;
    int16_t *Samples;
};

struct game_button_state
{
    int halfTransitionCount;
    bool endedDown;
};

struct game_controller_input
{
    bool isAnalog;

    float startX;
    float startY;
    
    float endX;
    float endY;
    
    float minX;
    float minY;
    
    float maxX;
    float maxY;



    union 
    {
        game_button_state buttons[6];
        struct 
        {
            game_button_state up;
            game_button_state down;
            game_button_state right;
            game_button_state left;
            game_button_state leftShoulder;
            game_button_state rightShoulder;
        };
    };
};         

struct game_input
{
    //float gameClock;    
    // TODO: Insert clock values here
    game_controller_input controllers[4];
};


struct game_memory
{
    bool isInitialized;
    
    uint64_t permanentStorageSize;
    void *permanentStorage; // NOTE: REQUIRED to be cleared to zero at startup
    
    uint64_t transientStorageSize;
    void *transientStorage; // NOTE: REQUIRED to be cleared to zero at startup

};

static void GameUpdateAndRender(game_memory *memory, game_input *input, game_offscreen_buffer *buffer, game_sound_output_buffer *SoundBuffer);

//
//
//
//


struct game_state
{
    int hz;
    int greenOffset;
    int blueOffset;
};




