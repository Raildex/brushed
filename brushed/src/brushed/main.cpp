#include "Fwog/BasicTypes.h"
#include "Fwog/DebugMarker.h"
#include "Fwog/Rendering.h"
#include "Fwog/Texture.h"
#include "brushed/brush.hpp"
#include "brushed/render.hpp"
#include "brushed/transform.hpp"
#include "brushed/world.hpp"
#include <print>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <Fwog/Context.h>
#include <Fwog/Pipeline.h>
#include <Fwog/Shader.h>
#include "brushed/render.hpp"
#include "glm/fwd.hpp"
namespace brushed {
	void file_menu() {
		if(ImGui::BeginMenu("File")) {
			if(ImGui::MenuItem("New")) {
			}
			if(ImGui::MenuItem("Open")) {
			}
			if(ImGui::MenuItem("Save")) {
			}
			if(ImGui::MenuItem("Save As")) {
			}
			ImGui::Separator();
			if(ImGui::MenuItem("Close")) {
			}
			ImGui::EndMenu();
		}
	}

	void edit_menu() {
		if(ImGui::BeginMenu("Edit")) {
			if(ImGui::MenuItem("Undo")) {
			}
			if(ImGui::MenuItem("Redo")) {
			}
			if(ImGui::MenuItem("Copy")) {
			}
			if(ImGui::MenuItem("Cut")) {
			}
			if(ImGui::MenuItem("Paste")) {
			}
			ImGui::EndMenu();
		}
	}

	void loop(GLFWwindow* win) {
		fwog_context ctx{};
		brushed::world world{};
		viewport vp{
			.cam_pos = transform(
				glm::vec3(0, 0, 0),
				glm::vec3(0, 0, 0),
				glm::vec3(1, 1, 1)),
			.fovy = 90.0f,
			.viewport_size = {
				1.0f,
				1.0f,
			},

		};
		world.add_brush(box());
		Fwog::Texture viewport0 = Fwog::CreateTexture2D(Fwog::Extent2D{ 1, 1 }, Fwog::Format::R32G32B32A32_FLOAT, "viewport0");
		Fwog::Texture viewport0_depth = Fwog::CreateTexture2D(Fwog::Extent2D{ 1, 1 }, Fwog::Format::D32_FLOAT, "viewport0 depth");
		bool running = true;
		while(!glfwWindowShouldClose(win)) {
			glfwPollEvents();
			render(world, ctx, viewport0, viewport0_depth, vp);
			ImGui_ImplGlfw_NewFrame();
			ImGui_ImplOpenGL3_NewFrame();
			ImGui::NewFrame();
			auto* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->WorkPos);
			ImGui::SetNextWindowSize(viewport->WorkSize);
			ImGui::Begin("brushed", nullptr, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoDecoration);
			if(ImGui::BeginMenuBar()) {
				file_menu();
				edit_menu();
				ImGui::EndMenuBar();
			}
			auto dockspace_id = ImGui::GetID("Dockspace");
			ImGui::DockSpace(dockspace_id);
			ImGui::End();
			ImGui::Begin("Brushes");
			if(ImGui::TreeNodeEx("world", ImGuiTreeNodeFlags_DefaultOpen)) {
				for(int i = 0; i < 1000; ++i) {
					ImGui::PushID("tree");
					if(i % 3 == 0 && i % 7 == 0) {
						ImGui::PushID(i);
						if(ImGui::TreeNodeEx("Group", ImGuiTreeNodeFlags_SpanFullWidth)) {
							ImGui::PushID(i);
							if(ImGui::TreeNodeEx("Brush", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_SpanFullWidth)) {
								ImGui::TreePop();
							}
							ImGui::PopID();
							if(ImGui::TreeNodeEx("Brush", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_SpanFullWidth)) {
								ImGui::TreePop();
							}
							ImGui::TreePop();
						}
						ImGui::PopID();
					} else {
						ImGui::PushID(i);
						if(ImGui::TreeNodeEx("Brush", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_SpanFullWidth)) {
							ImGui::TreePop();
						}
						ImGui::PopID();
					}
					ImGui::PopID();
				}
				ImGui::TreePop();
			}

			ImGui::End();
			ImGui::Begin("Entities");
			ImGui::End();
			ImGui::Begin("Textures");
			ImGui::End();
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
			if(ImGui::Begin("Viewport0", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBackground)) {

				if(ImGui::IsMouseDown(ImGuiMouseButton_Right)) {
					auto delta_rot = ImGui::GetMouseDragDelta(ImGuiMouseButton_Right);
					ImGui::ResetMouseDragDelta(ImGuiPopupFlags_MouseButtonRight);
					std::println("X: [{}], Y: [{}]", delta_rot.x, delta_rot.y);
					vp.cam_pos.add_rotation(glm::vec3(delta_rot.y, delta_rot.x, 0));
					if(ImGui::IsKeyDown(ImGuiKey_W)) {
						vp.cam_pos.move_foward(128);
					}
					if(ImGui::IsKeyDown(ImGuiKey_D)) {
						vp.cam_pos.move_right(128);
					}
				}

				auto size = ImGui::GetContentRegionAvail();
				auto tex_width = viewport0.GetCreateInfo().extent.width;
				auto tex_height = viewport0.GetCreateInfo().extent.height;
				if(size.x != tex_width || size.y != tex_height) {
					// viewport0 = Fwog::CreateTexture2D(Fwog::Extent2D{ (unsigned int)size.x, (unsigned int)size.y }, Fwog::Format::R32G32B32A32_FLOAT, "viewport0");
					// viewport0_depth = Fwog::CreateTexture2D(Fwog::Extent2D{ (unsigned int)size.x, (unsigned int)size.y }, Fwog::Format::D32_FLOAT, "viewport0 depth");
					// continue;
					// std::println("Resize Viewport0 render target");
				}
				vp.viewport_size.x = size.x;
				vp.viewport_size.y = size.y;
				ImGui::Image((ImTextureID)viewport0.Handle(), size);
				ImGui::End();
			}
			ImGui::PopStyleVar();
			ImGui::Render();
			int w;
			int h;
			glfwGetFramebufferSize(win, &w, &h);
			auto swapchain_render_info = Fwog::SwapchainRenderInfo{
				.name = "RenderToSwapchain",
				.viewport = {
					.drawRect = {
						.offset = { 0, 0 },
						.extent = {
							.width = (unsigned int)w,
							.height = (unsigned int)h,
						} },
					.minDepth = 0.0f,
					.maxDepth = 1.0f,
					.depthRange = Fwog::ClipDepthRange::NEGATIVE_ONE_TO_ONE,
				},
				.colorLoadOp = Fwog::AttachmentLoadOp::CLEAR,
				.clearColorValue = Fwog::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f),
				.depthLoadOp = Fwog::AttachmentLoadOp::CLEAR,
				.clearDepthValue = 0.0f,
			};
			Fwog::RenderToSwapchain(swapchain_render_info, []() {
				auto draw_data = ImGui::GetDrawData();
				auto _ = Fwog::ScopedDebugMarker("imgui");
				ImGui_ImplOpenGL3_RenderDrawData(draw_data);
			});
			glfwSwapBuffers(win);
		}
	}
}

