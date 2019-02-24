#pragma once

#include <Utils.h>

#include <RingBuffer.h>

template <typename Parent, typename StateType, typename EventType,
          bool Debug = false>
class EmbeddedFSM {
public:
  using State = StateType;
  using Event = EventType;

  using Action = void (Parent::*)(void);

  struct Transition {
    State fromState;
    Event event;

    Action action;
    State toState;

    bool terminator = false;
  };

  struct StateAction {
    State state;

    Action entryAction;

    bool terminator = false;
  };

  static constexpr Transition TransitionTerminator = {
      static_cast<State>(0),
      static_cast<Event>(0),
      nullptr,
      static_cast<State>(0),
      true,
  };

  static constexpr StateAction StateActionTerminator = {
      static_cast<State>(0),
      nullptr,
      true,
  };

private:
  RingBuffer<Event, 16> events_;
  Parent& parent_;
  // TODO: Investigate. How I wish I had std::array...
  Transition* transitions_;
  StateAction* stateActions_;

public:
  State state;

  EmbeddedFSM(State initialState, Parent& parent, Transition* transitions,
              StateAction* stateActions)
      : parent_(parent), transitions_(transitions), stateActions_(stateActions),
        state(initialState) {}

  bool pushEvent(Event event) { return events_.push(event); }

  bool processOneEvent() {
    if (events_.empty()) {
      return false;
    }

    Event event{};
    events_.pop(event);

    for (Transition* transition = transitions_; !transition->terminator;
         transition++) {
      if ((state == transition->fromState) && (event == transition->event)) {
        if (!!transition->action) {
          (parent_.*(transition->action))();
        }
        if (Debug) {
          DEBUG_PRINT("%x", transition - transitions_);
        }

        state = transition->toState;

        if (!!stateActions_) {
          for (StateAction* stateAction = stateActions_;
               !stateAction->terminator; stateAction++) {
            if (stateAction->state != state) {
              continue;
            }

            if (!!stateAction->entryAction) {
              (parent_.*(stateAction->entryAction))();
            }

            break;
          }
        }

        return true;
      }
    }

    // TODO: Non-matching??
    if (Debug) {
      DEBUG_PRINT("X");
    }
    return true;
  }
};
