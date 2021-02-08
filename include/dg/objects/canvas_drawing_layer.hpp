#pragma once

#include "canvas_object.hpp"

#include <dg/material/color.hpp>
#include <dg/core/method_string_interface.hpp>

#include <imgui/imgui.h>


namespace dg {


class CanvasDrawingLayer : public CanvasLayer
{
public:
	CanvasDrawingLayer(CanvasObject* parent_object);

	virtual void render() override;
	virtual void setOpacity(float opacity) override;


	virtual Eigen::Vector2f getSize() const override { return size_; }
	void setSize(const Eigen::Vector2f& size) { size_ = size; }
    
    virtual void clear() override;

	virtual bool isBackground() const override { return is_background_; }
	void setIsBackground(bool is_background) { is_background_ = is_background; }

public:

    void addLine(const Eigen::Vector2f& p1, const Eigen::Vector2f& p2, dg::Color color, float thickness = 1.0f);
    /// a: upper-left, b: lower-right (== upper-left + size), rounding_corners_flags: 4 bits corresponding to which corner to round
    void addRect(const Eigen::Vector2f& p_min, const Eigen::Vector2f& p_max, dg::Color color, float rounding = 0.0f, ImDrawCornerFlags rounding_corners = ImDrawCornerFlags_All, float thickness = 1.0f);
    // a: upper-left, b: lower-right (== upper-left + size)
    void addRectFilled(const Eigen::Vector2f& p_min, const Eigen::Vector2f& p_max, dg::Color color, float rounding = 0.0f, ImDrawCornerFlags rounding_corners = ImDrawCornerFlags_All);
    void addRectFilledMultiColor(const Eigen::Vector2f& p_min, const Eigen::Vector2f& p_max, dg::Color color_upr_left, dg::Color color_upr_right, dg::Color color_bot_right, dg::Color color_bot_left);
    void addQuad(const Eigen::Vector2f& p1, const Eigen::Vector2f& p2, const Eigen::Vector2f& p3, const Eigen::Vector2f& p4, dg::Color color, float thickness = 1.0f);
    void addQuadFilled(const Eigen::Vector2f& p1, const Eigen::Vector2f& p2, const Eigen::Vector2f& p3, const Eigen::Vector2f& p4, dg::Color color);
    void addTriangle(const Eigen::Vector2f& p1, const Eigen::Vector2f& p2, const Eigen::Vector2f& p3, dg::Color color, float thickness = 1.0f);
    void addTriangleFilled(const Eigen::Vector2f& p1, const Eigen::Vector2f& p2, const Eigen::Vector2f& p3, dg::Color color);
    void addCircle(float x, float y, float radius, dg::Color color, int num_segments = 12, float thickness = 1.0f);
    void addCircleFilled(float x, float y, float radius, dg::Color color, int num_segments = 12);
    void addText(float x, float y, dg::Color color, const std::string& text);
    void addText(float x, float y, dg::Color color, const std::string& text, float font_size);
    void addText(float x, float y, dg::Color color, const std::string& text, float font_size, const ImFont* font, float wrap_width = 0.0f);

    void addPolyline(const Eigen::Vector2f* points, int num_points, dg::Color color, bool closed, float thickness);
    void addPolyline(const ImVec2* points, const int num_points, ImU32 col, bool closed, float thickness);
    void addPolyFilled(const Eigen::Vector2f* points, int num_points, dg::Color color); // Note: Anti-aliased filling requires points to be in clockwise order.
    void addPolyFilled(const ImVector<ImVec2>& poly, ImU32 col);
    void addBezierCurve(const Eigen::Vector2f& p1, const Eigen::Vector2f& p2, const Eigen::Vector2f& p3, const Eigen::Vector2f& p4, dg::Color color, float thickness, int num_segments = 0);

    void addImage(ImTextureID user_texture_id, const Eigen::Vector2f& p_min, const Eigen::Vector2f& p_max, const Eigen::Vector2f& uv_min = Eigen::Vector2f(0, 0), const Eigen::Vector2f& uv_max = Eigen::Vector2f(1, 1), dg::Color color = dg::colors::White);
    void addImage(ImTextureID user_texture_id, const Eigen::Vector2f& p_min, const Eigen::Vector2f& p_max, dg::Color color);
    //void addImageQuad(ImTextureID user_texture_id, const Eigen::Vector2f& p1, const Eigen::Vector2f& p2, const Eigen::Vector2f& p3, const Eigen::Vector2f& p4, const Eigen::Vector2f& uv1 = Eigen::Vector2f(0, 0), const Eigen::Vector2f& uv2 = Eigen::Vector2f(1, 0), const Eigen::Vector2f& uv3 = Eigen::Vector2f(1, 1), const Eigen::Vector2f& uv4 = Eigen::Vector2f(0, 1), dg::Color color = IM_COL32_WHITE);
    //void addImageRounded(ImTextureID user_texture_id, const Eigen::Vector2f& p_min, const Eigen::Vector2f& p_max, const Eigen::Vector2f& uv_min, const Eigen::Vector2f& uv_max, dg::Color color, float rounding, ImDrawCornerFlags rounding_corners = ImDrawCornerFlags_All);

    // Stateful path API, add points then finish with pathFill() or pathStroke()
    void pathClear();
    void pathLineTo(const Eigen::Vector2f& pos);
    void pathLineTo(float x, float y);
    void pathFill(dg::Color color, bool clear_path = true);
    void pathStroke(dg::Color color, bool closed, float thickness = 1.0f, bool clear_path = true);
    void pathArcTo(const Eigen::Vector2f& center, float radius, float a_min, float a_max, int num_segments = 10);
    //void pathArcToFast(const Eigen::Vector2f& center, float radius, int a_min_of_12, int a_max_of_12);
    void pathBezierCurveTo(const Eigen::Vector2f& p2, const Eigen::Vector2f& p3, const Eigen::Vector2f& p4, int num_segments = 0);
    //void pathRect(const Eigen::Vector2f& rect_min, const Eigen::Vector2f& rect_max, float rounding = 0.0f, ImDrawCornerFlags rounding_corners = ImDrawCornerFlags_All);


    std::tuple<ImDrawIdx*, ImDrawVert*, unsigned int> addRawTriangleList(int idx_count, int vtx_count);


    void callCommand(const std::string& command);


private:

    Eigen::Vector2f size_ = Eigen::Vector2f(1.0f,1.0f);
    bool is_background_ = false;

	ImDrawList* draw_list_;
	MethodStringInterface method_ifc_;
	float opacity_ = 1.0f;

};


inline ImVec2 to_imgui(const Eigen::Vector2f& x)
{
	return ImVec2(x(0), x(1));
}

inline ImU32 to_imgui(const dg::Color& col)
{
	return ImGui::ColorConvertFloat4ToU32(ImVec4(col.r, col.g, col.b, col.a));
}

}
