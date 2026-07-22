#include <SDL3/SDL.h>
#include <stdio.h>

int main() {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_GPUDevice *device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, true, NULL);
    if (device) {
        printf("Chosen backend: %s\n", SDL_GetGPUDeviceDriver(device));
    } else {
        printf("Failed to create GPU device: %s\n", SDL_GetError());
    }
    return 0;
}
