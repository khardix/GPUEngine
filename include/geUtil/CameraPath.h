#ifndef _CAMERAPATH_H_
#define _CAMERAPATH_H_

#include<iostream>
#include<vector>

#define ENABLE_DRAWING

#ifdef ENABLE_DRAWING
  #include<geGL/ProgramObject.h>
#endif//ENABLE_DRAWING

#include<geUtil/CameraObject.h>

namespace ge{
  namespace util{
    class CameraKeyPoint
    {
      public:
        float position  [3];
        float viewVector[3];
        float upVector  [3];
        float fovy;
        CameraKeyPoint(float*position,float*viewVector,float*upVector,float fovy);
        CameraKeyPoint(CameraObject*Camera);
        CameraKeyPoint(){}
    };
    class CameraPath
    {
      private:
        unsigned                    _selected;
        std::vector<CameraKeyPoint*>_keyPoints;
        float                       _duration;
        bool                        _cyclic;
        float _catmullRom(float a,float b,float c,float d,float t);
#ifdef ENABLE_DRAWING
        ge::gl::ProgramObject*_lineProgram;
        ge::gl::ProgramObject*_pointProgram;
#endif//ENABLE_DRAWING
      public:
        CameraPath();
        ~CameraPath();
        void getCameraPoint(CameraKeyPoint*Point,float Time);
        void setDuration(float Duration);
        void insertAfter(int Index,CameraKeyPoint*CameraPoint);
        void deletePoint(int Index);
        void loadCSV(std::string FileName);
        void saveCSV(std::string FileName);
        void select(unsigned Index);
        void insertToEnd(CameraKeyPoint*CameraPoint);
        unsigned getLength();
#ifdef ENABLE_DRAWING
        void draw(float*MVP);
#endif//ENABLE_DRAWING
    };
  }//util
}//ge

#endif//_CCAMERAPATH_H_
