#pragma once

#include<vector>
#include<tuple>
#include<memory>
#include<limits>
#include<functional>
#include<geCore/Export.h>

namespace ge{
  namespace core{
    class TransitionChooser;

    class GECORE_EXPORT MealyMachine{
      public:
        using StateIndex       = size_t;
        using BasicUnit        = uint8_t;
        using TransitionSymbol = BasicUnit const*;
        using Callback         = std::function<void(MealyMachine*)>;
        using SimpleCallback   = std::function<void()>;
        MealyMachine(size_t largestState = 1);
        virtual ~MealyMachine();

        StateIndex addState(std::shared_ptr<TransitionChooser>const&chooser);
        StateIndex addState();
        void addTransition(
            StateIndex                   const&from                ,
            TransitionSymbol             const&symbol              ,
            StateIndex                   const&to                  ,
            Callback                     const&callback   = nullptr);
        void addTransition(
            StateIndex                   const&from                ,
            std::vector<TransitionSymbol>const&symbols             ,
            StateIndex                   const&to                  ,
            Callback                     const&callback   = nullptr);
        void addTransition(
            StateIndex                   const&from                ,
            TransitionSymbol             const&symbolFrom          ,
            TransitionSymbol             const&symbolTo            ,
            StateIndex                   const&to                  ,
            Callback                     const&callback   = nullptr);
        void addTransition(
            StateIndex                   const&from                ,
            char const*                  const&symbol              ,
            StateIndex                   const&to                  ,
            Callback                     const&callback   = nullptr);
        void addTransition(
            StateIndex                   const&from                ,
            std::vector<char const*>     const&symbols             ,
            StateIndex                   const&to                  ,
            Callback                     const&callback   = nullptr);
        void addTransition(
            StateIndex                   const&from                ,
            char const*                  const&symbolFrom          ,
            char const*                  const&symbloTo            ,
            StateIndex                   const&to                  ,
            Callback                     const&callback   = nullptr);
        void addElseTransition(
            StateIndex                   const&from                ,
            StateIndex                   const&to                  ,
            Callback                     const&callback   = nullptr);
        void addEOFTransition(
            StateIndex                   const&from                ,
            Callback                     const&callback   = nullptr);
        void addTransition(
            StateIndex                   const&from                ,
            TransitionSymbol             const&symbol              ,
            StateIndex                   const&to                  ,
            SimpleCallback               const&callback            );
        void addTransition(
            StateIndex                   const&from                ,
            std::vector<TransitionSymbol>const&symbols             ,
            StateIndex                   const&to                  ,
            SimpleCallback               const&callback            );
        void addTransition(
            StateIndex                   const&from                ,
            TransitionSymbol             const&symbolFrom          ,
            TransitionSymbol             const&symbolTo            ,
            StateIndex                   const&to                  ,
            SimpleCallback               const&callback            );
        void addTransition(
            StateIndex                   const&from                ,
            char const*                  const&symbol              ,
            StateIndex                   const&to                  ,
            SimpleCallback               const&callback            );
        void addTransition(
            StateIndex                   const&from                ,
            std::vector<char const*>     const&symbols             ,
            StateIndex                   const&to                  ,
            SimpleCallback               const&callback            );
        void addTransition(
            StateIndex                   const&from                ,
            char const*                  const&symbolFrom          ,
            char const*                  const&symbloTo            ,
            StateIndex                   const&to                  ,
            SimpleCallback               const&callback            );
        void addElseTransition(
            StateIndex                   const&from                ,
            StateIndex                   const&to                  ,
            SimpleCallback               const&callback            );
        void addEOFTransition(
            StateIndex                   const&from                ,
            SimpleCallback               const&callback            );

        virtual void begin();
        virtual bool parse(BasicUnit const*data,size_t size);
        bool parse(char const*data);
        virtual bool end();
        inline size_t     const&getReadingPosition()const;
        inline TransitionSymbol getCurrentSymbol()const;
        inline StateIndex const&getCurrentState()const;
        inline void dontMove();
        virtual std::string str()const;
      protected:
        using TransitionSymbolIndex = size_t;
        using Transition            = std::tuple<StateIndex,Callback>;
        using TransitionVector      = std::vector<Transition>;
      public:
        using TransitionIndex       = TransitionVector::size_type;
        static const TransitionIndex nonexistingTransition;
      protected:
        using State = std::tuple<
          TransitionVector,
          std::shared_ptr<TransitionChooser>,
          std::shared_ptr<Transition>,
          std::shared_ptr<Transition>>;
      protected:
        enum TransitionParts{
          STATE_INDEX = 0,
          CALLBACK    = 1,
        };
        enum StateParts{
          TRANSITIONS     = 0,
          CHOOSER         = 1,
          ELSE_TRANSITION = 2,
          EOF_TRANSITION  = 3,
        };
        inline void _call(Transition const&transitions);
        inline bool _nextState(State const&state);
        bool                   _dontMove          = false  ;
        size_t                 _readingPosition   = 0      ;
        TransitionSymbol       _currentSymbol     = nullptr;
        size_t                 _currentSymbolSize = 0      ;
        std::vector<State>     _states                     ;
        StateIndex             _currentState      = 0      ;
        std::vector<BasicUnit> _symbolBuffer               ;
        TransitionSymbolIndex  _symbolBufferIndex = 0      ;
    };

    inline size_t const& MealyMachine::getReadingPosition()const{
      return this->_readingPosition;
    }

    inline MealyMachine::TransitionSymbol MealyMachine::getCurrentSymbol()const{
      return this->_currentSymbol;
    }

    inline MealyMachine::StateIndex const&MealyMachine::getCurrentState()const{
      return this->_currentState;
    }

    inline void MealyMachine::dontMove(){
      this->_dontMove = true;
    }
  }
}
