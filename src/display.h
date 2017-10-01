#ifndef display_h
#define display_h

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <OpenGL/gl.h>
#include <OpenGL/OpenGL.h>
#include <string>
#include <array>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <map>
#include <vector>

struct Display {
    GLFWwindow *window;
    GLuint vbo;
    GLuint vao;
    std::array<GLfloat, 16> vertices = {};
    std::array<float, 224*256*3> pixels = {};
    GLuint shaderProgram, fragmentShader, vertexShader;
    int width;
    int height;
    std::string title;
    GLuint vertexBuffer;
    
    const GLchar *vertexSource = R"glsl(
#version 410 core
    in vec2 position;
    
    in vec2 texcoord;
    out vec2 Texcoord;
    
    uniform mat4 trans;
    
    void main()
    {
        Texcoord = texcoord;
        gl_Position = trans * vec4(position, 0.0, 1.0);
    }
    )glsl";
    
    const GLchar *fragmentSource = R"glsl(
#version 410 core
#define distortion 0.2
    
    in vec2 Texcoord;
    out vec4 outColor;
    uniform sampler2D tex;
    
    vec2 radialDistortion(vec2 coord) {
        vec2 cc = coord - vec2(0.5);
        float dist = dot(cc, cc) * distortion;
        return coord + cc * (1.0 - dist) * dist;
    }
    
    void main() {
        float x = floor(Texcoord.x * 256);
        float y = floor(Texcoord.y * 224);
        vec4 color = vec4(1.0, 1.0, 1.0, 1.0);
        
        if (x > (256 - 65) && x < (256 - 32)) {
            color = vec4(1.0, 0.0, 0.0, 1.0);
        } else if (x > 15 && x < (256 - 184)) {
            color = vec4(0.0, 1.0, 0.0, 1.0);
        } else if (x < 17 && (y < 122) && (y > 16)) {
            color = vec4(0.0, 1.0, 0.0, 1.0);
        }
        
        outColor = texture(tex, Texcoord) * color;
    }
    )glsl";
    
    void start();
    
    Display(int width, int height, std::string title);
    
    void window_size_callback(GLFWwindow *window, int width, int height);
    
    void draw();
    
    ~Display();
};


#endif /* display_h */
