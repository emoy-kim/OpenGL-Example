#include "Renderer.h"

//------------------------------------------------------------------
//
// Shader Class
//
//------------------------------------------------------------------

ShaderGL::ShaderGL() : ShaderProgram( 0 )
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

bool ShaderGL::checkCompileErrors(const GLuint& vertex_shader, const GLuint& fragment_shader)
{
   GLint compiled[2] = { 0, 0 };
   glGetShaderiv( vertex_shader, GL_COMPILE_STATUS, &compiled[0] );
   glGetShaderiv( fragment_shader, GL_COMPILE_STATUS, &compiled[1] );

   GLuint shaders[2] = { vertex_shader, fragment_shader };
   for (int i = 0; i <= 1; ++i) {
      if (compiled[i] == GL_FALSE) {
         GLint max_length = 0;
         glGetShaderiv( shaders[i], GL_INFO_LOG_LENGTH, &max_length );

         if (i == 0) cout << " === Vertex Shader Log ===" << endl;
         else cout << " === Fragment Shader Log ===" << endl;

         vector<GLchar> error_log(max_length);
         glGetShaderInfoLog( shaders[i], max_length, &max_length, &error_log[0] );
         for (const auto& c : error_log) {
            cout << c;
         }
         glDeleteShader( shaders[i] );
      }
   }
   return compiled[0] && compiled[1];
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
   if (!checkCompileErrors( vertex_shader, fragment_shader )) return;

   ShaderProgram = glCreateProgram();
   glAttachShader( ShaderProgram, vertex_shader );
   glAttachShader( ShaderProgram, fragment_shader );
   glLinkProgram( ShaderProgram );
   
   glDeleteShader( vertex_shader );
   glDeleteShader( fragment_shader );
}

void ShaderGL::setUniformLocations(const uint& light_num)
{
   Location.World = glGetUniformLocation( ShaderProgram, "WorldMatrix" );
   Location.View = glGetUniformLocation( ShaderProgram, "ViewMatrix" );
   Location.Projection = glGetUniformLocation( ShaderProgram, "ProjectionMatrix" );
   Location.ModelViewProjection = glGetUniformLocation( ShaderProgram, "ModelViewProjectionMatrix" );

   Location.MaterialEmission = glGetUniformLocation( ShaderProgram, "Material.EmissionColor" );
   Location.MaterialAmbient = glGetUniformLocation( ShaderProgram, "Material.AmbientColor" );
   Location.MaterialDiffuse = glGetUniformLocation( ShaderProgram, "Material.DiffuseColor" );
   Location.MaterialSpecular = glGetUniformLocation( ShaderProgram, "Material.SpecularColor" );
   Location.MaterialSpecularExponent = glGetUniformLocation( ShaderProgram, "Material.SpecularExponent" );

   Location.Texture = glGetUniformLocation( ShaderProgram, "BaseTexture" );

   Location.UseLight = glGetUniformLocation( ShaderProgram, "UseLight" );
   Location.LightNum = glGetUniformLocation( ShaderProgram, "LightNum" );
   Location.GlobalAmbient = glGetUniformLocation( ShaderProgram, "GlobalAmbient" );

   Location.Lights.resize( light_num );
   for (uint i = 0; i < light_num; ++i) {
      Location.Lights[i].LightSwitch = glGetUniformLocation( ShaderProgram, string("Lights[" + to_string( i ) + "].LightSwitch").c_str() );
      Location.Lights[i].LightPosition = glGetUniformLocation( ShaderProgram, string("Lights[" + to_string( i ) + "].Position").c_str() );
      Location.Lights[i].LightAmbient = glGetUniformLocation( ShaderProgram, string("Lights[" + to_string( i ) + "].AmbientColor").c_str() );
      Location.Lights[i].LightDiffuse = glGetUniformLocation( ShaderProgram, string("Lights[" + to_string( i ) + "].DiffuseColor").c_str() );
      Location.Lights[i].LightSpecular = glGetUniformLocation( ShaderProgram, string("Lights[" + to_string( i ) + "].SpecularColor").c_str() );
      Location.Lights[i].SpotlightDirection = glGetUniformLocation( ShaderProgram, string("Lights[" + to_string( i ) + "].SpotlightDirection").c_str() );
      Location.Lights[i].SpotlightExponent = glGetUniformLocation( ShaderProgram, string("Lights[" + to_string( i ) + "].SpotlightExponent").c_str() );
      Location.Lights[i].SpotlightCutoffAngle = glGetUniformLocation( ShaderProgram, string("Lights[" + to_string( i ) + "].SpotlightCutoffAngle").c_str() );
      Location.Lights[i].LightAttenuationFactors = glGetUniformLocation( ShaderProgram, string("Lights[" + to_string( i ) + "].AttenuationFactors").c_str() );
   }
}


