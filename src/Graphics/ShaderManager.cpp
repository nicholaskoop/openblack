/*****************************************************************************
 * Copyright (c) 2018-2021 openblack developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/openblack/openblack
 *
 * openblack is licensed under the GNU General Public License version 3.
 *****************************************************************************/

#include "ShaderManager.h"

#include <cstdint> // Shaders below need uint8_t

#include "Shaders/fs_line.bin.h"
#include "Shaders/fs_object.bin.h"
#include "Shaders/fs_terrain.bin.h"
#include "Shaders/fs_water.bin.h"
#include "Shaders/vs_line.bin.h"
#include "Shaders/vs_line_instanced.bin.h"
#include "Shaders/vs_object.bin.h"
#include "Shaders/vs_object_instanced.bin.h"
#include "Shaders/vs_terrain.bin.h"
#include "Shaders/vs_water.bin.h"

#include "3D/Camera.h"

#include <bgfx/embedded_shader.h>
#include <spdlog/spdlog.h>

namespace openblack::graphics
{

const bgfx::EmbeddedShader s_embeddedShaders[] = {BGFX_EMBEDDED_SHADER(vs_line),    BGFX_EMBEDDED_SHADER(vs_line_instanced),
                                                  BGFX_EMBEDDED_SHADER(fs_line),

                                                  BGFX_EMBEDDED_SHADER(vs_object),  BGFX_EMBEDDED_SHADER(vs_object_instanced),
                                                  BGFX_EMBEDDED_SHADER(fs_object),

                                                  BGFX_EMBEDDED_SHADER(vs_terrain), BGFX_EMBEDDED_SHADER(fs_terrain),

                                                  BGFX_EMBEDDED_SHADER(vs_water),   BGFX_EMBEDDED_SHADER(fs_water),

                                                  BGFX_EMBEDDED_SHADER_END()};

ShaderManager::~ShaderManager()
{
	// delete all mapped shaders
	ShaderMap::iterator iter;
	for (iter = _shaderPrograms.begin(); iter != _shaderPrograms.end(); ++iter) delete iter->second;

	_shaderPrograms.clear();
}

const ShaderProgram* ShaderManager::LoadShader(const std::string& name, const std::string& vertexShaderName,
											   const std::string& fragmentShaderName)
{
	spdlog::debug("Creating ShaderProgram: \"{}\" with \"{}\", \"{}\"", name, vertexShaderName, fragmentShaderName);
	bgfx::RendererType::Enum type = bgfx::getRendererType();

	ShaderMap::iterator i = _shaderPrograms.find(name);
	if (i != _shaderPrograms.end())
		return i->second;

	spdlog::debug("Creating vertex shader {}", vertexShaderName);
	auto vertexShader = bgfx::createEmbeddedShader(s_embeddedShaders, type, vertexShaderName.c_str());
	spdlog::debug("Creating fragment shader {}", fragmentShaderName);
	auto fragmentShader = bgfx::createEmbeddedShader(s_embeddedShaders, type, fragmentShaderName.c_str());
	ShaderProgram* program = new ShaderProgram(name, std::forward<bgfx::ShaderHandle>(vertexShader), std::forward<bgfx::ShaderHandle>(fragmentShader));
	_shaderPrograms[name] = program;
	return program;
}

const ShaderProgram* ShaderManager::GetShader(const std::string& name) const
{
	auto i = _shaderPrograms.find(name);
	if (i != _shaderPrograms.end())
		return i->second;

	// todo: return an empty shader?
	return nullptr;
}

void ShaderManager::SetCamera(graphics::RenderPass viewId, const Camera& camera)
{
	auto view = camera.GetViewMatrix();
	auto proj = camera.GetProjectionMatrix();
	bgfx::setViewTransform(static_cast<bgfx::ViewId>(viewId), &view, &proj);
}

} // namespace openblack::graphics
