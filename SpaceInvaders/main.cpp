#include <cstdio>
#include <cstdint>
#include <limits>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

bool game_running = false;
int move_dir = 0;
bool fire_pressed = 0;

#define GAME_MAX_BULLETS 128

#define GL_ERROR_CASE(glerror)\
  case glerror: snprintf(error, sizeof(error), "%s", #glerror)
  //the backslash '\' tells preprocessor that the macro continues on the next line
  //the C-preprocessor macro: before compiler starts, looks at code and scans file looking for GL_ERROR_CASE(glerror).
  //acts like the "find" box in a find-and-replace search, glerror is a temp placeholder variable.
  //once an instance is found, preprocessor will paste into the code.
  //e.g. in switch statement below, preprocessor reaches line GL_ERROR_CASE(GL_INVALID_ENUM), deletes the
  //  GL_ERROR_CASE(GL_INVALID_ENUM), and pastes replacement in that spot. So in this example, the compiler
  //  will see: case GL_INVALID_ENUM: snprintf(error, sizeoff(error), "%s", "GL_INVALID_ENUM"); break;
  //the '#' is the stringification operator, in macros, the # before a parameter converts that parameter
  //  into a string literal. For further info, the ## concatenates two tokens into a single token

inline void gl_debug(const char *file, int line) { 
//inline keyword: compile should paste funtions entire body into main code whenever called, not jump back in memory
//function takes text string file and int line, how does program know what file and line it's on?
//in main, the function is called via: gl_debug(__FILE__, __LINE__). Those are built-in c++ preprocessor macros.
//  the compiler automatically replaces them with a string of the current file's name (like main.cpp), and the
//  exact line where the code is written. Allows error message to pinpoint exact crash location
  GLenum err; //this is OpenGL's way of having architecture neutral unsigned int for err variable
  while((err = glGetError()) != GL_NO_ERROR){
  //OpenGL puts errors into a queue, err=glGetError() reaches into queue and pulls the oldest ticket, assigns
  //  the numeric value into the err variable.
  //in OpenGL, an error value of 0 means no error, basically a check if err != 0, keeps running till it does
    char error[128]; //new empty text array for holding each error

    switch(err) { //takes numeric err and routes to the matching GL_ERROR_CASE macro
      GL_ERROR_CASE(GL_INVALID_ENUM); break;
      GL_ERROR_CASE(GL_INVALID_VALUE); break;
      GL_ERROR_CASE(GL_INVALID_OPERATION); break;
      GL_ERROR_CASE(GL_INVALID_FRAMEBUFFER_OPERATION); break;
      GL_ERROR_CASE(GL_OUT_OF_MEMORY); break;
      default: snprintf(error, sizeof(error), "%s", "UNKNOWN_ERROR"); break;
    }

    fprintf(stderr, "%s - %s: %d\n", error, file, line);
    //in c++, terminal has 2 separate output streams: stdout (standard output), stderr (standard error)
    //  stdout is used for normal program text, often "buffered" so the computer may wait a few milliseconds
    //  to group messages before printing to save resources.
    //  stderr is used for emergencies, bypasses buffer and prints instantly to screnn. using stderr means that in the case
    //  where the game freezes or crashes, the error message will still successfully print to terminal before it dies
  }
}
#undef GL_ERROR_CASE //deletes teh macro so it does accidentally screw with other code in compilation process

void validate_shader(GLuint shader, const char *file = 0) { //acts as a "mouthpiece" for the GPU so we know when a failure occurs
//GLuint shader: the ID number of the shader complied (shader_vp or shader_fp) in main
//const char *file = 0: allows the passing of the file name where the shader is located, setting to 0
//  acts as a null pointer here and means if the function is called without a second argument it wont crash, just
//  defaults to 0.

//before asking OpenGL for error message, an empty container must be made in RAM to hold message
  static const unsigned int BUFFER_SIZE = 512; //hard limit of 512 characters
  char buffer[BUFFER_SIZE]; //creates empty string of 512 characters
  GLsizei length = 0; //creates int variable starting at 0, will be used to tally number of characters

  glGetShaderInfoLog(shader, BUFFER_SIZE, &length, buffer);
  //built-in OpenGL command, asks for the log
  //shader: "check compilation log for this specific shader ID"
  //BUFFER_SIZE: "do not write more than 512 characters, or you will overflow my memory"
  //&length: the memory address of the length variable "when you finish writing the error message,
  //  modify this variable to tell me exactly how many characters your wrote"
  //buffer: the empty array provided to OpenGL to dump text into

  if(length > 0){ //if shader compiled right, nothing will be written and length is 0. length > 0 means an error
    printf("Shader %d(%s) compile error: %s\n", shader, (file? file: ""), buffer);
    //simple print statement providing the relevant error information to the correct file
  }
}

bool validate_program(GLuint program){ //similar function to the shader validation above
  static const GLsizei BUFFER_SIZE = 512;
  GLchar buffer[BUFFER_SIZE];
  GLsizei length = 0;

  glGetProgramInfoLog(program, BUFFER_SIZE, &length, buffer);

  if(length>0){
    printf("Program %d link error: %s\n", program, buffer);
    return false;
  }

  return true;
}

void error_callback(int error, const char* description) {
  fprintf(stderr, "Error: %s\n", description);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
  switch(key) {
    case GLFW_KEY_ESCAPE:
      if(action == GLFW_PRESS) game_running = false;
      break;
    case GLFW_KEY_RIGHT:
      if(action == GLFW_PRESS) move_dir += 1;
      else if(action == GLFW_RELEASE) move_dir -= 1;
      break;
    case GLFW_KEY_LEFT:
      if(action == GLFW_PRESS) move_dir -= 1;
      else if(action == GLFW_RELEASE) move_dir += 1;
      break;
    case GLFW_KEY_SPACE:
      if(action == GLFW_RELEASE) fire_pressed = true;
      break;
    default:
      break;
  }
}

/* Algorithm "xor" from p. 4 of Marsaglia, "Xorshift RNGs" */
uint32_t xorshift32(uint32_t* rng)
{
  uint32_t x = *rng;
  x ^= x << 13;
  x ^= x >> 17;
  x ^= x << 5;
  *rng = x;
  return x;
}

double random(uint32_t* rng) {
  return (double)xorshift32(rng) / std::numeric_limits<uint32_t>::max();
}

struct Buffer {
  size_t width, height;
  uint32_t* data;
  //the data is a pointer with the value of a memory address, the uint32_t tells the compiler what to do
  //  when it reaches that memory address -> "go to this memory address, grab the next 4 bytes (32 bits) and
  //  interpreted them together as one unsigned integer."
  //on mac, 64 but computer, size_t and uint32_t takes up 8 bytes, so width, height, and data are 8 bytes
};

struct Sprite { //similar struct as the Buffer above
  size_t width, height;
  uint8_t* data; //smallest possible standard data type in c++ -> an 8 bit storage unit or 1 byte
};

struct Alien {
  size_t x, y;
  uint8_t type;
};

struct Player {
  size_t x, y;
  size_t life;
};
//for both of the above, alien and player both have positions x,y given in pixels from bottom left corner of window
//  in player struct, need to include number of lives
//  in alien struct, need to have what kind of alien it is (classic game has 3 different sprites)

struct Bullet {
  size_t x, y;
  int dir;
};

struct Game {
  size_t width, height;
  size_t num_aliens;
  size_t num_bullets;
  Alien* aliens;
  Player player;
  Bullet bullets[GAME_MAX_BULLETS];
};
//struct used to store all the relevant information for a new instance of the game

struct SpriteAnimation {
  bool loop;
  size_t num_frames;
  size_t frame_duration;
  size_t time;
  Sprite** frames;
};
//basically an array of Sprite structs, uses a pointer-to-pointer type for sprite storage so sprites can be shared

enum AlienType: uint8_t {
  ALIEN_DEAD   = 0,
  ALIEN_TYPE_A = 1,
  ALIEN_TYPE_B = 2,
  ALIEN_TYPE_C = 3
};

void buffer_clear(Buffer* buffer, uint32_t color) {
  //function takes a pointer to the buffer Buffer* buffer, pass a pointer is more efficient -> if we passed in
  //  buffer itself, the function would create a fully copy of buffer (57,344 pixels) -> a huge waste.
  //  
  for(size_t i =0; i < buffer->width * buffer->height; ++i){
    //since buffer is now a pointer and not an object, we cannot use the dot operator (buffer.width), which
    //  means we need to use the arrow operator '->' to access the width and height inside of buffer. This method is
    //  functionally equivalent to (*buffer).width * (*buffer).height
    buffer->data[i] = color;
    //go through each 8 bytes that 'data' is (since it's a uint32_t) and set it equal to the default color
    //  this essentially "clears" the screen by setting everything back to the default
  }
}

bool sprite_overlap_check(
  const Sprite& sp_a, size_t x_a, size_t y_a,
  const Sprite& sp_b, size_t x_b, size_t y_b
)
{
  // NOTE: For simplicity we just check for overlap of the sprite
  // rectangles. Instead, if the rectangles overlap, we should
  // further check if any pixel of sprite A overlap with any of
  // sprite B.
  if(x_a < x_b + sp_b.width && x_a + sp_a.width > x_b &&
    y_a < y_b + sp_b.height && y_a + sp_a.height > y_b)
  {
    return true;
  }

  return false;
}

void buffer_draw_sprite(Buffer* buffer, const Sprite& sprite, size_t x, size_t y, uint32_t color) {
//Buffer* buffer: pass pointer to the buffer (so we can "paint" it)
//const Sprite& sprite: pass sprite by reference (&) so we dont waste memory making a copy of it,
//  const promises compiler we will only read the sprite, never change it
//x & y: coordinates on screen where the bottom-left corner of alien will be
//uint32_t color: the 32-bit red color of the alien we want it to be
  for(size_t xi = 0; xi < sprite.width; ++xi) {
    for(size_t yi = 0; yi < sprite.height; ++yi) {
  //these for loops will ONLY be looping over the 11x8 grid of the sprite, which is an optimized functionality
      if(sprite.data[yi * sprite.width + xi] &&         //1
      (sprite.height - 1 + y - yi) < buffer->height &&  //2
      (x + xi) < buffer->width) {                       //3
      //the 3 conditions that must be true for a pixel to be drawn:
      //1. sprite is 2D grid 11 pixels wide, 8 pixels tall, stored in RAM as a 1D line of 88 numbers
      //   need to determine how many pixels to "skip". e.g. we want 5th pixel in 2nd row:
      //   xi=5, yi=1 => (1*11)+5 = 16th pixel. Since calcualtion is just a 0 or 1, dont need the
      //   evaulation. 0 will be false, any other number is true. 0: skip drawing and leave green, 1: draw pixel red
      //2. calculates exact vertical pixel on screen (screen_y) and ensures if hasnt gone off top or bottom edges
      //   In OpenGL, screen coordinates start with Y=0 at VERY BOTTOM of window, using something liek y+yi
      //   would draw sprite uside-down. This calculation flips sprite right-side up. e.g. we want to draw
      //   sprite at very bottom of screen (y=0), sprite height is 8. 
      //   Drawing head yi=0: 8-1+0-0 = 7 up on screen; drawing feet: yi=7: 8-1+0-7=0 down
      //   The check stops sprite from flying off top of screen (Y>256), what about bottom?
      //   In C++, variables are size_t, meaning they're UNSIGNED meaning the unsigned int will "underflow"
      //   and evaluate to the max possible value, failing the buffer->height check (<256) and evaluate false
      //3. nothing complicated like above, array and screen read X-coordinates left->right. Find exact horzontal
      //   pixel on screen (screen_x), take starting X position of sprite and add column of stencil we're look xi.
      //   Check if screen_x is less than total width of screen (buffer->width; 224)
        buffer->data[(sprite.height - 1 + y - yi) * buffer->width + (x + xi)] = color;
        //notice the massive index inside [] is just (screen_y)*width + (screen_x), same row-major math used above
        //  only scaled up to find correct pixel in the 57,344 background array
      }
    }
  }
}

void buffer_draw_number(Buffer* buffer, const Sprite& number_spritesheet, size_t number, size_t x, size_t y, uint32_t color) {
  uint8_t digits[64];
  size_t num_digits = 0;
  size_t current_number = number;
  do {
    digits[num_digits++] = current_number % 10;
    current_number = current_number / 10;
  }
  while(current_number > 0);

  size_t xp = x;
  size_t stride = number_spritesheet.width * number_spritesheet.height;
  Sprite sprite = number_spritesheet;
  for(size_t i = 0; i < num_digits; ++i)
  {
    uint8_t digit = digits[num_digits - i - 1];
    sprite.data = number_spritesheet.data + digit * stride;
    buffer_draw_sprite(buffer, sprite, xp, y, color);
    xp += sprite.width + 1;
  }
}

void buffer_draw_text(Buffer* buffer, const Sprite& text_spritesheet, const char* text, size_t x, size_t y, uint32_t color) {
  size_t xp = x;
  size_t stride = text_spritesheet.width * text_spritesheet.height;
  Sprite sprite = text_spritesheet;
  for(const char* charp = text; *charp != '\0'; ++charp) {
    char character = *charp - 32;
    if(character < 0 || character >= 65) continue;

    sprite.data = text_spritesheet.data + character * stride;
    buffer_draw_sprite(buffer, sprite, xp, y, color);
    xp += sprite.width + 1;
  }
}

uint32_t rgb_to_uint32(uint8_t r, uint8_t g, uint8_t b) {
  //goal is to take 3 8 bit colors - red, green, blue - and pack them into a 32-bit container uint32_t
  return (r << 24) | (g << 16) | (b << 8) | 255;
  //'<<' is the bitwise left shift; takes bits of a numver and pushes them left by a specific amount, fills
  //  empty space with 0's: 
  //red shift by 24 -> [rrrrrrrr] [00000000] [00000000] [00000000]
  //green shift by 16 -> [00000000] [gggggggg] [00000000] [00000000]
  //blue shift by 8 -> [00000000] [00000000] [bbbbbbb] [00000000]
  //alpha channel (255) -> [00000000] [00000000] [00000000] [11111111]
  //have 4 seprate 32 bit numbers, need to merge into one -> use the bitwise OR |
  //  combining them all will create: [rrrrrrrr] [gggggggg] [bbbbbbbb] [11111111] -> the final uint32_t
  //  this method ensures every pixel will be exactly 4 bytes of memory, then shipping the buffer.data
  //  array over to GPU, the card knows exactly how to read those 32 bits to draw the correct color on monitor
}

int main(int argc, char* argv[]) {
  const size_t buffer_width = 224;
  const size_t buffer_height = 256;

  glfwSetErrorCallback(error_callback); //hand GLFW library a custom error function defined above

  if (!glfwInit()) return -1;
  //OpenGL doesnt know what a "window" actually is, it only knows how to draw triangles.
  //  GLFW (graphics library framework) library, specifically this function, helps with creating
  //  a window with an "X" button. Failure instantly returns -1 and exits main.

  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  //essentially telling the operating system what we plan to put inside the window:
  //  OpenGL version 3.3, core profile and forward compatibility is essentially:
  //  "dont load outdated OpenGL features from 1990s, only use modern version"
  //  also happens to be required to run on MacOS

  /* Create a windowed mode window and its OpenGL context */
  GLFWwindow* window = glfwCreateWindow(buffer_width, buffer_height, "Space Invaders", NULL, NULL);
  //glfwCreateWindow makes the actually window pop open on monitor
  if(!window) { //window failure check + termination
    glfwTerminate();
    return -1;
  }
  glfwSetKeyCallback(window, key_callback);
  glfwMakeContextCurrent(window);
  //OpenGL is a giant state machine, a "context" is the official memory space for that state machine.
  //  essentially telling GPU "any openGL commands I send from now on should be drawn directly into
  //  this specific window"

  GLenum err = glewInit();
  //GLEW (OpenGL Extension Wrangler) finds the specific graphics card drivers, finds exact memory
  //  address for the modern OpenGL commans, and links them to c++ code for use
  if(err != GLEW_OK) { //GLEW failure check + termination
    fprintf(stderr, "Error initializing GLEW.\n");
    glfwTerminate();
    return -1;
  }
  
  int glVersion[2] = {-1, 1};
  glGetIntegerv(GL_MAJOR_VERSION, &glVersion[0]);
  glGetIntegerv(GL_MINOR_VERSION, &glVersion[1]);
  gl_debug(__FILE__, __LINE__);
  printf("Using OpenGL: %d.%d\n", glVersion[0], glVersion[1]);
  printf("Renderer used: %s\n", glGetString(GL_RENDERER));
  printf("Shading Language: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
  //the above block of code prints the version of OpenGL it loaded, prints the specific graphics card being used
  //  and calls the gl_debug function to make sure no silent errors occured during the massive boot-up sequence

  glfwSwapInterval(1);
  //synchs the game to the monitor refresh rate********* get more info

  glClearColor(1.0, 0.0, 0.0, 1.0);
  //a direct hardware command to GPU painting the actual window background red before the giant fullscreen
  //  triangle is pasted over it. Initially set to bright red so it is easy to find issues with the shader
  //  or the triangle

  //Create buffer
  uint32_t clear_color = rgb_to_uint32(0, 128, 0); //creates a uint32_t called clear_color and sets it to green
  Buffer buffer; //creates a Buffer object titled buffer
  buffer.width  = buffer_width; //sets buffer width to 224
  buffer.height = buffer_height; //sets buffer height to 256
  buffer.data   = new uint32_t[buffer.width * buffer.height]; 
  //dynamically allocates new memory to create an array of 224*256 32-bit integers. If we didn't use 'new' the
  //  memory would be allocated on the stack, and would cause an overflow due to the massive amount of data being
  //  requested. 'new' keyword directs allocation of RAM (the heap) to find the 57,344 uint32_t datas (4 bytes).
  //  The new keyword returns a pointer to the beginning of that chunk of memory
  
  buffer_clear(&buffer, clear_color);
  //calls buffer_clear function on the address of buffer (via &) and sets it to the green via clear_color

  // Create texture for presenting buffer to OpenGL
  GLuint buffer_texture;
  glGenTextures(1, &buffer_texture);
  glBindTexture(GL_TEXTURE_2D, buffer_texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, buffer.width, buffer.height, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, buffer.data);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);


  // Create vao for generating fullscreen triangle
  GLuint fullscreen_triangle_vao;
  glGenVertexArrays(1, &fullscreen_triangle_vao);


  // Create shader for displaying buffer
  static const char* fragment_shader =
    "\n"
    "#version 330\n"
    "\n"
    "uniform sampler2D buffer;\n"
    "noperspective in vec2 TexCoord;\n"
    "\n"
    "out vec3 outColor;\n"
    "\n"
    "void main(void){\n"
    "    outColor = texture(buffer, TexCoord).rgb;\n"
    "}\n";

  static const char* vertex_shader =
    "\n"
    "#version 330\n"
    "\n"
    "noperspective out vec2 TexCoord;\n"
    "\n"
    "void main(void){\n"
    "\n"
    "    TexCoord.x = (gl_VertexID == 2)? 2.0: 0.0;\n"
    "    TexCoord.y = (gl_VertexID == 1)? 2.0: 0.0;\n"
    "    \n"
    "    gl_Position = vec4(2.0 * TexCoord - 1.0, 0.0, 1.0);\n"
    "}\n";

  GLuint shader_id = glCreateProgram();

  {
    //Create vertex shader
    GLuint shader_vp = glCreateShader(GL_VERTEX_SHADER);

    glShaderSource(shader_vp, 1, &vertex_shader, 0);
    glCompileShader(shader_vp);
    validate_shader(shader_vp, vertex_shader);
    glAttachShader(shader_id, shader_vp);

    glDeleteShader(shader_vp);
  }

  {
    //Create fragment shader
    GLuint shader_fp = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(shader_fp, 1, &fragment_shader, 0);
    glCompileShader(shader_fp);
    validate_shader(shader_fp, fragment_shader);
    glAttachShader(shader_id, shader_fp);

    glDeleteShader(shader_fp);
  }

  glLinkProgram(shader_id);

  if(!validate_program(shader_id)){
    fprintf(stderr, "Error while validating shader.\n");
    glfwTerminate();
    glDeleteVertexArrays(1, &fullscreen_triangle_vao);
    delete[] buffer.data;
    return -1;
  }

  glUseProgram(shader_id);

  GLint location = glGetUniformLocation(shader_id, "buffer");
  glUniform1i(location, 0);


  //OpenGL setup
  glDisable(GL_DEPTH_TEST);
  glActiveTexture(GL_TEXTURE0);

  glBindVertexArray(fullscreen_triangle_vao);

  // Prepare game
  Sprite alien_sprites[6];
  alien_sprites[0].width = 8;
  alien_sprites[0].height = 8;
  alien_sprites[0].data = new uint8_t[64]
  {
    0,0,0,1,1,0,0,0, // ...@@...
    0,0,1,1,1,1,0,0, // ..@@@@..
    0,1,1,1,1,1,1,0, // .@@@@@@.
    1,1,0,1,1,0,1,1, // @@.@@.@@
    1,1,1,1,1,1,1,1, // @@@@@@@@
    0,1,0,1,1,0,1,0, // .@.@@.@.
    1,0,0,0,0,0,0,1, // @......@
    0,1,0,0,0,0,1,0  // .@....@.
  };

  alien_sprites[1].width = 8;
  alien_sprites[1].height = 8;
  alien_sprites[1].data = new uint8_t[64]
  {
    0,0,0,1,1,0,0,0, // ...@@...
    0,0,1,1,1,1,0,0, // ..@@@@..
    0,1,1,1,1,1,1,0, // .@@@@@@.
    1,1,0,1,1,0,1,1, // @@.@@.@@
    1,1,1,1,1,1,1,1, // @@@@@@@@
    0,0,1,0,0,1,0,0, // ..@..@..
    0,1,0,1,1,0,1,0, // .@.@@.@.
    1,0,1,0,0,1,0,1  // @.@..@.@
  };

  alien_sprites[2].width = 11;
  alien_sprites[2].height = 8;
  alien_sprites[2].data = new uint8_t[88]
  {
    0,0,1,0,0,0,0,0,1,0,0, // ..@.....@..
    0,0,0,1,0,0,0,1,0,0,0, // ...@...@...
    0,0,1,1,1,1,1,1,1,0,0, // ..@@@@@@@..
    0,1,1,0,1,1,1,0,1,1,0, // .@@.@@@.@@.
    1,1,1,1,1,1,1,1,1,1,1, // @@@@@@@@@@@
    1,0,1,1,1,1,1,1,1,0,1, // @.@@@@@@@.@
    1,0,1,0,0,0,0,0,1,0,1, // @.@.....@.@
    0,0,0,1,1,0,1,1,0,0,0  // ...@@.@@...
  };

  alien_sprites[3].width = 11;
  alien_sprites[3].height = 8;
  alien_sprites[3].data = new uint8_t[88]
  {
    0,0,1,0,0,0,0,0,1,0,0, // ..@.....@..
    1,0,0,1,0,0,0,1,0,0,1, // @..@...@..@
    1,0,1,1,1,1,1,1,1,0,1, // @.@@@@@@@.@
    1,1,1,0,1,1,1,0,1,1,1, // @@@.@@@.@@@
    1,1,1,1,1,1,1,1,1,1,1, // @@@@@@@@@@@
    0,1,1,1,1,1,1,1,1,1,0, // .@@@@@@@@@.
    0,0,1,0,0,0,0,0,1,0,0, // ..@.....@..
    0,1,0,0,0,0,0,0,0,1,0  // .@.......@.
  };

  alien_sprites[4].width = 12;
  alien_sprites[4].height = 8;
  alien_sprites[4].data = new uint8_t[96]
  {
    0,0,0,0,1,1,1,1,0,0,0,0, // ....@@@@....
    0,1,1,1,1,1,1,1,1,1,1,0, // .@@@@@@@@@@.
    1,1,1,1,1,1,1,1,1,1,1,1, // @@@@@@@@@@@@
    1,1,1,0,0,1,1,0,0,1,1,1, // @@@..@@..@@@
    1,1,1,1,1,1,1,1,1,1,1,1, // @@@@@@@@@@@@
    0,0,0,1,1,0,0,1,1,0,0,0, // ...@@..@@...
    0,0,1,1,0,1,1,0,1,1,0,0, // ..@@.@@.@@..
    1,1,0,0,0,0,0,0,0,0,1,1  // @@........@@
  };


  alien_sprites[5].width = 12;
  alien_sprites[5].height = 8;
  alien_sprites[5].data = new uint8_t[96]
  {
    0,0,0,0,1,1,1,1,0,0,0,0, // ....@@@@....
    0,1,1,1,1,1,1,1,1,1,1,0, // .@@@@@@@@@@.
    1,1,1,1,1,1,1,1,1,1,1,1, // @@@@@@@@@@@@
    1,1,1,0,0,1,1,0,0,1,1,1, // @@@..@@..@@@
    1,1,1,1,1,1,1,1,1,1,1,1, // @@@@@@@@@@@@
    0,0,1,1,1,0,0,1,1,1,0,0, // ..@@@..@@@..
    0,1,1,0,0,1,1,0,0,1,1,0, // .@@..@@..@@.
    0,0,1,1,0,0,0,0,1,1,0,0  // ..@@....@@..
  };

  Sprite player_sprite;
  player_sprite.width = 11;
  player_sprite.height = 7;
  player_sprite.data = new uint8_t[77] {
    0,0,0,0,0,1,0,0,0,0,0, // .....@.....
    0,0,0,0,1,1,1,0,0,0,0, // ....@@@....
    0,0,0,0,1,1,1,0,0,0,0, // ....@@@....
    0,1,1,1,1,1,1,1,1,1,0, // .@@@@@@@@@.
    1,1,1,1,1,1,1,1,1,1,1, // @@@@@@@@@@@
    1,1,1,1,1,1,1,1,1,1,1, // @@@@@@@@@@@
    1,1,1,1,1,1,1,1,1,1,1, // @@@@@@@@@@@
  };

  Sprite alien_death_sprite;
  alien_death_sprite.width = 13;
  alien_death_sprite.height = 7;
  alien_death_sprite.data = new uint8_t[91]
  {
      0,1,0,0,1,0,0,0,1,0,0,1,0, // .@..@...@..@.
      0,0,1,0,0,1,0,1,0,0,1,0,0, // ..@..@.@..@..
      0,0,0,1,0,0,0,0,0,1,0,0,0, // ...@.....@...
      1,1,0,0,0,0,0,0,0,0,0,1,1, // @@.........@@
      0,0,0,1,0,0,0,0,0,1,0,0,0, // ...@.....@...
      0,0,1,0,0,1,0,1,0,0,1,0,0, // ..@..@.@..@..
      0,1,0,0,1,0,0,0,1,0,0,1,0  // .@..@...@..@.
  };

  Sprite text_spritesheet;
  text_spritesheet.width = 5;
  text_spritesheet.height = 7;
  text_spritesheet.data = new uint8_t[65 * 35]
  {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,
    0,1,0,1,0,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,1,0,1,0,0,1,0,1,0,1,1,1,1,1,0,1,0,1,0,1,1,1,1,1,0,1,0,1,0,0,1,0,1,0,
    0,0,1,0,0,0,1,1,1,0,1,0,1,0,0,0,1,1,1,0,0,0,1,0,1,0,1,1,1,0,0,0,1,0,0,
    1,1,0,1,0,1,1,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,1,0,1,1,0,1,0,1,1,
    0,1,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,1,0,0,1,0,0,1,0,1,0,0,0,1,0,1,1,1,1,
    0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,
    1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,
    0,0,1,0,0,1,0,1,0,1,0,1,1,1,0,0,0,1,0,0,0,1,1,1,0,1,0,1,0,1,0,0,1,0,0,
    0,0,0,0,0,0,0,1,0,0,0,0,1,0,0,1,1,1,1,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,
    0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,

    0,1,1,1,0,1,0,0,0,1,1,0,0,1,1,1,0,1,0,1,1,1,0,0,1,1,0,0,0,1,0,1,1,1,0,
    0,0,1,0,0,0,1,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,1,1,1,0,
    0,1,1,1,0,1,0,0,0,1,0,0,0,0,1,0,0,1,1,0,0,1,0,0,0,1,0,0,0,0,1,1,1,1,1,
    1,1,1,1,1,0,0,0,0,1,0,0,0,1,0,0,0,1,1,0,0,0,0,0,1,1,0,0,0,1,0,1,1,1,0,
    0,0,0,1,0,0,0,1,1,0,0,1,0,1,0,1,0,0,1,0,1,1,1,1,1,0,0,0,1,0,0,0,0,1,0,
    1,1,1,1,1,1,0,0,0,0,1,1,1,1,0,0,0,0,0,1,0,0,0,0,1,1,0,0,0,1,0,1,1,1,0,
    0,1,1,1,0,1,0,0,0,1,1,0,0,0,0,1,1,1,1,0,1,0,0,0,1,1,0,0,0,1,0,1,1,1,0,
    1,1,1,1,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,
    0,1,1,1,0,1,0,0,0,1,1,0,0,0,1,0,1,1,1,0,1,0,0,0,1,1,0,0,0,1,0,1,1,1,0,
    0,1,1,1,0,1,0,0,0,1,1,0,0,0,1,0,1,1,1,1,0,0,0,0,1,1,0,0,0,1,0,1,1,1,0,

    0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,0,0,
    0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,
    0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,
    1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,
    0,1,1,1,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,
    0,1,1,1,0,1,0,0,0,1,1,0,1,0,1,1,1,0,1,1,1,0,1,0,0,1,0,0,0,1,0,1,1,1,0,

    0,0,1,0,0,0,1,0,1,0,1,0,0,0,1,1,0,0,0,1,1,1,1,1,1,1,0,0,0,1,1,0,0,0,1,
    1,1,1,1,0,1,0,0,0,1,1,0,0,0,1,1,1,1,1,0,1,0,0,0,1,1,0,0,0,1,1,1,1,1,0,
    0,1,1,1,0,1,0,0,0,1,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,1,0,1,1,1,0,
    1,1,1,1,0,1,0,0,0,1,1,0,0,0,1,1,0,0,0,1,1,0,0,0,1,1,0,0,0,1,1,1,1,1,0,
    1,1,1,1,1,1,0,0,0,0,1,0,0,0,0,1,1,1,1,0,1,0,0,0,0,1,0,0,0,0,1,1,1,1,1,
    1,1,1,1,1,1,0,0,0,0,1,0,0,0,0,1,1,1,1,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,
    0,1,1,1,0,1,0,0,0,1,1,0,0,0,0,1,0,1,1,1,1,0,0,0,1,1,0,0,0,1,0,1,1,1,0,
    1,0,0,0,1,1,0,0,0,1,1,0,0,0,1,1,1,1,1,1,1,0,0,0,1,1,0,0,0,1,1,0,0,0,1,
    0,1,1,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,1,1,1,0,
    0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,1,0,0,0,1,0,1,1,1,0,
    1,0,0,0,1,1,0,0,1,0,1,0,1,0,0,1,1,0,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,0,1,
    1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,1,1,1,1,
    1,0,0,0,1,1,1,0,1,1,1,0,1,0,1,1,0,1,0,1,1,0,0,0,1,1,0,0,0,1,1,0,0,0,1,
    1,0,0,0,1,1,0,0,0,1,1,1,0,0,1,1,0,1,0,1,1,0,0,1,1,1,0,0,0,1,1,0,0,0,1,
    0,1,1,1,0,1,0,0,0,1,1,0,0,0,1,1,0,0,0,1,1,0,0,0,1,1,0,0,0,1,0,1,1,1,0,
    1,1,1,1,0,1,0,0,0,1,1,0,0,0,1,1,1,1,1,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,
    0,1,1,1,0,1,0,0,0,1,1,0,0,0,1,1,0,0,0,1,1,0,1,0,1,1,0,0,1,1,0,1,1,1,1,
    1,1,1,1,0,1,0,0,0,1,1,0,0,0,1,1,1,1,1,0,1,0,1,0,0,1,0,0,1,0,1,0,0,0,1,
    0,1,1,1,0,1,0,0,0,1,1,0,0,0,0,0,1,1,1,0,1,0,0,0,1,0,0,0,0,1,0,1,1,1,0,
    1,1,1,1,1,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,
    1,0,0,0,1,1,0,0,0,1,1,0,0,0,1,1,0,0,0,1,1,0,0,0,1,1,0,0,0,1,0,1,1,1,0,
    1,0,0,0,1,1,0,0,0,1,1,0,0,0,1,1,0,0,0,1,1,0,0,0,1,0,1,0,1,0,0,0,1,0,0,
    1,0,0,0,1,1,0,0,0,1,1,0,0,0,1,1,0,1,0,1,1,0,1,0,1,1,1,0,1,1,1,0,0,0,1,
    1,0,0,0,1,1,0,0,0,1,0,1,0,1,0,0,0,1,0,0,0,1,0,1,0,1,0,0,0,1,1,0,0,0,1,
    1,0,0,0,1,1,0,0,0,1,0,1,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,
    1,1,1,1,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,1,1,1,1,

    0,0,0,1,1,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,1,
    0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,
    1,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,1,1,0,0,0,
    0,0,1,0,0,0,1,0,1,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,
    0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
  };

  Sprite number_spritesheet = text_spritesheet;
  number_spritesheet.data += 16 * 35;

  Sprite player_bullet_sprite;
  player_bullet_sprite.width = 1;
  player_bullet_sprite.height = 3;
  player_bullet_sprite.data = new uint8_t[3]
  {
    1, 1, 1
  };

  Sprite alien_bullet_sprite[2];
  alien_bullet_sprite[0].width = 3;
  alien_bullet_sprite[0].height = 7;
  alien_bullet_sprite[0].data = new uint8_t[21]
  {
    0,1,0,1,0,0,0,1,0,0,0,1,0,1,0,1,0,0,0,1,0,
  };

  alien_bullet_sprite[1].width = 3;
  alien_bullet_sprite[1].height = 7;
  alien_bullet_sprite[1].data = new uint8_t[21]
  {
    0,1,0,0,0,1,0,1,0,1,0,0,0,1,0,0,0,1,0,1,0,
  };

  SpriteAnimation alien_bullet_animation;
  alien_bullet_animation.loop = true;
  alien_bullet_animation.num_frames = 2;
  alien_bullet_animation.frame_duration = 5;
  alien_bullet_animation.time = 0;

  alien_bullet_animation.frames = new Sprite*[2];
  alien_bullet_animation.frames[0] = &alien_bullet_sprite[0];
  alien_bullet_animation.frames[1] = &alien_bullet_sprite[1];

  SpriteAnimation alien_animation[3];

  size_t alien_update_frequency = 120;

  for(size_t i = 0; i < 3; ++i) {
    alien_animation[i].loop = true;
    alien_animation[i].num_frames = 2;
    alien_animation[i].frame_duration = 10;
    alien_animation[i].time = 0;

    alien_animation[i].frames = new Sprite*[2];
    alien_animation[i].frames[0] = &alien_sprites[2 * i];
    alien_animation[i].frames[1] = &alien_sprites[2 * i + 1];
  }

  Game game;
  game.width = buffer_width;
  game.height = buffer_height;
  game.num_bullets = 0;
  game.num_aliens = 55;
  game.aliens = new Alien[game.num_aliens];

  game.player.x = 112 - 5;
  game.player.y = 32;

  game.player.life = 3;

  size_t alien_swarm_position = 24;
  size_t alien_swarm_max_position = game.width - 16 * 11 - 3;

  size_t aliens_killed = 0;
  size_t alien_update_timer = 0;
  bool should_change_speed = false;

  for(size_t xi = 0; xi < 11; ++xi)
  {
      for(size_t yi = 0; yi < 5; ++yi)
      {
          Alien& alien = game.aliens[xi * 5 + yi];
          alien.type = (5 - yi) / 2 + 1;

          const Sprite& sprite = alien_sprites[2 * (alien.type - 1)];

          alien.x = 16 * xi + alien_swarm_position + (alien_death_sprite.width - sprite.width)/2;
          alien.y = 17 * yi + 128;
      }
  }

  uint8_t* death_counters = new uint8_t[game.num_aliens];
  for(size_t i = 0; i < game.num_aliens; ++i)
  {
      death_counters[i] = 10;
  }

  //uint32_t clear_color = rgb_to_uint32(0, 128, 0);
  uint32_t rng = 13;

  int alien_move_dir = 4;

  size_t score = 0;
  size_t credits = 0;

  game_running = true;

  int player_move_dir = 0;
  while (!glfwWindowShouldClose(window) && game_running) {
    buffer_clear(&buffer, clear_color);


    if(game.player.life == 0)
    {

        buffer_draw_text(&buffer, text_spritesheet, "GAME OVER", game.width / 2 - 30, game.height / 2, rgb_to_uint32(128, 0, 0));
        buffer_draw_text(&buffer, text_spritesheet, "SCORE", 4, game.height - text_spritesheet.height - 7, rgb_to_uint32(128, 0, 0));
        buffer_draw_number(&buffer, number_spritesheet, score, 4 + 2 * number_spritesheet.width, game.height - 2 * number_spritesheet.height - 12, rgb_to_uint32(128, 0, 0));

        glTexSubImage2D(
            GL_TEXTURE_2D, 0, 0, 0,
            buffer.width, buffer.height,
            GL_RGBA, GL_UNSIGNED_INT_8_8_8_8,
            buffer.data
        );
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        glfwSwapBuffers(window);
        glfwPollEvents();
        continue;
    }

    // Draw
    buffer_draw_text(&buffer, text_spritesheet, "SCORE", 4, game.height - text_spritesheet.height - 7, rgb_to_uint32(128, 0, 0));
    buffer_draw_number(&buffer, number_spritesheet, score, 4 + 2 * number_spritesheet.width, game.height - 2 * number_spritesheet.height - 12, rgb_to_uint32(128, 0, 0));

    {
        char credit_text[16];
        snprintf(credit_text, sizeof(credit_text), "CREDIT %02lu", credits);
        buffer_draw_text(&buffer, text_spritesheet, credit_text, 164, 7, rgb_to_uint32(128, 0, 0));
    }


    buffer_draw_number(&buffer, number_spritesheet, game.player.life, 4, 7, rgb_to_uint32(128, 0, 0));
    size_t xp =  11 + number_spritesheet.width;
    for(size_t i = 0; i < game.player.life - 1; ++i)
    {
        buffer_draw_sprite(&buffer, player_sprite, xp, 7, rgb_to_uint32(128, 0, 0));
        xp += player_sprite.width + 2;
    }

    for(size_t i = 0; i < game.width; ++i)
    {
        buffer.data[game.width * 16 + i] = rgb_to_uint32(128, 0, 0);
    }


    for(size_t ai = 0; ai < game.num_aliens; ++ai)
    {
        if(death_counters[ai] == 0) continue;

        const Alien& alien = game.aliens[ai];
        if(alien.type == ALIEN_DEAD)
        {
            buffer_draw_sprite(&buffer, alien_death_sprite, alien.x, alien.y, rgb_to_uint32(128, 0, 0));
        }
        else
        {
            const SpriteAnimation& animation = alien_animation[alien.type - 1];
            size_t current_frame = animation.time / animation.frame_duration;
            const Sprite& sprite = *animation.frames[current_frame];
            buffer_draw_sprite(&buffer, sprite, alien.x, alien.y, rgb_to_uint32(128, 0, 0));
        }
    }

    for(size_t bi = 0; bi < game.num_bullets; ++bi)
    {
        const Bullet& bullet = game.bullets[bi];
        const Sprite* sprite;
        if(bullet.dir > 0) sprite = &player_bullet_sprite;
        else
        {
            size_t cf = alien_bullet_animation.time / alien_bullet_animation.frame_duration;
            sprite = &alien_bullet_sprite[cf];
        }
        buffer_draw_sprite(&buffer, *sprite, bullet.x, bullet.y, rgb_to_uint32(128, 0, 0));
    }

    buffer_draw_sprite(&buffer, player_sprite, game.player.x, game.player.y, rgb_to_uint32(128, 0, 0));

    glTexSubImage2D(
        GL_TEXTURE_2D, 0, 0, 0,
        buffer.width, buffer.height,
        GL_RGBA, GL_UNSIGNED_INT_8_8_8_8,
        buffer.data
    );
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glfwSwapBuffers(window);

    // Simulate bullets
    for(size_t bi = 0; bi < game.num_bullets; ++bi)
    {
        game.bullets[bi].y += game.bullets[bi].dir;
        if(game.bullets[bi].y >= game.height || game.bullets[bi].y < player_bullet_sprite.height)
        {
            game.bullets[bi] = game.bullets[game.num_bullets - 1];
            --game.num_bullets;
            continue;
        }

        // Alien bullet
        if(game.bullets[bi].dir < 0)
        {
            bool overlap = sprite_overlap_check(
                alien_bullet_sprite[0], game.bullets[bi].x, game.bullets[bi].y,
                player_sprite, game.player.x, game.player.y
            );

            if(overlap)
            {
                --game.player.life;
                game.bullets[bi] = game.bullets[game.num_bullets - 1];
                --game.num_bullets;
                //NOTE: The rest of the frame is still going to be simulated.
                //perhaps we need to check if the game is over or not.
                break;
            }
        }
        // Player bullet
        else
        {
            // Check if player bullet hits an alien bullet
            for(size_t bj = 0; bj < game.num_bullets; ++bj)
            {
                if(bi == bj) continue;

                bool overlap = sprite_overlap_check(
                    player_bullet_sprite, game.bullets[bi].x, game.bullets[bi].y,
                    alien_bullet_sprite[0], game.bullets[bj].x, game.bullets[bj].y
                );

                if(overlap)
                {
                    // NOTE: Make sure it works.
                    if(bj == game.num_bullets - 1)
                    {
                        game.bullets[bi] = game.bullets[game.num_bullets - 2];
                    }
                    else if(bi == game.num_bullets - 1)
                    {
                        game.bullets[bj] = game.bullets[game.num_bullets - 2];
                    }
                    else
                    {
                        game.bullets[(bi < bj)? bi: bj] = game.bullets[game.num_bullets - 1];
                        game.bullets[(bi < bj)? bj: bi] = game.bullets[game.num_bullets - 2];
                    }
                    game.num_bullets -= 2;
                    break;
                }
            }

            // Check hit
            for(size_t ai = 0; ai < game.num_aliens; ++ai)
            {
                const Alien& alien = game.aliens[ai];
                if(alien.type == ALIEN_DEAD) continue;

                const SpriteAnimation& animation = alien_animation[alien.type - 1];
                size_t current_frame = animation.time / animation.frame_duration;
                const Sprite& alien_sprite = *animation.frames[current_frame];
                bool overlap = sprite_overlap_check(
                    player_bullet_sprite, game.bullets[bi].x, game.bullets[bi].y,
                    alien_sprite, alien.x, alien.y
                );

                if(overlap)
                {
                    score += 10 * (4 - game.aliens[ai].type);
                    game.aliens[ai].type = ALIEN_DEAD;
                    // NOTE: Hack to recenter death sprite
                    game.aliens[ai].x -= (alien_death_sprite.width - alien_sprite.width)/2;
                    game.bullets[bi] = game.bullets[game.num_bullets - 1];
                    --game.num_bullets;
                    ++aliens_killed;

                    if(aliens_killed % 15 == 0) should_change_speed = true;

                    break;
                }
            }
        }
    }

    // Simulate aliens
    if(should_change_speed)
    {
        should_change_speed = false;
        alien_update_frequency /= 2;
        for(size_t i = 0; i < 3; ++i)
        {
            alien_animation[i].frame_duration = alien_update_frequency;
        }
    }

    // Update death counters
    for(size_t ai = 0; ai < game.num_aliens; ++ai)
    {
        const Alien& alien = game.aliens[ai];
        if(alien.type == ALIEN_DEAD && death_counters[ai])
        {
            --death_counters[ai];
        }
    }

    if(alien_update_timer >= alien_update_frequency)
    {
        alien_update_timer = 0;

        if((int)alien_swarm_position + alien_move_dir < 0)
        {
            alien_move_dir *= -1;
            //TODO: Perhaps if aliens get close enough to player, we need to check
            //for overlap. What happens when alien moves over line y = 0 line?
            for(size_t ai = 0; ai < game.num_aliens; ++ai)
            {
                Alien& alien = game.aliens[ai];
                alien.y -= 8;
            }
        }
        else if(alien_swarm_position > alien_swarm_max_position - alien_move_dir)
        {
            alien_move_dir *= -1;
        }
        alien_swarm_position += alien_move_dir;

        for(size_t ai = 0; ai < game.num_aliens; ++ai)
        {
            Alien& alien = game.aliens[ai];
            alien.x += alien_move_dir;
        }

        if(aliens_killed < game.num_aliens)
        {
            size_t rai = game.num_aliens * random(&rng);
            while(game.aliens[rai].type == ALIEN_DEAD)
            {
                rai = game.num_aliens * random(&rng);
            }
            const Sprite& alien_sprite = *alien_animation[game.aliens[rai].type - 1].frames[0];
            game.bullets[game.num_bullets].x = game.aliens[rai].x + alien_sprite.width / 2;
            game.bullets[game.num_bullets].y = game.aliens[rai].y - alien_bullet_sprite[0].height;
            game.bullets[game.num_bullets].dir = -2;
            ++game.num_bullets;
        }
    }

    // Update animations
    for(size_t i = 0; i < 3; ++i)
    {
        ++alien_animation[i].time;
        if(alien_animation[i].time >= alien_animation[i].num_frames * alien_animation[i].frame_duration)
        {
            alien_animation[i].time = 0;
        }
    }
    ++alien_bullet_animation.time;
    if(alien_bullet_animation.time >= alien_bullet_animation.num_frames * alien_bullet_animation.frame_duration)
    {
        alien_bullet_animation.time = 0;
    }

    ++alien_update_timer;

    // Simulate player
    player_move_dir = 2 * move_dir;

    if(player_move_dir != 0)
    {
        if(game.player.x + player_sprite.width + player_move_dir >= game.width)
        {
            game.player.x = game.width - player_sprite.width;
        }
        else if((int)game.player.x + player_move_dir <= 0)
        {
            game.player.x = 0;
        }
        else game.player.x += player_move_dir;
    }

    if(aliens_killed < game.num_aliens)
    {
        size_t ai = 0;
        while(game.aliens[ai].type == ALIEN_DEAD) ++ai;
        const Sprite& sprite = alien_sprites[2 * (game.aliens[ai].type - 1)];
        size_t pos = game.aliens[ai].x - (alien_death_sprite.width - sprite.width)/2;
        if(pos > alien_swarm_position) alien_swarm_position = pos;

        ai = game.num_aliens - 1;
        while(game.aliens[ai].type == ALIEN_DEAD) --ai;
        pos = game.width - game.aliens[ai].x - 13 + pos;
        if(pos > alien_swarm_max_position) alien_swarm_max_position = pos;
    }
    else
    {
        alien_update_frequency = 120;
        alien_swarm_position = 24;

        aliens_killed = 0;
        alien_update_timer = 0;

        alien_move_dir = 4;

        for(size_t xi = 0; xi < 11; ++xi)
        {
            for(size_t yi = 0; yi < 5; ++yi)
            {
                size_t ai = xi * 5 + yi;

                death_counters[ai] = 10;

                Alien& alien = game.aliens[ai];
                alien.type = (5 - yi) / 2 + 1;

                const Sprite& sprite = alien_sprites[2 * (alien.type - 1)];

                alien.x = 16 * xi + alien_swarm_position + (alien_death_sprite.width - sprite.width)/2;
                alien.y = 17 * yi + 128;
            }
        }
    }

    // Process events
    if(fire_pressed && game.num_bullets < GAME_MAX_BULLETS)
    {
        game.bullets[game.num_bullets].x = game.player.x + player_sprite.width / 2;
        game.bullets[game.num_bullets].y = game.player.y + player_sprite.height;
        game.bullets[game.num_bullets].dir = 2;
        ++game.num_bullets;
    }
    fire_pressed = false;

    glfwPollEvents();
  }

  glfwDestroyWindow(window);
  glfwTerminate();

  glDeleteVertexArrays(1, &fullscreen_triangle_vao);

  for(size_t i = 0; i < 6; ++i)
  {
      delete[] alien_sprites[i].data;
  }

  delete[] text_spritesheet.data;
  delete[] alien_death_sprite.data;
  delete[] player_bullet_sprite.data;
  delete[] alien_bullet_sprite[0].data;
  delete[] alien_bullet_sprite[1].data;
  delete[] alien_bullet_animation.frames;

  for(size_t i = 0; i < 3; ++i)
  {
      delete[] alien_animation[i].frames;
  }
  delete[] buffer.data;
  delete[] game.aliens;
  delete[] death_counters;

  return 0;
}