//------------------------------------------------------------------
//
// Light Class
//
//------------------------------------------------------------------

LightGL::LightGL() :
   TurnLightOn( false ), GlobalAmbientColor( 0.2f, 0.2f, 0.2f, 1.0f ), TotalLightNum( 0 )
{
}

bool LightGL::isLightOn() const
{
   return TurnLightOn;
}

void LightGL::toggleLightSwitch()
{
   TurnLightOn = !TurnLightOn;
}

void LightGL::addLight(
   const vec4& light_position,
   const vec4& ambient_color,
   const vec4& diffuse_color,
   const vec4& specular_color,
   const vec3& spotlight_direction,
   const float& spotlight_exponent,
   const float& spotlight_cutoff_angle_in_degree,
   const vec3& attenuation_factor
)
{
   Positions.emplace_back( light_position );
   
   AmbientColors.emplace_back( ambient_color );
   DiffuseColors.emplace_back( diffuse_color );
   SpecularColors.emplace_back( specular_color );

   SpotlightDirections.emplace_back( spotlight_direction );
   SpotlightExponents.emplace_back( spotlight_exponent );
   SpotlightCutoffAngles.emplace_back( spotlight_cutoff_angle_in_degree );

   AttenuationFactors.emplace_back( attenuation_factor );

   IsActivated.emplace_back( true );

   TotalLightNum = Positions.size();
}

void LightGL::setGlobalAmbientColor(const vec4& global_ambient_color)
{
   GlobalAmbientColor = global_ambient_color;
}

void LightGL::setAmbientColor(const vec4& ambient_color, const uint& light_index)
{
   if (light_index >= TotalLightNum) return;
   AmbientColors[light_index] = ambient_color;
}

void LightGL::setDiffuseColor(const vec4& diffuse_color, const uint& light_index)
{
   if (light_index >= TotalLightNum) return;
   DiffuseColors[light_index] = diffuse_color;
}

void LightGL::setSpecularColor(const vec4& specular_color, const uint& light_index)
{
   if (light_index >= TotalLightNum) return;
   SpecularColors[light_index] = specular_color;
}

void LightGL::setSpotlightDirection(const vec3& spotlight_direction, const uint& light_index)
{
   if (light_index >= TotalLightNum) return;
   SpotlightDirections[light_index] = spotlight_direction;
}

void LightGL::setSpotlightExponent(const float& spotlight_exponent, const uint& light_index)
{
   if (light_index >= TotalLightNum) return;
   SpotlightExponents[light_index] = spotlight_exponent;
}

void LightGL::setSpotlightCutoffAngle(const float& spotlight_cutoff_angle_in_degree, const uint& light_index)
{
   if (light_index >= TotalLightNum) return;
   SpotlightCutoffAngles[light_index] = spotlight_cutoff_angle_in_degree;
}

void LightGL::setAttenuationFactor(const vec3& attenuation_factor, const uint& light_index)
{
   if (light_index >= TotalLightNum) return;
   AttenuationFactors[light_index] = attenuation_factor;
}

void LightGL::setLightPosition(const vec4& light_position, const uint& light_index)
{
   if (light_index >= TotalLightNum) return;
   Positions[light_index] = light_position;
}

void LightGL::activateLight(const uint& light_index)
{
   if (light_index >= TotalLightNum) return;
   IsActivated[light_index] = true;
}

void LightGL::deactivateLight(const uint& light_index)
{
   if (light_index >= TotalLightNum) return;
   IsActivated[light_index] = false;
}

