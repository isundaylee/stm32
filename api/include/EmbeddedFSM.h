#pragma once

#include <RingBuffer.h>

template <typename Parent, typename StateType, typename EventType>
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
  State state_;
  Parent& parent_;
  // TODO: Investigate. How I wish I had std::array...
  Transition* transitions_;

public:
  EmbeddedFSM(State initialState, Parent& parent, Transition* transitions)
      : state_(initialState), parent_(parent), transitions_(transitions) {}

  bool pushEvent(Event event) { return events_.push(event); }

  bool processOneEvent() {
    if (events_.empty()) {
      return false;
    }

    Event event{};
    events_.pop(event);

    for (Transition* transition = transitions_; !transition->terminator;
         transition++) {
      if ((state_ == transition->fromState) && (event == transition->event)) {
        if (!!transition->action) {
          (parent_.*(transition->action))();
        }
        state_ = transition->toState;
        return true;
      }
    }

    // TODO: Non-matching??
    return true;
  }
};
