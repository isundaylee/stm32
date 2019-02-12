#pragma once

#include <RingBuffer.h>

template <typename Parent, typename StateType, typename EventType>
class EmbeddedFSM {
public:
  using State = StateType;
  using Event = EventType;

private:
  using TransitionFunction = State (Parent::*)(State, Event);

  RingBuffer<Event, 16> events_;
  State state_;
  Parent& parent_;
  TransitionFunction transitionFunction_;

public:
  EmbeddedFSM(State initialState, Parent& parent,
              TransitionFunction transitionFunction)
      : state_(initialState), parent_(parent),
        transitionFunction_(transitionFunction) {}

  bool pushEvent(Event event) { return events_.push(event); }

  bool processOneEvent() {
    if (events_.empty()) {
      return false;
    }

    Event event{};
    events_.pop(event);

    state_ = (parent_.*transitionFunction_)(state_, event);

    return true;
  }
};
