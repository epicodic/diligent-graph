#include <dg/input/shortcuts.hpp>

namespace dg {

void Shortcuts::addShortcut(const KeySequence& key_seq, std::function<void()> on_activate, const std::string& info_text)
{
	_shortcuts.push_back({key_seq,on_activate, info_text});
}

void Shortcuts::addShortcut(const std::string& key_seq, std::function<void()> on_activate, const std::string& info_text)
{
	addShortcut(keySequenceFromString(key_seq), on_activate, info_text);
}

/*
static void dumpSimKeys(const KeySequenceSimKeys& s)
{
	for(int key : s.all_keys)
		std::cout << keyToName(key) << ", ";

	std::cout << keyToName(s.last_key) << std::endl;
}
*/

void Shortcuts::keyPressEvent(const KeyEvent& event)
{
	_pressed_keys.all_keys.insert(event.key);
	_pressed_keys.last_key = event.key;

	//dumpSimKeys(_pressed_keys);

	matchNext();
	selectNewCandidates();
	handleCompleted();
}

void Shortcuts::keyReleaseEvent(const KeyEvent& event)
{
	_pressed_keys.all_keys.erase(event.key);
	_pressed_keys.last_key = 0;
}

void Shortcuts::focusOutEvent()
{
	reset();
	_pressed_keys.all_keys.clear();
	_pressed_keys.last_key = 0;
}

void Shortcuts::reset()
{
	_candidates.clear();
}

void Shortcuts::matchNext()
{
	std::list<Candidate>::iterator next, it;
	for(it = _candidates.begin(); it!=_candidates.end(); it=next)
	{
		Candidate& c = *it;
		next = it;
		++next;
		if(c.shortcut->key_seq[c.pos] == _pressed_keys)
		{
			// match!
			c.pos++;
		}
		else
		{
			// no match, remove candidate
			_candidates.erase(it);
		}
	}
}

void Shortcuts::selectNewCandidates()
{
	for(Shortcut& s : _shortcuts)
	{
		if(s.key_seq.front() == _pressed_keys)
			_candidates.push_back({&s,1});
	}
}

void Shortcuts::handleCompleted()
{
	std::vector<Shortcut*> completed;

	// collect completed shortcut sequences
	for(const Candidate& c : _candidates)
		if(c.pos >= c.shortcut->key_seq.size())
			completed.push_back(c.shortcut);

	if(completed.empty())
		return;

	// sort according to length and choose longest matching

	std::sort(completed.begin(), completed.end(),
	    [](Shortcut* a, Shortcut* b) -> bool
		{
			return a->key_seq.size() > b->key_seq.size();
		});


	completed.front()->on_activate();
	reset(); // reset since we completed a shortcut
}

}
