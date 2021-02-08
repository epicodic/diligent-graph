#include <dg/input/shortcuts.hpp>

namespace dg {

void Shortcuts::addShortcut(const KeySequence& key_seq, std::function<void()> on_activate, const std::string& info_text)
{
	shortcuts_.push_back({key_seq,on_activate, info_text});
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
	pressed_keys_.all_keys.insert(event.key);
	pressed_keys_.last_key = event.key;

	//dumpSimKeys(_pressed_keys);

	matchNext();
	selectNewCandidates();
	handleCompleted();
}

void Shortcuts::keyReleaseEvent(const KeyEvent& event)
{
	pressed_keys_.all_keys.erase(event.key);
	pressed_keys_.last_key = 0;
}

void Shortcuts::focusOutEvent()
{
	reset();
	pressed_keys_.all_keys.clear();
	pressed_keys_.last_key = 0;
}

void Shortcuts::reset()
{
	candidates_.clear();
}

void Shortcuts::matchNext()
{
	std::list<Candidate>::iterator next, it;
	for(it = candidates_.begin(); it!=candidates_.end(); it=next)
	{
		Candidate& c = *it;
		next = it;
		++next;
		if(c.shortcut->key_seq[c.pos] == pressed_keys_)
		{
			// match!
			c.pos++;
		}
		else
		{
			// no match, remove candidate
			candidates_.erase(it);
		}
	}
}

void Shortcuts::selectNewCandidates()
{
	for(Shortcut& s : shortcuts_)
	{
		if(s.key_seq.front() == pressed_keys_)
			candidates_.push_back({&s,1});
	}
}

void Shortcuts::handleCompleted()
{
	std::vector<Shortcut*> completed;

	// collect completed shortcut sequences
	for(const Candidate& c : candidates_)
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
