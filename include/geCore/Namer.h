#pragma once

#include<iostream>
#include<cassert>
#include<tuple>
#include<map>
#include<geCore/Export.h>
#include<geCore/dtemplates.h>

namespace ge{
  namespace core{

    class GECORE_EXPORT Namer{
      public:
        using ElementIndex = size_t;
        using Id = size_t;
        inline Namer();
        inline ~Namer();
        inline std::string getFceInputName(Id id,ElementIndex input)const;
        inline std::string getFceOutputName(Id id)const;
        inline void setFceInputName (Id id,ElementIndex input,std::string name);
        inline void setFceOutputName(Id id,std::string name);
        inline std::string getStructElementName(Id id,ElementIndex e)const;
        inline void setStructElementName(Id id,ElementIndex e,std::string name);
        inline ElementIndex getFceInput     (Id id,std::string name)const;
        inline ElementIndex getStructElement(Id id,std::string name)const;
        inline bool hasFceInput     (Id id,std::string name)const;
        inline bool hasStructElement(Id id,std::string name)const;
        void addFceNaming(Id id,ElementIndex nofInputs);
        void addStructNaming(Id id,ElementIndex nofElements);
        void removeFceNaming(Id id);
        void removeStructNaming(Id id);
      protected:
        using FunctionNaming = std::tuple<
          std::vector<std::string>,
          std::map<std::string,ElementIndex>,
          std::string>;
        enum FunctionNamingParts{
          INPUT_NAME   = 0,
          NAME_2_INPUT = 1,
          OUTPUT       = 2,
        };
        using StructureNaming = std::tuple<
          std::vector<std::string>,
          std::map<std::string,ElementIndex>>;
        enum StructureNameingParts{
          ELEMENT_NAME   = 0,
          NAME_2_ELEMENT = 1,
        };
        using FunctionMap  = std::map<Id,FunctionNaming >;
        using StructureMap = std::map<Id,StructureNaming>; 
        FunctionMap  _functionNaming;
        StructureMap _structureNaming;
    };

    inline Namer::Namer(){}

    inline Namer::~Namer(){}

    inline std::string Namer::getFceInputName(Id id,ElementIndex input)const{
      assert(this!=nullptr);
      assert(this->_functionNaming.count(id)!=0);
      assert(input<std::get<INPUT_NAME>(this->_functionNaming.find(id)->second).size());
      return std::get<INPUT_NAME>(this->_functionNaming.find(id)->second)[input];
    }

    inline std::string Namer::getFceOutputName(Id id)const{
      assert(this!=nullptr);
      assert(this->_functionNaming.count(id)!=0);
      return std::get<OUTPUT>(this->_functionNaming.find(id)->second);
    }

    inline void Namer::setFceInputName(Id id,ElementIndex input,std::string name){
      assert(this!=nullptr);
      assert(this->_functionNaming.count(id)!=0);
      assert(input<std::get<INPUT_NAME>(this->_functionNaming[id]).size());
      if(this->hasFceInput(id,name)&&this->getFceInput(id,name)!=input){
        ge::core::printError("Namer::setFceInputName","there is other input with that name",id,input,name);
        return;
      }
      std::get<NAME_2_INPUT>(this->_functionNaming[id]).erase(std::get<INPUT_NAME>(this->_functionNaming[id])[input]);
      std::get<INPUT_NAME>(this->_functionNaming[id])[input] = name;
      std::get<NAME_2_INPUT>(this->_functionNaming[id])[name] = input;
    }

    inline void Namer::setFceOutputName(Id id,std::string name){
      assert(this!=nullptr);
      assert(this->_functionNaming.count(id)!=0);
      std::get<OUTPUT>(this->_functionNaming[id]) = name;
    }

    inline std::string Namer::getStructElementName(Id id,ElementIndex element)const{
      assert(this!=nullptr);
      assert(this->_structureNaming.count(id)!=0);
      assert(element<std::get<ELEMENT_NAME>(this->_structureNaming.find(id)->second).size());
      return std::get<ELEMENT_NAME>(this->_structureNaming.find(id)->second)[element];
    }

    inline void Namer::setStructElementName(Id id,ElementIndex element,std::string name){
      assert(this!=nullptr);
      assert(this->_structureNaming.count(id)!=0);
      assert(element<std::get<ELEMENT_NAME>(this->_structureNaming.find(id)->second).size());
      std::get<NAME_2_ELEMENT>(this->_structureNaming[id]).erase(std::get<ELEMENT_NAME>(this->_functionNaming[id])[element]);
      std::get<ELEMENT_NAME>(this->_structureNaming[id])[element] = name;
      std::get<NAME_2_ELEMENT>(this->_structureNaming[id])[name] = element;
    }

    inline Namer::ElementIndex Namer::getFceInput(Id id,std::string name)const{
      assert(this!=nullptr);
      assert(this->_functionNaming.count(id)!=0);
      auto fi = this->_functionNaming.find(id);
      auto ii = std::get<NAME_2_INPUT>(fi->second).find(name);
      if(ii==std::get<NAME_2_INPUT>(fi->second).end()){
        ge::core::printError("Namer::getFceInput","no such function input",id,name);
        return 0;
      }
      return ii->second;
    }

    inline Namer::ElementIndex Namer::getStructElement(Id id,std::string name)const{
      assert(this!=nullptr);
      assert(this->_structureNaming.count(id)!=0);
      auto si = this->_structureNaming.find(id);
      auto ii = std::get<NAME_2_ELEMENT>(si->second).find(name);
      if(ii==std::get<NAME_2_ELEMENT>(si->second).end()){
        ge::core::printError("Namer::getStructElement","no such structure element",id,name);
        return 0;
      }
      return ii->second;
    }

    inline bool Namer::hasFceInput(Id id,std::string name)const{
      assert(this!=nullptr);
      assert(this->_functionNaming.count(id)!=0);
      return std::get<NAME_2_INPUT>(this->_functionNaming.find(id)->second).count(name)>0;
    }

    inline bool Namer::hasStructElement(Id id,std::string name)const{
      assert(this!=nullptr);
      assert(this->_structureNaming.count(id)!=0);
      return std::get<NAME_2_ELEMENT>(this->_structureNaming.find(id)->second).count(name)>0;
    }

  }
}