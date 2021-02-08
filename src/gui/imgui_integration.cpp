#include <dg/gui/imgui_integration.hpp>

#include <unordered_map>

#include <imgui/imgui.h>

namespace dg {

static const std::unordered_map<int, ImGuiKey>& keymapping();


ImGuiIntegration::ImGuiIntegration(IRenderDevice*    device,
                                   TEXTURE_FORMAT    backBufferFmt,
                                   TEXTURE_FORMAT    depthBufferFmt,
                                   std::uint32_t     displayWidht,
			                       std::uint32_t     displayHeight,
			                       std::uint32_t     initialVertexBufferSize,
			                       std::uint32_t     initialIndexBufferSize) :
    ImGuiImplDiligent(device, backBufferFmt, depthBufferFmt, initialVertexBufferSize, initialIndexBufferSize)
{
	ImGuiIO& io       = ImGui::GetIO();
    io.DisplaySize = ImVec2(displayWidht, displayHeight);

    io.BackendPlatformName = "dg::ImGuiIntegration";


    for (auto& p : keymapping())
        io.KeyMap[p.second] = p.second;


    last_timestamp_ = std::chrono::high_resolution_clock::now();
}


void ImGuiIntegration::NewFrame()
{
    auto now        = std::chrono::high_resolution_clock::now();
    auto elapsed_ns = now - last_timestamp_;
    last_timestamp_   = now;
    ImGuiIO& io        = ImGui::GetIO();
    io.DeltaTime    = static_cast<float>(elapsed_ns.count() / 1e+9);
    ImGuiImplDiligent::NewFrame();
}


void ImGuiIntegration::keyPressEvent(const KeyEvent& event)
{
    ImGuiIO& io = ImGui::GetIO();
    const std::unordered_map<int, ImGuiKey>& keymap = keymapping();

    auto it = keymap.find(event.key);
    if (it != keymap.end())
        io.KeysDown[it->second] = true;

    if(event.unicode > 0)
    	io.AddInputCharacter(event.unicode);
}

void ImGuiIntegration::keyReleaseEvent(const KeyEvent& event)
{
    ImGuiIO& io = ImGui::GetIO();
    const std::unordered_map<int, ImGuiKey>& keymap = keymapping();

    auto it = keymap.find(event.key);
    if (it != keymap.end())
        io.KeysDown[it->second] = false;
}


void ImGuiIntegration::mouseMoveEvent(const MouseEvent& event)
{
	ImGuiIO& io = ImGui::GetIO();
	io.MousePos = ImVec2(event.x, event.y);
}

void ImGuiIntegration::mousePressEvent(const MouseEvent& event)
{
	ImGuiIO& io     = ImGui::GetIO();
	io.MouseDown[0] = event.buttons & MouseEvent::LeftButton;
	io.MouseDown[1] = event.buttons & MouseEvent::RightButton;
	io.MouseDown[2] = event.buttons & MouseEvent::MiddleButton;
}

void ImGuiIntegration::mouseReleaseEvent(const MouseEvent& event)
{
	ImGuiIO& io     = ImGui::GetIO();
	io.MouseDown[0] = event.buttons & MouseEvent::LeftButton;
	io.MouseDown[1] = event.buttons & MouseEvent::RightButton;
	io.MouseDown[2] = event.buttons & MouseEvent::MiddleButton;
}

void ImGuiIntegration::wheelEvent(const WheelEvent& event)
{
	ImGuiIO& io     = ImGui::GetIO();
	io.MouseWheel  += event.delta_vertical;
	io.MouseWheelH += event.delta_horizontal;
}


static const std::unordered_map<int, ImGuiKey>& keymapping()
{
	static std::unordered_map<int, ImGuiKey> keymap = {
	    {Key_Tab,            ImGuiKey_Tab            },
	    {Key_Left,           ImGuiKey_LeftArrow      },
	    {Key_Right,          ImGuiKey_RightArrow     },
	    {Key_Up,             ImGuiKey_UpArrow        },
	    {Key_Down,           ImGuiKey_DownArrow      },
	    {Key_PageUp,         ImGuiKey_PageUp         },
	    {Key_PageDown,       ImGuiKey_PageDown       },
	    {Key_Home,           ImGuiKey_Home           },
	    {Key_End,            ImGuiKey_End            },
	    {Key_Insert,         ImGuiKey_Insert         },
	    {Key_Delete,         ImGuiKey_Delete         },
	    {Key_Backspace,      ImGuiKey_Backspace      },
	    {Key_Space,          ImGuiKey_Space          },
	    {Key_Return,         ImGuiKey_Enter          },
	    {Key_Escape,         ImGuiKey_Escape         },
	    {Key_Enter,          ImGuiKey_KeyPadEnter    },
	    {Key_A,              ImGuiKey_A              },
	    {Key_C,              ImGuiKey_C              },
	    {Key_V,              ImGuiKey_V              },
	    {Key_X,              ImGuiKey_X              },
	    {Key_Y,              ImGuiKey_Y              },
	    {Key_Z,              ImGuiKey_Z              },
	};

	return keymap;
}



}
