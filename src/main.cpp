// Copyright (c) 2025 Bryan Rykowski
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#define NSPRE_IMPL
#include "nspre.hpp"
static_assert(NSPRE_VERSION_MAJOR == 1);

#include "nspre-gui.hpp"
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"
#include <SDL2/SDL.h>
#include <GL/gl.h>
#include <cstdio>

#ifndef NSPRE_GUI_VERSION
#define NSPRE_GUI_VERSION "undefined"
#endif

#if !defined(_WIN32) && !defined(NSPRE_GUI_SLEEPWAIT)
#define NSPRE_GUI_SLEEPWAIT
#endif

#ifndef NSPRE_GUI_VSYNC
#define NSPRE_GUI_VSYNC 0
#endif

#ifndef NSPRE_GUI_FLIMIT
#define NSPRE_GUI_FLIMIT 60
#endif

namespace ns {

GlobalStruct global;
ExtractWindow extract_window;
CreateWindow create_window;

SDL_Window* window;
SDL_GLContext context;

int arg_vsync = NSPRE_GUI_VSYNC;
int arg_flimit = NSPRE_GUI_FLIMIT;

void top_window() {
	ImVec2 size;
	size.x = ns::global.io->DisplaySize.x;
	size.y = ns::global.io->DisplaySize.y;
	ImGui::SetNextWindowSize(size);
	ImGui::SetNextWindowPos({0,0});
	ImGui::Begin(
		global.open_mode ? "Extract###topwindow" : "Create###topwindow",
		0,
		ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBringToFrontOnFocus
	);

	ImGui::SetNextWindowSizeConstraints({300,50}, {500, global.io->DisplaySize.y - 24});
	if (ImGui::BeginPopupModal("Error", 0, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::PushItemWidth(400);
		ImGui::TextWrapped("%s", global.error_modal_text.str().c_str());
		ImGui::PopItemWidth();

		if (ImGui::Button("OK")) {
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}

	ImGui::SetNextWindowSizeConstraints({300,50}, {500, global.io->DisplaySize.y - 24});
	if (ImGui::BeginPopupModal("About", 0, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::Text("Version %s", NSPRE_GUI_VERSION);
		ImGui::Dummy({0,0});
		ImGui::Text("(c) 2025 Bryan Rykowski");
		ImGui::TextLinkOpenURL("https:/github.com/BryanRykowski/nspre-gui");
		ImGui::Dummy({0,0});

		if (ImGui::Button("OK")) {
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}

	if (global.open_mode) {
		extract_window.show();
	}
	else {
		create_window.show();
	}

	ImGui::End();
}

}

int main(int argc, char** argv) {

	for (int i = 1; i < argc; ++i) {
		bool has_val = (i + 1 < argc);
		if (std::strcmp("--imgui-demo", argv[i]) == 0) {
			ns::global.show_demo_window = true;
		}
		else if (std::strcmp("--imgui-debug", argv[i]) == 0) {
			ns::global.show_debug = true;
		}
		else if (std::strcmp("--vsync-disable", argv[i]) == 0) {
			ns::arg_vsync = 0;
		}
		else if (std::strcmp("--vsync-enable", argv[i]) == 0) {
			ns::arg_vsync = 1;
		}
		else if (has_val && (std::strcmp("--frame-limit", argv[i]) == 0)) {
			try {
				ns::arg_flimit = std::stoi(argv[i + 1]);
			}
			catch (...) {
				std::fprintf(stderr, "invalid frame limit value \"%s\"\n", argv[i + 1]);
			}

			++i;
		}
		else {
			ns::extract_window.open_pre(argv[i]);
		}
	}

	std::printf("nspre-gui version %s\n", NSPRE_GUI_VERSION);

	if (SDL_Init(SDL_INIT_EVERYTHING)) {
		std::fprintf(stderr, "sdl init failed\n");
		return -1;
	}

	if (SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3) ||
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3)
	) {
		std::fprintf(stderr, "gl set attribute failed\n");
		return -1;
	}
	
	ns::window = SDL_CreateWindow("nspre-gui", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1200, 800, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

	if (!ns::window) {
		std::fprintf(stderr, "create window failed\n");
		return -1;
	}
	
	ns::context = SDL_GL_CreateContext(ns::window);

	if (!ns::context) {
		std::fprintf(stderr, "create context failed\n");
		return -1;
	}

	SDL_GL_SetSwapInterval(ns::arg_vsync);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& ioref = ImGui::GetIO();
	ns::global.io = &ioref;
	ns::global.io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	ns::global.io->ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	ns::global.io->IniFilename = 0;
	
	// Setup Platform/Renderer backends
	ImGui_ImplSDL2_InitForOpenGL(ns::window, ns::context);
	ImGui_ImplOpenGL3_Init();

	ns::PathList drops;
	bool skip;
	SDL_Event e;
	uint64_t then = 0;
	uint64_t rate = SDL_GetPerformanceFrequency();
	uint64_t dtime = rate / ns::arg_flimit;

	while (!ns::global.quit) {

		// Limit the frame rate
		uint64_t now = SDL_GetPerformanceCounter();
		uint64_t delta = now - then;
		if (delta >= dtime) {
			then = now;
		}
		else {
#ifdef NSPRE_GUI_SLEEPWAIT
			if ((dtime - delta) > (rate / 100)) {
				SDL_Delay(1);
			}
#endif
			continue;
		}

		skip = false;
		while (SDL_PollEvent(&e)) {
			ImGui_ImplSDL2_ProcessEvent(&e); // Forward your event to backend
			if (e.type == SDL_QUIT) ns::global.quit = true;
			else if (e.type == SDL_DROPFILE && std::filesystem::is_regular_file(e.drop.file)) {
				drops.push_back(e.drop.file);
				SDL_free(e.drop.file);
			}
			else if (e.type == SDL_DROPCOMPLETE) {
				if (ns::global.open_mode) {
					if (drops.size()) {
						ns::extract_window.open_pre(drops[0]);
					}
				}
				else {
					ns::create_window.drop_files(drops);
				}

				drops.clear();
			}
			else if (e.type == SDL_WINDOWEVENT_RESIZED) {
				skip = true;
				continue;
			}
		}

		// Avoid some jitteryness by not updating the UI during resizing
		if (skip) {
			continue;
		}

		glClear(GL_COLOR_BUFFER_BIT);

		// (After event loop)
		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();

		ns::top_window();
		
		if (ns::global.show_debug) {
			ImGui::ShowDebugLogWindow();
		}

		if (ns::global.show_demo_window) {
			ImGui::ShowDemoWindow(); // Show demo window! :)
		}

		// Rendering
		// (Your code clears your framebuffer, renders your other stuff etc.)
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		// (Your code calls SDL_GL_SwapWindow() etc.)
		SDL_GL_SwapWindow(ns::window);

	}

	SDL_Quit();
	return 0;
}
