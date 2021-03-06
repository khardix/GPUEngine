#pragma once

#include <geSG/Image.h>

namespace fsg
{
   class DefaultImage : public ge::sg::Image
   {
   public:
      DefaultImage();

      virtual char* getBits() override;

      virtual size_t getSizeInBytes() override;
      virtual Format getFormat() override; //should this function return int or some template type which could be specified as GLenum
      virtual DataType getDataType() override;
      virtual size_t getWidth() override;
      virtual size_t getHeight() override;

      ~DefaultImage();

      char* _bits;
      Format _format;
      DataType _dataType;
      size_t _width;
      size_t _height;
   };
}