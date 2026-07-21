#include <SDL3/SDL.h>
#include <stdio.h>

int main() {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_GPUDevice *device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, true, NULL);
    if (device) {
        SDL_Log("Chosen backend: %s", SDL_GetGPUDeviceDriver(device));
    } else {
        SDL_Log("No usable GPU backend: %s", SDL_GetError());
    }
    return 0;
}
