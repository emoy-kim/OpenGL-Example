#include "object.h"

ObjectGL::ObjectGL() :
   ImageBuffer( nullptr ), VAO( 0 ), VBO( 0 ), DrawMode( 0 ), VerticesCount( 0 ),
   EmissionColor( 0.0f, 0.0f, 0.0f, 1.0f ),
   AmbientReflectionColor( 0.2f, 0.2f, 0.2f, 1.0f ),
   DiffuseReflectionColor( 0.8f, 0.8f, 0.8f, 1.0f ),
   SpecularReflectionColor( 0.0f, 0.0f, 0.0f, 1.0f ), SpecularReflectionExponent( 0.0f )
{
}

ObjectGL::~ObjectGL()
{
   if (VAO != 0) {
      glDeleteVertexArrays( 1, &VAO );
      glDeleteBuffers( 1, &VBO );
   }
   for (const auto& texture_id : TextureID) {
      if (texture_id != 0) glDeleteTextures( 1, &texture_id );
   }
   for (const auto& buffer : CustomBuffers) {
      if (buffer.second != 0) glDeleteBuffers( 1, &buffer.second );
   }
   delete [] ImageBuffer;
}

void ObjectGL::setEmissionColor(const glm::vec4& emission_color)
{
   EmissionColor = emission_color;
}

void ObjectGL::setAmbientReflectionColor(const glm::vec4& ambient_reflection_color)
{
   AmbientReflectionColor = ambient_reflection_color;
}

void ObjectGL::setDiffuseReflectionColor(const glm::vec4& diffuse_reflection_color)
{
   DiffuseReflectionColor = diffuse_reflection_color;
}

void ObjectGL::setSpecularReflectionColor(const glm::vec4& specular_reflection_color)
{
   SpecularReflectionColor = specular_reflection_color;
}

void ObjectGL::setSpecularReflectionExponent(const float& specular_reflection_exponent)
{
   SpecularReflectionExponent = specular_reflection_exponent;
}

bool ObjectGL::prepareTexture2DUsingFreeImage(const std::string& file_path, bool is_grayscale) const
{
   const FREE_IMAGE_FORMAT format = FreeImage_GetFileType( file_path.c_str(), 0 );
   FIBITMAP* texture = FreeImage_Load( format, file_path.c_str() );
   if (!texture) return false;

   FIBITMAP* texture_converted;
   const uint n_bits_per_pixel = FreeImage_GetBPP( texture );
   const uint n_bits = is_grayscale ? 8 : 32;
   if (is_grayscale) {
      texture_converted = n_bits_per_pixel == n_bits ? texture : FreeImage_GetChannel( texture, FICC_RED );
   }
   else {
      texture_converted = n_bits_per_pixel == n_bits ? texture : FreeImage_ConvertTo32Bits( texture );
   }

   const auto width = static_cast<GLsizei>(FreeImage_GetWidth( texture_converted ));
   const auto height = static_cast<GLsizei>(FreeImage_GetHeight( texture_converted ));
   GLvoid* data = FreeImage_GetBits( texture_converted );
   glTextureStorage2D( TextureID.back(), 1, is_grayscale ? GL_R8 : GL_RGBA8, width, height );
   glTextureSubImage2D( TextureID.back(), 0, 0, 0, width, height, is_grayscale ? GL_RED : GL_BGRA, GL_UNSIGNED_BYTE, data );

   FreeImage_Unload( texture_converted );
   if (n_bits_per_pixel != n_bits) FreeImage_Unload( texture );
   return true;
}