void LightGL::transferUniformsToShader(ShaderGL& shader)
{
   glUniform1i( shader.Location.UseLight, TurnLightOn ? 1 : 0 );
   glUniform1i( shader.Location.LightNum, TotalLightNum );
   glUniform4fv( shader.Location.GlobalAmbient, 1, &GlobalAmbientColor[0] );

   for (uint i = 0; i < TotalLightNum; ++i) {
      glUniform1i( shader.Location.Lights[i].LightSwitch, IsActivated[0] ? 1 : 0 );
      glUniform4fv( shader.Location.Lights[i].LightPosition, 1, &Positions[i][0] );
      glUniform4fv( shader.Location.Lights[i].LightAmbient, 1, &AmbientColors[i][0] );
      glUniform4fv( shader.Location.Lights[i].LightDiffuse, 1, &DiffuseColors[i][0] );
      glUniform4fv( shader.Location.Lights[i].LightSpecular, 1, &SpecularColors[i][0] );
      glUniform3fv( shader.Location.Lights[i].SpotlightDirection, 1, &SpotlightDirections[i][0] ); 
      glUniform1f( shader.Location.Lights[i].SpotlightExponent, SpotlightExponents[i] );
      glUniform1f( shader.Location.Lights[i].SpotlightCutoffAngle, SpotlightCutoffAngles[i] );
      glUniform3fv( shader.Location.Lights[i].LightAttenuationFactors, 1, &AttenuationFactors[i][0] );
   }
}


//------------------------------------------------------------------
//
// Camera Class
//
//------------------------------------------------------------------

CameraGL::CameraGL() : 
   CameraGL(vec3(0.0f, 0.0f, 10.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f))
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
   ZoomSensitivity( 1.0f ), MoveSensitivity( 0.05f ), RotationSensitivity( 0.005f ), IsMoving( false ),
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
   ViewMatrix = glm::rotate( ViewMatrix, static_cast<float>(-angle) * RotationSensitivity, u_axis );
   updateCamera();
}

void CameraGL::yaw(const int& angle)
{
   const vec3 v_axis(ViewMatrix[0][1], ViewMatrix[1][1], ViewMatrix[2][1]);
   ViewMatrix = glm::rotate( ViewMatrix, static_cast<float>(-angle) * RotationSensitivity, v_axis );
   updateCamera();
}

void CameraGL::rotateAroundWorldY(const int& angle)
{
   const vec3 world_y(0.0f, 1.0f, 0.0f);
   ViewMatrix = glm::rotate( mat4(1.0f), static_cast<float>(-angle) * RotationSensitivity, world_y ) * ViewMatrix;
   updateCamera();
}

void CameraGL::moveForward(const int& delta)
{
   const vec3 n_axis(ViewMatrix[0][2], ViewMatrix[1][2], ViewMatrix[2][2]);
   ViewMatrix = translate( ViewMatrix, MoveSensitivity * static_cast<float>(-delta) *  -n_axis );
   updateCamera();
}

void CameraGL::moveBackward(const int& delta)
{
   const vec3 n_axis(ViewMatrix[0][2], ViewMatrix[1][2], ViewMatrix[2][2]);
   ViewMatrix = translate( ViewMatrix, MoveSensitivity * static_cast<float>(-delta) * n_axis );
   updateCamera();
}

void CameraGL::moveLeft(const int& delta)
{
   const vec3 u_axis(ViewMatrix[0][0], ViewMatrix[1][0], ViewMatrix[2][0]);
   ViewMatrix = translate( ViewMatrix, MoveSensitivity * static_cast<float>(-delta) * -u_axis );
   updateCamera();
}

void CameraGL::moveRight(const int& delta)
{
   const vec3 u_axis(ViewMatrix[0][0], ViewMatrix[1][0], ViewMatrix[2][0]);
   ViewMatrix = translate( ViewMatrix, MoveSensitivity * static_cast<float>(-delta) * u_axis );
   updateCamera();
}

void CameraGL::moveUp(const int& delta)
{
   const vec3 v_axis(ViewMatrix[0][1], ViewMatrix[1][1], ViewMatrix[2][1]);
   ViewMatrix = translate( ViewMatrix, MoveSensitivity * static_cast<float>(-delta) * v_axis );
   updateCamera();
}

