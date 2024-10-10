// OpenGL and FreeGlut headers.
#include <GL/glew.h>
#include <GL/freeglut.h>

// GLM.
#include <GLM/glm.hpp>
#include <GLM/gtc/type_ptr.hpp>

// C++ STL headers.
#include <iostream>
#include <algorithm>
#include <vector>
#include <iomanip>

// My headers.
#include "TriangleMesh.h"

// Global variables.
const int screenWidth = 600;
const int screenHeight = 600;
TriangleMesh *mesh = nullptr;
std::vector<std::string> objFiles = {"Bunny.obj", "Cube.obj", "Forklift.obj", "Gengar.obj", "Koffing.obj", "Polygons.obj", "Teapot.obj", "Triangles.obj"};

// Function prototypes.
void SetupRenderState();
void SetupScene(const std::string &);
void ReleaseResources();
void RenderSceneCB();
void ReshapeCB(int, int);
void ProcessSpecialKeysCB(int, int, int);
void ProcessKeysCB(unsigned char, int, int);
// my function
void MenuCallback(int);
void SetupMenu();
void RenderText(float, float, const std::string &);

// define key
#define KEY_ESC 27

// Callback function for glutDisplayFunc.
void RenderSceneCB()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Render the triangle mesh.
    // ...
    glEnableVertexAttribArray(0);

    // Render Model.
    glColor3f(1.0f, 1.0f, 1.0f);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->GetVBOId());
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPTN), (void *)offsetof(VertexPTN, position)); // pos offset is 0
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->GetIBOId());
    glDrawElements(GL_TRIANGLES, mesh->GetNumIndices(), GL_UNSIGNED_INT, 0);

    glDisableVertexAttribArray(0);

    // 渲染提示文字
    glColor3f(1.0f, 1.0f, 1.0f); // 設置文本顏色為白色
    RenderText(-0.9f, 0.9f, "ESC: Exit, Mouse click right: open menu, F1: Point, F2: Line, F3: Fill");
    glutSwapBuffers();
}

// Callback function for glutReshapeFunc.
void ReshapeCB(int w, int h)
{
    // Adjust camera and projection here.
    // Implemented in HW2.
}

// Callback function for glutSpecialFunc.
void ProcessSpecialKeysCB(int key, int x, int y)
{
    // Handle special (functional) keyboard inputs such as F1, spacebar, page up, etc.
    switch (key)
    {
    case GLUT_KEY_F1:
        // Render with point mode.
        glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
        break;
    case GLUT_KEY_F2:
        // Render with line mode.
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        break;
    case GLUT_KEY_F3:
        // Render with fill mode.
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        break;
    default:
        break;
    }
}

// Callback function for glutKeyboardFunc.
void ProcessKeysCB(unsigned char key, int x, int y)
{
    // Handle other keyboard inputs those are not defined as special keys.
    if (key == KEY_ESC)
    {
        // Release memory allocation if needed.
        ReleaseResources();
        exit(0);
    }
}

void ReleaseResources()
{
    // Release memory if needed.
    if (mesh)
    {
        GLuint vboId = mesh->GetVBOId();
        GLuint iboId = mesh->GetIBOId();

        glDeleteBuffers(1, &vboId);
        glDeleteBuffers(1, &iboId);

        delete mesh;
        mesh = nullptr;
    }
}

void SetupRenderState()
{
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glLineWidth(0.5f);

    glm::vec4 clearColor = glm::vec4(0.44f, 0.57f, 0.75f, 1.00f);
    glClearColor(
        (GLclampf)(clearColor.r),
        (GLclampf)(clearColor.g),
        (GLclampf)(clearColor.b),
        (GLclampf)(clearColor.a));
}

// Load a model from obj file and apply transformation.
// You can alter the parameters for dynamically loading a model.
void SetupScene(const std::string &modelPath)
{
    printf("[debug] Loading model: %s\n", modelPath.c_str());
    mesh = new TriangleMesh();
    mesh->LoadFromFile(modelPath);

    // Please DO NOT TOUCH the following code.
    // ------------------------------------------------------------------------
    // Build transformation matrices.
    // World.
    glm::mat4x4 M(1.0f);
    // Camera.
    glm::vec3 cameraPos = glm::vec3(0.0f, 0.5f, 2.0f);
    glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::mat4x4 V = glm::lookAt(cameraPos, cameraTarget, cameraUp);
    // Projection.
    float fov = 40.0f;
    float aspectRatio = (float)screenWidth / (float)screenHeight;
    float zNear = 0.1f;
    float zFar = 100.0f;
    glm::mat4x4 P = glm::perspective(glm::radians(fov), aspectRatio, zNear, zFar);

    // Apply CPU transformation.
    glm::mat4x4 MVP = P * V * M;

    mesh->ApplyTransformCPU(MVP);

    // Create and upload vertex/index buffers.
    mesh->CreateBuffers();
}

// 建立菜單可供操作
void SetupMenu()
{
    int menu = glutCreateMenu(MenuCallback);

    // 將檔案名稱加入菜單
    for (int i = 0; i < objFiles.size(); i++)
    {
        glutAddMenuEntry(objFiles[i].c_str(), i);
    }
    glutAddMenuEntry("Delete Model", objFiles.size());
    glutAttachMenu(GLUT_RIGHT_BUTTON); // 右鍵單擊顯示菜單
}

// 菜單回調函數
void MenuCallback(int option)
{
    // 切換模型
    if (0 <= option && option < objFiles.size())
        SetupScene("TestModels_HW1/" + objFiles[option]);
    else if (option == objFiles.size()) { // 刪除模型
        mesh = new TriangleMesh();
    }
}

void RenderText(float x, float y, const std::string &text)
{
    glRasterPos2f(x, y); // 設置文本位置
    for (char c : text)
    {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c); // 使用位圖字體渲染字符
    }
}

int main(int argc, char **argv)
{
    // Setting window properties.
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(screenWidth, screenHeight);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("HW1: OBJ Loader");

    // Initialize GLEW.
    // Must be done after glut is initialized!
    GLenum res = glewInit();
    if (res != GLEW_OK)
    {
        std::cerr << "GLEW initialization error: "
                  << glewGetErrorString(res) << std::endl;
        return 1;
    }

    // Initialization.
    SetupRenderState();
    SetupScene("TestModels_HW1/Cube.obj");

    // 創建菜單
    SetupMenu();

    // Register callback functions.
    glutDisplayFunc(RenderSceneCB);
    glutIdleFunc(RenderSceneCB);
    glutReshapeFunc(ReshapeCB);
    glutSpecialFunc(ProcessSpecialKeysCB);
    glutKeyboardFunc(ProcessKeysCB);

    // Start rendering loop.
    glutMainLoop();

    return 0;
}
