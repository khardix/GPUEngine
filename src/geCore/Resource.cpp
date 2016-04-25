#include<geCore/Resource.h>

using namespace ge::core;

namespace ge{
  namespace core{
    class AtomicFunction;
  }
}

AtomicResource::AtomicResource(AtomicResource const& ac):Resource(ac._manager,ac._id){
  //std::cerr<<"AtomicResource::AtomicResource() - "<<this<<std::endl;
  this->_data   = ac._data  ;
  this->_offset = ac._offset;
}

AtomicResource::AtomicResource(
    std::shared_ptr<TypeRegister>const&manager,
    const void*                        data   ,
    TypeRegister::TypeID               id     ,
    size_t                             offset ):Resource(manager,id){
  //std::cerr<<"AtomicResource::AtomicResource() - "<<this<<std::endl;
  this->_data    = std::shared_ptr<char>((char*)data,[id,manager](char*ptr){AtomicResource::_callDestructors(ptr,id,manager);delete[]ptr;});
  this->_offset  =        offset ;
}

AtomicResource::AtomicResource(
    std::shared_ptr<TypeRegister>const&manager,
    std::shared_ptr<char>const&        data   ,
    TypeRegister::TypeID               id     ,
    size_t                             offset ):Resource(manager,id){
  //std::cerr<<"AtomicResource::AtomicResource() - "<<this<<std::endl;
  this->_data    = data   ;
  this->_offset  = offset ;
}

AtomicResource::AtomicResource(
    std::shared_ptr<TypeRegister>const&manager,
    TypeRegister::TypeID               id     ):Resource(manager,id){
  //std::cerr<<"AtomicResource::AtomicResource() - "<<this<<std::endl;
  this->_data    = nullptr;
  this->_offset  = 0      ;
}

AtomicResource::~AtomicResource(){
  //std::cerr<<"AtomicResource::~AtomicResource() - "<<this<<std::endl;
}

void*AtomicResource::getData()const{
  return (char*)&(*this->_data)+this->_offset;
}

void const*AtomicResource::getDataAddress()const{
  return &this->_data;
}

std::shared_ptr<Resource> AtomicResource::operator[](TypeRegister::DescriptionIndex elem)const{
  TypeRegister::TypeID innerType = 0;
  size_t               offset    = 0;
  switch(this->getManager()->getTypeIdType(this->_id)){
    case TypeRegister::ARRAY :
      innerType = this->getManager()->getArrayInnerTypeId(this->getId());
      offset    = this->getManager()->computeTypeIdSize(innerType)*elem;
      return std::make_shared<AtomicResource>(this->getManager(),this->_data,innerType,offset);
    case TypeRegister::STRUCT:
      innerType = this->getManager()->getStructElementTypeId(this->getId(),elem);
      for(TypeRegister::DescriptionIndex i=0;i<elem;++i)
        offset += this->getManager()->computeTypeIdSize(this->getManager()->getStructElementTypeId(this->getId(),i));
      return std::make_shared<AtomicResource>(this->getManager(),this->_data,innerType,offset);
    default:
      return std::make_shared<AtomicResource>(this->getManager(),this->_data,this->getId());
  }
}

TypeRegister::DescriptionElement AtomicResource::getNofElements()const{
  switch(this->_manager->getTypeIdType(this->_id)){
    case TypeRegister::ARRAY :
      return this->getManager()->getArraySize(this->getId());
    case TypeRegister::STRUCT:
      return this->getManager()->getNofStructElements(this->getId());
    default                 :
      return 1;
  }
}

