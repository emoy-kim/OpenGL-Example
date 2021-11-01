#include "renderer.h"

RendererGL::RendererGL() : 
   Window( nullptr ), FrameWidth( 1920 ), FrameHeight( 1080 ), ClickedPoint( -1, -1 ),
   MainCamera( std::make_unique<CameraGL>() ), ObjectShader( std::make_unique<ShaderGL>() ),
   Object( std::make_unique<ObjectGL>() ), Lights( std::make_unique<LightGL>() ), DrawMovingObject( false ),
   ObjectRotationAngle( 0 )
{
   Renderer = this;

   initialize();
   printOpenGLInformation();
}

void RendererGL::printOpenGLInformation()
{
   std::cout << "====================== [ Renderer Information ] ================================================\n";
   std::cout << " - GLFW version supported: " << glfwGetVersionString() << "\n";
   std::cout << " - OpenGL renderer: " << glGetString( GL_RENDERER ) << "\n";
   std::cout << " - OpenGL version supported: " << glGetString( GL_VERSION ) << "\n";
   std::cout << " - OpenGL shader version supported: " << glGetString( GL_SHADING_LANGUAGE_VERSION ) << "\n";
   std::cout << "================================================================================================\n";
}

void RendererGL::initialize()
{
   if (!glfwInit()) {
      std::cout << "Cannot Initialize OpenGL...\n";
      return;
   }
   glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
   glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 6 );
   glfwWindowHint( GLFW_DOUBLEBUFFER, GLFW_TRUE );
   glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );

   Window = glfwCreateWindow( FrameWidth, FrameHeight, "Main Camera", nullptr, nullptr );
   glfwMakeContextCurrent( Window );

   if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
      std::cout << "Failed to initialize GLAD" << std::endl;
      return;
   }
   
   registerCallbacks();
   
   glEnable( GL_DEPTH_TEST );
   glClearColor( 1.0f, 1.0f, 1.0f, 1.0f );

   MainCamera->updateWindowSize( FrameWidth, FrameHeight );

   const std::string shader_directory_path = std::string(CMAKE_SOURCE_DIR) + "/shaders";
   ObjectShader->setShader(
      std::string(shader_directory_path + "/BasicPipeline.vert").c_str(),
      std::string(shader_directory_path + "/BasicPipeline.frag").c_str()
   );
}

void RendererGL::error(int e, const char* description)
{
   std::ignore = e;
   puts( description );
}

void RendererGL::cleanup(GLFWwindow* window)
{
   glfwSetWindowShouldClose( window, GLFW_TRUE );
}

void RendererGL::keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
   std::ignore = scancode;
   std::ignore = mods;
   if (action != GLFW_PRESS) return;

   switch (key) {
      case GLFW_KEY_UP:
         MainCamera->moveForward();
         break;
      case GLFW_KEY_DOWN:
         MainCamera->moveBackward();
         break;
      case GLFW_KEY_LEFT:
         MainCamera->moveLeft();
         break;
      case GLFW_KEY_RIGHT:
         MainCamera->moveRight();
         break;
      case GLFW_KEY_W:
         MainCamera->moveUp();
         break;
      case GLFW_KEY_S:
         MainCamera->moveDown();
         break;
      case GLFW_KEY_I:
         MainCamera->resetCamera();
         break;
      case GLFW_KEY_L:
         Lights->toggleLightSwitch();
         std::cout << "Light Turned " << (Lights->isLightOn() ? "On!\n" : "Off!\n");
         break;
      case GLFW_KEY_SPACE:
         DrawMovingObject = !DrawMovingObject;
         break;
      case GLFW_KEY_P: {
         const glm::vec3 pos = MainCamera->getCameraPosition();
         std::cout << "Camera Position: " << pos.x << ", " << pos.y << ", " << pos.z << "\n";
      } break;
      case GLFW_KEY_Q:
      case GLFW_KEY_ESCAPE:
         cleanupWrapper( window );
         break;
      default:
         return;
   }
}

void RendererGL::cursor(GLFWwindow* window, double xpos, double ypos)
{
   if (MainCamera->getMovingState()) {
      const auto x = static_cast<int>(round( xpos ));
      const auto y = static_cast<int>(round( ypos ));
      const int dx = x - ClickedPoint.x;
      const int dy = y - ClickedPoint.y;
      MainCamera->moveForward( -dy );
      MainCamera->rotateAroundWorldY( -dx );

      if (glfwGetMouseButton( window, GLFW_MOUSE_BUTTON_RIGHT ) == GLFW_PRESS) {
         MainCamera->pitch( -dy );
      }

      ClickedPoint.x = x;
      ClickedPoint.y = y;
   }
}