void CameraGL::moveDown(const int& delta)
{
   const vec3 v_axis(ViewMatrix[0][1], ViewMatrix[1][1], ViewMatrix[2][1]);
   ViewMatrix = translate( ViewMatrix, MoveSensitivity * static_cast<float>(-delta) * -v_axis );
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

ObjectGL::ObjectGL() : 
   ObjVAO( 0 ), ObjVBO( 0 ), DrawMode( 0 ), TextureID( 0 ), VerticesCount( 0 ),
   EmissionColor( 0.0f, 0.0f, 0.0f, 1.0f ), AmbientReflectionColor( 0.2f, 0.2f, 0.2f, 1.0f ),
   DiffuseReflectionColor( 0.8f, 0.8f, 0.8f, 1.0f ), SpecularReflectionColor( 0.0f, 0.0f, 0.0f, 1.0f ),
   SpecularReflectionExponent( 0.0f )
{
}

void ObjectGL::setEmissionColor(const vec4& emission_color)
{
   EmissionColor = emission_color;
}

void ObjectGL::setAmbientReflectionColor(const vec4& ambient_reflection_color)
{
   AmbientReflectionColor = ambient_reflection_color;   
}

void ObjectGL::setDiffuseReflectionColor(const vec4& diffuse_reflection_color)
{
   DiffuseReflectionColor = diffuse_reflection_color;
}

void ObjectGL::setSpecularReflectionColor(const vec4& specular_reflection_color)
{
   SpecularReflectionColor = specular_reflection_color;
}

void ObjectGL::setSpecularReflectionExponent(const float& specular_reflection_exponent)
{
   SpecularReflectionExponent = specular_reflection_exponent;
}

void ObjectGL::prepareTexture2DFromMat(const Mat& texture) const
{
   // NOTE: 'texture' is going to be flipped vertically.
   // OpenGL texture's origin is bottom-left, but OpenCV Mat's is top-left.
   const int width = texture.cols;
   const int height = texture.rows;
   glTextureStorage2D( TextureID, 1, GL_RGBA8, width, height );
   glTextureSubImage2D( TextureID, 0, 0, 0, width, height, GL_BGR, GL_UNSIGNED_BYTE, texture.data );
}

void ObjectGL::prepareTexture(const Mat& texture, const bool& normals_exist)
{
   glCreateTextures( GL_TEXTURE_2D, 1, &TextureID );
   
   prepareTexture2DFromMat( texture );

   glTextureParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
   glTextureParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
   glTextureParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
   glTextureParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );

   const uint offset = normals_exist ? 6 : 3;
   glVertexArrayAttribFormat( ObjVAO, TextureLoc, 2, GL_FLOAT, GL_FALSE, offset * sizeof(GLfloat) );
   glVertexArrayAttribBinding( ObjVAO, TextureLoc, 0 );
   glEnableVertexArrayAttrib( ObjVAO, TextureLoc );
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
   glTextureStorage2D( TextureID, 1, GL_RGBA8, width, height );
   glTextureSubImage2D( TextureID, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data );
   cout << " * Loaded " << width << " x " << height << " RGBA texture into graphics memory." << endl << endl;

   FreeImage_Unload( texture_32bit );
   if (n_bits_per_pixel != 32) {
      FreeImage_Unload( texture );
   }
}

void ObjectGL::prepareTexture(const string& texture_file_name, const bool& normals_exist)
{
   glCreateTextures( GL_TEXTURE_2D, 1, &TextureID );
   
   prepareTexture2DFromFile( texture_file_name );
   
   glTextureParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
   glTextureParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
   glTextureParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
   glTextureParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );

   const uint offset = normals_exist ? 6 : 3;
   glVertexArrayAttribFormat( ObjVAO, TextureLoc, 2, GL_FLOAT, GL_FALSE, offset * sizeof(GLfloat) );
   glVertexArrayAttribBinding( ObjVAO, TextureLoc, 0 );
   glEnableVertexArrayAttrib( ObjVAO, TextureLoc );
}

void ObjectGL::prepareNormal() const
{
   glVertexArrayAttribFormat( ObjVAO, NormalLoc, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat) );
   glVertexArrayAttribBinding( ObjVAO, NormalLoc, 0 );
   glEnableVertexArrayAttrib( ObjVAO, NormalLoc );
}