void APIENTRY gl_message(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
	std::println("{}: {}", type == GL_DEBUG_TYPE_ERROR || type == GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR ? "ERROR" : "INFO", message);
}

int main() {
	if(glfwInit() == GLFW_FALSE) {
		return 1;
	}
	auto* win = glfwCreateWindow(800, 600, "brushed", nullptr, nullptr);
	if(!win) {
		return 2;
	}
	glfwMakeContextCurrent(win);
	if(gladLoadGLLoader((GLADloadproc)glfwGetProcAddress) == 0) {
		return 3;
	}
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(gl_message, nullptr);
	auto imgui = ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	(void)io;
	auto arial = io.Fonts->AddFontFromFileTTF("brushed/extras/Arial-bold.ttf", 20);
	io.FontDefault = arial;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	if(!ImGui_ImplGlfw_InitForOpenGL(win, true)) {
		return 4;
	}
	if(!ImGui_ImplOpenGL3_Init("#version 130")) {
		return 5;
	}
	if(!ImGui_ImplOpenGL3_CreateDeviceObjects()) {
		return 6;
	}
	if(!ImGui_ImplOpenGL3_CreateFontsTexture()) {
		return 7;
	}
	brushed::loop(win);
	ImGui_ImplOpenGL3_DestroyFontsTexture();
	ImGui_ImplOpenGL3_DestroyDeviceObjects();
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext(imgui);
	Fwog::Terminate();
	glfwDestroyWindow(win);
	glfwTerminate();

	return 1;
}