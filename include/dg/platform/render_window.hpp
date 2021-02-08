#pragma once

#include <memory>

#include <dg/input/shortcuts.hpp>

#include <dg/input/mouse_events.hpp>
#include <dg/platform/window_events.hpp>
#include <dg/platform/cursor.hpp>

class ImFont;

namespace Diligent {
class IRenderDevice;
class IDeviceContext;
class ISwapChain;
class IEngineFactory;
}

namespace dg {

using namespace Diligent;

class RenderWindowPrivate;
class RenderWindow;

class RenderWindowListener
{
public:
	virtual ~RenderWindowListener() = default;

	virtual void initialize() = 0;
	virtual void render() = 0;
	virtual bool present() { return false; }

	// events
	virtual void resizeEvent(const ResizeEvent& event) {}
	virtual void focusOutEvent() {}

	virtual void keyPressEvent(const KeyEvent& event) {}
	virtual void keyReleaseEvent(const KeyEvent& event) {}

	virtual void mouseMoveEvent(const MouseEvent& event) {}
	virtual void mousePressEvent(const MouseEvent& event) {}
	virtual void mouseReleaseEvent(const MouseEvent& event) {}
	virtual void wheelEvent(const WheelEvent& event) {}


};


struct Icon
{
	Icon() = default;
	Icon(int width, int height, const std::uint32_t* rgba_pixmap) 
		: width(width), height(height), rgba_pixmap(rgba_pixmap) {}

	int width = 0;
	int height = 0; 
	const std::uint32_t* rgba_pixmap = nullptr;
};
struct RenderWindowCreationOptions
{
	int posx = 0, posy = 0;
	int width = 1024, height = 768;
	int multi_sampling = 0;
	
	std::string window_title;
	Icon icon;
};

class RenderWindow
{
public:
	using CreationOptions = RenderWindowCreationOptions;

public:

	RenderWindow(RenderWindowListener* listener);
	virtual ~RenderWindow();


public:

	void setListener(RenderWindowListener* listener);

	virtual void create(const CreationOptions& options = CreationOptions()) = 0;

public:

	IRenderDevice* device();
	IDeviceContext* context();
	ISwapChain* swapChain();
	IEngineFactory* engineFactory();

public:

	virtual bool spinOnce() = 0;

	void spin();

public:

	virtual void setWindowTitle(const std::string& title) = 0;
	virtual void showFullscreen(bool fullscreen) = 0;
	virtual bool isFullscreen() const = 0;

	virtual void setCursor(Cursor cursor) = 0;
	virtual void setWindowIcon(const Icon& icon) = 0;

public:

	void addShortcut(const KeySequence& key_seq, std::function<void()> on_activate, const std::string& info_text = std::string());
	void addShortcut(const std::string& key_seq, std::function<void()> on_activate, const std::string& info_text = std::string());

public:

	/// Adds the font given as ttf file with font size.
	ImFont* addFont(const std::string& font_filename, int font_size);
	ImFont* addFontCompressedTTF(const void* compressed_ttf_data, int compressed_ttf_size, int font_size);

	void setDefaultFont(ImFont* font);

protected:

	virtual void initialize();
	virtual void render();

	virtual void resizeEvent(const ResizeEvent& event);
	virtual void focusOutEvent();

	virtual void keyPressEvent(const KeyEvent& event);
	virtual void keyReleaseEvent(const KeyEvent& event);

	virtual void mouseMoveEvent(const MouseEvent& event);
	virtual void mousePressEvent(const MouseEvent& event);
	virtual void mouseReleaseEvent(const MouseEvent& event);
	virtual void wheelEvent(const WheelEvent& event);


protected:

	std::unique_ptr<RenderWindowPrivate> d;
	Shortcuts shortcuts_;

private:

	RenderWindowListener* listener_;

};

class RenderWindowFactory
{
public:

	virtual ~RenderWindowFactory() = default;

public:

	virtual RenderWindow* createRenderWindow(RenderWindowListener* listener) = 0;


	static RenderWindowFactory* getFactory(const std::string& renderBackend);

protected:

	static void registerFactory(const std::string& renderBackend, RenderWindowFactory* factory);
	static void unregisterFactory(RenderWindowFactory* factory);

};

}

