#pragma once

#include<utility>
#include<tuple>
#include<geDE/Export.h>
#include<geDE/TypeRegister.h>
#include<geDE/StatementFactory.h>
#include<geDE/Statement.h>
#include<geDE/NameRegister.h>

namespace ge{
  namespace de{
    class Statement;
    class Function;

    /**
     * @brief FunctionRegister can register functions' factories
     * It can create new shared instances of registered functions
     * A function that is registered is described using 3 values
     * type of the function
     * name of the function
     * factory of the function
     * every triplet is associated with unique id - FunctionID.
     * FunctionRegister can also register precompiled function from
     * 3rd party libraries.
     * Every 3rd party function's pointer can be stored
     */
    class Empty;
    class GEDE_EXPORT FunctionRegister: public std::enable_shared_from_this<FunctionRegister>{
      public:
        typedef void(*Implementation)();
        typedef bool(*SignalingDecider)();
        using ClassImplementation = void(Empty::*)();
        using InputIndex = size_t;
        using FunctionID = size_t;
        inline FunctionRegister(
            std::shared_ptr<TypeRegister>const&typeRegister,
            std::shared_ptr<NameRegister>const&namer       );
        inline ~FunctionRegister();
        FunctionID addFunction(
            TypeRegister::TypeId             const&type   ,
            std::string                      const&name   ,
            std::shared_ptr<StatementFactory>const&factory);
        TypeRegister::TypeId             getType   (FunctionID id)const;
        std::string                      getName   (FunctionID id)const;
        std::shared_ptr<StatementFactory>getFactory(FunctionID id)const;
        FunctionID getFunctionId(std::string const&name)const;
        InputIndex  getNofInputs (FunctionID id)const;
        std::string getOutputName(FunctionID id)const;
        std::string getInputName (FunctionID id,InputIndex input)const;
        TypeRegister::TypeId getInputType (FunctionID id,InputIndex input)const;
        TypeRegister::TypeId getOutputType(FunctionID id)const;
        InputIndex getInputIndex(FunctionID id,std::string const&name)const;
        std::shared_ptr<TypeRegister>const&getTypeRegister()const;
        std::shared_ptr<NameRegister>const&getNameRegister()const;
        void addImplementation(FunctionID  id  ,Implementation impl);
        void addImplementation(std::string const&name,Implementation impl);
        Implementation getImplementation(FunctionID  id  )const;
        Implementation getImplementation(std::string const&name)const;
        void addSignalingDecider(FunctionID id,SignalingDecider sig);
        void addSignalingDecider(std::string const&name,SignalingDecider sig);
        SignalingDecider getSignalingDecider(FunctionID id)const;
        SignalingDecider getSignalingDecider(std::string const&name)const;
        void addClassImplementation(FunctionID id,ClassImplementation impl);
        void addClassImplementation(std::string const&name,ClassImplementation impl);
        ClassImplementation getClassImplementation(FunctionID id)const;
        ClassImplementation getClassImplementation(std::string const&name)const;

        std::shared_ptr<Function>sharedFunction(FunctionID  id  )const;
        std::shared_ptr<Function>sharedFunction(std::string const&name)const;
        std::shared_ptr<Statement>sharedStatement(FunctionID  id  )const;
        std::shared_ptr<Statement>sharedStatement(std::string const&name)const;
        std::shared_ptr<StatementFactory>sharedFactory(FunctionID  id  ,StatementFactory::Uses maxUses = 0)const;
        std::shared_ptr<StatementFactory>sharedFactory(std::string const&name,StatementFactory::Uses maxUses = 0)const;
        std::string str()const;
      protected:
        using FunctionDefinition = std::tuple<
          TypeRegister::TypeId,
          std::string,
          std::shared_ptr<StatementFactory>>;
        enum FunctionDefinitionParts{
          TYPE    = 0,
          NAME    = 1,
          FACTORY = 2,
        };
        std::shared_ptr<NameRegister>       _namer;
        std::shared_ptr<TypeRegister>_typeRegister;
        std::map<FunctionID,FunctionDefinition>_functions;
        std::map<FunctionID,Implementation>_implementations;
        std::map<FunctionID,SignalingDecider>_signalingDeciders;
        std::map<FunctionID,ClassImplementation>_classImplementations;
        std::map<std::string,FunctionID>_name2Function;
        inline FunctionDefinition      & _getDefinition(FunctionID id);
        inline FunctionDefinition const& _getDefinition(FunctionID id)const;
        std::string _genDefaultName(InputIndex i)const;

    };

    inline FunctionRegister::FunctionRegister(
        std::shared_ptr<TypeRegister>const&typeRegister,
        std::shared_ptr<NameRegister>const&namer){
      assert(typeRegister!=nullptr);
      this->_typeRegister = typeRegister;
      this->_functions[0]=FunctionDefinition(
          TypeRegister::UNREGISTERED,
          "unregistered",
          nullptr);
      this->_namer = namer;
    }


    inline FunctionRegister::~FunctionRegister(){
    }

    inline FunctionRegister::FunctionDefinition &FunctionRegister::_getDefinition(FunctionID id){
      assert(this!=nullptr);
      auto ii=this->_functions.find(id);
      if(ii==this->_functions.end()){
        ge::core::printError("FunctionRegister::_getDefinition","there is no such function id",id);
        return this->_functions.find(0)->second;
      }
      return ii->second;
    }

    
    inline FunctionRegister::FunctionDefinition const&FunctionRegister::_getDefinition(FunctionID id)const{
      assert(this!=nullptr);
      auto ii=this->_functions.find(id);
      if(ii==this->_functions.end()){
        ge::core::printError("FunctionRegister::_getDefinition","there is no such function id",id);
        return this->_functions.find(0)->second;
      }
      return ii->second;
    }

