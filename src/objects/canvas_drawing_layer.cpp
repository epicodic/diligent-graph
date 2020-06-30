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
	_draw_list = parent_object->createDrawList();
	clear();
}

void CanvasDrawingLayer::render()
{
	getParentObject()->getSceneManager()->context()->SetStencilRef(1);
	getParentObject()->renderDrawList(_draw_list, _opacity);
}

void CanvasDrawingLayer::setOpacity(float opacity)
{
	_opacity = opacity;
}

void CanvasDrawingLayer::clear()
{
	_draw_list->Clear();
	_draw_list->Flags = ImDrawListFlags_AllowVtxOffset;
	_draw_list->PushTextureID(getParentObject()->getImGuiContext()->Font->ContainerAtlas->TexID);
	//m_draw_list->PushClipRectFullScreen();
	_draw_list->PushClipRect(ImVec2(0,0), ImVec2(10000,10000));

}

void CanvasDrawingLayer::addLine(const Eigen::Vector2f &p1,
		const Eigen::Vector2f &p2, dg::Color color, float thickness)
{
	_draw_list->AddLine(to_imgui(p1), to_imgui(p2), to_imgui(color), thickness);
}

void CanvasDrawingLayer::addRect(const Eigen::Vector2f &p_min,
		const Eigen::Vector2f &p_max, dg::Color color, float rounding,
		ImDrawCornerFlags rounding_corners, float thickness)
{
	_draw_list->AddRect(to_imgui(p_min), to_imgui(p_max), to_imgui(color), rounding, rounding_corners, thickness);
}

void CanvasDrawingLayer::addRectFilled(const Eigen::Vector2f &p_min,
		const Eigen::Vector2f &p_max, dg::Color color, float rounding,
		ImDrawCornerFlags rounding_corners)
{
	_draw_list->AddRectFilled(to_imgui(p_min), to_imgui(p_max), to_imgui(color), rounding, rounding_corners);
}

void CanvasDrawingLayer::addRectFilledMultiColor(const Eigen::Vector2f &p_min,
		const Eigen::Vector2f &p_max, dg::Color color_upr_left,
		dg::Color color_upr_right, dg::Color color_bot_right,
		dg::Color color_bot_left)
{
	_draw_list->AddRectFilledMultiColor(to_imgui(p_min), to_imgui(p_max), to_imgui(color_upr_left), to_imgui(color_upr_right), to_imgui(color_bot_right), to_imgui(color_bot_left));
}

void CanvasDrawingLayer::addQuad(const Eigen::Vector2f &p1,
		const Eigen::Vector2f &p2, const Eigen::Vector2f &p3,
		const Eigen::Vector2f &p4, dg::Color color, float thickness)
{
	_draw_list->AddQuad(to_imgui(p1), to_imgui(p2), to_imgui(p3), to_imgui(p4), to_imgui(color), thickness);
}

void CanvasDrawingLayer::addQuadFilled(const Eigen::Vector2f &p1,
		const Eigen::Vector2f &p2, const Eigen::Vector2f &p3,
		const Eigen::Vector2f &p4, dg::Color color)
{
	_draw_list->AddQuadFilled(to_imgui(p1), to_imgui(p2), to_imgui(p3), to_imgui(p4), to_imgui(color));
}

void CanvasDrawingLayer::addTriangle(const Eigen::Vector2f &p1,
		const Eigen::Vector2f &p2, const Eigen::Vector2f &p3, dg::Color color,
		float thickness)
{
	_draw_list->AddTriangle(to_imgui(p1), to_imgui(p2), to_imgui(p3), to_imgui(color), thickness);
}

void CanvasDrawingLayer::addTriangleFilled(const Eigen::Vector2f &p1,
		const Eigen::Vector2f &p2, const Eigen::Vector2f &p3, dg::Color color)
{
	_draw_list->AddTriangleFilled(to_imgui(p1), to_imgui(p2), to_imgui(p3), to_imgui(color));
}

void CanvasDrawingLayer::addCircle(float x, float y, float radius,
		dg::Color color, int num_segments, float thickness)
{
	_draw_list->AddCircle(ImVec2(x,y), radius, to_imgui(color), num_segments, thickness);
}

void CanvasDrawingLayer::addCircleFilled(float x, float y,
		float radius, dg::Color color, int num_segments)
{
	_draw_list->AddCircleFilled(ImVec2(x,y), radius, to_imgui(color), num_segments);
}

