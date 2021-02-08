
#pragma once

#include <string>
#include <map>
#include <list>
#include <typeindex>

#include <dg/core/fwds.hpp>
#include <dg/core/common.hpp>

#include <DiligentCore/Graphics/GraphicsEngine/interface/Buffer.h>
#include <DiligentCore/Graphics/GraphicsEngine/interface/Shader.h>
#include <DiligentCore/Graphics/GraphicsEngine/interface/ShaderResourceBinding.h>
#include <DiligentCore/Graphics/GraphicsTools/interface/MapHelper.hpp>


namespace dg {

/**
 * A shader program encapsulates a vertex and fragment shader pair linked to form a shader program.
 */
class ShaderProgram
{
public:

	struct ConstantDef
	{
		std::string name;
		SHADER_TYPE shader_type;
		const std::type_info* type;
		std::size_t size;
	};

public:

	ShaderProgram() = default;

	ShaderProgram(IRenderDevice* device,
			      const std::string& name, const std::string& vs_code, const std::string& ps_code);

public:


	/**
	 * List of macros definitions.
	 * Each definitions is given as string pairs: ( macro name : macro definition/value )
	 */
	typedef std::list<std::pair<std::string,std::string>> MacroDefinitions;

	void setShaders(IRenderDevice* device, const std::string& name,
	                const std::string& vs_code, const std::string& ps_code,
	                const MacroDefinitions& macros= MacroDefinitions());

	RefCntAutoPtr<IShader> getVertexShader() { return vertex_shader_; }
	RefCntAutoPtr<IShader> getPixelShader() { return pixel_shader_; }

	template <typename T>
	void addConstant(IRenderDevice* device,
			         const std::string& name, SHADER_TYPE shader_type = SHADER_TYPE_VERTEX)
	{
		addConstant(device, {name, shader_type, &typeid(T), sizeof(T)});
	};

	template <typename T>
	MapHelper<T> mapConstant(IDeviceContext* context, const std::string& name)
	{
		auto it = constants_.find(name);
		if(it==constants_.end())
			DG_THROW("No such constant found: " + name);

		if(typeid(T) != *it->second.type)
			DG_THROW(std::string("Types mismatch - declared type: ") + it->second.type->name() + ", mapped type: " + typeid(T).name());

		return MapHelper<T>(context, it->second.buffer, MAP_WRITE, MAP_FLAG_DISCARD);
	}

public:

	/**
	 * Bind this shader program to the given pipeline state object.
	 */
	void bind(IPipelineState* pso);


private:

	void addConstant(IRenderDevice* device, const ConstantDef& def);

private:

	struct Constant : public ConstantDef
	{
		Constant() = default;
		Constant(const ConstantDef& other) : ConstantDef(other) {}

		RefCntAutoPtr<IBuffer> buffer;
	};

	RefCntAutoPtr<IShader> vertex_shader_;
	RefCntAutoPtr<IShader> pixel_shader_;
	std::map<std::string, Constant> constants_;

};


}