void RendererGL::mouse(GLFWwindow* window, int button, int action, int mods)
{
   std::ignore = mods;
   if (button == GLFW_MOUSE_BUTTON_LEFT) {
      const bool moving_state = action == GLFW_PRESS;
      if (moving_state) {
         double x, y;
         glfwGetCursorPos( window, &x, &y );
         ClickedPoint.x = static_cast<int>(round( x ));
         ClickedPoint.y = static_cast<int>(round( y ));
      }
      MainCamera->setMovingState( moving_state );
   }
}

void RendererGL::mousewheel(GLFWwindow* window, double xoffset, double yoffset) const
{
   std::ignore = window;
   std::ignore = xoffset;
   if (yoffset >= 0.0) MainCamera->zoomIn();
   else MainCamera->zoomOut();
}

void RendererGL::reshape(GLFWwindow* window, int width, int height) const
{
   std::ignore = window;
   MainCamera->updateWindowSize( width, height );
   glViewport( 0, 0, width, height );
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

void RendererGL::setLights() const
{  
   glm::vec4 light_position(-10.0f, 10.0f, 10.0f, 1.0f);
   glm::vec4 ambient_color(0.3f, 0.3f, 0.3f, 1.0f);
   glm::vec4 diffuse_color(0.7f, 0.7f, 0.7f, 1.0f);
   glm::vec4 specular_color(0.9f, 0.9f, 0.9f, 1.0f);
   Lights->addLight( light_position, ambient_color, diffuse_color, specular_color );

   light_position = glm::vec4(0.0f, 35.0f, 10.0f, 1.0f);
   ambient_color = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);
   diffuse_color = glm::vec4(0.9f, 0.5f, 0.1f, 1.0f);
   specular_color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
   glm::vec3 spotlight_direction(0.0f, -1.0f, -1.5f);
   float spotlight_exponent = 128;
   float spotlight_cutoff_angle_in_degree = 7.0f;
   Lights->addLight( 
      light_position, 
      ambient_color, 
      diffuse_color, 
      specular_color,
      spotlight_direction,
      spotlight_exponent,
      spotlight_cutoff_angle_in_degree
   );  
}

void RendererGL::setObject() const
{
   if (Object->getVAO() != 0) return;

   Object->setSquareObject( GL_TRIANGLES );

   const glm::vec4 diffuse_color = { 0.5f, 0.7f, 0.2f, 1.0f };
   Object->setDiffuseReflectionColor( diffuse_color );
}

void RendererGL::drawObject(const float& scale_factor) const
{
   MainCamera->updateWindowSize( FrameWidth, FrameHeight );
   glViewport( 0, 0, FrameWidth, FrameHeight );

   glBindFramebuffer( GL_FRAMEBUFFER, 0 );
   glUseProgram( ObjectShader->getShaderProgram() );

   const glm::mat4 to_origin = translate( glm::mat4(1.0f), glm::vec3(-0.5f, -0.5f, 0.0f) );
   const glm::mat4 scale_matrix = scale( glm::mat4(1.0f), glm::vec3(scale_factor, scale_factor, scale_factor) );
   const glm::mat4 move_back = translate( glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -50.0f) );
   glm::mat4 to_world = move_back * scale_matrix * to_origin;
   if (DrawMovingObject) {
      to_world = rotate( glm::mat4(1.0f), static_cast<float>(ObjectRotationAngle), glm::vec3(0.0f, 0.0f, 1.0f) ) * to_world;
   }

   ObjectShader->transferBasicTransformationUniforms( to_world, MainCamera.get() );
   Object->transferUniformsToShader( ObjectShader.get() );
   Lights->transferUniformsToShader( ObjectShader.get() );
   
   glBindVertexArray( Object->getVAO() );
   glDrawArrays( Object->getDrawMode(), 0, Object->getVertexNum() );
}

void RendererGL::render() const
{
   glClear( OPENGL_COLOR_BUFFER_BIT | OPENGL_DEPTH_BUFFER_BIT );

   drawObject( 20.0f );

   glBindVertexArray( 0 );
   glUseProgram( 0 );
}

void RendererGL::update()
{
   if (DrawMovingObject) {
      ObjectRotationAngle += 3;
      if (ObjectRotationAngle == 360) ObjectRotationAngle = 0;
   }
}

void RendererGL::play()
{
   if (glfwWindowShouldClose( Window )) initialize();

   setLights();
   setObject();
   ObjectShader->setUniformLocations( Lights->getTotalLightNum() );

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

      glfwSwapBuffers( Window );
      glfwPollEvents();
   }
   glfwDestroyWindow( Window );
}