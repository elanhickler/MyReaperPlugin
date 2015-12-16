#pragma once

#include "MyFirstClass.hpp"
#include "MyFirstDialog.h"
#include "utilfuncs.h"

void doAction1() {
	ShowMessageBox("Hello World!", "Reaper extension", 0);
}

void doAction2(action_entry& act) {
	// this action does nothing else but toggles the variable that keeps track of the toggle state
	// so it's useless as such but you can see the action state changing in the toolbar buttons and the actions list
	if (act.m_togglestate==ToggleOff)
		act.m_togglestate=ToggleOn;
	else act.m_togglestate=ToggleOff;
	// store new state of toggle action to ini file immediately
	char buf[8];
	// the REAPER api for ini-files only deals with strings, so form a string from the action
	// toggle state number.
	int toggletemp = 0;
	if (act.m_togglestate == ToggleOn)
		toggletemp = 1;
	sprintf(buf, "%d", toggletemp);
	SetExtState("simple_extension", "toggleaction_state", buf, true);
}

void doAction3(action_entry& act) {
	readbg() << "action in cycle state " << act.m_cycle_state << "\n";
	act.m_cycle_state = (act.m_cycle_state + 1) % 3;
}
