#include "Renderer.h"


//------------------------------------------------------------------
//
// Camera Class
//
//------------------------------------------------------------------

CameraGL::CameraGL() : 
   CameraGL(vec3(0.0f, 0.0f, -10.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f))
{   
}

CameraGL::CameraGL(
   const vec3& cam_position,
   const vec3& view_reference_position,
   const vec3& view_up_vector,
   float fov,
   float near_plane,
   float far_plane
) : 
   ZoomSensitivity( 1.0f ), MoveSensitivity( 0.5f ), RotationSensitivity( 0.01f ), IsMoving( false ),
   AspectRatio( 0.0f ), InitFOV( fov ), NearPlane( near_plane ), FarPlane( far_plane ), 
   InitCamPos( cam_position ), InitRefPos( view_reference_position ), InitUpVec( view_up_vector ), 
   FOV( fov ), CamPos( cam_position ),
   ViewMatrix( lookAt( InitCamPos, InitRefPos, InitUpVec ) ), ProjectionMatrix( mat4(1.0f) )
{
}

bool CameraGL::getMovingState() const
{
   return IsMoving;
}

void CameraGL::setMovingState(const bool& is_moving)
{
   IsMoving = is_moving;
}

void CameraGL::updateCamera()
{
   const mat4 inverse_view = inverse( ViewMatrix );
   CamPos.x = inverse_view[3][0];
   CamPos.y = inverse_view[3][1];
   CamPos.z = inverse_view[3][2];
}

void CameraGL::pitch(const int& angle)
{
   const vec3 u_axis(ViewMatrix[0][0], ViewMatrix[1][0], ViewMatrix[2][0]);
   ViewMatrix = glm::rotate( ViewMatrix, static_cast<float>(angle) * RotationSensitivity, u_axis );
   updateCamera();
}

void CameraGL::yaw(const int& angle)
{
   const vec3 v_axis(ViewMatrix[0][1], ViewMatrix[1][1], ViewMatrix[2][1]);
   ViewMatrix = glm::rotate( ViewMatrix, static_cast<float>(angle) * RotationSensitivity, v_axis );
   updateCamera();
}

void CameraGL::moveForward()
{
   const vec3 n_axis(ViewMatrix[0][2], ViewMatrix[1][2], ViewMatrix[2][2]);
   ViewMatrix = translate( ViewMatrix, MoveSensitivity * n_axis );
   updateCamera();
}

void CameraGL::moveBackward()
{
   const vec3 n_axis(ViewMatrix[0][2], ViewMatrix[1][2], ViewMatrix[2][2]);
   ViewMatrix = translate( ViewMatrix, -MoveSensitivity * n_axis );
   updateCamera();
}

void CameraGL::moveLeft()
{
   const vec3 u_axis(ViewMatrix[0][0], ViewMatrix[1][0], ViewMatrix[2][0]);
   ViewMatrix = translate( ViewMatrix, MoveSensitivity * u_axis );
   updateCamera();
}

void CameraGL::moveRight()
{
   const vec3 u_axis(ViewMatrix[0][0], ViewMatrix[1][0], ViewMatrix[2][0]);
   ViewMatrix = translate( ViewMatrix, -MoveSensitivity * u_axis );
   updateCamera();
}

void CameraGL::moveUp()
{
   const vec3 v_axis(ViewMatrix[0][1], ViewMatrix[1][1], ViewMatrix[2][1]);
   ViewMatrix = translate( ViewMatrix, -MoveSensitivity * v_axis );
   updateCamera();
}

void CameraGL::moveDown()
{
   const vec3 v_axis(ViewMatrix[0][1], ViewMatrix[1][1], ViewMatrix[2][1]);
   ViewMatrix = translate( ViewMatrix, MoveSensitivity * v_axis );
   updateCamera();
}

void CameraGL::zoomIn()
{
   if (FOV > 0.0f) {
      FOV -= ZoomSensitivity;
      ProjectionMatrix = perspective( radians( FOV ), AspectRatio, NearPlane, FarPlane );
   }
}

void CameraGL::zoomOut()
{
   if (FOV < 90.0f) {
      FOV += ZoomSensitivity;
      ProjectionMatrix = perspective( radians( FOV ), AspectRatio, NearPlane, FarPlane );
   }
}

