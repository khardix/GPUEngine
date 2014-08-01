#include<geGL/VertexArrays/VertexArrayObjectsCommands.h>

namespace ge
{
  namespace gl
  {
    GenVertexArrays::GenVertexArrays(
        GLsizei  n,
        GLuint  *arrays){
      this->n      = n;
      this->arrays = arrays;
    }
    void GenVertexArrays::apply(){
      glGenVertexArrays(
          this->n,
          this->arrays);
    }
    DeleteVertexArrays::DeleteVertexArrays(
        GLsizei  n,
        GLuint  *arrays){
      this->n = n;
      this->arrays = arrays;
    }
    void DeleteVertexArrays::apply(){
      glDeleteVertexArrays(
          this->n,
          this->arrays);
    }
    BindVertexArray::BindVertexArray(GLuint array){
      this->array = array;
    }
    void BindVertexArray::apply(){
      glBindVertexArray(this->array);
    }
  }//ogl
}//ge

