#include <emscripten.h>
#include <emscripten/html5.h>
#include <GLES2/gl2.h>
#include <cmath>
#include <cstdio>

// Vertex Shader (GLSL)
static const char* vertexShaderSource = R"(
    attribute vec2 a_position;
    uniform float u_angle;

    void main() {
        // Apply a simple 2D rotation around the origin
        float cosAngle = cos(u_angle);
        float sinAngle = sin(u_angle);
        vec2 rotatedPos = vec2(
            a_position.x * cosAngle - a_position.y * sinAngle,
            a_position.x * sinAngle + a_position.y * cosAngle
        );
        // Move it slightly so it's visible
        gl_Position = vec4(rotatedPos, 0.0, 1.0);
    }
)";

// Fragment Shader (GLSL)
static const char* fragmentShaderSource = R"(
    precision mediump float;
    void main() {
        // Simple green color
        gl_FragColor = vec4(0.2, 0.8, 0.2, 1.0);
    }
)";

// Globals
static GLuint program     = 0;
static GLuint vbo         = 0;
static GLint  aPosLoc     = -1;
static GLint  uAngleLoc   = -1;
static float  angle       = 0.0f;

// Comile Shader
GLuint compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    // Check for errors
    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE) {
        char buffer[512];
        glGetShaderInfoLog(shader, 512, nullptr, buffer);
        emscripten_log(EM_LOG_ERROR, "Shader compile error: %s", buffer);
        return 0;
    }
    return shader;
}

// link shaders
GLuint createProgram(const char* vsSource, const char* fsSource) {
    GLuint vs = compileShader(GL_VERTEX_SHADER, vsSource);
    if (!vs) return 0;

    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fsSource);
    if (!fs) return 0;

    GLuint prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);

    // Check for errors
    GLint status;
    glGetProgramiv(prog, GL_LINK_STATUS, &status);
    if (status != GL_TRUE) {
        char buffer[512];
        glGetProgramInfoLog(prog, 512, nullptr, buffer);
        emscripten_log(EM_LOG_ERROR, "Program link error: %s", buffer);
        return 0;
    }
    // Cleanup (shaders are now linked into the program)
    glDeleteShader(vs);
    glDeleteShader(fs);

    return prog;
}

// This function is called every frame
void update_frame(void* userData) {
    // Increase angle
    angle += 0.01f;

    // Set the viewport and clear the screen
    glViewport(0, 0, 800, 600);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Use our program
    glUseProgram(program);

    // Update the uniform angle
    glUniform1f(uAngleLoc, angle);

    // Bind the vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glEnableVertexAttribArray(aPosLoc);
    glVertexAttribPointer(aPosLoc, 2, GL_FLOAT, GL_FALSE, 0, 0);

    // Draw the triangle
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // Cleanup
    glDisableVertexAttribArray(aPosLoc);
}

int main() {
    // 1. Initialize the WebGL context
    EmscriptenWebGLContextAttributes attrs;
    emscripten_webgl_init_context_attributes(&attrs);
    attrs.alpha = false;
    attrs.depth = true;
    attrs.antialias = true;

    EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context = emscripten_webgl_create_context("#canvas", &attrs);
    if (context <= 0) {
        emscripten_log(EM_LOG_ERROR, "Failed to create WebGL context");
        return 1;
    }
    emscripten_webgl_make_context_current(context);

    // 2. Create the shader program
    program = createProgram(vertexShaderSource, fragmentShaderSource);
    if (!program) {
        emscripten_log(EM_LOG_ERROR, "Failed to create program");
        return 1;
    }

    // 3. Look up attribute locations
    aPosLoc   = glGetAttribLocation(program, "a_position");
    uAngleLoc = glGetUniformLocation(program, "u_angle");

    // 4. Create a simple triangle vertex buffer
    GLfloat vertices[] = {
         0.0f,  0.5f,  // top
        -0.5f, -0.5f,  // left
         0.5f, -0.5f   // right
    };

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // 5. Start the main loop
    emscripten_set_main_loop_arg(update_frame, nullptr, 0, 1);
    return 0;
}