void CameraGL::resetCamera()
{
   CamPos = InitCamPos; 
   ViewMatrix = lookAt( InitCamPos, InitRefPos, InitUpVec );
   ProjectionMatrix = perspective( radians( InitFOV ), AspectRatio, NearPlane, FarPlane );
}

void CameraGL::updateWindowSize(const int& width, const int& height)
{
   AspectRatio = static_cast<float>(width) / static_cast<float>(height);
   ProjectionMatrix = perspective( radians( FOV ), AspectRatio, NearPlane, FarPlane );
}


//------------------------------------------------------------------
//
// Object Class
//
//------------------------------------------------------------------

ObjectGL::ObjectGL() : ObjVAO( 0 ), ObjVBO( 0 ), DrawMode( 0 ), TextureID( 0 ), VerticesCount( 0 ), Colors{}
{
}

void ObjectGL::prepareTexture2DFromMat(const Mat& texture) const
{
   // NOTE: 'texture' is going to be flipped vertically.
   // OpenGL texture's origin is bottom-left, but OpenCV Mat's is top-left.
   const int width = texture.cols;
   const int height = texture.rows;
   glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, texture.data );
}

void ObjectGL::prepareTexture(
   const int& n_bytes_per_vertex, 
   const Mat& texture, 
   const bool& normals_exist
)
{
   glGenTextures( 1, &TextureID );
   glActiveTexture( GL_TEXTURE0 + TextureID );
   glBindTexture( GL_TEXTURE_2D, TextureID );
   
   prepareTexture2DFromMat( texture );
   
   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );

   const uint offset = normals_exist ? 6 : 3;
   glVertexAttribPointer( TextureLoc, 2, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, bufferOffset( offset * sizeof(GLfloat) ) );
   glEnableVertexAttribArray( TextureLoc );
}

void ObjectGL::prepareTexture2DFromFile(const string& file_name) const
{
   const FREE_IMAGE_FORMAT format = FreeImage_GetFileType( file_name.c_str(), 0 );
   FIBITMAP* texture = FreeImage_Load( format, file_name.c_str() );
   const uint n_bits_per_pixel = FreeImage_GetBPP( texture );
   cout << " * A " << n_bits_per_pixel << "-bit texture was read from " << file_name << endl;
   
   FIBITMAP* texture_32bit;
   if (n_bits_per_pixel == 32) {
      texture_32bit = texture;
   }
   else {
      cout << " * Converting texture from " << n_bits_per_pixel << " bits to 32 bits..." << endl;
      texture_32bit = FreeImage_ConvertTo32Bits( texture );
   }

   const uint width = FreeImage_GetWidth( texture_32bit );
   const uint height = FreeImage_GetHeight( texture_32bit );
   GLvoid* data = FreeImage_GetBits( texture_32bit );
   glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, data );
   cout << " * Loaded " << width << " x " << height << " RGBA texture into graphics memory." << endl << endl;

   FreeImage_Unload( texture_32bit );
   if (n_bits_per_pixel != 32) {
      FreeImage_Unload( texture );
   }
}

void ObjectGL::prepareTexture(
   const int& n_bytes_per_vertex, 
   const string& texture_file_name, 
   const bool& normals_exist
)
{
   glGenTextures( 1, &TextureID );
   glActiveTexture( GL_TEXTURE0 + TextureID );
   glBindTexture( GL_TEXTURE_2D, TextureID );
   
   prepareTexture2DFromFile( texture_file_name );
   
   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );

   const uint offset = normals_exist ? 6 : 3;
   glVertexAttribPointer( TextureLoc, 2, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, bufferOffset( offset * sizeof(GLfloat) ) );
   glEnableVertexAttribArray( TextureLoc );
}

void ObjectGL::prepareNormal(const int& n_bytes_per_vertex) const
{
   glVertexAttribPointer( NormalLoc, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, bufferOffset( 3 * sizeof(GLfloat) ) );
   glEnableVertexAttribArray( NormalLoc );	
}

void ObjectGL::prepareVertexBuffer(const int& n_bytes_per_vertex)
{
   // initialize vertex buffer object
   glGenBuffers( 1, &ObjVBO );
   glBindBuffer( GL_ARRAY_BUFFER, ObjVBO );
   glBufferData( GL_ARRAY_BUFFER, sizeof(GLfloat) * DataBuffer.size(), DataBuffer.data(), GL_STATIC_DRAW );
   glBindBuffer( GL_ARRAY_BUFFER, 0 );

   // initialize vertex array object
   glGenVertexArrays( 1, &ObjVAO );
   glBindVertexArray( ObjVAO );
   glBindBuffer( GL_ARRAY_BUFFER, ObjVBO );
   glVertexAttribPointer( VertexLoc, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, bufferOffset( 0 ) );
   glEnableVertexAttribArray( VertexLoc );

   //glBindBuffer( GL_ARRAY_BUFFER, 0 );
   //glBindVertexArray( 0 );
}

