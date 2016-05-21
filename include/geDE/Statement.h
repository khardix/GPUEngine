#pragma once

#include<geCore/Command.h>
#include<geDE/Export.h>

namespace ge{
  namespace de{
    class GEDE_EXPORT Statement: public ge::core::Command{
      public:
        enum Type{
          FUNCTION,
          BODY    ,
          WHILE   ,
          IF      ,
        };
      protected:
        Type _type;
      public:
        inline Statement(Type const&type);
        inline virtual ~Statement();
        virtual void operator()()=0;
    };

    inline Statement::Statement(Type const&type){
      this->_type = type;
    }

    inline Statement::~Statement(){
    }

  }
}

