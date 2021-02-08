#pragma once

#include <memory>
#include <unordered_map>

#include <dg/platform/render_window.hpp>
#include "xcb_keyboard.hpp"

#include <xcb/xcb.h>


class _XDisplay;
typedef struct _XDisplay Display;

namespace dg {

class XCBRenderWindow : public RenderWindow
{

public:

    using RenderWindow::RenderWindow;
    ~XCBRenderWindow();

    virtual bool spinOnce() override;

    virtual void setWindowTitle(const std::string& title) override;
    virtual void showFullscreen(bool fullscreen) override;
    virtual bool isFullscreen() const override;

    virtual void setCursor(Cursor cursor) override;
    virtual void setWindowIcon(const Icon& icon) override;

protected:

    void createWindow(xcb_connection_t* connection, xcb_window_t screen_root,  int visual, const CreationOptions& options, xcb_colormap_t colormap = 0);

protected:

    enum Atom
    {
        WM_PROTOCOLS,
        WM_DELETE_WINDOW,
        _NET_WM_STATE,
        _NET_WM_STATE_FULLSCREEN,
        _NET_WM_ICON,

        CARDINAL,

        NUM_ATOMS
    };

    void initializeAtoms(xcb_connection_t *connection);

    inline xcb_atom_t atom(Atom atom) const { return atoms_[atom]; }

    void setNetWmState(bool set, xcb_atom_t one, xcb_atom_t two = XCB_ATOM_NONE);
    bool getNetWmState(xcb_atom_t one) const;

protected:

    Display* display_;
    xcb_screen_t* screen_ = nullptr;
    xcb_connection_t* connection_ = nullptr;
    xcb_window_t window_ = 0;
    xcb_window_t screen_root_ = 0;

    int width_ = 0, height_ = 0;

    std::unique_ptr<XCBKeyboard> keyboard_;
    int mouseButtonState_ = 0;

    xcb_atom_t atoms_[NUM_ATOMS];

    bool destroyed_ = false;

private:

    std::unordered_map<int,xcb_cursor_t> cursors_;
    Cursor current_cursor_ = Cursor::Arrow;

};


}