void ObjectGL::setObject(
   GLenum draw_mode, 
   const vec3& color, 
   const vector<vec3>& vertices
)
{
   DrawMode = draw_mode;
   Colors = { color.r, color.g, color.b };
   for (auto& vertex : vertices) {
      DataBuffer.push_back( vertex.x );
      DataBuffer.push_back( vertex.y );
      DataBuffer.push_back( vertex.z );
      VerticesCount++;
   }
   const int n_bytes_per_vertex = 3 * sizeof(GLfloat);
   prepareVertexBuffer( n_bytes_per_vertex );
}

void ObjectGL::setObject(
   GLenum draw_mode, 
   const vec3& color, 
   const vector<vec3>& vertices,
   const vector<vec3>& normals
)
{
   DrawMode = draw_mode;
   Colors = { color.r, color.g, color.b };
   for (uint i = 0; i < vertices.size(); ++i) {
      DataBuffer.push_back( vertices[i].x );
      DataBuffer.push_back( vertices[i].y );
      DataBuffer.push_back( vertices[i].z );
      DataBuffer.push_back( normals[i].x );
      DataBuffer.push_back( normals[i].y );
      DataBuffer.push_back( normals[i].z );
      VerticesCount++;
   }
   const int n_bytes_per_vertex = 6 * sizeof(GLfloat);
   prepareVertexBuffer( n_bytes_per_vertex );
   prepareNormal( n_bytes_per_vertex );
}

void ObjectGL::setObject(
   GLenum draw_mode, 
   const vec3& color, 
   const vector<vec3>& vertices,
   const vector<vec2>& textures, 
   const string& texture_file_name
)
{
   DrawMode = draw_mode;
   Colors = { color.r, color.g, color.b };
   for (uint i = 0; i < vertices.size(); ++i) {
      DataBuffer.push_back( vertices[i].x );
      DataBuffer.push_back( vertices[i].y );
      DataBuffer.push_back( vertices[i].z );
      DataBuffer.push_back( textures[i].x );
      DataBuffer.push_back( textures[i].y );
      VerticesCount++;
   }
   const int n_bytes_per_vertex = 5 * sizeof(GLfloat);
   prepareVertexBuffer( n_bytes_per_vertex );
   prepareTexture( n_bytes_per_vertex, texture_file_name, false );
}

void ObjectGL::setObject(
   GLenum draw_mode, 
   const vec3& color, 
   const vector<vec3>& vertices,
   const vector<vec2>& textures, 
   const Mat& texture
)
{
   DrawMode = draw_mode;
   Colors = { color.r, color.g, color.b };
   for (uint i = 0; i < vertices.size(); ++i) {
      DataBuffer.push_back( vertices[i].x );
      DataBuffer.push_back( vertices[i].y );
      DataBuffer.push_back( vertices[i].z );
      DataBuffer.push_back( textures[i].x );
      DataBuffer.push_back( textures[i].y );
      VerticesCount++;
   }
   const int n_bytes_per_vertex = 5 * sizeof(GLfloat);
   prepareVertexBuffer( n_bytes_per_vertex );
   prepareTexture( n_bytes_per_vertex, texture, false );
}

void ObjectGL::setObject(
   GLenum draw_mode, 
   const vec3& color, 
   const vector<vec3>& vertices, 
   const vector<vec3>& normals, 
   const vector<vec2>& textures,
   const string& texture_file_name
)
{
   DrawMode = draw_mode;
   Colors = { color.r, color.g, color.b };
   for (uint i = 0; i < vertices.size(); ++i) {
      DataBuffer.push_back( vertices[i].x );
      DataBuffer.push_back( vertices[i].y );
      DataBuffer.push_back( vertices[i].z );
      DataBuffer.push_back( normals[i].x );
      DataBuffer.push_back( normals[i].y );
      DataBuffer.push_back( normals[i].z );
      DataBuffer.push_back( textures[i].x );
      DataBuffer.push_back( textures[i].y );
      VerticesCount++;
   }
   const int n_bytes_per_vertex = 8 * sizeof(GLfloat);
   prepareVertexBuffer( n_bytes_per_vertex );
   prepareNormal( n_bytes_per_vertex );
   prepareTexture( n_bytes_per_vertex, texture_file_name, true );
}