void ObjectGL::prepareVertexBuffer(const int& n_bytes_per_vertex)
{
   glCreateBuffers( 1, &ObjVBO );
   glNamedBufferStorage( ObjVBO, sizeof(GLfloat) * DataBuffer.size(), DataBuffer.data(), GL_DYNAMIC_STORAGE_BIT );

   glCreateVertexArrays( 1, &ObjVAO );
   glVertexArrayVertexBuffer( ObjVAO, 0, ObjVBO, 0, n_bytes_per_vertex );
   glVertexArrayAttribFormat( ObjVAO, VertexLoc, 3, GL_FLOAT, GL_FALSE, 0 );
   glVertexArrayAttribBinding( ObjVAO, VertexLoc, 0 );
   glEnableVertexArrayAttrib( ObjVAO, VertexLoc );
}

void ObjectGL::setObject(
   const GLenum& draw_mode,  
   const vector<vec3>& vertices
)
{
   DrawMode = draw_mode;
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
   const GLenum& draw_mode, 
   const vector<vec3>& vertices,
   const vector<vec3>& normals
)
{
   DrawMode = draw_mode;
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
   prepareNormal();
}

void ObjectGL::setObject(
   const GLenum& draw_mode, 
   const vector<vec3>& vertices,
   const vector<vec2>& textures, 
   const string& texture_file_name
)
{
   DrawMode = draw_mode;
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
   prepareTexture( texture_file_name, false );
}

void ObjectGL::setObject(
   const GLenum& draw_mode, 
   const vector<vec3>& vertices,
   const vector<vec2>& textures, 
   const Mat& texture
)
{
   DrawMode = draw_mode;
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
   prepareTexture( texture, false );
}

void ObjectGL::setObject(
   const GLenum& draw_mode, 
   const vector<vec3>& vertices, 
   const vector<vec3>& normals, 
   const vector<vec2>& textures,
   const string& texture_file_name
)
{
   DrawMode = draw_mode;
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
   prepareNormal();
   prepareTexture( texture_file_name, true );
}

void ObjectGL::setObject(
   const GLenum& draw_mode, 
   const vector<vec3>& vertices,
   const vector<vec3>& normals, 
   const vector<vec2>& textures, 
   const Mat& texture
)
{
   DrawMode = draw_mode;
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
   prepareNormal();
   prepareTexture( texture, true );
}

void ObjectGL::transferUniformsToShader(ShaderGL& shader)
{
   glUniform4fv( shader.Location.MaterialEmission, 1, &EmissionColor[0] );
   glUniform4fv( shader.Location.MaterialAmbient, 1, &AmbientReflectionColor[0] );
   glUniform4fv( shader.Location.MaterialDiffuse, 1, &DiffuseReflectionColor[0] );
   glUniform4fv( shader.Location.MaterialSpecular, 1, &SpecularReflectionColor[0] );
   glUniform1f( shader.Location.MaterialSpecularExponent, SpecularReflectionExponent );

   glUniform1i( shader.Location.Texture, shader.Location.TextureUnit );
}


//------------------------------------------------------------------
//
// Renderer Class
//
//------------------------------------------------------------------

RendererGL* RendererGL::Renderer = nullptr;
RendererGL::RendererGL() : 
   Window( nullptr ), ClickedPoint( -1, -1 ), DrawMovingObject( false ), ObjectRotationAngle( 0 )
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

void RendererGL::initialize()
{
   if (!glfwInit()) {
      cout << "Cannot Initialize OpenGL..." << endl;
      return;
   }
   glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
   glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 5 );
   glfwWindowHint( GLFW_DOUBLEBUFFER, GLFW_TRUE );
   glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );

   const int width = 1920;
   const int height = 1080;
   Window = glfwCreateWindow( width, height, "Main Camera", nullptr, nullptr );
   glfwMakeContextCurrent( Window );
   glewInit();
   
   registerCallbacks();
   
   glEnable( GL_DEPTH_TEST );
   glClearColor( 1.0f, 1.0f, 1.0f, 1.0f );

   MainCamera.updateWindowSize( width, height );
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
      case GLFW_KEY_L:
         Lights.toggleLightSwitch();
         cout << "Light Turned " << (Lights.isLightOn() ? "On!" : "Off!") << endl;
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
      const auto x = static_cast<int>(round( xpos ));
      const auto y = static_cast<int>(round( ypos ));
      const int dx = x - ClickedPoint.x;
      const int dy = y - ClickedPoint.y;
      MainCamera.moveForward( -dy );
      MainCamera.rotateAroundWorldY( -dx );

      if (glfwGetMouseButton( window, GLFW_MOUSE_BUTTON_RIGHT ) == GLFW_PRESS) {
         MainCamera.pitch( -dy );
      }

      ClickedPoint.x = x;
      ClickedPoint.y = y;
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
         ClickedPoint.x = static_cast<int>(round( x ));
         ClickedPoint.y = static_cast<int>(round( y ));
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

