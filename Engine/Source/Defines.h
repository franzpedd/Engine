#pragma once

// how many frames are simultaniously rendered on gpu
#define RENDERER_MAX_FRAMES_IN_FLIGHT 2

// how many chars in total an entity may have to represent it's name
#define ENTITY_NAME_MAX_CHARS 128

// how many chars a file may have in total (starting at 'Data')
#define MAX_SEARCH_PATH_SIZE 256

// how many threads the sound resources have
#define RESOURCES_THREAD_SOUND_COUNT 4