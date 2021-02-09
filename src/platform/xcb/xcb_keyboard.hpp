#pragma once

#include <memory>

#include <xkbcommon/xkbcommon.h>
#include <xcb/xcb.h>

#include <dg/core/common.hpp>
#include <dg/input/keys.hpp>


namespace dg {

class XCBKeyboard
{

    struct XKBStateDeleter {
        void operator()(struct xkb_state *state) const { return xkb_state_unref(state); }
    };
    using XKBStatePtr = std::unique_ptr<struct xkb_state, XKBStateDeleter>;

    struct XKBKeymapDeleter {
        void operator()(struct xkb_keymap *keymap) const { return xkb_keymap_unref(keymap); }
    };
    using XKBKeymapPtr = std::unique_ptr<struct xkb_keymap, XKBKeymapDeleter>;

    struct XKBContextDeleter {
        void operator()(struct xkb_context *context) const { return xkb_context_unref(context); }
    };
    using XKBContextPtr = std::unique_ptr<struct xkb_context, XKBContextDeleter>;

public:

    XCBKeyboard(xcb_connection_t* c);


    void updateKeymap();

    int keycodeToKey(xcb_keycode_t code, std::uint16_t state, std::string* text = nullptr, std::uint32_t* unicode = nullptr);


    static void initializeXKB(xcb_connection_t* connection);


private:

    void updateXKBMods();
    xkb_mod_mask_t xkbModMask(std::uint16_t state);
    int keysymToKey(xkb_keysym_t keysym);
    ModifierKey stateToModifier(std::uint16_t state);




private:

    xcb_connection_t* connection_;
    XKBContextPtr xkb_context_;
    XKBStatePtr xkb_state_;
    XKBKeymapPtr xkb_keymap_;


private:

    struct XkbMods {
        xkb_mod_index_t shift;
        xkb_mod_index_t lock;
        xkb_mod_index_t control;
        xkb_mod_index_t mod1;
        xkb_mod_index_t mod2;
        xkb_mod_index_t mod3;
        xkb_mod_index_t mod4;
        xkb_mod_index_t mod5;
    };
    XkbMods xkb_mods_;



};


}
