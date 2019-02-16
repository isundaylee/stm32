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

  static constexpr Transition TransitionTerminator = {
      static_cast<State>(0),
      static_cast<Event>(0),
      nullptr,
      static_cast<State>(0),
      true,
  };

private:
  RingBuffer<Event, 16> events_;
  Parent& parent_;
  // TODO: Investigate. How I wish I had std::array...
  Transition* transitions_;

public:
  State state;

  EmbeddedFSM(State initialState, Parent& parent, Transition* transitions)
      : parent_(parent), transitions_(transitions), state(initialState) {}

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
