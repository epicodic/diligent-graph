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

    { Key_ScrollLock,    "ScrollLock"},
    { Key_Menu,          "Menu"},
    { Key_Help,          "Help"},

    { Key_Back,                        "Back"},
    { Key_Forward,                     "Forward"},
    { Key_Stop,                        "Stop"},
    { Key_Refresh,                     "Refresh"},
    { Key_VolumeDown,                  "VolumeDown"},
    { Key_VolumeMute,                  "VolumeMute"},
    { Key_VolumeUp,                    "VolumeUp"},
    { Key_BassBoost,                   "BassBoost"},
    { Key_BassUp,                      "BassUp"},
    { Key_BassDown,                    "BassDown"},
    { Key_TrebleUp,                    "TrebleUp"},
    { Key_TrebleDown,                  "TrebleDown"},
    { Key_MediaPlay,                   "MediaPlay"},
    { Key_MediaStop,                   "MediaStop"},
    { Key_MediaPrevious,               "MediaPrevious"},
    { Key_MediaNext,                   "MediaNext"},
    { Key_MediaRecord,                 "MediaRecord"},
    { Key_MediaPause,                  "MediaPause"},
    { Key_MediaTogglePlayPause,        "ToggleMediaPlayPause"},
    { Key_HomePage,                    "HomePage"},
    { Key_Favorites,                   "Favorites"},
    { Key_Search,                      "Search"},
    { Key_Standby,                     "Standby"},
    { Key_OpenUrl,                     "OpenURL"},
    { Key_LaunchMail,                  "LaunchMail"},
    { Key_LaunchMedia,                 "LaunchMedia"},
    { Key_Launch0,                     "Launch0"},
    { Key_Launch1,                     "Launch1"},
    { Key_Launch2,                     "Launch2"},
    { Key_Launch3,                     "Launch3"},
    { Key_Launch4,                     "Launch4"},
    { Key_Launch5,                     "Launch5"},
    { Key_Launch6,                     "Launch6"},
    { Key_Launch7,                     "Launch7"},
    { Key_Launch8,                     "Launch8"},
    { Key_Launch9,                     "Launch9"},
    { Key_LaunchA,                     "LaunchA"},
    { Key_LaunchB,                     "LaunchB"},
    { Key_LaunchC,                     "LaunchC"},
    { Key_LaunchD,                     "LaunchD"},
    { Key_LaunchE,                     "LaunchE"},
    { Key_LaunchF,                     "LaunchF"},
    { Key_LaunchG,                     "LaunchG"},
    { Key_LaunchH,                     "LaunchH"},
    { Key_MonBrightnessUp,             "MonitorBrightnessUp"},
    { Key_MonBrightnessDown,           "MonitorBrightnessDown"},
    { Key_KeyboardLightOnOff,          "KeyboardLightOnOff"},
    { Key_KeyboardBrightnessUp,        "KeyboardBrightnessUp"},
    { Key_KeyboardBrightnessDown,      "KeyboardBrightnessDown"},
    { Key_PowerOff,                    "PowerOff"},
    { Key_WakeUp,                      "WakeUp"},
    { Key_Eject,                       "Eject"},
    { Key_ScreenSaver,                 "Screensaver"},
    { Key_WWW,                         "WWW"},
    { Key_Sleep,                       "Sleep"},
    { Key_LightBulb,                   "LightBulb"},
    { Key_Shop,                        "Shop"},
    { Key_History,                     "History"},
    { Key_AddFavorite,                 "AddFavorite"},
    { Key_HotLinks,                    "HotLinks"},
    { Key_BrightnessAdjust,            "AdjustBrightness"},
    { Key_Finance,                     "Finance"},
    { Key_Community,                   "Community"},
    { Key_AudioRewind,                 "MediaRewind"},
    { Key_BackForward,                 "BackForward"},
    { Key_ApplicationLeft,             "ApplicationLeft"},
    { Key_ApplicationRight,            "ApplicationRight"},
    { Key_Book,                        "Book"},
    { Key_CD,                          "CD"},
    { Key_Calculator,                  "Calculator"},
    { Key_Calendar,                    "Calendar"},
    { Key_Clear,                       "Clear"},
    { Key_ClearGrab,                   "ClearGrab"},
    { Key_Close,                       "Close"},
    { Key_ContrastAdjust,              "Adjustcontrast"},
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
    { Key_MenuKB,                      "KeyboardMenu"},
    { Key_MenuPB,                      "MenuPB"},
    { Key_MySites,                     "MySites"},
    { Key_News,                        "News"},
    { Key_OfficeHome,                  "HomeOffice"},
    { Key_Option,                      "Option"},
    { Key_Paste,                       "Paste"},
    { Key_Phone,                       "Phone"},
    { Key_Reply,                       "Reply"},
    { Key_Reload,                      "Reload"},
    { Key_RotateWindows,               "RotateWindows"},
    { Key_RotationPB,                  "RotationPB"},
    { Key_RotationKB,                  "RotationKB"},
    { Key_Save,                        "Save"},
    { Key_Send,                        "Send"},
    { Key_Spell,                       "Spellchecker"},
    { Key_SplitScreen,                 "SplitScreen"},
    { Key_Support,                     "Support"},
    { Key_TaskPane,                    "TaskPanel"},
    { Key_Terminal,                    "Terminal"},
    { Key_ToDoList,                    "Todolist"},
    { Key_Tools,                       "Tools"},
    { Key_Travel,                      "Travel"},
    { Key_Video,                       "Video"},
    { Key_Word,                        "Word"},
    { Key_Xfer,                        "XFer"},
    { Key_ZoomIn,                      "ZoomIn"},
    { Key_ZoomOut,                     "ZoomOut"},
    { Key_Away,                        "Away"},
    { Key_Messenger,                   "Messenger"},
    { Key_WebCam,                      "WebCam"},
    { Key_MailForward,                 "MailForward"},
    { Key_Pictures,                    "Pictures"},
    { Key_Music,                       "Music"},
    { Key_Battery,                     "Battery"},
    { Key_Bluetooth,                   "Bluetooth"},
    { Key_WLAN,                        "Wireless"},
    { Key_UWB,                         "UWB"},
    { Key_AudioForward,                "MediaFastForward"},
    { Key_AudioRepeat,                 "AudioRepeat"},
    { Key_AudioRandomPlay,             "AudioRandomPlay"},
    { Key_Subtitle,                    "Subtitle"},
    { Key_AudioCycleTrack,             "AudioCycleTrack"},
    { Key_Time,                        "Time"},
    { Key_Hibernate,                   "Hibernate"},
    { Key_View,                        "View"},
    { Key_TopMenu,                     "TopMenu"},
    { Key_PowerDown,                   "PowerDown"},
    { Key_Suspend,                     "Suspend"},
    { Key_MicMute,                     "MicMute"},
    { Key_Red,                         "Red"},
    { Key_Green,                       "Green"},
    { Key_Yellow,                      "Yellow"},
    { Key_Blue,                        "Blue"},
    { Key_ChannelUp,                   "ChannelUp"},
    { Key_ChannelDown,                 "ChannelDown"},
    { Key_Guide,                       "Guide"},
    { Key_Info,                        "Info"},
    { Key_Settings,                    "Settings"},
    { Key_MicVolumeUp,                 "MicVolumeUp"},
    { Key_MicVolumeDown,               "MicVolumeDown"},
    { Key_New,                         "New"},
    { Key_Open,                        "Open"},
    { Key_Find,                        "Find"},
    { Key_Undo,                        "Undo"},
    { Key_Redo,                        "Redo"},
    { Key_Print,         "Print"},
    { Key_PageUp,        "PageUp"},
    { Key_PageDown,      "PageDown"},
    { Key_CapsLock,      "CapsLock"},
	{ Key_NumLock,       "NumLock"},
    { Key_NumLock,       "NumberLock"},
    { Key_ScrollLock,    "ScrollLock"},
    { Key_Insert,        "Insert"},
    { Key_Delete,        "Delete"},
    { Key_Escape,        "Escape"},
    { Key_SysReq,        "SystemRequest"},
    { Key_Select,        "Select"},
    { Key_Yes,           "Yes"},
    { Key_No,            "No"},
    { Key_Codeinput,         "Codeinput"},
    { Key_MultipleCandidate, "MultipleCandidate"},
    { Key_PreviousCandidate, "PreviousCandidate"},
    { Key_Cancel,   "Cancel"},
    { Key_Printer,   "Printer"},
    { Key_Execute,   "Execute"},
    { Key_Play,   "Play"},
    { Key_Zoom,   "Zoom"},
    { Key_Exit,   "Exit"},
    { Key_TouchpadToggle,   "TouchpadToggle"},
    { Key_TouchpadOn,   "TouchpadOn"},
    { Key_TouchpadOff,   "TouchpadOff"},
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
