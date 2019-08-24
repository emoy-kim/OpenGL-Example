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

class ShaderGL
{
   void readShaderFile(string& shader_contents, const char* shader_path) const;
   bool checkCompileErrors(const GLuint& vertex_shader, const GLuint& fragment_shader);

public:
   struct LightLocationSet
   {
      GLint LightSwitch, LightPosition;
      GLint LightAmbient, LightDiffuse, LightSpecular, LightAttenuationFactors;
      GLint SpotlightDirection, SpotlightExponent, SpotlightCutoffAngle;
      LightLocationSet() : LightSwitch( 0 ), LightPosition( 0 ), LightAmbient( 0 ), LightDiffuse( 0 ), LightSpecular( 0 ), 
      LightAttenuationFactors( 0 ), SpotlightDirection( 0 ), SpotlightExponent( 0 ), SpotlightCutoffAngle( 0 ) {}
   };

   struct LocationSet
   {
      GLint World, View, Projection, ModelViewProjection;
      GLint MaterialEmission, MaterialAmbient, MaterialDiffuse, MaterialSpecular, MaterialSpecularExponent;
      GLint TextureUnit, Texture;
      GLint UseLight, LightNum, GlobalAmbient;
      vector<LightLocationSet> Lights;
      LocationSet() : World( 0 ), View( 0 ), Projection( 0 ), ModelViewProjection( 0 ), MaterialEmission( 0 ),
      MaterialAmbient( 0 ), MaterialDiffuse( 0 ), MaterialSpecular( 0 ), MaterialSpecularExponent( 0 ), 
      TextureUnit( 0 ), Texture( 0 ), UseLight( 0 ), LightNum( 0 ), GlobalAmbient( 0 ) {}
   };
   
   LocationSet Location;
   GLuint ShaderProgram;

   ShaderGL();

   void setShader(const char* vertex_shader_path, const char* fragment_shader_path);
   void setUniformLocations(const uint& light_num);
};

class LightGL
{
   bool TurnLightOn;

   vec4 GlobalAmbientColor;

   vector<bool> IsActivated;
   vector<vec4> Positions;
   
   vector<vec4> AmbientColors;
   vector<vec4> DiffuseColors;
   vector<vec4> SpecularColors;

   vector<vec3> SpotlightDirections;
   vector<float> SpotlightExponents;
   vector<float> SpotlightCutoffAngles;

   vector<vec3> AttenuationFactors;

public:
   uint TotalLightNum;

   LightGL();

   bool isLightOn() const;
   void toggleLightSwitch();

   void addLight(
      const vec4& light_position,
      const vec4& ambient_color = vec4(0.0f, 0.0f, 0.0f, 1.0f),
      const vec4& diffuse_color = vec4(1.0f, 1.0f, 1.0f, 1.0f),
      const vec4& specular_color = vec4(1.0f, 1.0f, 1.0f, 1.0f),
      const vec3& spotlight_direction = vec3(0.0f, 0.0f, -1.0f),
      const float& spotlight_exponent = 0.0f,
      const float& spotlight_cutoff_angle_in_degree = 180.0f,
      const vec3& attenuation_factor = vec3(1.0f, 0.0f, 0.0f)
   );

   void setGlobalAmbientColor(const vec4& global_ambient_color);
   void setAmbientColor(const vec4& ambient_color, const uint& light_index);
   void setDiffuseColor(const vec4& diffuse_color, const uint& light_index);
   void setSpecularColor(const vec4& specular_color, const uint& light_index);
   void setSpotlightDirection(const vec3& spotlight_direction, const uint& light_index);
   void setSpotlightExponent(const float& spotlight_exponent, const uint& light_index);
   void setSpotlightCutoffAngle(const float& spotlight_cutoff_angle_in_degree, const uint& light_index);
   void setAttenuationFactor(const vec3& attenuation_factor, const uint& light_index);
   void setLightPosition(const vec4& light_position, const uint& light_index);
   void activateLight(const uint& light_index);
   void deactivateLight(const uint& light_index);

   void transferUniformsToShader(ShaderGL& shader);
};

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
   void rotateAroundWorldY(const int& angle);
   void moveForward(const int& delta = 1);
   void moveBackward(const int& delta = 1);
   void moveLeft(const int& delta = 1);
   void moveRight(const int& delta = 1);
   void moveUp(const int& delta = 1);
   void moveDown(const int& delta = 1);
   void zoomIn();
   void zoomOut();
   void resetCamera();
   void updateWindowSize(const int& width, const int& height);
};

class ObjectGL
{
   vector<GLfloat> DataBuffer; // 3 for vertex, 3 for normal, and 2 for texture

   void prepareTexture2DFromMat(const Mat& texture) const;
   void prepareTexture(const Mat& texture, const bool& normals_exist);
   
   void prepareTexture2DFromFile(const string& file_name) const;
   void prepareTexture(const string& texture_file_name, const bool& normals_exist);

   void prepareVertexBuffer(const int& n_bytes_per_vertex);
   void prepareNormal() const;
   GLvoid* bufferOffset(uint offset) const { return reinterpret_cast<GLvoid *>(offset); }

public:
   enum LayoutLocation { VertexLoc=0, NormalLoc, TextureLoc };

   GLuint ObjVAO, ObjVBO;
   GLenum DrawMode;
   GLuint TextureID;
   GLsizei VerticesCount;
   vec4 EmissionColor;
   vec4 AmbientReflectionColor; // It is usually set to the same color with DiffuseReflectionColor.
                                // Otherwise, it should be in balance with DiffuseReflectionColor.
   vec4 DiffuseReflectionColor; // the intrinsic color
   vec4 SpecularReflectionColor;
   float SpecularReflectionExponent;


   ObjectGL();

   void setEmissionColor(const vec4& emission_color);
   void setAmbientReflectionColor(const vec4& ambient_reflection_color);
   void setDiffuseReflectionColor(const vec4& diffuse_reflection_color);
   void setSpecularReflectionColor(const vec4& specular_reflection_color);
   void setSpecularReflectionExponent(const float& specular_reflection_exponent);

   void setObject(
      const GLenum& draw_mode, 
      const vector<vec3>& vertices
   );

   void setObject(
      const GLenum& draw_mode, 
      const vector<vec3>& vertices,
      const vector<vec3>& normals
   );

   void setObject(
      const GLenum& draw_mode, 
      const vector<vec3>& vertices,
      const vector<vec2>& textures,
      const string& texture_file_name
   );

   void setObject(
      const GLenum& draw_mode, 
      const vector<vec3>& vertices,
      const vector<vec2>& textures,
      const Mat& texture
   );

   void setObject(
      const GLenum& draw_mode, 
      const vector<vec3>& vertices, 
      const vector<vec3>& normals, 
      const vector<vec2>& textures,
      const string& texture_file_name
   );

   void setObject(
      const GLenum& draw_mode, 
      const vector<vec3>& vertices, 
      const vector<vec3>& normals, 
      const vector<vec2>& textures,
      const Mat& texture
   );

   void transferUniformsToShader(ShaderGL& shader);
};

class RendererGL
{
   static RendererGL* Renderer;
   GLFWwindow* Window;

   ivec2 ClickedPoint;

   CameraGL MainCamera;
   ShaderGL ObjectShader;
   ObjectGL Object;

   LightGL Lights;

   bool DrawMovingObject;
   int ObjectRotationAngle;
 
   void registerCallbacks() const;
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

   void setLights();
   void setObject();
   void drawObject(const float& scale_factor = 1.0f);
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