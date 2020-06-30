#include "xcb_keyboard.hpp"

#include <cctype>
#include <map>

#include <xkbcommon/xkbcommon-x11.h>
#include <X11/Xutil.h>

#define explicit dont_use_cxx_explicit
#include <xcb/xkb.h>
#undef explicit



#define DG_XCB_REPLY_CONNECTION_ARG(connection, ...) connection

struct DGXcbStdDeleter {
    void operator()(void *p) const noexcept { return std::free(p); }
};

#define DG_XCB_REPLY(call, ...) \
    std::unique_ptr<call##_reply_t, DGXcbStdDeleter>( \
        call##_reply(DG_XCB_REPLY_CONNECTION_ARG(__VA_ARGS__), call(__VA_ARGS__), nullptr) \
    )

#define DG_XCB_REPLY_UNCHECKED(call, ...) \
    std::unique_ptr<call##_reply_t, DGXcbStdDeleter>( \
        call##_reply(DG_XCB_REPLY_CONNECTION_ARG(__VA_ARGS__), call##_unchecked(__VA_ARGS__), nullptr) \
    )



namespace dg {

typedef std::map<xcb_keysym_t, Key> KeyTable;
static const KeyTable& keyTable();

static bool isLatin(xkb_keysym_t sym) {
    return ((sym >= 'a' && sym <= 'z') || (sym >= 'A' && sym <= 'Z'));
}
static bool isKeypad(xkb_keysym_t sym) {
    return sym >= XKB_KEY_KP_Space && sym <= XKB_KEY_KP_9;
}

XCBKeyboard::XCBKeyboard(xcb_connection_t* c) : _connection(c)
{
	int32_t device_id;

	_xkb_context.reset(xkb_context_new(XKB_CONTEXT_NO_FLAGS));
	if (!_xkb_context)
		DG_THROW("Failed to create keyboard context");

	device_id = xkb_x11_get_core_keyboard_device_id(_connection);
	if (device_id == -1)
		DG_THROW("Failed to get core keyboard device id");

	_xkb_keymap.reset(xkb_x11_keymap_new_from_device(_xkb_context.get(), _connection, device_id, XKB_KEYMAP_COMPILE_NO_FLAGS));
	if (!_xkb_keymap)
		DG_THROW("Failed to create keymap");

	_xkb_state.reset(xkb_x11_state_new_from_device(_xkb_keymap.get(), _connection, device_id));
	if (!_xkb_state)
		DG_THROW("Failed to create keyboard state");

	updateXKBMods();
}

void XCBKeyboard::initializeXKB(xcb_connection_t* connection)
{
	const xcb_query_extension_reply_t *reply = xcb_get_extension_data(connection, &xcb_xkb_id);
	if (!reply || !reply->present)
		DG_THROW("XKeyboard extension not present on the X server");

	int wantMajor = 1;
	int wantMinor = 0;
	auto xkbQuery = DG_XCB_REPLY(xcb_xkb_use_extension, connection, wantMajor, wantMinor);
	if (!xkbQuery)
		DG_THROW("Failed to initialize XKeyboard extension");

	if (!xkbQuery->supported)
		DG_THROW("unsupported XKB version"); //  wantMajor, wantMinor, xkbQuery->serverMajor, xkbQuery->serverMinor);
}


void XCBKeyboard::updateXKBMods()
{
    _xkb_mods.shift = xkb_keymap_mod_get_index(_xkb_keymap.get(), XKB_MOD_NAME_SHIFT);
    _xkb_mods.lock = xkb_keymap_mod_get_index(_xkb_keymap.get(), XKB_MOD_NAME_CAPS);
    _xkb_mods.control = xkb_keymap_mod_get_index(_xkb_keymap.get(), XKB_MOD_NAME_CTRL);
    _xkb_mods.mod1 = xkb_keymap_mod_get_index(_xkb_keymap.get(), "Mod1");
    _xkb_mods.mod2 = xkb_keymap_mod_get_index(_xkb_keymap.get(), "Mod2");
    _xkb_mods.mod3 = xkb_keymap_mod_get_index(_xkb_keymap.get(), "Mod3");
    _xkb_mods.mod4 = xkb_keymap_mod_get_index(_xkb_keymap.get(), "Mod4");
    _xkb_mods.mod5 = xkb_keymap_mod_get_index(_xkb_keymap.get(), "Mod5");
}



int XCBKeyboard::keysymToKey(xkb_keysym_t ks)
{
    int key = 0;

    if (ks >= XKB_KEY_F1 && ks <= XKB_KEY_F35) {
        key = Key_F1 + (ks - XKB_KEY_F1);
    } else if (ks >= XKB_KEY_KP_0 && ks <= XKB_KEY_KP_9) {
        key = Key_0 + (ks - XKB_KEY_KP_0);
    } else if (isLatin(ks)) {
    	key = std::toupper(ks);
    } else {
    	auto it = keyTable().find(ks);

    	if(it!=keyTable().end())
    		key = it->second;
    }

    if(key)
    	return key;

    //xkb_keysym_t ks_upper = xkb_keysym_to_upper(keysym);
    KeySym ks_upper, ks_lower;
    XConvertCase(ks, &ks_lower, &ks_upper);
    std::uint32_t unicode = xkb_keysym_to_utf32(ks_upper);

    return unicode;
}

ModifierKey XCBKeyboard::stateToModifier(std::uint16_t state)
{
	ModifierKey ret = Modifier_None;
	if (state & XCB_MOD_MASK_SHIFT)
		ret |= Modifier_Shift;
	if (state & XCB_MOD_MASK_CONTROL)
		ret |= Modifier_Ctrl;
	if (state & XCB_MOD_MASK_1)
		ret |= Modifier_Alt;
	if (state & XCB_MOD_MASK_5)
		ret |= Modifier_GroupSwitch;

	return ret;
}


xkb_mod_mask_t XCBKeyboard::xkbModMask(std::uint16_t state)
{
    xkb_mod_mask_t xkb_mask = 0;

    if ((state & XCB_MOD_MASK_SHIFT) && _xkb_mods.shift != XKB_MOD_INVALID)
        xkb_mask |= (1 << _xkb_mods.shift);
    if ((state & XCB_MOD_MASK_LOCK) && _xkb_mods.lock != XKB_MOD_INVALID)
        xkb_mask |= (1 << _xkb_mods.lock);
    if ((state & XCB_MOD_MASK_CONTROL) && _xkb_mods.control != XKB_MOD_INVALID)
        xkb_mask |= (1 << _xkb_mods.control);
    if ((state & XCB_MOD_MASK_1) && _xkb_mods.mod1 != XKB_MOD_INVALID)
        xkb_mask |= (1 << _xkb_mods.mod1);
    if ((state & XCB_MOD_MASK_2) && _xkb_mods.mod2 != XKB_MOD_INVALID)
        xkb_mask |= (1 << _xkb_mods.mod2);
    if ((state & XCB_MOD_MASK_3) && _xkb_mods.mod3 != XKB_MOD_INVALID)
        xkb_mask |= (1 << _xkb_mods.mod3);
    if ((state & XCB_MOD_MASK_4) && _xkb_mods.mod4 != XKB_MOD_INVALID)
        xkb_mask |= (1 << _xkb_mods.mod4);
    if ((state & XCB_MOD_MASK_5) && _xkb_mods.mod5 != XKB_MOD_INVALID)
        xkb_mask |= (1 << _xkb_mods.mod5);

    return xkb_mask;
}

static xkb_layout_index_t lockedGroup(std::uint16_t state)
{
    return (state >> 13) & 3; // bits 13 and 14 report the state keyboard group
}


int XCBKeyboard::keycodeToKey(xcb_keycode_t code, std::uint16_t state, std::string* text, std::uint32_t* unicode)
{
	//ModifierKey modifiers = stateToModifier(state);

    struct xkb_state *xkbState = _xkb_state.get();
	        xkb_mod_mask_t modsDepressed = xkb_state_serialize_mods(xkbState, XKB_STATE_MODS_DEPRESSED);
	        xkb_mod_mask_t modsLatched = xkb_state_serialize_mods(xkbState, XKB_STATE_MODS_LATCHED);
	        xkb_mod_mask_t modsLocked = xkb_state_serialize_mods(xkbState, XKB_STATE_MODS_LOCKED);
	        xkb_mod_mask_t xkbMask = xkbModMask(state);

	        xkb_mod_mask_t latched = modsLatched & xkbMask;
	        xkb_mod_mask_t locked = modsLocked & xkbMask;
	        xkb_mod_mask_t depressed = modsDepressed & xkbMask;
	        // set modifiers in depressed if they don't appear in any of the final masks
	        depressed |= ~(depressed | latched | locked) & xkbMask;

	        xkb_state_component changedComponents = xkb_state_update_mask(
	                    xkbState, depressed, latched, locked, 0, 0, lockedGroup(state));


	xcb_keysym_t ks = xkb_state_key_get_one_sym(_xkb_state.get(), code);
	int key = keysymToKey(ks);

	if(text)
	{
		char buf[32];
		int size = xkb_state_key_get_utf8(_xkb_state.get(), code, buf, 32);
		*text = std::string(buf, size);
	}

	if(unicode)
		*unicode = xkb_keysym_to_utf32(ks);

	return key;
}


static const KeyTable& keyTable()
{
	static KeyTable keyTbl =
	{
		{XKB_KEY_Escape,                  Key_Escape},
		{XKB_KEY_Tab,                     Key_Tab},
		{XKB_KEY_ISO_Left_Tab,            Key_Backtab},
		{XKB_KEY_BackSpace,               Key_Backspace},
		{XKB_KEY_Return,                  Key_Return},
		{XKB_KEY_Insert,                  Key_Insert},
		{XKB_KEY_Delete,                  Key_Delete},
		{XKB_KEY_Clear,                   Key_Delete},
		{XKB_KEY_Pause,                   Key_Pause},
		{XKB_KEY_Print,                   Key_Print},
		{0x1005FF60,                      Key_SysReq},         // hardcoded Sun SysReq
		{0x1007ff00,                      Key_SysReq},         // hardcoded X386 SysReq

		{XKB_KEY_Home,                    Key_Home},
		{XKB_KEY_End,                     Key_End},
		{XKB_KEY_Left,                    Key_Left},
		{XKB_KEY_Up,                      Key_Up},
		{XKB_KEY_Right,                   Key_Right},
		{XKB_KEY_Down,                    Key_Down},
		{XKB_KEY_Prior,                   Key_PageUp},
		{XKB_KEY_Next,                    Key_PageDown},

		{XKB_KEY_Shift_L,                 Key_Shift},
		{XKB_KEY_Shift_R,                 Key_Shift},
		{XKB_KEY_Shift_Lock,              Key_Shift},
		{XKB_KEY_Control_L,               Key_Control},
		{XKB_KEY_Control_R,               Key_Control},
		{XKB_KEY_Meta_L,                  Key_Meta},
		{XKB_KEY_Meta_R,                  Key_Meta},
		{XKB_KEY_Alt_L,                   Key_Alt},
		{XKB_KEY_Alt_R,                   Key_Alt},
		{XKB_KEY_Caps_Lock,               Key_CapsLock},
		{XKB_KEY_Num_Lock,                Key_NumLock},
		{XKB_KEY_Scroll_Lock,             Key_ScrollLock},
		{XKB_KEY_Super_L,                 Key_Super_L},
		{XKB_KEY_Super_R,                 Key_Super_R},
		{XKB_KEY_Menu,                    Key_Menu},
		{XKB_KEY_Hyper_L,                 Key_Hyper_L},
		{XKB_KEY_Hyper_R,                 Key_Hyper_R},
		{XKB_KEY_Help,                    Key_Help},
		{0x1000FF74,                      Key_Backtab},        // hardcoded HP backtab
		{0x1005FF10,                      Key_F11},            // hardcoded Sun F36 (labeled F11)
		{0x1005FF11,                      Key_F12},            // hardcoded Sun F37 (labeled F12)

		{XKB_KEY_KP_Space,                Key_Space},
		{XKB_KEY_KP_Tab,                  Key_Tab},
		{XKB_KEY_KP_Enter,                Key_Enter},
		{XKB_KEY_KP_Home,                 Key_Home},
		{XKB_KEY_KP_Left,                 Key_Left},
		{XKB_KEY_KP_Up,                   Key_Up},
		{XKB_KEY_KP_Right,                Key_Right},
		{XKB_KEY_KP_Down,                 Key_Down},
		{XKB_KEY_KP_Prior,                Key_PageUp},
		{XKB_KEY_KP_Next,                 Key_PageDown},
		{XKB_KEY_KP_End,                  Key_End},
		{XKB_KEY_KP_Begin,                Key_Clear},
		{XKB_KEY_KP_Insert,               Key_Insert},
		{XKB_KEY_KP_Delete,               Key_Delete},
		{XKB_KEY_KP_Equal,                Key_Equal},
		{XKB_KEY_KP_Multiply,             Key_Asterisk},
		{XKB_KEY_KP_Add,                  Key_Plus},
		{XKB_KEY_KP_Separator,            Key_Comma},
		{XKB_KEY_KP_Subtract,             Key_Minus},
		{XKB_KEY_KP_Decimal,              Key_Period},
		{XKB_KEY_KP_Divide,               Key_Slash},

		{XKB_KEY_Undo,                    Key_Undo},
		{XKB_KEY_Redo,                    Key_Redo},
		{XKB_KEY_Find,                    Key_Find},
		{XKB_KEY_Cancel,                  Key_Cancel},

		{XKB_KEY_ISO_Level3_Shift,        Key_AltGr},
		{XKB_KEY_Multi_key,               Key_Multi_key},
		{XKB_KEY_Codeinput,               Key_Codeinput},
		{XKB_KEY_SingleCandidate,         Key_SingleCandidate},
		{XKB_KEY_MultipleCandidate,       Key_MultipleCandidate},
		{XKB_KEY_PreviousCandidate,       Key_PreviousCandidate},

		{XKB_KEY_Mode_switch,             Key_Mode_switch},
		{XKB_KEY_script_switch,           Key_Mode_switch},

		{XKB_KEY_XF86Back,                Key_Back},
		{XKB_KEY_XF86Forward,             Key_Forward},
		{XKB_KEY_XF86Stop,                Key_Stop},
		{XKB_KEY_XF86Refresh,             Key_Refresh},
		{XKB_KEY_XF86Favorites,           Key_Favorites},
		{XKB_KEY_XF86AudioMedia,          Key_LaunchMedia},
		{XKB_KEY_XF86OpenURL,             Key_OpenUrl},
		{XKB_KEY_XF86HomePage,            Key_HomePage},
		{XKB_KEY_XF86Search,              Key_Search},
		{XKB_KEY_XF86AudioLowerVolume,    Key_VolumeDown},
		{XKB_KEY_XF86AudioMute,           Key_VolumeMute},
		{XKB_KEY_XF86AudioRaiseVolume,    Key_VolumeUp},
		{XKB_KEY_XF86AudioPlay,           Key_MediaPlay},
		{XKB_KEY_XF86AudioStop,           Key_MediaStop},
		{XKB_KEY_XF86AudioPrev,           Key_MediaPrevious},
		{XKB_KEY_XF86AudioNext,           Key_MediaNext},
		{XKB_KEY_XF86AudioRecord,         Key_MediaRecord},
		{XKB_KEY_XF86AudioPause,          Key_MediaPause},
		{XKB_KEY_XF86Mail,                Key_LaunchMail},
		{XKB_KEY_XF86MyComputer,          Key_Launch0},  // ### Qt 6: remap properly
		{XKB_KEY_XF86Calculator,          Key_Launch1},
		{XKB_KEY_XF86Memo,                Key_Memo},
		{XKB_KEY_XF86ToDoList,            Key_ToDoList},
		{XKB_KEY_XF86Calendar,            Key_Calendar},
		{XKB_KEY_XF86PowerDown,           Key_PowerDown},
		{XKB_KEY_XF86ContrastAdjust,      Key_ContrastAdjust},
		{XKB_KEY_XF86Standby,             Key_Standby},
		{XKB_KEY_XF86MonBrightnessUp,     Key_MonBrightnessUp},
		{XKB_KEY_XF86MonBrightnessDown,   Key_MonBrightnessDown},
		{XKB_KEY_XF86KbdLightOnOff,       Key_KeyboardLightOnOff},
		{XKB_KEY_XF86KbdBrightnessUp,     Key_KeyboardBrightnessUp},
		{XKB_KEY_XF86KbdBrightnessDown,   Key_KeyboardBrightnessDown},
		{XKB_KEY_XF86PowerOff,            Key_PowerOff},
		{XKB_KEY_XF86WakeUp,              Key_WakeUp},
		{XKB_KEY_XF86Eject,               Key_Eject},
		{XKB_KEY_XF86ScreenSaver,         Key_ScreenSaver},
		{XKB_KEY_XF86WWW,                 Key_WWW},
		{XKB_KEY_XF86Sleep,               Key_Sleep},
		{XKB_KEY_XF86LightBulb,           Key_LightBulb},
		{XKB_KEY_XF86Shop,                Key_Shop},
		{XKB_KEY_XF86History,             Key_History},
		{XKB_KEY_XF86AddFavorite,         Key_AddFavorite},
		{XKB_KEY_XF86HotLinks,            Key_HotLinks},
		{XKB_KEY_XF86BrightnessAdjust,    Key_BrightnessAdjust},
		{XKB_KEY_XF86Finance,             Key_Finance},
		{XKB_KEY_XF86Community,           Key_Community},
		{XKB_KEY_XF86AudioRewind,         Key_AudioRewind},
		{XKB_KEY_XF86BackForward,         Key_BackForward},
		{XKB_KEY_XF86ApplicationLeft,     Key_ApplicationLeft},
		{XKB_KEY_XF86ApplicationRight,    Key_ApplicationRight},
		{XKB_KEY_XF86Book,                Key_Book},
		{XKB_KEY_XF86CD,                  Key_CD},
		{XKB_KEY_XF86Calculater,          Key_Calculator},
		{XKB_KEY_XF86Clear,               Key_Clear},
		{XKB_KEY_XF86ClearGrab,           Key_ClearGrab},
		{XKB_KEY_XF86Close,               Key_Close},
		{XKB_KEY_XF86Copy,                Key_Copy},
		{XKB_KEY_XF86Cut,                 Key_Cut},
		{XKB_KEY_XF86Display,             Key_Display},
		{XKB_KEY_XF86DOS,                 Key_DOS},
		{XKB_KEY_XF86Documents,           Key_Documents},
		{XKB_KEY_XF86Excel,               Key_Excel},
		{XKB_KEY_XF86Explorer,            Key_Explorer},
		{XKB_KEY_XF86Game,                Key_Game},
		{XKB_KEY_XF86Go,                  Key_Go},
		{XKB_KEY_XF86iTouch,              Key_iTouch},
		{XKB_KEY_XF86LogOff,              Key_LogOff},
		{XKB_KEY_XF86Market,              Key_Market},
		{XKB_KEY_XF86Meeting,             Key_Meeting},
		{XKB_KEY_XF86MenuKB,              Key_MenuKB},
		{XKB_KEY_XF86MenuPB,              Key_MenuPB},
		{XKB_KEY_XF86MySites,             Key_MySites},
		{XKB_KEY_XF86New,                 Key_New},
		{XKB_KEY_XF86News,                Key_News},
		{XKB_KEY_XF86OfficeHome,          Key_OfficeHome},
		{XKB_KEY_XF86Open,                Key_Open},
		{XKB_KEY_XF86Option,              Key_Option},
		{XKB_KEY_XF86Paste,               Key_Paste},
		{XKB_KEY_XF86Phone,               Key_Phone},
		{XKB_KEY_XF86Reply,               Key_Reply},
		{XKB_KEY_XF86Reload,              Key_Reload},
		{XKB_KEY_XF86RotateWindows,       Key_RotateWindows},
		{XKB_KEY_XF86RotationPB,          Key_RotationPB},
		{XKB_KEY_XF86RotationKB,          Key_RotationKB},
		{XKB_KEY_XF86Save,                Key_Save},
		{XKB_KEY_XF86Send,                Key_Send},
		{XKB_KEY_XF86Spell,               Key_Spell},
		{XKB_KEY_XF86SplitScreen,         Key_SplitScreen},
		{XKB_KEY_XF86Support,             Key_Support},
		{XKB_KEY_XF86TaskPane,            Key_TaskPane},
		{XKB_KEY_XF86Terminal,            Key_Terminal},
		{XKB_KEY_XF86Tools,               Key_Tools},
		{XKB_KEY_XF86Travel,              Key_Travel},
		{XKB_KEY_XF86Video,               Key_Video},
		{XKB_KEY_XF86Word,                Key_Word},
		{XKB_KEY_XF86Xfer,                Key_Xfer},
		{XKB_KEY_XF86ZoomIn,              Key_ZoomIn},
		{XKB_KEY_XF86ZoomOut,             Key_ZoomOut},
		{XKB_KEY_XF86Away,                Key_Away},
		{XKB_KEY_XF86Messenger,           Key_Messenger},
		{XKB_KEY_XF86WebCam,              Key_WebCam},
		{XKB_KEY_XF86MailForward,         Key_MailForward},
		{XKB_KEY_XF86Pictures,            Key_Pictures},
		{XKB_KEY_XF86Music,               Key_Music},
		{XKB_KEY_XF86Battery,             Key_Battery},
		{XKB_KEY_XF86Bluetooth,           Key_Bluetooth},
		{XKB_KEY_XF86WLAN,                Key_WLAN},
		{XKB_KEY_XF86UWB,                 Key_UWB},
		{XKB_KEY_XF86AudioForward,        Key_AudioForward},
		{XKB_KEY_XF86AudioRepeat,         Key_AudioRepeat},
		{XKB_KEY_XF86AudioRandomPlay,     Key_AudioRandomPlay},
		{XKB_KEY_XF86Subtitle,            Key_Subtitle},
		{XKB_KEY_XF86AudioCycleTrack,     Key_AudioCycleTrack},
		{XKB_KEY_XF86Time,                Key_Time},
		{XKB_KEY_XF86Select,              Key_Select},
		{XKB_KEY_XF86View,                Key_View},
		{XKB_KEY_XF86TopMenu,             Key_TopMenu},
		{XKB_KEY_XF86Red,                 Key_Red},
		{XKB_KEY_XF86Green,               Key_Green},
		{XKB_KEY_XF86Yellow,              Key_Yellow},
		{XKB_KEY_XF86Blue,                Key_Blue},
		{XKB_KEY_XF86Bluetooth,           Key_Bluetooth},
		{XKB_KEY_XF86Suspend,             Key_Suspend},
		{XKB_KEY_XF86Hibernate,           Key_Hibernate},
		{XKB_KEY_XF86TouchpadToggle,      Key_TouchpadToggle},
		{XKB_KEY_XF86TouchpadOn,          Key_TouchpadOn},
		{XKB_KEY_XF86TouchpadOff,         Key_TouchpadOff},
		{XKB_KEY_XF86AudioMicMute,        Key_MicMute},
		{XKB_KEY_XF86Launch0,             Key_Launch2}, // ### Qt 6: remap properly
		{XKB_KEY_XF86Launch1,             Key_Launch3},
		{XKB_KEY_XF86Launch2,             Key_Launch4},
		{XKB_KEY_XF86Launch3,             Key_Launch5},
		{XKB_KEY_XF86Launch4,             Key_Launch6},
		{XKB_KEY_XF86Launch5,             Key_Launch7},
		{XKB_KEY_XF86Launch6,             Key_Launch8},
		{XKB_KEY_XF86Launch7,             Key_Launch9},
		{XKB_KEY_XF86Launch8,             Key_LaunchA},
		{XKB_KEY_XF86Launch9,             Key_LaunchB},
		{XKB_KEY_XF86LaunchA,             Key_LaunchC},
		{XKB_KEY_XF86LaunchB,             Key_LaunchD},
		{XKB_KEY_XF86LaunchC,             Key_LaunchE},
		{XKB_KEY_XF86LaunchD,             Key_LaunchF},
		{XKB_KEY_XF86LaunchE,             Key_LaunchG},
		{XKB_KEY_XF86LaunchF,             Key_LaunchH}
	};

	return keyTbl;
}

}