void ObjectGL::setObject(
   GLenum draw_mode, 
   const vec3& color, 
   const vector<vec3>& vertices,
   const vector<vec3>& normals, 
   const vector<vec2>& textures, 
   const Mat& texture
)
{
   DrawMode = draw_mode;
   Colors = { color.r, color.g, color.b };
   for (uint i = 0; i < vertices.size(); ++i) {
      DataBuffer.push_back( vertices[i].x );
      DataBuffer.push_back( vertices[i].y );
      DataBuffer.push_back( vertices[i].z );
      DataBuffer.push_back( normals[i].x );
      DataBuffer.push_back( normals[i].y );
      DataBuffer.push_back( normals[i].z );
      DataBuffer.push_back( textures[i].x );
      DataBuffer.push_back( textures[i].y );
      VerticesCount++;
   }
   const int n_bytes_per_vertex = 8 * sizeof(GLfloat);
   prepareVertexBuffer( n_bytes_per_vertex );
   prepareNormal( n_bytes_per_vertex );
   prepareTexture( n_bytes_per_vertex, texture, true );
}


//------------------------------------------------------------------
//
// Shader Class
//
//------------------------------------------------------------------

ShaderGL::ShaderGL() : 
   ShaderProgram( 0 ), MVPLocation( 0 ), WorldLocation( 0 ), ViewLocation( 0 ), 
   ProjectionLocation( 0 ), ColorLocation( 0 ), TextureLocation( 0 ), 
   LightLocation( 0 ), LightColorLocation( 0 )
{
}

void ShaderGL::readShaderFile(string& shader_contents, const char* shader_path) const
{
   ifstream file(shader_path, ios::in);
   if (!file.is_open()) {
      cout << "Cannot Open Shader File: " << shader_path << endl;
      return;
   }

   string line;
   while (!file.eof()) {
      getline( file, line );
      shader_contents.append( line + "\n" );
   }
   file.close();
}

void ShaderGL::setShader(const char* vertex_shader_path, const char* fragment_shader_path)
{
   string vertex_contents, fragment_contents;
   readShaderFile( vertex_contents, vertex_shader_path );
   readShaderFile( fragment_contents, fragment_shader_path );

   const GLuint vertex_shader = glCreateShader( GL_VERTEX_SHADER );
   const GLuint fragment_shader = glCreateShader( GL_FRAGMENT_SHADER );
   const char* vertex_source = vertex_contents.c_str();
   const char* fragment_source = fragment_contents.c_str();
   glShaderSource( vertex_shader, 1, &vertex_source, nullptr );
   glShaderSource( fragment_shader, 1, &fragment_source, nullptr );
   glCompileShader( vertex_shader );
   glCompileShader( fragment_shader );

   ShaderProgram = glCreateProgram();
   glAttachShader( ShaderProgram, vertex_shader );
   glAttachShader( ShaderProgram, fragment_shader );
   glLinkProgram( ShaderProgram );

   MVPLocation = glGetUniformLocation( ShaderProgram, "ModelViewProjectionMatrix" );
   WorldLocation = glGetUniformLocation( ShaderProgram, "WorldMatrix" );
   ViewLocation = glGetUniformLocation( ShaderProgram, "ViewMatrix" );
   ProjectionLocation = glGetUniformLocation( ShaderProgram, "ProjectionMatrix" );

   ColorLocation = glGetUniformLocation( ShaderProgram, "PrimitiveColor" );
   TextureLocation = glGetUniformLocation( ShaderProgram, "BaseTexture" );

   LightLocation = glGetUniformLocation( ShaderProgram, "LightPosition" );
   LightColorLocation = glGetUniformLocation( ShaderProgram, "LightColor" );

   glDeleteShader( vertex_shader );
   glDeleteShader( fragment_shader );
}


//------------------------------------------------------------------
//
// Renderer Class
//
//------------------------------------------------------------------

RendererGL* RendererGL::Renderer = nullptr;
RendererGL::RendererGL() : Window( nullptr ), DrawMovingObject( false )
{
   Renderer = this;

   initialize();
   printOpenGLInformation();
}

RendererGL::~RendererGL()
{
   glfwTerminate();
}

