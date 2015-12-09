#pragma once
#include<iostream>
#include<vector>
#include<map>
#include<set>
#include<geCore/Export.h>
#include<geCore/geCore.h>
#include<geCore/fsa/transition.h>
#include<geCore/fsa/state.h>

namespace ge{
  namespace core{
    class FSAState;
    class GECORE_EXPORT FSA{
      private:
        std::string                    _start            = "";
        std::set<FSAState*>            _endStates            ;
        std::map<std::string,FSAState*>_name2State           ;
        std::string                    _alreadyRead      = "";
        char                           _currentChar      = 0 ;
        std::string                    _currentStateName = "";
        unsigned                       _currentPosition  = 0 ;
      public:
        template<typename...Args>
          FSA(std::string start,Args... args){
            this->_start=start;
            this->_processArgs(args...);
          }
        FSA(std::string start = "");
        virtual ~FSA();
        void minimalize();
        void addTransition(
            std::string      stateA            ,
            char             lex               ,
            std::string      stateB            ,
            FSACallback::Fce callback = nullptr,
            void*            data     = nullptr);
        void addTransition(
            std::string  stateA                ,
            std::string  lex                   ,
            std::string  stateB                ,
            FSACallback::Fce callback = nullptr,
            void*        data         = nullptr);
        void addAllTransition(
            std::string      stateA            ,
            std::string      stateB            ,
            FSACallback::Fce callback = nullptr,
            void*            data     = nullptr);
        void addElseTransition(
            std::string      stateA            ,
            std::string      stateB            ,
            FSACallback::Fce callback = nullptr,
            void*            data     = nullptr);
        void addEOFTransition(
            std::string      stateA            ,
            std::string      stateB            ,
            FSACallback::Fce callback = nullptr,
            void*            data     = nullptr);
        void addTransition(
            std::string            stateA  ,
            char                   lex     ,
            std::string            stateB  ,
            FSAFusedCallback const&callback);
        void addTransition(
            std::string            stateA  ,
            std::string            lex     ,
            std::string            stateB  ,
            FSAFusedCallback const&callback);
        void addAllTransition(
            std::string            stateA  ,
            std::string            stateB  ,
            FSAFusedCallback const&callback);
        void addElseTransition(
            std::string            stateA  ,
            std::string            stateB  ,
            FSAFusedCallback const&callback);
        void addEOFTransition(
            std::string            stateA  ,
            std::string            stateB  ,
            FSAFusedCallback const&callback);
        bool run(std::string text);
        char        getCurrentChar      ()const;
        std::string getAlreadyReadString()const;
        std::string getCurrentStateName ()const;
        unsigned    getCurrentPosition  ()const;
        std::string toStr()const;
        void removeUnreachableStates();
        void removeUndistinguishabeStates();
        FSA operator+(FSA const&other)const;
        FSA operator*(FSA const&other)const;
        typedef std::map<std::string,FSAState*>::const_iterator Iterator;
        Iterator begin()const;
        Iterator end  ()const;
        static const std::string els  ;
        static const std::string eof  ;
        static const std::string digit;
        static const std::string range;
        static const std::string all  ;
        static const std::string space;
      private:
        void _initRun();
        FSAState*_addState (std::string name,bool end=false);
        FSAState*_getState (std::string name)const;
        std::string _expandLex(std::string lex )const;
        bool _createStates(FSAState**sa,FSAState**sb,std::string nameA,std::string nameB,bool end = false);
        void _computeEndStates();

        template<typename...Args>
        void _processArgs(std::string,std::string,std::string,FSACallback::Fce c=nullptr,void*d=nullptr);
        template<typename...Args>
        void _processArgs(std::string,std::string,std::string,FSACallback::Fce,void*,Args...);
        template<typename...Args>
        void _processArgs(std::string,std::string,std::string,FSACallback::Fce,std::string,Args...);
        template<typename...Args>
        void _processArgs(std::string,std::string,std::string,std::string,Args...);
    };


    template<typename...Args>
      void FSA::_processArgs(
          std::string      stateA  ,
          std::string      lex     ,
          std::string      stateB  ,
          FSACallback::Fce callback,
          void*            data    ){
        if(lex==FSA::all)this->addAllTransition (stateA,stateB,callback,data);
        if(lex==FSA::els)this->addElseTransition(stateA,stateB,callback,data);
        if(lex==FSA::eof)this->addEOFTransition (stateA,stateB,callback,data);
        else this->addTransition(stateA,lex,stateB,callback,data);
      }
    template<typename...Args>
      void FSA::_processArgs(
          std::string      stateA  ,
          std::string      lex     ,
          std::string      stateB  ,
          FSACallback::Fce callback,
          void*            data    ,
          Args...          args    ){
        this->_processArgs(stateA,lex,stateB,callback,data);
        this->_processArgs(args...);
      }
    template<typename...Args>
      void FSA::_processArgs(
          std::string      stateA  ,
          std::string      lex     ,
          std::string      stateB  ,
          FSACallback::Fce callback,
          std::string      a0      ,
          Args...          args    ){
        this->_processArgs(stateA,lex,stateB,callback);
        this->_processArgs(a0,args...);
      }
    template<typename...Args>
      void FSA::_processArgs(
          std::string  stateA,
          std::string  lex   ,
          std::string  stateB,
          std::string  a0    ,
          Args...      args  ){
        this->_processArgs(stateA,lex,stateB);
        this->_processArgs(a0,args...);
      }
  }
}