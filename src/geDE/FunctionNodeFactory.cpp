#include<geDE/FunctionNodeFactory.h>
#include<geDE/Function.h>
#include<geDE/ResourceFactory.h>

using namespace ge::de;

FunctionNodeFactory::FunctionNodeFactory(
    std::string name,Uses maxUses):FunctionFactory(name,maxUses){
}

void FunctionNodeFactory::setFactory(
    std::shared_ptr<StatementFactory>const&fac){
  assert(this!=nullptr);
  this->functionFactory = std::dynamic_pointer_cast<FunctionFactory>(fac);
}

void FunctionNodeFactory::addResourceFactory(
    std::shared_ptr<ResourceFactory>const&factory){
  assert(this!=nullptr);
  this->resourceFactories.push_back(factory);
  if(factory)factory->setUses(factory->getUses()+1);
}

void FunctionNodeFactory::addInputFactory(
    std::shared_ptr<StatementFactory>const&factory){
  assert(this!=nullptr);
  this->inputFactories.push_back(std::dynamic_pointer_cast<FunctionFactory>(factory));
  if(factory)factory->setUses(factory->getUses()+1);
}

FunctionNodeFactory::~FunctionNodeFactory(){
}

void FunctionNodeFactory::reset(){
  assert(this!=nullptr);
  this->_uses    = 0      ;
  this->_first   = true   ;
  this->_result  = nullptr;
}

std::shared_ptr<Statement>FunctionNodeFactory::_do(
    std::shared_ptr<FunctionRegister> const&fr){
  assert(this!=nullptr);
  assert(fr!=nullptr);
  this->_first = this->_uses == 0;
  if(this->resourceFactories.size()!=this->inputFactories.size()){
    std::cerr<<"ERROR: FunctionNodeFactory::operator()() - different ";
    std::cerr<<"number of input functions and input resources"<<std::endl;
    return nullptr;
  }
  if(!this->functionFactory)return nullptr;

  auto res = (*this->functionFactory)(fr);

  auto fce=std::dynamic_pointer_cast<Function>(res);
  using Iter=decltype(this->resourceFactories)::size_type;
  for(Iter i=0;i<this->resourceFactories.size();++i){
    if(!this->resourceFactories[i]!=!this->inputFactories[i]){
      std::cerr<<"ERROR: FunctionNodeFactory::operator()() - input factory ";
      std::cerr<<"and input resource does not correspond"<<std::endl;
      return nullptr;
    }
    if(!this->resourceFactories[i])continue;
    auto in=(*this->inputFactories[i])(fr);
    auto inFce = std::dynamic_pointer_cast<Function>(in);
    assert(inFce!=nullptr);
    inFce->bindOutput(fr,(*this->resourceFactories[i])(fr));
    assert(fce!=nullptr);
    fce->bindInput(fr,i,inFce);
  }
  return res;
}

std::shared_ptr<FunctionFactory>const&FunctionNodeFactory::getFactory()const{
  return this->functionFactory;
}

TypeRegister::TypeId FunctionNodeFactory::getOutputType(std::shared_ptr<FunctionRegister>const&fr)const{
  assert(this!=nullptr);
  return this->functionFactory->getOutputType(fr);
}

size_t FunctionNodeFactory::getNofInputs(std::shared_ptr<FunctionRegister>const&fr)const{
  assert(this!=nullptr);
  return this->functionFactory->getNofInputs(fr);
}

TypeRegister::TypeId FunctionNodeFactory::getInputType(std::shared_ptr<FunctionRegister>const&fr,size_t i)const{
  assert(this!=nullptr);
  return this->functionFactory->getInputType(fr,i);
}

