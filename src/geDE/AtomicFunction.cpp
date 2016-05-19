#include<geDE/AtomicFunction.h>
#include<geDE/Resource.h>

using namespace ge::de;

AtomicFunctionInput::AtomicFunctionInput(
    std::shared_ptr<Function>const&fce        ,
    Function::Ticks                updateTicks,
    bool                           changed    ){
  assert(this!=nullptr);
  this->updateTicks = updateTicks;
  this->changed     = changed;
  this->function    = fce;
}

AtomicFunctionInput::~AtomicFunctionInput(){
}

AtomicFunction::AtomicFunction(
    std::shared_ptr<FunctionRegister>const&fr,
    FunctionRegister::FunctionID id):Function(fr,id){
  assert(this!=nullptr);
  auto nofInputs = fr->getNofInputs(id);
  for(decltype(nofInputs)i=0;i<nofInputs;++i)
    this->_inputs.push_back(AtomicFunctionInput());
}

AtomicFunction::AtomicFunction(
    std::shared_ptr<FunctionRegister>const&fr,
    TypeRegister::DescriptionList const&typeDescription,
    std::string name,
    std::shared_ptr<StatementFactory>const&factory):AtomicFunction(fr,fr->addFunction(fr->getTypeRegister()->addType("",typeDescription),name,factory)){
}

AtomicFunction::AtomicFunction(std::shared_ptr<FunctionRegister>const&fr,FunctionRegister::FunctionID id,std::shared_ptr<Resource>const&output):AtomicFunction(fr,id){
  assert(this!=nullptr);
  this->bindOutput(fr,output);
}


AtomicFunction::~AtomicFunction(){
  //std::cerr<<"AtomicFunction::~AtomicFunction() - "<<this->_name<<" "<<this<<std::endl;
}

bool AtomicFunction::bindInput(
    std::shared_ptr<FunctionRegister>const&fr      ,
    InputIndex                             i       ,
    std::shared_ptr<Function>        const&function){
  assert(this!=nullptr);
  if(!this->_inputBindingCheck(fr,i,function))
    return false;
  //std::cerr<<this->_name<<".bindInput("<<i<<","<<function<<")"<<std::endl;
  if(function)function->_addOutputFunction(this,i);
  else if(this->_inputs[i].function){
    this->_inputs[i].function->_removeOutputFunction(this,i);
  }
  this->setDirty();
  this->_inputs[i].function = function;
  if(function)this->_inputs[i].updateTicks = function->getUpdateTicks() - 1;
  this->_inputs[i].changed  = true;
  return true;
}

bool AtomicFunction::bindOutput(
    std::shared_ptr<FunctionRegister>const&fr  ,
    std::shared_ptr<Resource>        const&data){
  assert(this!=nullptr);
  if(!this->_outputBindingCheck(fr,data))
    return false;
  this->_outputData = data;
  this->setDirty();
  return true;
}

void AtomicFunction::operator()(){
  assert(this!=nullptr);
  this->_checkTicks++;
  this->_processInputs();
  if(this->_do())
    this->_updateTicks++;

  //new scheme
  this->_dirtyFlag = false;
}

void AtomicFunction::_processInputs(){
  //new scheme
  assert(this!=nullptr);
  for(InputIndex i=0;i<this->_inputs.size();++i){
    assert(this->_inputs[i].function!=nullptr);
    if(!this->hasInput(i)||!this->_inputs[i].function->_dirtyFlag){
      this->_inputs[i].changed = false;
      continue;
    }
    (*this->_inputs[i].function)();
    this->_inputs[i].function->setCheckTicks(this->_checkTicks);
    this->_inputs[i].changed=
      this->_inputs[i].updateTicks<this->_inputs[i].function->getUpdateTicks();
    if(this->_inputs[i].changed)
      this->_inputs[i].updateTicks=this->_inputs[i].function->getUpdateTicks();
  }

  //old scheme based on checkTicks and updateTicks
  /*
  assert(this!=nullptr);
  for(InputIndex i=0;i<this->_inputs.size();++i){
    assert(this->_inputs[i].function!=nullptr);
    if(!this->hasInput(i)||this->_inputs[i].function->getCheckTicks()>=this->_checkTicks){
      this->_inputs[i].changed = false;
      continue;
    }
    (*this->_inputs[i].function)();
    this->_inputs[i].function->setCheckTicks(this->_checkTicks);
    this->_inputs[i].changed=
      this->_inputs[i].updateTicks<this->_inputs[i].function->getUpdateTicks();
    if(this->_inputs[i].changed)
      this->_inputs[i].updateTicks=this->_inputs[i].function->getUpdateTicks();
  }
  */
}

bool AtomicFunction::_do(){
  return true;
}
