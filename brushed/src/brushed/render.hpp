#ifndef BRUSHED_SRC_BRUSHED_RENDER_HPP
#define BRUSHED_SRC_BRUSHED_RENDER_HPP
#include "Fwog/BasicTypes.h"
#include "Fwog/Context.h"
#include "Fwog/Pipeline.h"
#include "Fwog/Shader.h"
#include "brushed/transform.hpp"
#include "brushed/triangle.hpp"
#include "brushed/world.hpp"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/vector_int2.hpp"
#include "glm/fwd.hpp"
#include <glad/glad.h>
#include <Fwog/Texture.h>
#include <Fwog/Rendering.h>
#include <Fwog/Buffer.h>
#include <Fwog/DebugMarker.h>
#include <optional>
#include <print>
#include <span>
namespace brushed {

	inline auto vtx = R"(
	#version 460

	layout(location = 0) in vec3 aPos;

	layout(location = 0) uniform mat4 model;
	layout(location = 1) uniform mat4 view;
	layout(location = 2) uniform mat4 projection;

	void main()
	{
		gl_Position = projection * view * model * vec4(aPos, 1.0);
	}
	)";

	inline auto frg = R"(
	
	#version 460

	layout(location = 0)out vec4 FragColor;

	void main() {
		FragColor = vec4(0.0, 1.0, 0.0, 1.0); // Green color
	}
	)";

	struct viewport {
		transform cam_pos;
		float fovy;
		glm::ivec2 viewport_size;
	};
	struct fwog_context {
		std::optional<Fwog::GraphicsPipeline> solid;

		std::optional<Fwog::TypedBuffer<glm::mat4>> uniform_view;
		std::optional<Fwog::TypedBuffer<glm::mat4>> uniform_projection;
		std::optional<Fwog::TypedBuffer<glm::mat4>> uniform_model;
		std::optional<Fwog::Buffer> vtx_buffer;
		std::optional<Fwog::Buffer> idx_buffer;
		fwog_context() {
			Fwog::Initialize(Fwog::ContextInitializeInfo{
				.verboseMessageCallback = [](std::string_view msg) {
					std::println("Fwog: {}", msg);
				} });
			auto vtx_shader = Fwog::Shader(Fwog::PipelineStage::VERTEX_SHADER, vtx);
			auto frg_shader = Fwog::Shader(Fwog::PipelineStage::FRAGMENT_SHADER, frg);

			std::array vertex_layout = { Fwog::VertexInputBindingDescription{
				.binding = 0,
				.format = Fwog::Format::R32G32B32_FLOAT,
				.offset = 0 } };
			solid = Fwog::GraphicsPipeline(
				Fwog::GraphicsPipelineInfo{
					.name = "solid brush",
					.vertexShader = &vtx_shader,
					.fragmentShader = &frg_shader,
					.inputAssemblyState = Fwog::InputAssemblyState{
						.topology = Fwog::PrimitiveTopology::TRIANGLE_LIST },
					.vertexInputState = Fwog::VertexInputState{
						.vertexBindingDescriptions = vertex_layout,
					},
				});
			uniform_view = Fwog::TypedBuffer<glm::mat4>(Fwog::BufferStorageFlag::DYNAMIC_STORAGE, "view");
			uniform_projection = Fwog::TypedBuffer<glm::mat4>(Fwog::BufferStorageFlag::DYNAMIC_STORAGE, "projection");
			uniform_model = Fwog::TypedBuffer<glm::mat4>(Fwog::BufferStorageFlag::DYNAMIC_STORAGE, "model");
			vtx_buffer = Fwog::Buffer(sizeof(triangle) * 102400, Fwog::BufferStorageFlag::MAP_MEMORY | Fwog::BufferStorageFlag::DYNAMIC_STORAGE, "vertex buffer");
			idx_buffer = Fwog::Buffer(sizeof(4), Fwog::BufferStorageFlag::NONE, "index buffer");
		}
		fwog_context(fwog_context&&) = delete;
		fwog_context(const fwog_context&) = delete;
		fwog_context& operator=(fwog_context&&) = delete;
		fwog_context& operator=(const fwog_context&) = delete;
	};
	inline void render(world& w, fwog_context& ctx, Fwog::Texture& render_target, Fwog::Texture& render_target_depth, viewport vp) {
		Fwog::InvalidatePipelineState();
		auto _ = Fwog::ScopedDebugMarker("Render [Solid]");
		std::array color_attachment{ Fwog::RenderColorAttachment{
			.texture = render_target,
			.loadOp = Fwog::AttachmentLoadOp::CLEAR,
			.clearValue = Fwog::ClearColorValue{ 1.0f, 1.0f, 0.0f, 1.0f },
		} };
		Fwog::RenderDepthStencilAttachment depth_attachment{
			.texture = render_target_depth,
			.loadOp = Fwog::AttachmentLoadOp::CLEAR,
			.clearValue = Fwog::ClearDepthStencilValue{ 0.0f, 0 }
		};
		Fwog::RenderInfo render_info{
			.viewport = Fwog::Viewport{
				.drawRect = {
					.offset = { 0, 0 },
					.extent = { (unsigned int)vp.viewport_size.x, (unsigned int)vp.viewport_size.y } } },
			.colorAttachments = color_attachment,
			.depthAttachment = depth_attachment
		};
		auto triangles = w.generate_mesh_from_brushes(w.brushes);
		auto triangle_span = std::span(triangles);
		auto proj = glm::perspectiveFov(vp.fovy, (float)vp.viewport_size.x, (float)vp.viewport_size.y, 1.0f, 819200.0f);
		ctx.uniform_projection.value().UpdateData(proj);
		ctx.uniform_view.value().UpdateData(glm::inverse(vp.cam_pos.to_mat()));
		ctx.vtx_buffer.value().UpdateData(triangle_span);
		Fwog::Render(render_info, [&triangles, &w, &ctx, &vp]() {
			Fwog::Cmd::BindGraphicsPipeline(ctx.solid.value());
			Fwog::Cmd::BindVertexBuffer(0, ctx.vtx_buffer.value(), 0, sizeof(triangle));
			// Fwog::Cmd::BindIndexBuffer(ctx.idx_buffer.value(), Fwog::IndexType::UNSIGNED_INT);
			Fwog::Cmd::BindUniformBuffer(0, ctx.uniform_model.value());
			Fwog::Cmd::BindUniformBuffer(1, ctx.uniform_view.value());
			Fwog::Cmd::BindUniformBuffer(2, ctx.uniform_projection.value());
			GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
			if(status != GL_FRAMEBUFFER_COMPLETE)
				throw 0;
			Fwog::Cmd::Draw(triangles.size() * 3, 1, 0, 0);
		});
	}
}
#endif