void AtomicResource::_callDestructors(char*ptr,TypeRegister::TypeID id,std::shared_ptr<const TypeRegister>const&manager){
  TypeRegister::Type type=manager->getTypeIdType(id);
  switch(type){
    case TypeRegister::VOID  :
    case TypeRegister::BOOL  :
    case TypeRegister::I8    :
    case TypeRegister::I16   :
    case TypeRegister::I32   :
    case TypeRegister::I64   :
    case TypeRegister::U8    :
    case TypeRegister::U16   :
    case TypeRegister::U32   :
    case TypeRegister::U64   :
    case TypeRegister::F32   :
    case TypeRegister::F64   :
    case TypeRegister::PTR   :break;
    case TypeRegister::STRING:
                              ((std::string*)ptr)->~basic_string();
                              break;
    case TypeRegister::ARRAY:
                              for(TypeRegister::DescriptionElement i=0;i<manager->getArraySize(id);++i)
                                AtomicResource::_callDestructors(ptr+manager->computeTypeIdSize(manager->getArrayInnerTypeId(id))*i,manager->getArrayInnerTypeId(id),manager);
                              break;
    case TypeRegister::STRUCT:
                              for(TypeRegister::DescriptionElement e=0;e<manager->getNofStructElements(id);++e){
                                AtomicResource::_callDestructors(ptr,manager->getStructElementTypeId(id,e),manager);
                                ptr+=manager->computeTypeIdSize(manager->getStructElementTypeId(id,e));
                              }
                              break;
    case TypeRegister::FCE:
                              ((std::shared_ptr<ge::core::AtomicFunction>*)ptr)->~shared_ptr();
                              break;
    case TypeRegister::OBJ:
                              manager->destroyUsingCustomDestroyer((unsigned char*)ptr,id);
                              break;
    default:
                              break;
  }
}

const void*AtomicResource::getPointer()const{
  return (void*)this->getData();
}

std::string AtomicResource::data2Str()const{
  TypeRegister::Type type=this->_manager->getTypeIdType(this->_id);
  std::stringstream ss;
  bool first;
  switch(type){
    case TypeRegister::VOID  :
      break;
    case TypeRegister::BOOL  :
      ss<<((bool)(*this)?"true":"false");
      break;
    case TypeRegister::I8    :
      ss<<(int)(char)(*this);
      break;
    case TypeRegister::I16   :
      ss<<(short)(*this);
      break;
    case TypeRegister::I32   :
      ss<<(int)(*this);
      break;
    case TypeRegister::I64   :
      ss<<(long long int)(*this);
      break;
    case TypeRegister::U8    :
      ss<<(unsigned)(unsigned char)(*this);
      break;
    case TypeRegister::U16   :
      ss<<(unsigned short)(*this);
      break;
    case TypeRegister::U32   :
      ss<<(unsigned)(*this);
      break;
    case TypeRegister::U64   :
      ss<<(unsigned long long)(*this);
      break;
    case TypeRegister::F32   :
      ss<<(float)(*this);
      break;
    case TypeRegister::F64   :
      ss<<(double)(*this);
      break;
    case TypeRegister::PTR   :
      ss<<(void*)(*this);
      break;
    case TypeRegister::STRING:
      ss<<(std::string&)(*this);
      break;
    case TypeRegister::ARRAY:
      ss<<"[";
      first=true;
      for(TypeRegister::DescriptionElement i=0;i<this->_manager->getArraySize(this->_id);++i){
        if(first)first=false;
        else ss<<",";
        ss<<((*this)[i])->data2Str();
      }
      ss<<"]";
      break;
    case TypeRegister::STRUCT:
      ss<<"{";
      first=true;
      for(TypeRegister::DescriptionElement e=0;e<this->_manager->getNofStructElements(this->_id);++e){
        if(first)first=false;
        else ss<<",";
        ss<<((*this)[e])->data2Str();
      }
      ss<<"}";
      break;
    case TypeRegister::FCE:
      break;
    case TypeRegister::OBJ:
      break;
    default:
      break;
  }
  return ss.str();
}

void AtomicResource::callDestructor(){
  this->_callDestructors(&*this->_data,this->_id,this->_manager);
}