int ObjectGL::addTexture(const std::string& texture_file_path, bool is_grayscale)
{
   GLuint texture_id = 0;
   glCreateTextures( GL_TEXTURE_2D, 1, &texture_id );
   TextureID.emplace_back( texture_id );
   if (!prepareTexture2DUsingFreeImage( texture_file_path, is_grayscale )) {
      glDeleteTextures( 1, &texture_id );
      TextureID.erase( TextureID.end() - 1 );
      std::cerr << "Could not read image file " << texture_file_path.c_str() << "\n";
      return -1;
   }

   glTextureParameteri( texture_id, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
   glTextureParameteri( texture_id, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
   glTextureParameteri( texture_id, GL_TEXTURE_WRAP_S, GL_REPEAT );
   glTextureParameteri( texture_id, GL_TEXTURE_WRAP_T, GL_REPEAT );
   glGenerateTextureMipmap( texture_id );
   return static_cast<int>(TextureID.size() - 1);
}

void ObjectGL::addTexture(int width, int height, bool is_grayscale)
{
   GLuint texture_id = 0;
   glCreateTextures( GL_TEXTURE_2D, 1, &texture_id );
   glTextureStorage2D(
      texture_id,
      1,
      is_grayscale ? GL_R8 : GL_RGBA8,
      width,
      height
   );
   glTextureParameteri( texture_id, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
   glTextureParameteri( texture_id, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
   glTextureParameteri( texture_id, GL_TEXTURE_WRAP_S, GL_REPEAT );
   glTextureParameteri( texture_id, GL_TEXTURE_WRAP_T, GL_REPEAT );
   glGenerateTextureMipmap( texture_id );
   TextureID.emplace_back( texture_id );
}

int ObjectGL::addTexture(const uint8_t* image_buffer, int width, int height, bool is_grayscale)
{
   addTexture( width, height, is_grayscale );
   glTextureSubImage2D(
      TextureID.back(),
      0,
      0,
      0,
      width,
      height,
      is_grayscale ? GL_RED : GL_RGBA,
      GL_UNSIGNED_BYTE,
      image_buffer
   );
   return static_cast<int>(TextureID.size() - 1);
}

void ObjectGL::prepareTexture(bool normals_exist) const
{
   const uint offset = normals_exist ? 6 : 3;
   glVertexArrayAttribFormat( VAO, TextureLocation, 2, GL_FLOAT, GL_FALSE, offset * sizeof( GLfloat ) );
   glEnableVertexArrayAttrib( VAO, TextureLocation );
   glVertexArrayAttribBinding( VAO, TextureLocation, 0 );
}

void ObjectGL::prepareNormal() const
{
   glVertexArrayAttribFormat( VAO, NormalLocation, 3, GL_FLOAT, GL_FALSE, 3 * sizeof( GLfloat ) );
   glEnableVertexArrayAttrib( VAO, NormalLocation );
   glVertexArrayAttribBinding( VAO, NormalLocation, 0 );
}

void ObjectGL::prepareVertexBuffer(int n_bytes_per_vertex)
{
   glCreateBuffers( 1, &VBO );
   glNamedBufferStorage( VBO, sizeof( GLfloat ) * DataBuffer.size(), DataBuffer.data(), GL_DYNAMIC_STORAGE_BIT );

   glCreateVertexArrays( 1, &VAO );
   glVertexArrayVertexBuffer( VAO, 0, VBO, 0, n_bytes_per_vertex );
   glVertexArrayAttribFormat( VAO, VertexLocation, 3, GL_FLOAT, GL_FALSE, 0 );
   glEnableVertexArrayAttrib( VAO, VertexLocation );
   glVertexArrayAttribBinding( VAO, VertexLocation, 0 );
}

void ObjectGL::getSquareObject(
   std::vector<glm::vec3>& vertices,
   std::vector<glm::vec3>& normals,
   std::vector<glm::vec2>& textures
)
{
   vertices = {
      { 1.0f, 0.0f, 0.0f },
      { 1.0f, 1.0f, 0.0f },
      { 0.0f, 1.0f, 0.0f },

      { 1.0f, 0.0f, 0.0f },
      { 0.0f, 1.0f, 0.0f },
      { 0.0f, 0.0f, 0.0f }
   };
   normals = {
      { 0.0f, 0.0f, 1.0f },
      { 0.0f, 0.0f, 1.0f },
      { 0.0f, 0.0f, 1.0f },

      { 0.0f, 0.0f, 1.0f },
      { 0.0f, 0.0f, 1.0f },
      { 0.0f, 0.0f, 1.0f }
   };
   textures = {
      { 1.0f, 0.0f },
      { 1.0f, 1.0f },
      { 0.0f, 1.0f },

      { 1.0f, 0.0f },
      { 0.0f, 1.0f },
      { 0.0f, 0.0f }
   };
}

void ObjectGL::setObject(GLenum draw_mode, const std::vector<glm::vec3>& vertices)
{
   DrawMode = draw_mode;
   VerticesCount = 0;
   DataBuffer.clear();
   for (auto& vertex : vertices) {
      DataBuffer.emplace_back( vertex.x );
      DataBuffer.emplace_back( vertex.y );
      DataBuffer.emplace_back( vertex.z );
      VerticesCount++;
   }
   const int n_bytes_per_vertex = 3 * sizeof( GLfloat );
   prepareVertexBuffer( n_bytes_per_vertex );
}

void ObjectGL::setObject(
   GLenum draw_mode,
   const std::vector<glm::vec3>& vertices,
   const std::vector<glm::vec3>& normals
)
{
   DrawMode = draw_mode;
   VerticesCount = 0;
   DataBuffer.clear();
   for (size_t i = 0; i < vertices.size(); ++i) {
      DataBuffer.emplace_back( vertices[i].x );
      DataBuffer.emplace_back( vertices[i].y );
      DataBuffer.emplace_back( vertices[i].z );
      DataBuffer.emplace_back( normals[i].x );
      DataBuffer.emplace_back( normals[i].y );
      DataBuffer.emplace_back( normals[i].z );
      VerticesCount++;
   }
   const int n_bytes_per_vertex = 6 * sizeof( GLfloat );
   prepareVertexBuffer( n_bytes_per_vertex );
   prepareNormal();
}

void ObjectGL::setObject(
   GLenum draw_mode,
   const std::vector<glm::vec3>& vertices,
   const std::vector<glm::vec2>& textures,
   const std::string& texture_file_path,
   bool is_grayscale
)
{
   DrawMode = draw_mode;
   VerticesCount = 0;
   DataBuffer.clear();
   for (size_t i = 0; i < vertices.size(); ++i) {
      DataBuffer.emplace_back( vertices[i].x );
      DataBuffer.emplace_back( vertices[i].y );
      DataBuffer.emplace_back( vertices[i].z );
      DataBuffer.emplace_back( textures[i].x );
      DataBuffer.emplace_back( textures[i].y );
      VerticesCount++;
   }
   const int n_bytes_per_vertex = 5 * sizeof( GLfloat );
   prepareVertexBuffer( n_bytes_per_vertex );
   prepareTexture( false );
   addTexture( texture_file_path, is_grayscale );
}

void ObjectGL::setObject(
   GLenum draw_mode,
   const std::vector<glm::vec3>& vertices,
   const std::vector<glm::vec3>& normals,
   const std::vector<glm::vec2>& textures
)
{
   DrawMode = draw_mode;
   VerticesCount = 0;
   DataBuffer.clear();
   for (size_t i = 0; i < vertices.size(); ++i) {
      DataBuffer.emplace_back( vertices[i].x );
      DataBuffer.emplace_back( vertices[i].y );
      DataBuffer.emplace_back( vertices[i].z );
      DataBuffer.emplace_back( normals[i].x );
      DataBuffer.emplace_back( normals[i].y );
      DataBuffer.emplace_back( normals[i].z );
      DataBuffer.emplace_back( textures[i].x );
      DataBuffer.emplace_back( textures[i].y );
      VerticesCount++;
   }
   const int n_bytes_per_vertex = 8 * sizeof( GLfloat );
   prepareVertexBuffer( n_bytes_per_vertex );
   prepareNormal();
   prepareTexture( true );
}

void ObjectGL::setObject(
   GLenum draw_mode,
   const std::vector<glm::vec3>& vertices,
   const std::vector<glm::vec3>& normals,
   const std::vector<glm::vec2>& textures,
   const std::string& texture_file_path,
   bool is_grayscale
)
{
   setObject( draw_mode, vertices, normals, textures );
   addTexture( texture_file_path, is_grayscale );
}

void ObjectGL::setSquareObject(GLenum draw_mode, bool use_texture)
{
   std::vector<glm::vec3> square_vertices, square_normals;
   std::vector<glm::vec2> square_textures;
   getSquareObject( square_vertices, square_normals, square_textures );
   if (use_texture) setObject( draw_mode, square_vertices, square_normals, square_textures );
   else setObject( draw_mode, square_vertices, square_normals );
}

void ObjectGL::setSquareObject(
   GLenum draw_mode,
   const std::string& texture_file_path,
   bool is_grayscale
)
{
   std::vector<glm::vec3> square_vertices, square_normals;
   std::vector<glm::vec2> square_textures;
   getSquareObject( square_vertices, square_normals, square_textures );
   setObject( draw_mode, square_vertices, square_normals, square_textures, texture_file_path, is_grayscale );
}

void ObjectGL::updateDataBuffer(const std::vector<glm::vec3>& vertices, const std::vector<glm::vec3>& normals)
{
   assert( VBO != 0 );

   VerticesCount = 0;
   DataBuffer.clear();
   for (size_t i = 0; i < vertices.size(); ++i) {
      DataBuffer.push_back( vertices[i].x );
      DataBuffer.push_back( vertices[i].y );
      DataBuffer.push_back( vertices[i].z );
      DataBuffer.push_back( normals[i].x );
      DataBuffer.push_back( normals[i].y );
      DataBuffer.push_back( normals[i].z );
      VerticesCount++;
   }
   glNamedBufferSubData( VBO, 0, static_cast<GLsizeiptr>(sizeof( GLfloat ) * DataBuffer.size()), DataBuffer.data() );
}

void ObjectGL::updateDataBuffer(
   const std::vector<glm::vec3>& vertices,
   const std::vector<glm::vec3>& normals,
   const std::vector<glm::vec2>& textures
)
{
   assert( VBO != 0 );

   VerticesCount = 0;
   DataBuffer.clear();
   for (size_t i = 0; i < vertices.size(); ++i) {
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
   glNamedBufferSubData( VBO, 0, static_cast<GLsizeiptr>(sizeof( GLfloat ) * DataBuffer.size()), DataBuffer.data() );
}

void ObjectGL::replaceVertices(
   const std::vector<glm::vec3>& vertices,
   bool normals_exist,
   bool textures_exist
)
{
   assert( VBO != 0 );

   VerticesCount = 0;
   int step = 3;
   if (normals_exist) step += 3;
   if (textures_exist) step += 2;
   for (size_t i = 0; i < vertices.size(); ++i) {
      DataBuffer[i * step] = vertices[i].x;
      DataBuffer[i * step + 1] = vertices[i].y;
      DataBuffer[i * step + 2] = vertices[i].z;
      VerticesCount++;
   }
   glNamedBufferSubData( VBO, 0, static_cast<GLsizeiptr>(sizeof( GLfloat ) * VerticesCount * step), DataBuffer.data() );
}

void ObjectGL::replaceVertices(
   const std::vector<float>& vertices,
   bool normals_exist,
   bool textures_exist
)
{
   assert( VBO != 0 );

   VerticesCount = 0;
   int step = 3;
   if (normals_exist) step += 3;
   if (textures_exist) step += 2;
   for (size_t i = 0, j = 0; i < vertices.size(); i += 3, ++j) {
      DataBuffer[j * step] = vertices[i];
      DataBuffer[j * step + 1] = vertices[i + 1];
      DataBuffer[j * step + 2] = vertices[i + 2];
      VerticesCount++;
   }
   glNamedBufferSubData( VBO, 0, static_cast<GLsizeiptr>(sizeof( GLfloat ) * VerticesCount * step), DataBuffer.data() );
}