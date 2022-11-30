#include "StateMachine.h"
#include <iostream>

StateMachine::~StateMachine() {}

void StateMachine::SetInitialState(Ref<State> initial_) {
	initialState = initial_;
	currentState = initialState;
}

void StateMachine::Update() {
	Transition* triggered = nullptr;

	//store first transition that triggers
	for (auto transition : currentState->GetTransitions()) {
		if (transition->IsTriggered()) {
			triggered = transition;
			break;
		}
	}
	if (triggered) {
		Ref<State> targetState = triggered->GetTargetState();
		//TODO collect exit actions, transition action, add target state actions
		currentState = targetState;
	}
	//return any actions
	currentState->GetAction().MakeDecision();
	return;
}