void RendererGL::printOpenGLInformation() const
{
   cout << "****************************************************************" << endl;
   cout << " - GLFW version supported: " << glfwGetVersionString() << endl;
   cout << " - GLEW version supported: " << glewGetString( GLEW_VERSION ) << endl;
   cout << " - OpenGL renderer: " << glGetString( GL_RENDERER ) << endl;
   cout << " - OpenGL version supported: " << glGetString( GL_VERSION ) << endl;
   cout << " - OpenGL shader version supported: " << glGetString( GL_SHADING_LANGUAGE_VERSION ) << endl  ;
   cout << "****************************************************************" << endl << endl;
}

void RendererGL::initializeOpenGL(const int& width, const int& height)
{
   if (!glfwInit()) {
      cout << "Cannot Initialize OpenGL..." << endl;
      return;
   }
   glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
   glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );
   glfwWindowHint( GLFW_DOUBLEBUFFER, GLFW_TRUE );
   glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );

   Window = glfwCreateWindow( width, height, "Main Camera", nullptr, nullptr );
   glfwMakeContextCurrent( Window );
   glewInit();
   
   glEnable( GL_DEPTH_TEST );
   glClearColor( 1.0f, 1.0f, 1.0f, 1.0f );

   MainCamera.updateWindowSize( width, height );
}

void RendererGL::initialize()
{
   const int width = 1920;
   const int height = 1080;
   initializeOpenGL( width, height );
   registerCallbacks();

   ObjectShader.setShader(
      "Shaders/VertexShaderForObject.glsl",
      "Shaders/FragmentShaderForObject.glsl"
   );
}

void RendererGL::error(int error, const char* description) const
{
   puts( description );
}

void RendererGL::errorWrapper(int error, const char* description)
{
   Renderer->error( error, description );
}

void RendererGL::cleanup(GLFWwindow* window)
{
   glDeleteProgram( ObjectShader.ShaderProgram );
   glDeleteVertexArrays( 1, &Object.ObjVAO );
   glDeleteBuffers( 1, &Object.ObjVBO );

   glfwSetWindowShouldClose( window, GLFW_TRUE );
}

void RendererGL::cleanupWrapper(GLFWwindow* window)
{
   Renderer->cleanup( window );
}

void RendererGL::keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
   if (action != GLFW_PRESS) return;

   switch (key) {
      case GLFW_KEY_UP:
         MainCamera.moveForward();
         break;
      case GLFW_KEY_DOWN:
         MainCamera.moveBackward();
         break;
      case GLFW_KEY_LEFT:
         MainCamera.moveLeft();
         break;
      case GLFW_KEY_RIGHT:
         MainCamera.moveRight();
         break;
      case GLFW_KEY_W:
         MainCamera.moveUp();
         break;
      case GLFW_KEY_S:
         MainCamera.moveDown();
         break;
      case GLFW_KEY_I:
         MainCamera.resetCamera();
         break;
      case GLFW_KEY_SPACE:
         DrawMovingObject = !DrawMovingObject;
         break;
      case GLFW_KEY_P:
         cout << "Camera Position: " << 
            MainCamera.CamPos.x << ", " << MainCamera.CamPos.y << ", " << MainCamera.CamPos.z << endl;
         break;
      case GLFW_KEY_Q:
      case GLFW_KEY_ESCAPE:
         cleanupWrapper( window );
         break;
      default:
         return;
   }
}

void RendererGL::keyboardWrapper(GLFWwindow* window, int key, int scancode, int action, int mods)
{
   Renderer->keyboard( window, key, scancode, action, mods );
}

void RendererGL::cursor(GLFWwindow* window, double xpos, double ypos)
{
   if (MainCamera.getMovingState()) {
      const int dx = static_cast<int>(round( xpos ) - ClickedPoint.x);
      const int dy = static_cast<int>(round( ypos ) - ClickedPoint.y);
      MainCamera.pitch( dy );
      MainCamera.yaw( dx );
   }
}

void RendererGL::cursorWrapper(GLFWwindow* window, double xpos, double ypos)
{
   Renderer->cursor( window, xpos, ypos );
}

void RendererGL::mouse(GLFWwindow* window, int button, int action, int mods)
{
   if (button == GLFW_MOUSE_BUTTON_LEFT) {
      const bool moving_state = action == GLFW_PRESS;
      if (moving_state) {
         double x, y;
         glfwGetCursorPos( window, &x, &y );
         ClickedPoint.x = static_cast<float>(round( x ));
         ClickedPoint.y = static_cast<float>(round( y ));
      }
      MainCamera.setMovingState( moving_state );
   }
}