void RendererGL::setLights()
{  
   vec4 light_position(-10.0f, 10.0f, 10.0f, 1.0f);
   vec4 ambient_color(0.3f, 0.3f, 0.3f, 1.0f);
   vec4 diffuse_color(0.7f, 0.7f, 0.7f, 1.0f);
   vec4 specular_color(0.9f, 0.9f, 0.9f, 1.0f);
   Lights.addLight( light_position, ambient_color, diffuse_color, specular_color );

   light_position = vec4(0.0f, 35.0f, 10.0f, 1.0f);
   ambient_color = vec4(0.2f, 0.2f, 0.2f, 1.0f);
   diffuse_color = vec4(0.9f, 0.5f, 0.1f, 1.0f);
   specular_color = vec4(1.0f, 1.0f, 1.0f, 1.0f);
   vec3 spotlight_direction(0.0f, -1.0f, -1.5f);
   float spotlight_exponent = 128;
   float spotlight_cutoff_angle_in_degree = 7.0f;
   Lights.addLight( 
      light_position, 
      ambient_color, 
      diffuse_color, 
      specular_color,
      spotlight_direction,
      spotlight_exponent,
      spotlight_cutoff_angle_in_degree
   );  
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

   vector<vec3> square_normals;
   square_normals.emplace_back( 0.0f, 0.0f, 1.0f );
   square_normals.emplace_back( 0.0f, 0.0f, 1.0f );
   square_normals.emplace_back( 0.0f, 0.0f, 1.0f );
   
   square_normals.emplace_back( 0.0f, 0.0f, 1.0f );
   square_normals.emplace_back( 0.0f, 0.0f, 1.0f );
   square_normals.emplace_back( 0.0f, 0.0f, 1.0f );
   
   Object.setObject( GL_TRIANGLES, square_vertices, square_normals );

   const vec4 diffuse_color = { 0.5f, 0.7f, 0.2f, 1.0f };
   Object.setDiffuseReflectionColor( diffuse_color );
}

void RendererGL::drawObject(const float& scale_factor)
{
   glUseProgram( ObjectShader.ShaderProgram );

   const mat4 to_origin = translate( mat4(1.0f), vec3(-0.5f, -0.5f, 0.0f) );
   const mat4 scale_matrix = scale( mat4(1.0f), vec3(scale_factor, scale_factor, scale_factor) );
   const mat4 move_back = translate( mat4(1.0f), vec3(0.0f, 0.0f, -50.0f) );
   mat4 to_world = move_back * scale_matrix * to_origin;
   if (DrawMovingObject) {
      to_world = rotate( mat4(1.0f), static_cast<float>(ObjectRotationAngle), vec3(0.0f, 0.0f, 1.0f) ) * to_world;
   }

   const mat4 model_view_projection = MainCamera.ProjectionMatrix * MainCamera.ViewMatrix * to_world;
   glUniformMatrix4fv( ObjectShader.Location.World, 1, GL_FALSE, &to_world[0][0] );
   glUniformMatrix4fv( ObjectShader.Location.View, 1, GL_FALSE, &MainCamera.ViewMatrix[0][0] );
   glUniformMatrix4fv( ObjectShader.Location.Projection, 1, GL_FALSE, &MainCamera.ProjectionMatrix[0][0] );
   glUniformMatrix4fv( ObjectShader.Location.ModelViewProjection, 1, GL_FALSE, &model_view_projection[0][0] );

   Object.transferUniformsToShader( ObjectShader );
   Lights.transferUniformsToShader( ObjectShader );
   
   glBindVertexArray( Object.ObjVAO );
   glDrawArrays( Object.DrawMode, 0, Object.VerticesCount );
}

void RendererGL::render()
{
   glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

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
   ObjectShader.setUniformLocations( Lights.TotalLightNum );

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