#include "./connection.h"
#include "./emu.h"
#include "./display.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <zip.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <OpenGL/gl.h>
#include <OpenGL/OpenGL.h>

#include <array>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <map>
#include <vector>

intel8080 i8080;
Display display(224, 256, "Space Invaders");

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

bool init() {
#ifdef __APPLE__
    //GLint                       sync = 0;
    //CGLContextObj               ctx = CGLGetCurrentContext();
    
    //CGLSetParameter(ctx, kCGLCPSwapInterval, &sync);
#endif
  return true;
}

void draw() {
    std::vector<int> indBits;
    for (int i = 0x2400; i < 0x4000; ++i) {
        indBits.push_back(i8080.memory.at(i) & 0b00000001);
        indBits.push_back(i8080.memory.at(i) & 0b00000010);
        indBits.push_back(i8080.memory.at(i) & 0b00000100);
        indBits.push_back(i8080.memory.at(i) & 0b00001000);
        indBits.push_back(i8080.memory.at(i) & 0b00010000);
        indBits.push_back(i8080.memory.at(i) & 0b00100000);
        indBits.push_back(i8080.memory.at(i) & 0b01000000);
        indBits.push_back(i8080.memory.at(i) & 0b10000000);
    }
    //std::cout << indBits.size() << std::endl;
    
    for (int i = 0; i < indBits.size(); ++i) {
        display.pixels.at(i * 3) = indBits.at(i);
        display.pixels.at(i * 3 + 1) = indBits.at(i);
        display.pixels.at(i * 3 + 2) = indBits.at(i);
    }
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    switch(action) {
        case(GLFW_PRESS): {
            switch(key) {
                case(GLFW_KEY_0):
                    i8080.Read0 |= 0b00000001;
                    break;
                case(GLFW_KEY_1):
                    i8080.Read0 |= 0b00000100;
                    break;
                case(GLFW_KEY_2):
                    i8080.Read0 |= 0b00000010;
                    break;
                case(GLFW_KEY_SPACE):
                    i8080.Read0 |= 0b00010000;
                    i8080.Read1 |= 0b00010000;
                    break;
                case(GLFW_KEY_LEFT):
                    i8080.Read0 |= 0b00100000;
                    i8080.Read1 |= 0b00100000;
                    break;
                case(GLFW_KEY_RIGHT):
                    i8080.Read0 |= 0b01000000;
                    i8080.Read1 |= 0b01000000;
                    break;
            }
            break;
        }
        case(GLFW_RELEASE): {
            switch(key) {
                case (GLFW_KEY_0):
                    i8080.Read0 &= 0b11111110;
                    break;
                case (GLFW_KEY_1):
                    i8080.Read0 &= 0b11111011;
                    break;
                case (GLFW_KEY_2):
                    i8080.Read0 &= 0b11111101;
                    break;
                case (GLFW_KEY_SPACE):
                    i8080.Read0 &= 0b11101111;
                    i8080.Read1 &= 0b11101111;
                    break;
                case (GLFW_KEY_LEFT):
                    i8080.Read0 &= 0b11011111;
                    i8080.Read1 &= 0b11011111;
                    break;
                case (GLFW_KEY_RIGHT):
                    i8080.Read0 &= 0b10111111;
                    i8080.Read1 &= 0b10111111;
                    break;
            }
            break;
        }
    }
}

int main2(void *window2, const char *zipFile) {
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

  // Choose which interrupt to execute
  bool interruptSwitch = false;
  uint32_t refresh = (2000000 / 60) / 2; // Refresh rate
    
    display.start();
    glfwSetKeyCallback(display.window, key_callback);

    while (!static_cast<bool>(glfwWindowShouldClose(display.window))) {
        i8080.emulateCycle();
        
        if (i8080.interrupts && (i8080.cycles >= refresh)) {
            glfwPollEvents();
            
            if (interruptSwitch) {
                interruptExecute(0xd7);
            } else {
                interruptExecute(0xcf);
            }
            
            interruptSwitch = !interruptSwitch;
            i8080.interrupts = false;
            

            
            // Update pixels
            draw();
            display.draw();

            glfwPollEvents();
            
            i8080.cycles = 0;
        }
    }
    

  return 0;
}