void RendererGL::mouseWrapper(GLFWwindow* window, int button, int action, int mods)
{
   Renderer->mouse( window, button, action, mods );
}

void RendererGL::mousewheel(GLFWwindow* window, double xoffset, double yoffset)
{
   if (yoffset >= 0.0) {
      MainCamera.zoomIn();
   }
   else {
      MainCamera.zoomOut();
   }
}

void RendererGL::mousewheelWrapper(GLFWwindow* window, double xoffset, double yoffset)
{
   Renderer->mousewheel( window, xoffset, yoffset );
}

void RendererGL::reshape(GLFWwindow* window, int width, int height)
{
   MainCamera.updateWindowSize( width, height );
   glViewport( 0, 0, width, height );
}

void RendererGL::reshapeWrapper(GLFWwindow* window, int width, int height)
{
   Renderer->reshape( window, width, height );
}

void RendererGL::registerCallbacks() const
{
   glfwSetErrorCallback( errorWrapper );
   glfwSetWindowCloseCallback( Window, cleanupWrapper );
   glfwSetKeyCallback( Window, keyboardWrapper );
   glfwSetCursorPosCallback( Window, cursorWrapper );
   glfwSetMouseButtonCallback( Window, mouseWrapper );
   glfwSetScrollCallback( Window, mousewheelWrapper );
   glfwSetFramebufferSizeCallback( Window, reshapeWrapper );
}

void RendererGL::setObject()
{
   if (Object.ObjVAO != 0) {
      glDeleteVertexArrays( 1, &Object.ObjVAO );
      glDeleteBuffers( 1, &Object.ObjVBO );
   }

   vector<vec3> square_vertices;
   square_vertices.emplace_back( 1.0f, 0.0f, 0.0f );
   square_vertices.emplace_back( 1.0f, 1.0f, 0.0f );
   square_vertices.emplace_back( 0.0f, 1.0f, 0.0f );
   
   square_vertices.emplace_back( 1.0f, 0.0f, 0.0f );
   square_vertices.emplace_back( 0.0f, 1.0f, 0.0f );
   square_vertices.emplace_back( 0.0f, 0.0f, 0.0f );
   
   const vec3 color = { 0.5f, 0.7f, 0.2f };
   Object.setObject( GL_TRIANGLES, color, square_vertices );
}

void RendererGL::drawObject(const float& scale_factor)
{
   glUseProgram( ObjectShader.ShaderProgram );

   const mat4 to_origin = translate( mat4(1.0f), vec3(-0.5f, -0.5f, 0.0f) );
   const mat4 scale_matrix = scale( mat4(1.0f), vec3(scale_factor, scale_factor, scale_factor) );
   const mat4 model_view_projection = MainCamera.ProjectionMatrix * MainCamera.ViewMatrix * scale_matrix * to_origin;
   glUniformMatrix4fv( ObjectShader.MVPLocation, 1, GL_FALSE, &model_view_projection[0][0] );
   glUniform1i( ObjectShader.TextureLocation, Object.TextureID );
   
   glBindVertexArray( Object.ObjVAO );
   glUniform3fv( ObjectShader.ColorLocation, 1, value_ptr( Object.Colors ) );
   glDrawArrays( Object.DrawMode, 0, Object.VerticesCount );
}

void RendererGL::render()
{
   glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

   drawObject( 3.0f );

   glBindVertexArray( 0 );
   glUseProgram( 0 );
}

void RendererGL::update()
{
   if (DrawMovingObject) {
      //TigerRotationAngle += 3;
      //if (TigerRotationAngle == 360) TigerRotationAngle = 0;
   }
}

void RendererGL::play()
{
   if (glfwWindowShouldClose( Window )) initialize();

   setObject();

   const double update_time = 0.1;
   double last = glfwGetTime(), time_delta = 0.0;
   while (!glfwWindowShouldClose( Window )) {
      const double now = glfwGetTime();
      time_delta += now - last;
      last = now;
      if (time_delta >= update_time) {
         update();
         time_delta -= update_time;
      }

      render();

      glfwPollEvents();
      glfwSwapBuffers( Window );
   }
   glfwDestroyWindow( Window );
}