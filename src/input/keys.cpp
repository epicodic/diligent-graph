#include <dg/input/keys.hpp>

#include <dg/input/string_utils.hpp>

namespace dg {


static const struct {
    int key;
    const char* name;
} g_key_names[] = {
    { Key_Space,         "Space"},
    { Key_Escape,        "Esc"},
    { Key_Tab,           "Tab"},
    { Key_Backtab,       "Backtab"},
    { Key_Backspace,     "Backspace"},
    { Key_Return,        "Return"},
    { Key_Enter,         "Enter"},
    { Key_Insert,        "Ins"},
    { Key_Delete,        "Del"},
    { Key_Pause,         "Pause"},
    { Key_Print,         "Print"},
    { Key_SysReq,        "SysReq"},
    { Key_Home,          "Home"},
    { Key_End,           "End"},
    { Key_Left,          "Left"},
    { Key_Up,            "Up"},
    { Key_Right,         "Right"},
    { Key_Down,          "Down"},
    { Key_PageUp,        "PgUp"},
    { Key_PageDown,      "PgDown"},

	{ Key_Shift,         "Shift"},
    { Key_Control,       "Ctrl"},
    { Key_Meta,          "Meta"},
    { Key_Alt,           "Alt"},
	{ Key_AltGr,         "AltGr"},
	{ Key_Multi_key,     "Multi"},
    { Key_NumLock,       "Num"},

    { Key_CapsLock,      "CapsLock"},
    { Key_ScrollLock,    "ScrollLock"},
    { Key_Menu,          "Menu"},
    { Key_Help,          "Help"},

    { Key_Back,                        "Back"},
    { Key_Forward,                     "Forward"},
    { Key_Stop,                        "Stop"},
    { Key_Refresh,                     "Refresh"},
    { Key_VolumeDown,                  "Volume Down"},
    { Key_VolumeMute,                  "Volume Mute"},
    { Key_VolumeUp,                    "Volume Up"},
    { Key_BassBoost,                   "Bass Boost"},
    { Key_BassUp,                      "Bass Up"},
    { Key_BassDown,                    "Bass Down"},
    { Key_TrebleUp,                    "Treble Up"},
    { Key_TrebleDown,                  "Treble Down"},
    { Key_MediaPlay,                   "Media Play"},
    { Key_MediaStop,                   "Media Stop"},
    { Key_MediaPrevious,               "Media Previous"},
    { Key_MediaNext,                   "Media Next"},
    { Key_MediaRecord,                 "Media Record"},
    { Key_MediaPause,                  "Media Pause"},
    { Key_MediaTogglePlayPause,        "Toggle Media Play/Pause"},
    { Key_HomePage,                    "Home Page"},
    { Key_Favorites,                   "Favorites"},
    { Key_Search,                      "Search"},
    { Key_Standby,                     "Standby"},
    { Key_OpenUrl,                     "Open URL"},
    { Key_LaunchMail,                  "Launch Mail"},
    { Key_LaunchMedia,                 "Launch Media"},
    { Key_Launch0,                     "Launch (0)"},
    { Key_Launch1,                     "Launch (1)"},
    { Key_Launch2,                     "Launch (2)"},
    { Key_Launch3,                     "Launch (3)"},
    { Key_Launch4,                     "Launch (4)"},
    { Key_Launch5,                     "Launch (5)"},
    { Key_Launch6,                     "Launch (6)"},
    { Key_Launch7,                     "Launch (7)"},
    { Key_Launch8,                     "Launch (8)"},
    { Key_Launch9,                     "Launch (9)"},
    { Key_LaunchA,                     "Launch (A)"},
    { Key_LaunchB,                     "Launch (B)"},
    { Key_LaunchC,                     "Launch (C)"},
    { Key_LaunchD,                     "Launch (D)"},
    { Key_LaunchE,                     "Launch (E)"},
    { Key_LaunchF,                     "Launch (F)"},
    { Key_LaunchG,                     "Launch (G)"},
    { Key_LaunchH,                     "Launch (H)"},
    { Key_MonBrightnessUp,             "Monitor Brightness Up"},
    { Key_MonBrightnessDown,           "Monitor Brightness Down"},
    { Key_KeyboardLightOnOff,          "Keyboard Light On/Off"},
    { Key_KeyboardBrightnessUp,        "Keyboard Brightness Up"},
    { Key_KeyboardBrightnessDown,      "Keyboard Brightness Down"},
    { Key_PowerOff,                    "Power Off"},
    { Key_WakeUp,                      "Wake Up"},
    { Key_Eject,                       "Eject"},
    { Key_ScreenSaver,                 "Screensaver"},
    { Key_WWW,                         "WWW"},
    { Key_Sleep,                       "Sleep"},
    { Key_LightBulb,                   "LightBulb"},
    { Key_Shop,                        "Shop"},
    { Key_History,                     "History"},
    { Key_AddFavorite,                 "Add Favorite"},
    { Key_HotLinks,                    "Hot Links"},
    { Key_BrightnessAdjust,            "Adjust Brightness"},
    { Key_Finance,                     "Finance"},
    { Key_Community,                   "Community"},
    { Key_AudioRewind,                 "Media Rewind"},
    { Key_BackForward,                 "Back Forward"},
    { Key_ApplicationLeft,             "Application Left"},
    { Key_ApplicationRight,            "Application Right"},
    { Key_Book,                        "Book"},
    { Key_CD,                          "CD"},
    { Key_Calculator,                  "Calculator"},
    { Key_Calendar,                    "Calendar"},
    { Key_Clear,                       "Clear"},
    { Key_ClearGrab,                   "Clear Grab"},
    { Key_Close,                       "Close"},
    { Key_ContrastAdjust,              "Adjust contrast"},
    { Key_Copy,                        "Copy"},
    { Key_Cut,                         "Cut"},
    { Key_Display,                     "Display"},
    { Key_DOS,                         "DOS"},
    { Key_Documents,                   "Documents"},
    { Key_Excel,                       "Spreadsheet"},
    { Key_Explorer,                    "Browser"},
    { Key_Game,                        "Game"},
    { Key_Go,                          "Go"},
    { Key_iTouch,                      "iTouch"},
    { Key_LogOff,                      "Logoff"},
    { Key_Market,                      "Market"},
    { Key_Meeting,                     "Meeting"},
    { Key_Memo,                        "Memo"},
    { Key_MenuKB,                      "Keyboard Menu"},
    { Key_MenuPB,                      "Menu PB"},
    { Key_MySites,                     "My Sites"},
    { Key_News,                        "News"},
    { Key_OfficeHome,                  "Home Office"},
    { Key_Option,                      "Option"},
    { Key_Paste,                       "Paste"},
    { Key_Phone,                       "Phone"},
    { Key_Reply,                       "Reply"},
    { Key_Reload,                      "Reload"},
    { Key_RotateWindows,               "Rotate Windows"},
    { Key_RotationPB,                  "Rotation PB"},
    { Key_RotationKB,                  "Rotation KB"},
    { Key_Save,                        "Save"},
    { Key_Send,                        "Send"},
    { Key_Spell,                       "Spellchecker"},
    { Key_SplitScreen,                 "Split Screen"},
    { Key_Support,                     "Support"},
    { Key_TaskPane,                    "Task Panel"},
    { Key_Terminal,                    "Terminal"},
    { Key_ToDoList,                    "To-do list"},
    { Key_Tools,                       "Tools"},
    { Key_Travel,                      "Travel"},
    { Key_Video,                       "Video"},
    { Key_Word,                        "Word Processor"},
    { Key_Xfer,                        "XFer"},
    { Key_ZoomIn,                      "Zoom In"},
    { Key_ZoomOut,                     "Zoom Out"},
    { Key_Away,                        "Away"},
    { Key_Messenger,                   "Messenger"},
    { Key_WebCam,                      "WebCam"},
    { Key_MailForward,                 "Mail Forward"},
    { Key_Pictures,                    "Pictures"},
    { Key_Music,                       "Music"},
    { Key_Battery,                     "Battery"},
    { Key_Bluetooth,                   "Bluetooth"},
    { Key_WLAN,                        "Wireless"},
    { Key_UWB,                         "Ultra Wide Band"},
    { Key_AudioForward,                "Media Fast Forward"},
    { Key_AudioRepeat,                 "Audio Repeat"},
    { Key_AudioRandomPlay,             "Audio Random Play"},
    { Key_Subtitle,                    "Subtitle"},
    { Key_AudioCycleTrack,             "Audio Cycle Track"},
    { Key_Time,                        "Time"},
    { Key_Hibernate,                   "Hibernate"},
    { Key_View,                        "View"},
    { Key_TopMenu,                     "Top Menu"},
    { Key_PowerDown,                   "Power Down"},
    { Key_Suspend,                     "Suspend"},

    { Key_MicMute,                     "Microphone Mute"},

    { Key_Red,                         "Red"},
    { Key_Green,                       "Green"},
    { Key_Yellow,                      "Yellow"},
    { Key_Blue,                        "Blue"},

    { Key_ChannelUp,                   "Channel Up"},
    { Key_ChannelDown,                 "Channel Down"},

    { Key_Guide,                       "Guide"},
    { Key_Info,                        "Info"},
    { Key_Settings,                    "Settings"},

    { Key_MicVolumeUp,                 "Microphone Volume Up"},
    { Key_MicVolumeDown,               "Microphone Volume Down"},

    { Key_New,                         "New"},
    { Key_Open,                        "Open"},
    { Key_Find,                        "Find"},
    { Key_Undo,                        "Undo"},
    { Key_Redo,                        "Redo"},

    { Key_Print,         "Print Screen"},
    { Key_PageUp,        "Page Up"},
    { Key_PageDown,      "Page Down"},
    { Key_CapsLock,      "Caps Lock"},
	{ Key_NumLock,       "NumLock"},
    { Key_NumLock,       "Num Lock"},
    { Key_NumLock,       "Number Lock"},
    { Key_ScrollLock,    "Scroll Lock"},
    { Key_Insert,        "Insert"},
    { Key_Delete,        "Delete"},
    { Key_Escape,        "Escape"},
    { Key_SysReq,        "System Request"},

    { Key_Select,        "Select"},
    { Key_Yes,           "Yes"},
    { Key_No,            "No"},

    { Key_Codeinput,         "Code input"},
    { Key_MultipleCandidate, "Multiple Candidate"},
    { Key_PreviousCandidate, "Previous Candidate"},

    { Key_Cancel,   "Cancel"},
    { Key_Printer,   "Printer"},
    { Key_Execute,   "Execute"},
    { Key_Play,   "Play"},
    { Key_Zoom,   "Zoom"},
    { Key_Exit,   "Exit"},
    { Key_TouchpadToggle,   "Touchpad Toggle"},
    { Key_TouchpadOn,   "Touchpad On"},
    { Key_TouchpadOff,   "Touchpad Off"},
};


std::string keyToName(int key)
{
	const int num_key_names = sizeof g_key_names / sizeof *g_key_names;

	if (key && key < Key_Escape && key != Key_Space)
	{
		std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> utf16conv;

		char16_t unicode = key;
		return utf16conv.to_bytes(unicode);
	}
	else if (key >= Key_F1 && key <= Key_F35)
	{
		return "F"+std::to_string(key - Key_F1 + 1);
	}
	else
	{
		for(int i=0; i<num_key_names; ++i)
		{
			if(g_key_names[i].key == key)
				return g_key_names[i].name;
		}
	}

	return std::string("<unknown, code: " + std::to_string(key) + ">" );
}



int keyFromName(const std::string& name)
{
	if(name.empty())
		return 0;

	std::wstring utf16_name = string_to_wstring(name);
	std::locale loc("en_US.utf8");
	std::wstring utf16_upper_name = wstr_toupper(utf16_name, loc);

	if(utf16_upper_name.size()==1)
		return utf16_upper_name[0];

	std::string upper_name = wstring_to_string(utf16_upper_name);

	if(upper_name[0]=='F' && upper_name.size()>=2 && upper_name.size()<=3)
	{
		std::stringstream ss;
		ss << upper_name.substr(1);
		int nr;
		ss >> nr;
		if(nr>0 && nr<=35)
			return Key_F1 + nr - 1;
	}

	const int num_key_names = sizeof g_key_names / sizeof *g_key_names;
	for(int i=0; i<num_key_names; ++i)
	{
		if(str_toupper(g_key_names[i].name) == upper_name)
			return g_key_names[i].key;
	}

	return 0;
}




}
