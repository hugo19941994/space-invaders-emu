#include "./connection.h"
#include "./emu.h"

#include "SDL2/SDL.h"
#include "SDL2/SDL_endian.h"
#include "libzip/zip.h"

#include <array>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <map>
#include <vector>

intel8080 i8080;

// Screen dimension constants
const int SCALE = 6;

// The window we'll be rendering to
SDL_Window *gWindow = nullptr;

// The window renderer
SDL_Renderer *gRenderer = nullptr;

// The window surface
SDL_Surface *gScreenSurface = nullptr;
SDL_Surface *gScreenSurface2 = nullptr;

// Current displayed texture
SDL_Texture *gTexture = nullptr;

void loadRom(const char *file, int offset) {
  FILE *ROM = fopen(file, "rb");
  fseek(ROM, 0, SEEK_END);
  uint64_t size = ftell(ROM);
  rewind(ROM);

  // Allocate memory
  std::vector<uint8_t> buffer(size);

  // Copy file to buffer
  fread(buffer.data(), 1, size, ROM);

  std::copy(buffer.begin(), buffer.end(), i8080.memory.begin() + offset);
  fclose(ROM);
}

void loadRomZip(zip *z, const char *name, int offset) {
  struct zip_stat st = {};
  zip_stat_init(&st);
  zip_stat(z, name, 0, &st);

  zip_file *f = zip_fopen(z, name, 0);
  std::vector<uint8_t> buffer(st.size);
  zip_fread(f, buffer.data(), st.size);

  // Copy file to buffer
  std::copy(buffer.begin(), buffer.end(), i8080.memory.begin() + offset);

  zip_fclose(f);
}

#define op(id, oper)                                                           \
  case id:                                                                     \
    i8080.oper;                                                                \
    break;
void interruptExecute(int opcode) {
  switch (opcode) {
    op(0xC7, RST(0x00 * 8));
    op(0xD7, RST(0x02 * 8));
    op(0xE7, RST(0x04 * 8));
    op(0xF7, RST(0x06 * 8));
    op(0xCF, RST(0x01 * 8));
    op(0xDF, RST(0x03 * 8));
    op(0xEF, RST(0x05 * 8));
    op(0xFF, RST(0x07 * 8));
  }
}
#undef op

bool init(void *window) {
  // Initialize SDL
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    std::cout << "SDL could not initialize! SDL Error: " << SDL_GetError()
              << std::endl;
    return false;
  }

  // Set texture filtering to linear
  if (SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1") == 0) {
    std::cout << "Warning: Linear texture filtering not enabled!" << std::endl;
  }

  // Create window
  gWindow = SDL_CreateWindowFrom(window);
  if (gWindow == nullptr) {
    std::cout << "Window could not be created! SDL Error: " << SDL_GetError()
              << std::endl;
    return false;
  }

  // Create renderer for window
  gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);
  if (gRenderer == nullptr) {
    std::cout << "Renderer could not be created! SDL Error: " << SDL_GetError()
              << std::endl;
    return false;
  }
  // Initialize renderer color
  SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);

  // Scaled window surface
  gScreenSurface2 =
      SDL_CreateRGBSurface(0, 224 * SCALE, 256 * SCALE, 32, 0, 0, 0, 0);

  // Game surface (original arcade size)
  gScreenSurface = SDL_CreateRGBSurface(0, 224, 256, 32, 0, 0, 0, 0);
  gTexture = SDL_CreateTextureFromSurface(gRenderer, gScreenSurface2);

  return true;
}

