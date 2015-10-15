#pragma once

#include<geUtil/Export.h>
#include<geUtil/LibraryLoader.h>
#include<geCore/TypeRegister.h>
#include<geUtil/RuntimeClassInterface.h>

namespace ge{
  namespace util{
    /**
     * @brief This class loads object that is created by factory function getObject inside
     * dynamic library
     * It can be used for easy runtime creation of new object from dynamic library
     */
    class GEUTIL_EXPORT ObjectLoader: public LibraryLoader{
      public:
        /**
         * @brief This function returns object that is created by factory function getObject inside lib
         * dynamic library
         *
         * @param lib name of library
         *
         * @return created object
         */
        void*getObject(std::string lib);
        /**
         * @brief This function returns runtime class interface that is created by factory function getInterface inside lib
         *
         * @param lib name of library
         * @param typeRegister type register
         *
         * @return created interface
         */
        RuntimeClassInterface*getInterface(
            std::string lib,
            std::shared_ptr<ge::core::TypeRegister>&typeRegister);
    };
  }
}