#pragma once

#include <geCore/Export.h>
#include <geCore/Functor.h>
#include<iostream>
#include<vector>

namespace ge{
  namespace core{
    class GECORE_EXPORT Command : public Functor
    {
      public:
        virtual void operator()()=0;
        virtual ~Command(){}
    };

    class GECORE_EXPORT CommandContainer: public Command
    {
      protected:
        Command**_command;
      public:
        CommandContainer(Command**command=NULL);
        ~CommandContainer();
        virtual void operator()();
        void set(Command**command=NULL);
        Command**get();
        Command* getCommand();
    };

    class GECORE_EXPORT CommandList: public Command
    {
      protected:
        bool _outOfOrder;
        bool _commutative;
        bool _associative;
        std::vector<Command*> _commands;
        unsigned _commandToExecute;///<index of command that will be executed using step()
      public:
        CommandList(bool outOfOrder=false);
        ~CommandList();
        unsigned add(Command*command);
        Command* getCommand(unsigned i);
        virtual void operator()();
        void step();
    };

    class GECORE_EXPORT CommandStatement: public Command
    {
      public:
        bool     isTrue;
    };
    class GECORE_EXPORT CommandIf: public Command
    {
      public:
        CommandStatement *statement;
        Command* trueBranch;
        Command* falseBranch;
        virtual void operator()();
    };
    class GECORE_EXPORT CommandWhile: public Command
    {
      public:
        CommandStatement* statement;
        Command*          body;
        virtual void operator()();
    };
    class GECORE_EXPORT CommandInterpret
    {

    };

  }
}
