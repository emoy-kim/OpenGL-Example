/*
 * Author: Emoy Kim
 * E-mail: emoy.kim_AT_gmail.com
 * 
 * This code is a free software; it can be freely used, changed and redistributed.
 * If you use any version of the code, please reference the code.
 * 
 */

#pragma once

#include <OpenCVLinker.h>
#include <OpenGLLinker.h>
#include <iostream>
#include <vector>
#include <fstream>

using namespace std;
using namespace cv;
using namespace glm;

class CameraGL
{
   const float ZoomSensitivity;
   const float MoveSensitivity;
   const float RotationSensitivity;
   bool IsMoving;
   float AspectRatio;
   float InitFOV;
   float NearPlane, FarPlane;
   vec3 InitCamPos, InitRefPos, InitUpVec;

public:
   float FOV;
   vec3 CamPos;
   mat4 ViewMatrix, ProjectionMatrix;

   CameraGL();
   CameraGL(
      const vec3& cam_position,
      const vec3& view_reference_position,
      const vec3& view_up_vector,
      float fov = 30.0f,
      float near_plane = 0.1f,
      float far_plane = 10000.0f
   );

   bool getMovingState() const;
   void setMovingState(const bool& is_moving);
   void updateCamera();
   void pitch(const int& angle);
   void yaw(const int& angle);
   void moveForward();
   void moveBackward();
   void moveLeft();
   void moveRight();
   void moveUp();
   void moveDown();
   void zoomIn();
   void zoomOut();
   void resetCamera();
   void updateWindowSize(const int& width, const int& height);
};

class ObjectGL
{
   vector<GLfloat> DataBuffer; // 3 for vertex, 3 for normal, and 2 for texture

   void prepareTexture2DFromMat(const Mat& texture) const;
   void prepareTexture(
      const int& n_bytes_per_vertex, 
      const Mat& texture, 
      const bool& normals_exist
   );
   
   void prepareTexture2DFromFile(const string& file_name) const;
   void prepareTexture(
      const int& n_bytes_per_vertex, 
      const string& texture_file_name, 
      const bool& normals_exist
   );

   void prepareVertexBuffer(const int& n_bytes_per_vertex);
   void prepareNormal(const int& n_bytes_per_vertex) const;
   GLvoid* bufferOffset(uint offset) const { return reinterpret_cast<GLvoid *>(offset); }

public:
   enum LayoutLocation { VertexLoc=0, NormalLoc, TextureLoc };

   GLuint ObjVAO, ObjVBO;
   GLenum DrawMode;
   GLuint TextureID;
   GLsizei VerticesCount;
   vec3 Colors;

   ObjectGL();

   void setObject(
      GLenum draw_mode, 
      const vec3& color,
      const vector<vec3>& vertices
   );

   void setObject(
      GLenum draw_mode, 
      const vec3& color,
      const vector<vec3>& vertices,
      const vector<vec3>& normals
   );

   void setObject(
      GLenum draw_mode, 
      const vec3& color,
      const vector<vec3>& vertices,
      const vector<vec2>& textures,
      const string& texture_file_name
   );

   void setObject(
      GLenum draw_mode, 
      const vec3& color,
      const vector<vec3>& vertices,
      const vector<vec2>& textures,
      const Mat& texture
   );

   void setObject(
      GLenum draw_mode, 
      const vec3& color,
      const vector<vec3>& vertices, 
      const vector<vec3>& normals, 
      const vector<vec2>& textures,
      const string& texture_file_name
   );

   void setObject(
      GLenum draw_mode, 
      const vec3& color,
      const vector<vec3>& vertices, 
      const vector<vec3>& normals, 
      const vector<vec2>& textures,
      const Mat& texture
   );
};

class ShaderGL
{
   void readShaderFile(string& shader_contents, const char* shader_path) const;

public:
   GLuint ShaderProgram;
   GLint MVPLocation, WorldLocation, ViewLocation, ProjectionLocation;
   GLint ColorLocation, TextureLocation;
   GLint LightLocation, LightColorLocation;
   
   ShaderGL();

   void setShader(const char* vertex_shader_path, const char* fragment_shader_path);
};

class RendererGL
{
   struct Light
   {
      bool TurnLightOn;
      int TotalNumber;
      int ActivatedIndex;
      vector<vec3> Colors;
      vector<vec3> Positions;
      Light() : TurnLightOn( false ), TotalNumber( 0 ), ActivatedIndex( 0 ) {}
   };

   static RendererGL* Renderer;
   GLFWwindow* Window;

   vec2 ClickedPoint;

   CameraGL MainCamera;
   ShaderGL ObjectShader;
   ObjectGL Object;

   bool DrawMovingObject;
 
   void registerCallbacks() const;
   void initializeOpenGL(const int& width, const int& height);
   void initialize();

   void printOpenGLInformation() const;

   void error(int error, const char* description) const;
   void cleanup(GLFWwindow* window);
   void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods);
   void cursor(GLFWwindow* window, double xpos, double ypos);
   void mouse(GLFWwindow* window, int button, int action, int mods);
   void mousewheel(GLFWwindow* window, double xoffset, double yoffset);
   void reshape(GLFWwindow* window, int width, int height);
   static void errorWrapper(int error, const char* description);
   static void cleanupWrapper(GLFWwindow* window);
   static void keyboardWrapper(GLFWwindow* window, int key, int scancode, int action, int mods);
   static void cursorWrapper(GLFWwindow* window, double xpos, double ypos);
   static void mouseWrapper(GLFWwindow* window, int button, int action, int mods);
   static void mousewheelWrapper(GLFWwindow* window, double xoffset, double yoffset);
   static void reshapeWrapper(GLFWwindow* window, int width, int height);

   void setObject();
   void drawObject(const float& scale_factor);
   void render();
   void update();


public:
   RendererGL(const RendererGL&) = delete;
   RendererGL(const RendererGL&&) = delete;
   RendererGL& operator=(const RendererGL&) = delete;
   RendererGL& operator=(const RendererGL&&) = delete;


   RendererGL();
   ~RendererGL();

   void play();
};