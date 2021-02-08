#include <dg/objects/canvas_drawing_layer.hpp>

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

#include <earcut/earcut.hpp>

#include <DiligentCore/Graphics/GraphicsEngine/interface/DeviceContext.h>

// for earcut algorithm
namespace mapbox { namespace util {

template <>
struct nth<0, ImVec2> {
    inline static float get(const ImVec2 &p) {
        return p.x;
    };
};
template <>
struct nth<1, ImVec2> {
    inline static float get(const ImVec2 &p) {
        return p.y;
    };
};

} }

namespace dg {


CanvasDrawingLayer::CanvasDrawingLayer(CanvasObject* parent_object) : CanvasLayer(parent_object)
{
    draw_list_ = parent_object->createDrawList();
    clear();
}

void CanvasDrawingLayer::render()
{
    getParentObject()->getSceneManager()->context()->SetStencilRef(1);
    getParentObject()->renderDrawList(draw_list_, opacity_);
}

void CanvasDrawingLayer::setOpacity(float opacity)
{
    opacity_ = opacity;
}

void CanvasDrawingLayer::clear()
{
    draw_list_->Clear();
    draw_list_->Flags = ImDrawListFlags_AllowVtxOffset;
    draw_list_->PushTextureID(getParentObject()->getImGuiContext()->Font->ContainerAtlas->TexID);
    //m_draw_list->PushClipRectFullScreen();
    draw_list_->PushClipRect(ImVec2(0,0), ImVec2(10000,10000));

}

void CanvasDrawingLayer::addLine(const Eigen::Vector2f &p1,
        const Eigen::Vector2f &p2, dg::Color color, float thickness)
{
    draw_list_->AddLine(to_imgui(p1), to_imgui(p2), to_imgui(color), thickness);
}

void CanvasDrawingLayer::addRect(const Eigen::Vector2f &p_min,
        const Eigen::Vector2f &p_max, dg::Color color, float rounding,
        ImDrawCornerFlags rounding_corners, float thickness)
{
    draw_list_->AddRect(to_imgui(p_min), to_imgui(p_max), to_imgui(color), rounding, rounding_corners, thickness);
}

void CanvasDrawingLayer::addRectFilled(const Eigen::Vector2f &p_min,
        const Eigen::Vector2f &p_max, dg::Color color, float rounding,
        ImDrawCornerFlags rounding_corners)
{
    draw_list_->AddRectFilled(to_imgui(p_min), to_imgui(p_max), to_imgui(color), rounding, rounding_corners);
}

void CanvasDrawingLayer::addRectFilledMultiColor(const Eigen::Vector2f &p_min,
        const Eigen::Vector2f &p_max, dg::Color color_upr_left,
        dg::Color color_upr_right, dg::Color color_bot_right,
        dg::Color color_bot_left)
{
    draw_list_->AddRectFilledMultiColor(to_imgui(p_min), to_imgui(p_max), to_imgui(color_upr_left), to_imgui(color_upr_right), to_imgui(color_bot_right), to_imgui(color_bot_left));
}

void CanvasDrawingLayer::addQuad(const Eigen::Vector2f &p1,
        const Eigen::Vector2f &p2, const Eigen::Vector2f &p3,
        const Eigen::Vector2f &p4, dg::Color color, float thickness)
{
    draw_list_->AddQuad(to_imgui(p1), to_imgui(p2), to_imgui(p3), to_imgui(p4), to_imgui(color), thickness);
}

void CanvasDrawingLayer::addQuadFilled(const Eigen::Vector2f &p1,
        const Eigen::Vector2f &p2, const Eigen::Vector2f &p3,
        const Eigen::Vector2f &p4, dg::Color color)
{
    draw_list_->AddQuadFilled(to_imgui(p1), to_imgui(p2), to_imgui(p3), to_imgui(p4), to_imgui(color));
}

void CanvasDrawingLayer::addTriangle(const Eigen::Vector2f &p1,
        const Eigen::Vector2f &p2, const Eigen::Vector2f &p3, dg::Color color,
        float thickness)
{
    draw_list_->AddTriangle(to_imgui(p1), to_imgui(p2), to_imgui(p3), to_imgui(color), thickness);
}

void CanvasDrawingLayer::addTriangleFilled(const Eigen::Vector2f &p1,
        const Eigen::Vector2f &p2, const Eigen::Vector2f &p3, dg::Color color)
{
    draw_list_->AddTriangleFilled(to_imgui(p1), to_imgui(p2), to_imgui(p3), to_imgui(color));
}

void CanvasDrawingLayer::addCircle(float x, float y, float radius,
        dg::Color color, int num_segments, float thickness)
{
    draw_list_->AddCircle(ImVec2(x,y), radius, to_imgui(color), num_segments, thickness);
}

void CanvasDrawingLayer::addCircleFilled(float x, float y,
        float radius, dg::Color color, int num_segments)
{
    draw_list_->AddCircleFilled(ImVec2(x,y), radius, to_imgui(color), num_segments);
}

void CanvasDrawingLayer::addText(float x, float y, dg::Color color, const std::string& text)
{
    draw_list_->AddText(ImVec2(x,y), to_imgui(color), text.c_str());
}

void CanvasDrawingLayer::addText(float x, float y, dg::Color color, const std::string& text, float font_size)
{
    addText(x,y,color,text,font_size,nullptr);
}

void CanvasDrawingLayer::addText(float x, float y, dg::Color color, const std::string& text, float font_size, const ImFont* font, float wrap_width)
{
    draw_list_->AddText(font, font_size, ImVec2(x,y), to_imgui(color), text.c_str(), nullptr, wrap_width);
}

void CanvasDrawingLayer::addPolyline(const Eigen::Vector2f *points,
        int num_points, dg::Color color, bool closed, float thickness)
{
    std::vector<ImVec2> im_points(num_points);
    for(int i=0; i<num_points; ++i)
        im_points[i] = to_imgui(points[i]);

    addPolyline(im_points.data(), num_points, to_imgui(color), closed, thickness);
}


inline void scale(ImVec2& v, float s) { v.x *= s; v.y *= s; }
inline void normalize(ImVec2& v) { scale(v, 1.0f / ImSqrt(v.x*v.x+v.y*v.y)); }
inline float dot(const ImVec2& a, const ImVec2& b) { return a.x*b.x + a.y*b.y; }
inline ImVec2 add(const ImVec2& a, const ImVec2& b) { return ImVec2(a.x+b.x, a.y+b.y); }
inline ImVec2 sub(const ImVec2& a, const ImVec2& b) { return ImVec2(a.x-b.x, a.y-b.y); }

void CanvasDrawingLayer::addPolyline(const ImVec2* points, int count, ImU32 col, bool closed, float thickness)
{
    //m_draw_list->AddPolyline(points, num_points, col, closed, thickness);

    if (count < 2)
        return;

    const int idx_count = closed ? count*6 : (count-1)*6;
    const int vtx_count = count*2;
    draw_list_->PrimReserve(idx_count, vtx_count);

    const ImVec2 uv =  draw_list_->_Data->TexUvWhitePixel;

    ImDrawVert* &vtx_write_ptr = draw_list_->_VtxWritePtr;
    ImDrawIdx* &idx_write_ptr = draw_list_->_IdxWritePtr;
    unsigned int &vtx_current_idx = draw_list_->_VtxCurrentIdx;

    for(int i=0; i<count; ++i)
    {
        int i0 = i-1; // index of p0
        int i1 = i;   // index of p1
        int i2 = i+1; // index of p2

        if(closed)
        {
            if(i0<0) i0=count-1;
            if(i2>=count) i2=0;
        }

        const ImVec2& p1 = points[i1];
        ImVec2 n;
        bool swap_indices = false;

        if(i0<0) // p1 is the first point
        {
            const ImVec2& p2 = points[i2];
            n = ImVec2 (p1.y-p2.y, p2.x-p1.x); normalize(n);
            scale(n, thickness * 0.5f);
        }
        else if(i2>=count) // p1 is the last point
        {
            const ImVec2& p0 = points[i0];
            n = ImVec2 (p0.y-p1.y, p1.x-p0.x); normalize(n);
            scale(n, thickness * 0.5f);
        }
        else // some point in between
        {
            const ImVec2& p0 = points[i0];
            const ImVec2& p2 = points[i2];
            //ImVec2 n01(p0.y-p1.y, p1.x-p0.x); normalize(n01);
            //ImVec2 n12(p1.y-p2.y, p2.x-p1.x); normalize(n12);
            //n.x = 0.5f * (n01.x+n12.x);
            //n.y = 0.5f * (n01.y+n12.y);
            //normalize(n);

            ImVec2 p01 = sub(p1,p0);
            normalize(p01);
            ImVec2 p12 = sub(p2,p1);
            normalize(p12);
            ImVec2 tangent = add(p01,p12); normalize(tangent);
            n = ImVec2(-tangent.y, tangent.x);

            ImVec2 n01(-p01.y, p01.x);

            float d = dot(n,n01);

            if(d<0.33)
            {
                n = tangent;
                d = -dot(n,n01);
                swap_indices = true;
            }

            scale(n, thickness * 0.5f / d);
        }

        vtx_write_ptr[0].pos.x = p1.x + n.x; vtx_write_ptr[0].pos.y = p1.y + n.y; vtx_write_ptr[0].uv = uv; vtx_write_ptr[0].col = col;
        vtx_write_ptr[1].pos.x = p1.x - n.x; vtx_write_ptr[1].pos.y = p1.y - n.y; vtx_write_ptr[1].uv = uv; vtx_write_ptr[1].col = col;
        vtx_write_ptr += 2;
        vtx_current_idx += 2;

        if(i>0)
        {
            if(!swap_indices)
            {
                idx_write_ptr[0] = (ImDrawIdx)(vtx_current_idx-3); idx_write_ptr[1] = (ImDrawIdx)(vtx_current_idx-1); idx_write_ptr[2] = (ImDrawIdx)(vtx_current_idx-4);
                idx_write_ptr[3] = (ImDrawIdx)(vtx_current_idx-4); idx_write_ptr[4] = (ImDrawIdx)(vtx_current_idx-1); idx_write_ptr[5] = (ImDrawIdx)(vtx_current_idx-2);
            }
            else
            {
                idx_write_ptr[0] = (ImDrawIdx)(vtx_current_idx-3); idx_write_ptr[1] = (ImDrawIdx)(vtx_current_idx-2); idx_write_ptr[2] = (ImDrawIdx)(vtx_current_idx-4);
                idx_write_ptr[3] = (ImDrawIdx)(vtx_current_idx-4); idx_write_ptr[4] = (ImDrawIdx)(vtx_current_idx-2); idx_write_ptr[5] = (ImDrawIdx)(vtx_current_idx-1);
            }
            idx_write_ptr += 6;
        }
    }

    if(closed)
    {
        int di = (count-1)*2;
        idx_write_ptr[0] = (ImDrawIdx)(vtx_current_idx-1-di); idx_write_ptr[1] = (ImDrawIdx)(vtx_current_idx-1); idx_write_ptr[2] = (ImDrawIdx)(vtx_current_idx-2-di);
        idx_write_ptr[3] = (ImDrawIdx)(vtx_current_idx-2-di); idx_write_ptr[4] = (ImDrawIdx)(vtx_current_idx-1); idx_write_ptr[5] = (ImDrawIdx)(vtx_current_idx-2);
        idx_write_ptr += 6;
    }

}

void CanvasDrawingLayer::addPolyFilled(const Eigen::Vector2f* points, int num_points, dg::Color color)
{
    ImVector<ImVec2> im_points;
    im_points.resize(num_points);
    for(int i=0; i<num_points; ++i)
        im_points[i] = to_imgui(points[i]);

    addPolyFilled(im_points, to_imgui(color));

}

void CanvasDrawingLayer::addPolyFilled(const ImVector<ImVec2>& poly, ImU32 col)
{
    if(poly.empty())
        return;

    struct PolygonWrapper
    {
        bool empty() const { return false; }
        int size() const { return 1; }
        const ImVector<ImVec2>& operator[](int) const { return poly; }
        const ImVector<ImVec2>& poly;
    };

    std::vector<int> indices = mapbox::earcut<int>(PolygonWrapper{poly});

    // Non Anti-aliased Fill
    int points_count = poly.Size;
    const ImVec2* points = poly.Data;
    const ImVec2 uv =  draw_list_->_Data->TexUvWhitePixel;

    const int idx_count = indices.size();
    const int vtx_count = points_count;
    draw_list_->PrimReserve(idx_count, vtx_count);

    ImDrawVert* &vtx_write_ptr = draw_list_->_VtxWritePtr;
    ImDrawIdx* &idx_write_ptr = draw_list_->_IdxWritePtr;
    for (int i = 0; i < vtx_count; i++)
    {
        vtx_write_ptr[0].pos = points[i];
        vtx_write_ptr[0].uv = uv;
        vtx_write_ptr[0].col = col;
        vtx_write_ptr++;
    }

    for (int i = 0; i < idx_count; i++)
    {
        idx_write_ptr[0] = indices[i] + draw_list_->_VtxCurrentIdx;
        idx_write_ptr++;
    }
    draw_list_->_VtxCurrentIdx += (ImDrawIdx)vtx_count;
}

void CanvasDrawingLayer::addBezierCurve(const Eigen::Vector2f &p1,
        const Eigen::Vector2f &p2, const Eigen::Vector2f &p3,
        const Eigen::Vector2f &p4, dg::Color color, float thickness,
        int num_segments)
{
    draw_list_->AddBezierCurve(to_imgui(p1), to_imgui(p2), to_imgui(p3), to_imgui(p4), to_imgui(color), thickness, num_segments);
}

void CanvasDrawingLayer::addImage(ImTextureID user_texture_id, const Eigen::Vector2f& p_min, const Eigen::Vector2f& p_max, const Eigen::Vector2f& uv_min, const Eigen::Vector2f& uv_max, dg::Color color)
{
    draw_list_->AddImage(user_texture_id, to_imgui(p_min), to_imgui(p_max), to_imgui(uv_min), to_imgui(uv_max), to_imgui(color));
}

void CanvasDrawingLayer::addImage(ImTextureID user_texture_id, const Eigen::Vector2f& p_min, const Eigen::Vector2f& p_max, dg::Color color)
{
    addImage(user_texture_id, p_min, p_max, Eigen::Vector2f(0, 0), Eigen::Vector2f(1, 1), color);
}

void CanvasDrawingLayer::pathClear()
{
    draw_list_->PathClear();
}

void CanvasDrawingLayer::pathLineTo(const Eigen::Vector2f &pos)
{
    draw_list_->PathLineToMergeDuplicate(to_imgui(pos));
    //draw_list_->PathLineTo(to_imgui(pos));
}

void CanvasDrawingLayer::pathLineTo(float x, float y)
{
    draw_list_->PathLineToMergeDuplicate(ImVec2(x,y));
    //draw_list_->PathLineTo(ImVec2(x,y));
}

void CanvasDrawingLayer::pathFill(dg::Color color, bool clear_path)
{
    addPolyFilled(draw_list_->_Path, to_imgui(color));
    if(clear_path)
        draw_list_->_Path.Size = 0;
}

void CanvasDrawingLayer::pathStroke(dg::Color color, bool closed,
        float thickness, bool clear_path)
{
    //m_draw_list->PathStroke(to_imgui(color), closed, thickness);
    addPolyline(draw_list_->_Path.Data, draw_list_->_Path.Size, to_imgui(color), closed, thickness);
    if(clear_path)
        draw_list_->_Path.Size = 0;
}

void CanvasDrawingLayer::pathArcTo(const Eigen::Vector2f &center, float radius,
        float a_min, float a_max, int num_segments)
{
    draw_list_->PathArcTo(to_imgui(center), radius, a_min, a_max, num_segments);
}

void CanvasDrawingLayer::pathBezierCurveTo(const Eigen::Vector2f &p2,
        const Eigen::Vector2f &p3, const Eigen::Vector2f &p4,
        int num_segments)
{
    draw_list_->PathBezierCurveTo(to_imgui(p2), to_imgui(p3), to_imgui(p4), num_segments);
}


std::tuple<ImDrawIdx*, ImDrawVert*, unsigned int> CanvasDrawingLayer::addRawTriangleList(int idx_count, int vtx_count)
{
    draw_list_->PrimReserve(idx_count, vtx_count);
    const ImVec2 uv =  draw_list_->_Data->TexUvWhitePixel;

    for(int i=0; i<vtx_count; ++i)
        draw_list_->_VtxWritePtr[i].uv = uv;

    std::tuple<ImDrawIdx*, ImDrawVert*, unsigned int> r(draw_list_->_IdxWritePtr, draw_list_->_VtxWritePtr, draw_list_->_VtxCurrentIdx);

    draw_list_->_VtxWritePtr += vtx_count;
    draw_list_->_VtxCurrentIdx += vtx_count;
    draw_list_->_IdxWritePtr  += idx_count;

    return r;
}

void CanvasDrawingLayer::callCommand(const std::string& command)
{
    if(method_ifc_.empty())
    {
        method_ifc_.registerMethod("clear", &CanvasDrawingLayer::clear, this);
        method_ifc_.registerMethod("pathClear", &CanvasDrawingLayer::pathClear, this);
        method_ifc_.registerMethod<void,float,float>("pathLineTo", &CanvasDrawingLayer::pathLineTo, this);
        method_ifc_.registerMethod("pathStroke", &CanvasDrawingLayer::pathStroke, this);
        method_ifc_.registerMethod("pathStroke", [this](dg::Color p0, bool p1, float p2){this->pathStroke(p0,p1,p2);});
        method_ifc_.registerMethod<void,float,float,dg::Color,const std::string&>("addText", &CanvasDrawingLayer::addText, this);
        method_ifc_.registerMethod<void,float,float,dg::Color,const std::string&,float>("addText", &CanvasDrawingLayer::addText, this);
        method_ifc_.registerMethod("addRectFilled", [this](float x1, float y1, float x2, float y2, dg::Color color){this->addRectFilled(Eigen::Vector2f(x1,y1),Eigen::Vector2f(x2,y2),color);});
    }

    method_ifc_.callMethod(command);

}






}
