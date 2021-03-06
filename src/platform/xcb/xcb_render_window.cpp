#include "xcb_render_window.hpp"
#include "../render_window_private.hpp"

#include <X11/cursorfont.h>
#include <X11/Xlib.h>


namespace dg {

static const char *g_xcb_atomnames[] =
{
    "WM_PROTOCOLS",
    "WM_DELETE_WINDOW",
    "_NET_WM_STATE",
    "_NET_WM_STATE_FULLSCREEN",
    "_NET_WM_ICON",

    "CARDINAL"
};

static MouseEvent::Buttons translateMouseButton(int s)
{
    switch(s)
    {
    case 1: return MouseEvent::LeftButton;
    case 2: return MouseEvent::MiddleButton;
    case 3: return MouseEvent::RightButton;
    case 8: return MouseEvent::BackButton;
    case 9: return MouseEvent::ForwardButton;
    default:
        return MouseEvent::NoButton;
    }
}

static xcb_cursor_t createCursor(::Display *dpy, int cshape)
{
    xcb_cursor_t cursor = XCB_NONE;

    unsigned int shape;
    switch((Cursor)cshape)
    {
        case Cursor::Arrow: shape=XC_left_ptr; break;
        case Cursor::Wait: shape=XC_watch; break;
        case Cursor::IBeam: shape=XC_xterm; break;
        case Cursor::SizeVer: shape=XC_sb_v_double_arrow; break;
        case Cursor::SizeHor: shape=XC_sb_h_double_arrow; break;
        case Cursor::SizeBDiag: shape=XC_bottom_left_corner; break;
        case Cursor::SizeFDiag: shape=XC_bottom_right_corner; break;
        case Cursor::SizeAll: shape=XC_fleur; break;
        case Cursor::Forbidden: shape=XC_circle; break;
        case Cursor::Busy: shape=XC_watch; break;
        default: return 0;
    }

    cursor = XCreateFontCursor(dpy, shape);

    return cursor;
}

XCBRenderWindow::~XCBRenderWindow()
{
    for(auto& p : cursors_)
        xcb_free_cursor(connection_, p.second);
}

void XCBRenderWindow::createWindow(xcb_connection_t* connection, xcb_window_t screen_root,  int visual, const CreationOptions& options, xcb_colormap_t colormap)
{

    XCBKeyboard::initializeXKB(connection);

    int scr         = 0;
    const xcb_setup_t*    setup = xcb_get_setup(connection);
    xcb_screen_iterator_t iter  = xcb_setup_roots_iterator(setup);
    while (scr-- > 0)
        xcb_screen_next(&iter);

    screen_ = iter.data;

    display_ = XOpenDisplay(0);
    connection_ = connection;
    screen_root_ = screen_root;
    width_ = options.width;
    height_ = options.height;

    window_ = xcb_generate_id(connection_);

    // create window
    uint32_t eventmask = XCB_EVENT_MASK_EXPOSURE |
                         XCB_EVENT_MASK_PROPERTY_CHANGE |
                         XCB_EVENT_MASK_STRUCTURE_NOTIFY |
                         XCB_EVENT_MASK_FOCUS_CHANGE |

                         XCB_EVENT_MASK_KEY_PRESS |
                         XCB_EVENT_MASK_KEY_RELEASE |
                         XCB_EVENT_MASK_POINTER_MOTION |
                         XCB_EVENT_MASK_BUTTON_PRESS |
                         XCB_EVENT_MASK_BUTTON_RELEASE;

    uint32_t valuelist[] = { eventmask, colormap, 0 };
    uint32_t valuemask = XCB_CW_EVENT_MASK;

    if(colormap!=0)
        valuemask |= XCB_CW_COLORMAP;

    //value_mask    = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
    //value_list[0] = screen->black_pixel;
    //value_list[1] = XCB_EVENT_MASK_KEY_RELEASE | XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_STRUCTURE_NOTIFY;

    xcb_create_window(
          connection_,
          XCB_COPY_FROM_PARENT,
          window_,
          screen_root_,
          options.posx, options.posy,
          width_, height_,
          0,
          XCB_WINDOW_CLASS_INPUT_OUTPUT,
          visual,
          valuemask,
          valuelist
          );

    xcb_map_window(connection_, window_);

    initializeAtoms(connection);

    xcb_atom_t properties[5];
    int property_count = 0;
    properties[property_count++] = atom(WM_DELETE_WINDOW);

    xcb_change_property(connection,
                        XCB_PROP_MODE_REPLACE,
                        window_,
                        atom(WM_PROTOCOLS),
                        XCB_ATOM_ATOM,
                        32,
                        property_count,
                        properties);

    if(options.icon.rgba_pixmap)
        setWindowIcon(options.icon);

    if(!options.window_title.empty())
        setWindowTitle(options.window_title);


    // on some systems the position given in xcb_create_window is not respected, hence, move the window here explicitly
    const static int coords[] = { options.posx, options.posy };
    xcb_configure_window (connection, window_, XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y, (const uint32_t*) coords);

    keyboard_.reset(new XCBKeyboard(connection_));

    for(int i=0; i<=(int)Cursor::Busy; ++i)
    {
        xcb_cursor_t c = createCursor(display_, i);
        cursors_[i] = c;
    }

    xcb_flush(connection);

    xcb_generic_event_t* e;
    while ((e = xcb_wait_for_event(connection)))
    {
        if ((e->response_type & ~0x80) == XCB_EXPOSE) break;
    }

}

/*
void DestroyXCBConnectionAndWindow(XCBInfo& info)
{
    xcb_destroy_window(info.connection, info.window);
    xcb_disconnect(info.connection);
}
 *
 */

void XCBRenderWindow::initializeAtoms(xcb_connection_t *connection)
{
    xcb_intern_atom_cookie_t cookies[NUM_ATOMS];

    for (int i = 0; i < NUM_ATOMS; ++i)
        cookies[i] = xcb_intern_atom(connection, false, strlen(g_xcb_atomnames[i]), g_xcb_atomnames[i]);

    for (int i = 0; i < NUM_ATOMS; ++i)
    {
        xcb_intern_atom_reply_t* reply = xcb_intern_atom_reply(connection, cookies[i], nullptr);

        if(!reply)
            DG_THROW("Could not get atom");

        atoms_[i] = reply->atom;
        free(reply);
    }
}


void XCBRenderWindow::setCursor(Cursor cursor)
{
    if(cursor == current_cursor_)
        return;

    xcb_cursor_t c = XCB_NONE;
    auto it = cursors_.find((int)cursor);
    if(it==cursors_.end())
    {
        std::cout << "Cursor not found " << std::endl;
        return;
    }
    else
        c = it->second;

    xcb_change_window_attributes(connection_, window_, XCB_CW_CURSOR, &c);
    xcb_flush(connection_);

    current_cursor_ = cursor;
}

void XCBRenderWindow::setWindowIcon(const Icon& icon) 
{
    if(icon.rgba_pixmap != nullptr)
    {
        std::vector<std::uint32_t> icon_data;
        icon_data.resize(icon.width*icon.height+2);

        std::cout << "ICON " << icon.width << std::endl;

        icon_data[0] = icon.width;
        icon_data[1] = icon.height;

        for(int i=0; i<icon.width*icon.height; ++i)
        {
            int r = (icon.rgba_pixmap[i] >>  0) & 0xFF;
            int g = (icon.rgba_pixmap[i] >>  8) & 0xFF;
            int b = (icon.rgba_pixmap[i] >> 16) & 0xFF;
            int a = (icon.rgba_pixmap[i] >> 24) & 0xFF;
            icon_data[i+2] = (b<<0) | (g<<8) | (r<<16) | (a<<24);
        }


        xcb_change_property(connection_,
            XCB_PROP_MODE_REPLACE,
            window_,
            atom(_NET_WM_ICON),
            atom(CARDINAL),
            32,
            icon_data.size(),
            (unsigned char *) icon_data.data());
    }
    else // remove any custom icon
    {
        xcb_delete_property(connection_,
            window_,
            atom(_NET_WM_ICON));
    }


}

bool XCBRenderWindow::spinOnce()
{
    if(destroyed_)
        return false;


    xcb_generic_event_t* event;
    while ((event = xcb_poll_for_event(connection_)) != nullptr)
    {

        switch(event->response_type & ~0x80)
        {
            case XCB_CLIENT_MESSAGE:
            {
                const xcb_client_message_event_t* e = reinterpret_cast<const xcb_client_message_event_t*>(event);
                xcb_atom_t protocol_atom = e->data.data32[0];
                if (protocol_atom == atom(WM_DELETE_WINDOW))
                {
                     destroyed_ = true;
                    return false;
                }
                break;
            }

            case XCB_DESTROY_NOTIFY:
            {
                destroyed_ = true;
                return false;
            }

            case XCB_EXPOSE:
                //draw();
                //this->render();
                break;

            case XCB_CONFIGURE_NOTIFY:
            {
                const auto* cfg_event = reinterpret_cast<const xcb_configure_notify_event_t*>(event);

                if ((cfg_event->width != width_) || (cfg_event->height != height_))
                {
                    width_  = cfg_event->width;
                    height_ = cfg_event->height;
                    if (width_ > 0 && height_ > 0)
                    {
                        d->swap_chain->Resize( cfg_event->width,  cfg_event->height);
                        resizeEvent(ResizeEvent{cfg_event->width,  cfg_event->height});
                    }
                }
                break;
            }

            case XCB_KEY_PRESS:
            {
                const xcb_key_press_event_t* key_event = reinterpret_cast<const xcb_key_press_event_t*>(event);
                std::string text;
                std::uint32_t unicode;
                int key = keyboard_->keycodeToKey(key_event->detail, key_event->state, &text, &unicode);
                keyPressEvent(dg::KeyEvent{key,text, unicode});
                break;
            }

            case XCB_KEY_RELEASE:
            {
                const xcb_key_press_event_t* key_event = reinterpret_cast<const xcb_key_release_event_t*>(event);
                std::string text;
                std::uint32_t unicode;
                int key = keyboard_->keycodeToKey(key_event->detail, key_event->state, &text, &unicode);
                keyReleaseEvent(dg::KeyEvent{key,text, unicode});
                break;
            }

            case XCB_FOCUS_OUT:
            {
                focusOutEvent();
                break;
            }

            case XCB_MOTION_NOTIFY:
            {
                xcb_motion_notify_event_t* motion = (xcb_motion_notify_event_t*)event;
                MouseEvent ev;
                ev.x = motion->event_x;
                ev.y = motion->event_y;
                mouseMoveEvent(ev);
                // TODO: return if event was accepted
                break;
            }

            case XCB_BUTTON_PRESS:
            {
                xcb_button_press_event_t* press = (xcb_button_press_event_t*)event;

                if(press->detail >= 4 && press->detail <= 7)
                {
                    WheelEvent ev;
                    ev.delta_horizontal = 0;
                    ev.delta_vertical   = 0;
                    switch(press->detail)
                    {
                    case 4: ev.delta_vertical   =  1; break;
                    case 5: ev.delta_vertical   = -1; break;
                    case 6: ev.delta_horizontal =  1; break;
                    case 7: ev.delta_horizontal = -1; break;
                    }
                    wheelEvent(ev);
                }
                else
                {
                    MouseEvent ev;
                    int button = translateMouseButton(press->detail);
                    mouseButtonState_ |= button;
                    ev.buttons = mouseButtonState_;
                    mousePressEvent(ev);
                }

                // TODO: return if event was accepted

                break;
            }

            case XCB_BUTTON_RELEASE:
            {
                xcb_button_release_event_t* press = (xcb_button_release_event_t*)event;

                if(press->detail >= 4 && press->detail <= 7)
                {
                    // wheel -> do nothing
                }
                else
                {
                    int button = translateMouseButton(press->detail);
                    mouseButtonState_ &= ~button;
                    MouseEvent ev;
                    ev.buttons = mouseButtonState_;
                    mouseReleaseEvent(ev);
                }
                // TODO: return if event was accepted
                break;
            }

            default:
                break;
        }

        free(event);
    }


    render();

    return true;

}

void XCBRenderWindow::setNetWmState(bool set, xcb_atom_t one, xcb_atom_t two)
{
    xcb_client_message_event_t ev;
    ev.response_type = XCB_CLIENT_MESSAGE;
    ev.format = 32;
    ev.type = atom(_NET_WM_STATE);
    ev.window = window_;
    ev.data.data32[0] = set ? 1 : 0;
    ev.data.data32[1] = one;
    ev.data.data32[2] = two;
    ev.data.data32[3] = 0;
    ev.data.data32[4] = 0;

    xcb_send_event(
            connection_,
            0,
            screen_root_,
            XCB_EVENT_MASK_STRUCTURE_NOTIFY | XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT,
            (const char*)(&ev));
}

bool XCBRenderWindow::getNetWmState(xcb_atom_t one) const
{
    xcb_get_property_cookie_t cookie = xcb_get_property_unchecked(connection_, 0, window_, atom(_NET_WM_STATE), XCB_ATOM_ATOM, 0, 1);

    xcb_get_property_reply_t *prop_reply = xcb_get_property_reply(connection_, cookie, nullptr);

    if (prop_reply) {
        if (prop_reply->format == 32) {
            xcb_atom_t *v = (xcb_atom_t*)xcb_get_property_value(prop_reply);
            for (unsigned int i = 0; i < prop_reply->value_len; i++) {
                if (v[i] == one)
                    return true;
            }
        }
        free(prop_reply);
    }

    return false;
}

void XCBRenderWindow::setWindowTitle(const std::string& title)
{
    xcb_change_property(connection_, XCB_PROP_MODE_REPLACE, window_, XCB_ATOM_WM_NAME, XCB_ATOM_STRING,
                        8, title.length(), title.c_str());
    xcb_flush(connection_);
}

void XCBRenderWindow::showFullscreen(bool fullscreen)
{
    setNetWmState(fullscreen, atom(_NET_WM_STATE_FULLSCREEN));
}

bool XCBRenderWindow::isFullscreen() const
{
    return getNetWmState(atom(_NET_WM_STATE_FULLSCREEN));
}

}