    inline TypeRegister::TypeId FunctionRegister::getType(FunctionID id)const{
      assert(this!=nullptr);
      return std::get<TYPE>(this->_getDefinition(id));
    }

    inline std::string FunctionRegister::getName(FunctionID id)const{
      assert(this!=nullptr);
      return std::get<NAME>(this->_getDefinition(id));
    }

    inline std::shared_ptr<StatementFactory>FunctionRegister::getFactory(FunctionID id)const{
      assert(this!=nullptr);
      return std::get<FACTORY>(this->_getDefinition(id));
    }

    inline FunctionRegister::FunctionID FunctionRegister::getFunctionId(std::string const&name)const{
      assert(this!=nullptr);
      auto ii=this->_name2Function.find(name);
      if(ii==this->_name2Function.end()){
        ge::core::printError("FunctionRegister::getFunctionId","there is no such function name",name);
        return 0;
      }
      return ii->second;
    }

    inline FunctionRegister::InputIndex FunctionRegister::getNofInputs(FunctionID id)const{
      assert(this!=nullptr);
      assert(this->_typeRegister!=nullptr);
      auto t=this->getType(id);
      if(t==TypeRegister::UNREGISTERED){
        ge::core::printError("FunctionRegister::getNofInputs","there is no such function",id);
        return 0;
      }
      return this->_typeRegister->getNofFceArgs(t);
    }

    inline std::string FunctionRegister::getOutputName(FunctionID id)const{
      assert(this!=nullptr);
      assert(this->_namer!=nullptr);
      return this->_namer->getFceOutputName(id);
    }

    inline std::string FunctionRegister::getInputName(FunctionID id,InputIndex input)const{
      assert(this!=nullptr);
      assert(this->_namer!=nullptr);
      return this->_namer->getFceInputName(id,input);
    }

    inline TypeRegister::TypeId FunctionRegister::getInputType(FunctionID id,InputIndex input)const{
      assert(this!=nullptr);
      assert(this->_typeRegister!=nullptr);
      auto type = this->getType(id);
      return this->_typeRegister->getFceArgTypeId(type,input);
    }

    inline TypeRegister::TypeId FunctionRegister::getOutputType(FunctionID id)const{
      assert(this!=nullptr);
      assert(this->_typeRegister!=nullptr);
      auto type = this->getType(id);
      return this->_typeRegister->getFceReturnTypeId(type);
    }


    inline FunctionRegister::InputIndex FunctionRegister::getInputIndex(FunctionID id,std::string const&name)const{
      assert(this!=nullptr);
      assert(this->_namer!=nullptr);
      return this->_namer->getFceInput(id,name);
    }

    inline std::shared_ptr<TypeRegister>const&FunctionRegister::getTypeRegister()const{
      assert(this!=nullptr);
      return this->_typeRegister;
    }

    inline std::shared_ptr<NameRegister>const&FunctionRegister::getNameRegister()const{
      assert(this!=nullptr);
      return this->_namer;
    }

    inline void FunctionRegister::addImplementation(FunctionID id,Implementation impl){
      assert(this!=nullptr);
      this->_implementations[id]=impl;
    }
    
    inline void FunctionRegister::addImplementation(std::string const&name,Implementation impl){
      assert(this!=nullptr);
      this->addImplementation(this->getFunctionId(name),impl);
    }

    inline FunctionRegister::Implementation FunctionRegister::getImplementation(FunctionID id)const{
      assert(this!=nullptr);
      assert(this->_implementations.count(id)!=0);
      return this->_implementations.find(id)->second;
    }

    inline FunctionRegister::Implementation FunctionRegister::getImplementation(std::string const&name)const{
      assert(this!=nullptr);
      return this->getImplementation(this->getFunctionId(name));
    }

    inline void FunctionRegister::addSignalingDecider(FunctionID id,SignalingDecider sig){
      assert(this!=nullptr);
      this->_signalingDeciders[id]=sig;
    }

    inline void FunctionRegister::addSignalingDecider(std::string const&name,SignalingDecider sig){
      assert(this!=nullptr);
      this->addSignalingDecider(this->getFunctionId(name),sig);
    }

    inline FunctionRegister::SignalingDecider FunctionRegister::getSignalingDecider(FunctionID id)const{
      assert(this!=nullptr);
      assert(this->_signalingDeciders.count(id)!=0);
      return this->_signalingDeciders.find(id)->second;
    }

    inline FunctionRegister::SignalingDecider FunctionRegister::getSignalingDecider(std::string const&name)const{
      assert(this!=nullptr);
      return this->getSignalingDecider(this->getFunctionId(name));
    }

    inline void FunctionRegister::addClassImplementation(FunctionID id,ClassImplementation impl){
      assert(this!=nullptr);
      this->_classImplementations[id] = impl;
    }

    inline void FunctionRegister::addClassImplementation(std::string const&name,ClassImplementation impl){
      assert(this!=nullptr);
      this->addClassImplementation(this->getFunctionId(name),impl);
    }

    inline FunctionRegister::ClassImplementation FunctionRegister::getClassImplementation(FunctionID id)const{
      assert(this!=nullptr);
      assert(this->_classImplementations.count(id)!=0);
      return this->_classImplementations.find(id)->second;
    }

    inline FunctionRegister::ClassImplementation FunctionRegister::getClassImplementation(std::string const&name)const{
      assert(this!=nullptr);
      return this->getClassImplementation(this->getFunctionId(name));
    }

  }
}