CompositeResource::CompositeResource(
    std::shared_ptr<TypeRegister>const&         manager  ,
    TypeRegister::TypeID                        id       ,
    std::vector<std::shared_ptr<Resource>>const&accessors):Resource(manager,id){
  this->_nofElements = 0;
  switch(manager->getTypeIdType(id)){
    case TypeRegister::ARRAY:
      if(manager->getArraySize(id)!=accessors.size()){
        std::cerr<<"ERROR - CompositeResource::CompositeResource - number of components does not match array size - ";
        std::cerr<<manager->getArraySize(id)<<" != "<<accessors.size();
        std::cerr<<std::endl;
        return;
      }
      for(auto x:accessors){
        if(manager->getArrayInnerTypeId(id)!=x->getId()){
          std::cerr<<"ERROR - CompositeResource::CompositeResource - one of components differs from arrays inner type - ";
          std::cerr<<manager->getTypeIdName(manager->getArrayInnerTypeId(id))<<" != ";
          std::cerr<<manager->getTypeIdName(x->getId());
          std::cerr<<std::endl;
          return;
        }
      }
      break;
    case TypeRegister::STRUCT:
      if(manager->getNofStructElements(id)!=accessors.size()){
        std::cerr<<"ERROR - CompositeResource::CompositeResource() - number of components differs from structs number of components - ";
        std::cerr<<manager->getNofStructElements(id)<<" != "<<accessors.size();
        std::cerr<<std::endl;
        return;
      }
      for(std::vector<std::shared_ptr<Resource>>::size_type i=0;i<accessors.size();++i){
        if(manager->getStructElementTypeId(id,i)!=accessors[i]->getId()){
          std::cerr<<"ERROR - CompositeResource::CompositeResource() - one of components differs from structs element type - ";
          std::cerr<<manager->getTypeIdName(manager->getStructElementTypeId(id,i))<<" != ";
          std::cerr<<manager->getTypeIdName(accessors[i]->getId());
          std::cerr<<std::endl;
          return;
        }
      }
      break;
    default                  :
      std::cerr<<"ERROR - CompositeResource::CompositeResource() - cannot composite Resources into non composable type - ";
      std::cerr<<manager->getTypeIdName(id)<<std::endl;;
      return;
  }
  this->_components = accessors;
  for(auto x:accessors)
    this->_nofElements += x->getNofElements();
}

void*CompositeResource::getData()const{
  return this->_components[0]->getData();
}

void const*CompositeResource::getDataAddress()const{
  return this->_components[0]->getDataAddress();
}

std::shared_ptr<Resource> CompositeResource::operator[](TypeRegister::DescriptionIndex elem)const{
  TypeRegister::DescriptionElement offset=0;
  decltype(this->_components)::size_type i=0;
  while(elem>offset+this->_components[i]->getNofElements()){
    offset+=this->_components[i]->getNofElements();
    ++i;
  }
  return this->_components[i]->operator[](elem-offset);

}

TypeRegister::DescriptionElement CompositeResource::getNofElements()const{
  return this->_nofElements;
}

std::string CompositeResource::data2Str()const{
  std::stringstream ss;
  bool first;
  switch(this->_manager->getTypeIdType(this->_id)){
    case TypeRegister::ARRAY:
      ss<<"[";
      first=true;
      for(TypeRegister::DescriptionElement i=0;i<this->_manager->getArraySize(this->_id);++i){
        if(first)first=false;
        else ss<<",";
        ss<<((*this)[i])->data2Str();
      }
      ss<<"]";
      break;
    case TypeRegister::STRUCT:
      ss<<"{";
      first=true;
      for(TypeRegister::DescriptionElement e=0;e<this->_manager->getNofStructElements(this->_id);++e){
        if(first)first=false;
        else ss<<",";
        ss<<((*this)[e])->data2Str();
      }
      ss<<"}";
      break;
    default:
      break;
  }
  return ss.str();
}
