#include <stdlib.h>
#include <stdio.h>
#include <iostream>

#include <GLES3/gl32.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#define PROGRAM_NAME "Hello Triangle"


// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

const char *vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "}\0";
const char *fragmentShaderSource = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "void main()\n"
    "{\n"
    "   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
    "}\n\0";

/* A simple function that prints a message, the error code returned by SDL, and quits the application */
void sdldie(const char *msg)
{
    printf("%s: %s\n", msg, SDL_GetError());
    SDL_Quit();
    exit(1);
}

void setupwindow(SDL_Window* & window, SDL_GLContext & context)
{
    // Declare display mode structure to be filled in.
    SDL_DisplayMode current;

    // This sets up the video subsystem -- some examples ignore it, but I left it in
    // because we used it in our first SDL exercise
    printf("Initializing video subsystem.\n");
    if (SDL_Init(SDL_INIT_VIDEO) < 0) /* Initialize SDL's Video subsystem */
       sdldie("Unable to initialize SDL"); /* Or die on error */

    // Get current display mode of all displays.
    // I am only doing this to check on multiple displays and their resolutions
    printf("Checking video modes.\n");
    for(int i = 0; i < SDL_GetNumVideoDisplays(); ++i){

        int should_be_zero = SDL_GetCurrentDisplayMode(i, &current);

        if(should_be_zero != 0)
            // In case of error...
            SDL_Log("Could not get display mode for video display #%d: %s", i, SDL_GetError());

        else
            // On success, print the current display mode.
            SDL_Log("Display #%d: current display mode is %dx%dpx @ %dhz.", i, current.w, current.h, current.refresh_rate);
    }

    /* Request an opengl 3.2 context.
     * SDL doesn't have the ability to choose which profile at this time of writing,
     * but it should default to the core profile */
    printf("Setting OpenGL version context.\n");
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

    /* Turn on double buffering with a 24bit Z buffer.
     * You may need to change this to 16 or 32 for your system */
    printf("Setting up OpenGL double buffering and depth buffer.\n");
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    /* Create our window centered at 512x512 resolution */
    printf("Creating the Window - prep for centered OpenGL Window.\n");
    window = SDL_CreateWindow(PROGRAM_NAME, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        SCR_WIDTH, SCR_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (!window) /* Die if creation failed */
        sdldie("Unable to create window");



    /* Create our opengl context and attach it to our window */
    printf("Get the OpenGL window context.\n");
    context = SDL_GL_CreateContext(window);



    printf("Synchronize with the display V-Sync.\n");
    /* This makes our buffer swap syncronized with the monitor's vertical refresh */
    SDL_GL_SetSwapInterval(1);


    printf("Finished setupwindow\n");
}

void setup_opengl(unsigned int & shaderProgram, unsigned int & VAO)
{
  // build and compile our shader program
  // ------------------------------------
  // vertex shader
  unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
  glCompileShader(vertexShader);
  // check for shader compile errors
  int success;
  char infoLog[512];
  glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
  if (!success)
  {
      glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
      std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
  }
  // fragment shader
  unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
  glCompileShader(fragmentShader);
  // check for shader compile errors
  glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
  if (!success)
  {
      glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
      std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
  }
  // link shaders
  shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);
  glLinkProgram(shaderProgram);
  // check for linking errors
  glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
  if (!success) {
      glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
      std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
  }
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);

  // set up vertex data (and buffer(s)) and configure vertex attributes
  // ------------------------------------------------------------------
  float vertices[] = {
      -0.5f, -0.5f, 0.0f, // left
       0.5f, -0.5f, 0.0f, // right
       0.0f,  0.5f, 0.0f  // top
  };

  unsigned int VBO;
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
  glBindVertexArray(VAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
  // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
  glBindVertexArray(0);

}

void quit_program( int code )
{
    SDL_Quit( );
    exit( code );
}

void handle_key_down( SDL_Keysym* keysym )
{
  //handler for key presses
    switch( keysym->sym )
    {
      case SDLK_ESCAPE: //quit
          quit_program( 0 );
          break;
      default:
          break;
    }

}

void process_events( void )
{
    /* Our SDL event placeholder. */
    SDL_Event event;

    /* Grab all the events off the queue. */
    while( SDL_PollEvent( &event ) ) {

        switch( event.type ) {
        case SDL_KEYDOWN:
            /* Handle key presses. */
            handle_key_down( &event.key.keysym );
            break;
        case SDL_QUIT:
            /* Handle quit requests (like Ctrl-c). */
            quit_program( 0 );
            break;
        }
    }
}


void destroywindow(SDL_Window* & window, SDL_GLContext & context)
{
    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

/* Our program's entry point */
int main()
{
    SDL_Window* mainwindow; /* Our window handle */
    SDL_GLContext maincontext; /* Our opengl context handle */

    /* Create our window, opengl context, etc... */
    setupwindow(mainwindow, maincontext);

    unsigned int shaderProgram, VAO; 
    setup_opengl(shaderProgram, VAO);

    while(1)
    {
      // input
      // -----
      process_events();

      // render
      // ------
      glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT);

      // draw our first triangle
      glUseProgram(shaderProgram);
      glBindVertexArray(VAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
      glDrawArrays(GL_TRIANGLES, 0, 3);
      // glBindVertexArray(0); // no need to unbind it every time


      SDL_GL_SwapWindow( mainwindow );
    }

    /* Delete our opengl context, destroy our window, and shutdown SDL */
    destroywindow(mainwindow, maincontext);

    return 0;
}
