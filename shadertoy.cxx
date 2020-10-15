#define GL_GLEXT_PROTOTYPES
#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#include <cstdio>
#include <cstdlib>
#include <complex>
#include <memory>
#include <vector>

std::vector<char> read_file(const char* filename) {
  FILE* f = fopen(filename, "r");

  // Should really use stat here.
  fseek(f, 0, SEEK_END);
  size_t length = ftell(f);
  fseek(f, 0, SEEK_SET);

  std::vector<char> v;
  v.resize(length);

  // Read the data.
  fread(v.data(), sizeof(char), length, f);

  // Close the file.
  fclose(f);

  // Return the file.
  return v;
}

void create_shader(GLuint shader, const char* filename, const char* name) {
  printf("Loading shader %s from %s\n", name, filename);
  auto data = read_file(filename);
  glShaderBinary(1, &shader, GL_SHADER_BINARY_FORMAT_SPIR_V_ARB, data.data(),
    data.size());
  glSpecializeShader(shader, name, 0, nullptr, nullptr);
}

struct modulation_t {
  float Zoom = 3;
  float LineWeight = 4.3;
  bool InvertColors = false;   

  float Sharpness = .2;
  
  float StarRotationSpeed = -.5;
  float StarSize = 1.8;
  int StarPoints = 3;
  float StarWeight = 4;
  
  float WaveSpacing = .3;
  float WaveAmp = .4;
  float WaveFreq = 25;
  float PhaseSpeed = .33;
  
  float WaveAmpOffset = .01;

  float Tint[3] { 1, .5, .4 };
};

struct bands_t {
  float Zoom = 1;
  float BandSpacing = .05;
  float LineSize = .008;
  float SegmentLength = .3;
  bool Warp = true;
};

int main() {
  glfwInit();
  gl3wInit();
  
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6); 
  glfwWindowHint(GLFW_STENCIL_BITS, 8);
  glfwWindowHint(GLFW_SAMPLES, 4); // HQ 4x multisample.
  glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);

  GLFWwindow* window = glfwCreateWindow(800, 800, "Circle does Shadertoy", 
    NULL, NULL);
  if(!window) {
    printf("Cannot create GLFW window\n");
    exit(1);
  }

  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);

  const float vertices[][2] { { -1, 1 }, { 1, 1 }, { -1, -1 }, { 1, -1 } };

  // Load into an array object.
  GLuint array_buffer;
  glCreateBuffers(1, &array_buffer);
  glNamedBufferStorage(array_buffer, sizeof(vertices), vertices, 0);

  // Declare a vertex array object and bind the array buffer.
  GLuint vao;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, array_buffer);

  // Bind to slot 0
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(0);

  // Load the common vertex shader.
  GLuint vs = glCreateShader(GL_VERTEX_SHADER);
  create_shader(vs, "vertex.spv", "_Z9vert_mainv");

  // Load the fragment shader.
  const char* module;
  const char* shader;

  #ifdef LINKED
    module = "linked.spv";
  #elif defined MODULATION
    module = "modulation.spv";
  #elif defined BANDS
    module = "bands.spv";
  #else
    #error "Must define MODULATION, BANDS or LINKED"
  #endif

  #if defined MODULATION
    typedef modulation_t shader_t;
    shader = "_Z9frag_mainI12modulation_tEvv";
  #else
    typedef bands_t shader_t;
    shader = "_Z9frag_mainI13hypno_bands_tEvv";    
  #endif

  // Create vertex and fragment shader handles.
  GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
  create_shader(fs, module, shader);

  // Link the shaders into a program.
  GLuint program = glCreateProgram();
  glAttachShader(program, vs);
  glAttachShader(program, fs);
  glLinkProgram(program);

  // Create the UBO.
  shader_t uniforms { };
  GLuint ubo;
  glCreateBuffers(1, &ubo);
  glNamedBufferStorage(ubo, sizeof(shader_t), &uniforms, 
    GL_DYNAMIC_STORAGE_BIT);

  while(!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    // Set the shadertoy uniforms.
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    glViewport(0, 0, width, height);

    // Bind and execute the input program.
    glUseProgram(program);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo);

    glBindVertexArray(vao);

    glUniform2f(0, width, height);
    glUniform1f(1, glfwGetTime());

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    // Swap buffers.
    glfwSwapBuffers(window);
  }


  return 0;
}