void CanvasDrawingLayer::addText(float x, float y, dg::Color color, const std::string& text)
{
	_draw_list->AddText(ImVec2(x,y), to_imgui(color), text.c_str());
}

void CanvasDrawingLayer::addText(float x, float y, dg::Color color, const std::string& text, float font_size)
{
	addText(x,y,color,text,font_size,nullptr);
}

void CanvasDrawingLayer::addText(float x, float y, dg::Color color, const std::string& text, float font_size, const ImFont* font, float wrap_width)
{
	_draw_list->AddText(font, font_size, ImVec2(x,y), to_imgui(color), text.c_str(), nullptr, wrap_width);
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
	_draw_list->PrimReserve(idx_count, vtx_count);

	const ImVec2 uv =  _draw_list->_Data->TexUvWhitePixel;

	ImDrawVert* &vtxWritePtr = _draw_list->_VtxWritePtr;
	ImDrawIdx* &idxWritePtr = _draw_list->_IdxWritePtr;
	unsigned int &vtxCurrentIdx = _draw_list->_VtxCurrentIdx;

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

		vtxWritePtr[0].pos.x = p1.x + n.x; vtxWritePtr[0].pos.y = p1.y + n.y; vtxWritePtr[0].uv = uv; vtxWritePtr[0].col = col;
		vtxWritePtr[1].pos.x = p1.x - n.x; vtxWritePtr[1].pos.y = p1.y - n.y; vtxWritePtr[1].uv = uv; vtxWritePtr[1].col = col;
		vtxWritePtr += 2;
		vtxCurrentIdx += 2;

		if(i>0)
		{
			if(!swap_indices)
			{
				idxWritePtr[0] = (ImDrawIdx)(vtxCurrentIdx-3); idxWritePtr[1] = (ImDrawIdx)(vtxCurrentIdx-1); idxWritePtr[2] = (ImDrawIdx)(vtxCurrentIdx-4);
				idxWritePtr[3] = (ImDrawIdx)(vtxCurrentIdx-4); idxWritePtr[4] = (ImDrawIdx)(vtxCurrentIdx-1); idxWritePtr[5] = (ImDrawIdx)(vtxCurrentIdx-2);
			}
			else
			{
				idxWritePtr[0] = (ImDrawIdx)(vtxCurrentIdx-3); idxWritePtr[1] = (ImDrawIdx)(vtxCurrentIdx-2); idxWritePtr[2] = (ImDrawIdx)(vtxCurrentIdx-4);
				idxWritePtr[3] = (ImDrawIdx)(vtxCurrentIdx-4); idxWritePtr[4] = (ImDrawIdx)(vtxCurrentIdx-2); idxWritePtr[5] = (ImDrawIdx)(vtxCurrentIdx-1);
			}
			idxWritePtr += 6;
		}
	}

	if(closed)
	{
		int di = (count-1)*2;
		idxWritePtr[0] = (ImDrawIdx)(vtxCurrentIdx-1-di); idxWritePtr[1] = (ImDrawIdx)(vtxCurrentIdx-1); idxWritePtr[2] = (ImDrawIdx)(vtxCurrentIdx-2-di);
		idxWritePtr[3] = (ImDrawIdx)(vtxCurrentIdx-2-di); idxWritePtr[4] = (ImDrawIdx)(vtxCurrentIdx-1); idxWritePtr[5] = (ImDrawIdx)(vtxCurrentIdx-2);
		idxWritePtr += 6;
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
	const ImVec2 uv =  _draw_list->_Data->TexUvWhitePixel;

    const int idx_count = indices.size();
    const int vtx_count = points_count;
    _draw_list->PrimReserve(idx_count, vtx_count);

	ImDrawVert* &vtxWritePtr = _draw_list->_VtxWritePtr;
	ImDrawIdx* &idxWritePtr = _draw_list->_IdxWritePtr;
    for (int i = 0; i < vtx_count; i++)
    {
    	vtxWritePtr[0].pos = points[i];
    	vtxWritePtr[0].uv = uv;
    	vtxWritePtr[0].col = col;
    	vtxWritePtr++;
    }

    for (int i = 0; i < idx_count; i++)
    {
    	idxWritePtr[0] = indices[i] + _draw_list->_VtxCurrentIdx;
    	idxWritePtr++;
    }
    _draw_list->_VtxCurrentIdx += (ImDrawIdx)vtx_count;
}

