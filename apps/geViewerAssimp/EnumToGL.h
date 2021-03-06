#pragma once

#include <geSG/AttributeDescriptor.h>
#include <geSG/Image.h>
#include <geSG/Mesh.h>

namespace fsg
{
   GLuint semnatic2Attribute(ge::sg::AttributeDescriptor::Semantic semantic);
   GLenum translateEnum(ge::sg::AttributeDescriptor::DataType type);
   GLenum translateEnum(ge::sg::Image::DataType type);
   GLenum translateEnum(ge::sg::Image::Format type);
   GLenum translateEnum(ge::sg::Mesh::PrimitiveType type);
}