void draw() {
  uint32_t *bits;
  uint32_t pixel;
  uint16_t i;
  uint8_t j;

  memset(gScreenSurface->pixels, 0, 256 * gScreenSurface->pitch);
  pixel = SDL_MapRGB(gScreenSurface->format, 0xFF, 0xFF, 0xFF);

  SDL_LockSurface(gScreenSurface);
  for (i = 0x2400; i < 0x3fff; i++) {
    if (i8080.memory.at(i) != 0) {
      for (j = 0; j < 8; j++) {
        if ((i8080.memory.at(i) & (1 << j)) != 0) {
          bits =
              reinterpret_cast<uint32_t *>(gScreenSurface->pixels) +
              ((255 - ((((i - 2400) % 0x20) << 3) + j)) * gScreenSurface->w) +
              ((i - 2400) >> 5) + 11;
          *bits = pixel;
        }
      }
    }
  }
  SDL_UnlockSurface(gScreenSurface);

  // Scale original surface to window surface
  SDL_BlitScaled(gScreenSurface, nullptr, gScreenSurface2, nullptr);

  // Update texture
  SDL_UpdateTexture(gTexture, nullptr, gScreenSurface2->pixels,
                    gScreenSurface2->pitch);

  // Clear screen
  SDL_RenderClear(gRenderer);

  // Render texture to screen
  SDL_RenderCopy(gRenderer, gTexture, nullptr, nullptr);

  // Update screen
  SDL_RenderPresent(gRenderer);
}

void handleInput(bool *exit) {
  SDL_Event event = {};

  while (SDL_PollEvent(&event)) {
    if (event.type == SDL_QUIT) {
      *exit = true;
    } else if (event.type == SDL_KEYDOWN) {
      switch (event.key.keysym.sym) {
      case (SDLK_0): {
        i8080.Read0 |= 0b00000001;
        break;
      }
      case (SDLK_2): {
        i8080.Read0 |= 0b00000010;
        break;
      }
      case (SDLK_1): {
        i8080.Read0 |= 0b00000100;
        break;
      }
      case (SDLK_SPACE): {
        i8080.Read0 |= 0b00010000;
        i8080.Read1 |= 0b00010000;
        break;
      }
      case (SDLK_LEFT): {
        i8080.Read0 |= 0b00100000;
        i8080.Read1 |= 0b00100000;
        break;
      }
      case (SDLK_RIGHT): {
        i8080.Read0 |= 0b01000000;
        i8080.Read1 |= 0b01000000;
        break;
      }
      }
    } else if (event.type == SDL_KEYUP) {
      switch (event.key.keysym.sym) {
      case (SDLK_0): {
        i8080.Read0 &= 0b11111110;
        break;
      }
      case (SDLK_2): {
        i8080.Read0 &= 0b11111101;
        break;
      }
      case (SDLK_1): {
        i8080.Read0 &= 0b11111011;
        break;
      }
      case (SDLK_SPACE): {
        i8080.Read0 &= 0b11101111;
        i8080.Read1 &= 0b11101111;
        break;
      }
      case (SDLK_LEFT): {
        i8080.Read0 &= 0b11011111;
        i8080.Read1 &= 0b11011111;
        break;
      }
      case (SDLK_RIGHT): {
        i8080.Read0 &= 0b10111111;
        i8080.Read1 &= 0b10111111;
        break;
      }
      }
    }
  }
}

int main2(void *window, const char *zipFile) {
  // Initialize Program Counter & Stack Pointer
  i8080.pc = 0x0;
  i8080.sp = 0xf000;

  // Extract ROM files from the selected ZIP file
  std::string zipF(zipFile);
  zipF.erase(0, 6); // Remove file://
  int32_t err = ZIP_ER_OK;
  zip *z = zip_open(zipF.c_str(), 0, &err);
  if (err != ZIP_ER_OK) {
    return err;
  }

  // Load ROMs
  loadRomZip(z, "invaders.h", 0);
  loadRomZip(z, "invaders.g", 0x800);
  loadRomZip(z, "invaders.f", 0x1000);
  loadRomZip(z, "invaders.e", 0x1800);

  // Close ZIP file
  zip_close(z);

  // Setup windows, renderer & surface
  init(window);

  // Choose which interrupt to execute
  bool interruptSwitch = false;
  uint32_t refresh = (2000000 / 60) / 2; // Refresh rate
  bool exit = false;

  while (!exit) {
    i8080.emulateCycle();

    if (i8080.interrupts && (i8080.cycles >= refresh)) {
      handleInput(&exit);

      if (interruptSwitch) {
        interruptExecute(0xd7);
      } else {
        interruptExecute(0xcf);
      }

      interruptSwitch = !interruptSwitch;
      i8080.interrupts = false;

      draw();
      SDL_Delay(10);
      i8080.cycles = 0;
    }
  }
  return 0;
}