void CanvasDrawingLayer::addBezierCurve(const Eigen::Vector2f &p1,
		const Eigen::Vector2f &p2, const Eigen::Vector2f &p3,
		const Eigen::Vector2f &p4, dg::Color color, float thickness,
		int num_segments)
{
	_draw_list->AddBezierCurve(to_imgui(p1), to_imgui(p2), to_imgui(p3), to_imgui(p4), to_imgui(color), thickness, num_segments);
}

void CanvasDrawingLayer::pathClear()
{
	_draw_list->PathClear();
}

void CanvasDrawingLayer::pathLineTo(const Eigen::Vector2f &pos)
{
	_draw_list->PathLineToMergeDuplicate(to_imgui(pos));
	//_draw_list->PathLineTo(to_imgui(pos));
}

void CanvasDrawingLayer::pathLineTo(float x, float y)
{
	_draw_list->PathLineToMergeDuplicate(ImVec2(x,y));
	//_draw_list->PathLineTo(ImVec2(x,y));
}

void CanvasDrawingLayer::pathFill(dg::Color color, bool clear_path)
{
	addPolyFilled(_draw_list->_Path, to_imgui(color));
	if(clear_path)
		_draw_list->_Path.Size = 0;
}

void CanvasDrawingLayer::pathStroke(dg::Color color, bool closed,
		float thickness, bool clear_path)
{
	//m_draw_list->PathStroke(to_imgui(color), closed, thickness);
	addPolyline(_draw_list->_Path.Data, _draw_list->_Path.Size, to_imgui(color), closed, thickness);
	if(clear_path)
		_draw_list->_Path.Size = 0;
}

void CanvasDrawingLayer::pathArcTo(const Eigen::Vector2f &center, float radius,
		float a_min, float a_max, int num_segments)
{
	_draw_list->PathArcTo(to_imgui(center), radius, a_min, a_max, num_segments);
}

void CanvasDrawingLayer::pathBezierCurveTo(const Eigen::Vector2f &p2,
		const Eigen::Vector2f &p3, const Eigen::Vector2f &p4,
		int num_segments)
{
	_draw_list->PathBezierCurveTo(to_imgui(p2), to_imgui(p3), to_imgui(p4), num_segments);
}


std::tuple<ImDrawIdx*, ImDrawVert*, unsigned int> CanvasDrawingLayer::addRawTriangleList(int idx_count, int vtx_count)
{
	_draw_list->PrimReserve(idx_count, vtx_count);
	const ImVec2 uv =  _draw_list->_Data->TexUvWhitePixel;

	for(int i=0; i<vtx_count; ++i)
		_draw_list->_VtxWritePtr[i].uv = uv;

	std::tuple<ImDrawIdx*, ImDrawVert*, unsigned int> r(_draw_list->_IdxWritePtr, _draw_list->_VtxWritePtr, _draw_list->_VtxCurrentIdx);

	_draw_list->_VtxWritePtr += vtx_count;
	_draw_list->_VtxCurrentIdx += vtx_count;
	_draw_list->_IdxWritePtr  += idx_count;

	return r;
}

void CanvasDrawingLayer::callCommand(const std::string& command)
{
	if(_method_ifc.empty())
	{
		_method_ifc.registerMethod("clear", &CanvasDrawingLayer::clear, this);
		_method_ifc.registerMethod("pathClear", &CanvasDrawingLayer::pathClear, this);
		_method_ifc.registerMethod<void,float,float>("pathLineTo", &CanvasDrawingLayer::pathLineTo, this);
		_method_ifc.registerMethod("pathStroke", &CanvasDrawingLayer::pathStroke, this);
		_method_ifc.registerMethod("pathStroke", [this](dg::Color p0, bool p1, float p2){this->pathStroke(p0,p1,p2);});
		_method_ifc.registerMethod<void,float,float,dg::Color,const std::string&>("addText", &CanvasDrawingLayer::addText, this);
		_method_ifc.registerMethod<void,float,float,dg::Color,const std::string&,float>("addText", &CanvasDrawingLayer::addText, this);
		_method_ifc.registerMethod("addRectFilled", [this](float x1, float y1, float x2, float y2, dg::Color color){this->addRectFilled(Eigen::Vector2f(x1,y1),Eigen::Vector2f(x2,y2),color);});
	}

	_method_ifc.callMethod(command);

}






}
