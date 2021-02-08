#pragma once

#include <chrono>

#include <dg/input/key_events.hpp>
#include <dg/input/mouse_events.hpp>
#include <dg/platform/window_events.hpp>

#include "ImGuiImplDiligent.hpp"

namespace dg {

using namespace Diligent;

class ImGuiIntegration final : public Diligent::ImGuiImplDiligent
{
public:
    ImGuiIntegration(IRenderDevice*    device,
                     TEXTURE_FORMAT    backBufferFmt,
                     TEXTURE_FORMAT    depthBufferFmt,
                     std::uint32_t     displayWidht,
					 std::uint32_t     displayHeight,
					 std::uint32_t     initialVertexBufferSize = ImGuiImplDiligent::DefaultInitialVBSize,
					 std::uint32_t     initialIndexBufferSize  = ImGuiImplDiligent::DefaultInitialIBSize);

    ImGuiIntegration             (const ImGuiIntegration&)  = delete;
    ImGuiIntegration             (      ImGuiIntegration&&) = delete;
    ImGuiIntegration& operator = (const ImGuiIntegration&)  = delete;
    ImGuiIntegration& operator = (      ImGuiIntegration&&) = delete;

public:

    virtual void NewFrame() override final;

public:

	// events
	virtual void keyPressEvent(const KeyEvent& event);
	virtual void keyReleaseEvent(const KeyEvent& event);

	virtual void mouseMoveEvent(const MouseEvent& event);
	virtual void mousePressEvent(const MouseEvent& event);
	virtual void mouseReleaseEvent(const MouseEvent& event);
	virtual void wheelEvent(const WheelEvent& event);

private:

    std::chrono::time_point<std::chrono::high_resolution_clock> lastTimestamp_;

};

}
