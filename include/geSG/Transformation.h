#ifndef GE_SG_TRANSFORMATION_H
#define GE_SG_TRANSFORMATION_H

#include <memory>
#include <glm/mat4x4.hpp>
#include <geSG/ParentChildList.h>

namespace ge
{
   namespace sg
   {
      class InstancingMatrices;


      struct TransformationThreadGpuData {
         unsigned transformationIndicesOffset4;
         unsigned computedMatrixOffset64;
      };


      struct TransformationMatrixGpuData {
         float matrix[16];
         inline float* asFloats()  { return matrix; }
         inline glm::mat4& asGlmMatrix()  { return *reinterpret_cast<glm::mat4*>(&matrix[0]); }
      };


      class Transformation {
      public:

         typedef ChildListTemplate<Transformation> ChildList;
         typedef ParentListTemplate<Transformation> ParentList;

      protected:

         struct SharedDataOffset {
            unsigned _gpuDataOffset64;
            unsigned _refCounter;
         };

         unsigned *_gpuDataOffsetPtr;  ///< It points either to _gpuDataOffset64 member or to externally allocated SharedDataOffset::_gpuDataOffset64.
         unsigned _gpuDataOffset64;
         GESG_CHILD_LIST(Transformation);
         ParentList _parentList;
         std::shared_ptr<InstancingMatrices> _instancingMatrices;

         void cancelSharedTransformation();

      public:

         void uploadMatrix(const float *matrix);
         void downloadMatrix(float *matrix);
         float* getMatrixPtr();

         inline unsigned gpuDataOffset64() const;
         void allocTransformationGpuData();
         void shareTransformationFrom(const Transformation &t);

         inline std::shared_ptr<InstancingMatrices>& getOrCreateInstancingMatrices();
         inline const std::shared_ptr<InstancingMatrices>& getOrCreateInstancingMatrices() const;
         inline std::shared_ptr<InstancingMatrices>& instancingMatrices();
         inline const std::shared_ptr<InstancingMatrices>& instancingMatrices() const;
         inline void setInstancingMatrices(std::shared_ptr<InstancingMatrices>& im);

         enum ConstructionFlags { SHARE_MATRIX=0x1, SHARE_INSTANCING_MATRIX_COLLECTION=0x2,
                                  COPY_CHILDREN=0x4, SHARE_AND_COPY_ALL=0x7 };
         inline Transformation();
         //inline Transformation(const Transformation &t,unsigned constructionFlags=SHARE_AND_COPY_ALL);
         inline ~Transformation();

         /*ChildList::iterator addChild(std::shared_ptr<Transformation> &m);
         void removeChild(ChildList::iterator it);
         void removeChild(std::shared_ptr<Transformation> m);*/
         inline ChildList& getChildren();
         inline const ChildList& getChildren() const;
         inline ParentList& getParents();
         inline const ParentList& getParents() const;

         inline void instanceRef();
         inline void instanceUnref();
      };

   }
}


#include <geSG/InstancingMatrices.h>

namespace ge
{
   namespace sg
   {
      inline unsigned Transformation::gpuDataOffset64() const  { return *_gpuDataOffsetPtr; }
      inline std::shared_ptr<InstancingMatrices>& Transformation::getOrCreateInstancingMatrices()
      { if(_instancingMatrices==nullptr) _instancingMatrices=std::make_shared<InstancingMatrices>(); return _instancingMatrices; }
      inline const std::shared_ptr<InstancingMatrices>& Transformation::getOrCreateInstancingMatrices() const
      { if(_instancingMatrices==nullptr) const_cast<Transformation*>(this)->_instancingMatrices=std::make_shared<InstancingMatrices>(); return _instancingMatrices; }
      inline std::shared_ptr<InstancingMatrices>& Transformation::instancingMatrices()  { return _instancingMatrices; }
      inline const std::shared_ptr<InstancingMatrices>& Transformation::instancingMatrices() const  { return _instancingMatrices; }
      inline void Transformation::setInstancingMatrices(std::shared_ptr<InstancingMatrices>& im)  { _instancingMatrices=im; }
      inline Transformation::Transformation() : _gpuDataOffsetPtr(&_gpuDataOffset64), _gpuDataOffset64(0)  {}
      inline Transformation::~Transformation()  { RenderingContext::current()->transformationAllocationManager().free(_gpuDataOffsetPtr[0]);
         if(_gpuDataOffsetPtr!=&_gpuDataOffset64) { _gpuDataOffsetPtr[1]--; if((--_gpuDataOffsetPtr[1])==0) delete reinterpret_cast<SharedDataOffset*>(_gpuDataOffsetPtr); } }
      inline Transformation::ChildList& Transformation::getChildren()  { return _childList; }
      inline const Transformation::ChildList& Transformation::getChildren() const  { return _childList; }
      inline Transformation::ParentList& Transformation::getParents()  { return _parentList; }
      inline const Transformation::ParentList& Transformation::getParents() const  { return _parentList; }
   }
}

#endif // GE_SG_TRANSFORMATION_H