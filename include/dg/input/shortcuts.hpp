#pragma once

#include <dg/input/key_events.hpp>
#include <functional>
#include <list>
#include <set>

#include <dg/input/key_sequence.hpp>


namespace dg {

class Shortcuts
{
public:

	void addShortcut(const KeySequence& key_seq, std::function<void()> on_activate, const std::string& info_text = std::string());
	void addShortcut(const std::string& key_seq, std::function<void()> on_activate, const std::string& info_text = std::string());

public:

	void keyPressEvent(const KeyEvent& event);
	void keyReleaseEvent(const KeyEvent& event);
	void focusOutEvent();


	void reset();

private:

	void matchNext();
	void selectNewCandidates();
	void handleCompleted();

private:

	struct Shortcut
	{
		KeySequence key_seq;
		std::function<void()> on_activate;
		std::string info_text;
	};

	struct Candidate
	{
		Shortcut* shortcut;
		int pos;
	};

	std::list<Shortcut> _shortcuts;
	KeySequenceSimKeys _pressed_keys;

	std::list<Candidate> _candidates;

};



}

