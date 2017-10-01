#include "./display.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>
#include <array>
#include <cstdio>
#include <cstdlib>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <map>
#include <string>
#include <vector>

void Display::start() {
  glfwInit();

  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

  window = glfwCreateWindow(224, 256, "Space Invaders", nullptr, nullptr);
    if (window == NULL) {
        std::cout << "Error" << std::endl;
    }
    
  glfwSetWindowAspectRatio(window, width, height);
  glfwMakeContextCurrent(window);

  glewExperimental = GL_TRUE;
  glewInit();

  glGenBuffers(1, &vertexBuffer);

  if (Display::vertexBuffer != 1) {
    std::cout << " Error: couldn't create a vertex buffer" << std::endl;
  }

  // glfwSetWindowSizeCallback(window, *window_size_callback);

  // Create Vertex Array Object
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  // Create a Vertex Buffer Object and copy the vertex data to it
  glGenBuffers(1, &vbo);

  vertices = {
      -1.0f, 1.0f,  0.0f, 0.0f, // top left (before rotation)
      1.0f,  1.0f,  1.0f, 0.0f, // top right
      1.0f,  -1.0f, 1.0f, 1.0f, // bottom right
      -1.0f, -1.0f, 0.0f, 1.0f  // bottom left
  };

  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float),
               vertices.data(), GL_DYNAMIC_DRAW);

  // Create an element buffer to reuse verteces
  GLuint ebo;
  glGenBuffers(1, &ebo);

  std::array<GLuint, 6> elements = {
      0, 1, 2, // top right triangle
      2, 3, 0  // bottom left triangle
  };

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, elements.size() * sizeof(float),
               elements.data(), GL_DYNAMIC_DRAW);

  // Texture
  GLuint tex;
  glGenTextures(1, &tex);
  glBindTexture(GL_TEXTURE_2D, tex);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  // glGenerateMipmap(GL_TEXTURE_2D);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 256, 224, 0, GL_RGB, GL_FLOAT,
               pixels.data());

  // Create and compile the vertex shader
  vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &vertexSource, 0);
  glCompileShader(vertexShader);

  // Create and compile the fragment shader
  fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &fragmentSource, 0);
  glCompileShader(fragmentShader);

  // Link the vertex and fragment shader into a shader program
  shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);
  glBindFragDataLocation(shaderProgram, 0, "outColor");
  glLinkProgram(shaderProgram);
  glUseProgram(shaderProgram);

  // Specify the layout of the vertex data
  GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
  glEnableVertexAttribArray(posAttrib);
  glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);

  // Layout of texture coordinates in vertex data
  GLint texAttrib = glGetAttribLocation(shaderProgram, "texcoord");
  glEnableVertexAttribArray(texAttrib);
  glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                        (void *)(2 * sizeof(float)));

  // Transofrmation to rotate vertices
  // The space invaders screen is rotated 90º
  glm::mat4 trans;
  trans = glm::rotate(trans, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

  GLint uniTrans = glGetUniformLocation(shaderProgram, "trans");
  glUniformMatrix4fv(uniTrans, 1, GL_FALSE, glm::value_ptr(trans));
}

Display::Display(int width, int height, std::string title) {
  width = width;
  height = height;
  title = title;
}

void Display::window_size_callback(GLFWwindow *window, int width, int height) {
  // Modify height

  if (width > height) { // Squish width
    float w = 224.0f / 256.0f * height;
    float remaining =
        1 - ((static_cast<float>(width) - w) / static_cast<float>(width));

    vertices = {
        -1.0f, remaining,  0.0f, 0.0f, // top left
        1.0f,  remaining,  1.0f, 0.0f, // top right
        1.0f,  -remaining, 1.0f, 1.0f, // bottom right
        -1.0f, -remaining, 0.0f, 1.0f  // bottom left
    };

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float),
                 vertices.data(), GL_DYNAMIC_DRAW);

  } else { // Modify width
    float h = 256.0f / 224.0f * width;
    float remaining =
        1 - ((static_cast<float>(height) - h) / static_cast<float>(height));

    vertices = {
        -remaining, 1.0f,  0.0f, 0.0f, // top left
        remaining,  1.0f,  1.0f, 0.0f, // top right
        remaining,  -1.0f, 1.0f, 1.0f, // bottom right
        -remaining, -1.0f, 0.0f, 1.0f  // bottom left
    };

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float),
                 vertices.data(), GL_DYNAMIC_DRAW);
  }
}

void Display::draw() {
  // Clear the screen to black
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  // Draw triangles
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 256, 224, 0, GL_RGB, GL_FLOAT,
               pixels.data());

  // Swap back and front buffers
  glfwSwapBuffers(window);
}

Display::~Display() {
  glDeleteProgram(shaderProgram);
  glDeleteShader(fragmentShader);
  glDeleteShader(vertexShader);

  glDeleteBuffers(1, &vbo);

  glDeleteVertexArrays(1, &vao);
  glfwTerminate();
}
