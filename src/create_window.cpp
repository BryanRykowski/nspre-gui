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

#include "nspre-gui.hpp"

namespace ns {

bool CreateWindow::files_ready() {
	if (!files.size()) {
		return false;
	}

	for (auto& f : files) {
		if (f.second.empty()) {
			return false;
		}
	}

	return true;
}

void CreateWindow::create_pre() {
	std::vector<nspre::Subfile> subfiles;
	for (auto& f : files) {
		subfiles.push_back({f.first, f.second});
	}

	int err;
	if ((err = nspre::write(subfiles, out_file))) {
		if (err == nspre::Error::FILE_OPEN_OUTPUT) {
			global.error_modal_text.str("");
			global.error_modal_text << "Can't create file \"" << out_file.string() << "\"";
		}
		else {
			global.error_modal_text.str("");
			global.error_modal_text << "Error writing to file \"" << out_file.string() << "\"";
		}

		std::fprintf(stderr, "%s\n", global.error_modal_text.str().c_str());
		ImGui::OpenPopup("Error");
		return;
	}

	std::printf("%zu files written to file \"%s\"\n", subfiles.size(), out_file.c_str());
}

void CreateWindow::drop_files(const PathList& path_list) {
	for (const std::filesystem::path& p : path_list) {
		files.push_back({p, std::string("\\levels\\placeholder\\") + p.filename().string()});
	}
}

void CreateWindow::drop_file(const std::filesystem::path& path) {
	files.push_back({path, std::string("\\levels\\placeholder\\") + path.filename().string()});
}

void CreateWindow::edit_popup() {
	if (edit_index < 0) {
		ImGui::CloseCurrentPopup();
	}

	if (edit_init) {
		std::strncpy(ipath_buffer, files[edit_index].second.c_str(), INPUTTEXT_BUFFER_SIZE);
		edit_init = false;
	}

	ImGui::Text("File: %s", files[edit_index].first.c_str());
	ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
	if (ImGui::InputText("###ipath", ipath_buffer, INPUTTEXT_BUFFER_SIZE, ImGuiInputTextFlags_EnterReturnsTrue)) {
		files[edit_index].second = ipath_buffer;
		edit_init = true;
		ImGui::CloseCurrentPopup();
	}
	ImGui::PopItemWidth();

	if (ImGui::Button("OK")) {
		files[edit_index].second = ipath_buffer;
		edit_init = true;
		ImGui::CloseCurrentPopup();
	}
	ImGui::SameLine();
	if (ImGui::Button("Cancel")) {
		edit_init = true;
		ImGui::CloseCurrentPopup();
	}
}

void CreateWindow::show() {
	if (do_create) {
		create_pre();
		do_create = false;
	}

	ImGui::SetNextWindowSizeConstraints({400,400}, {global.io->DisplaySize.x - 24,global.io->DisplaySize.y - 24});
	if (ImGui::BeginPopupModal("Save", 0, ImGuiWindowFlags_NoScrollbar)) {
		fb_save.show("out.pre");
		ImGui::EndPopup();
	}

	ImGui::SetNextWindowSizeConstraints({400,104}, {global.io->DisplaySize.x - 24,global.io->DisplaySize.y - 24});
	if (ImGui::BeginPopupModal("Edit", 0, ImGuiWindowFlags_NoScrollbar)) {
		edit_popup();
		ImGui::EndPopup();
	}

	ImGui::SetNextWindowSizeConstraints({400,400}, {global.io->DisplaySize.x - 24,global.io->DisplaySize.y - 24});
	if (ImGui::BeginPopupModal("Add file(s)", 0, ImGuiWindowFlags_NoScrollbar)) {
		fb_openmulti.show();
		ImGui::EndPopup();
	}

	bool create_popup = false;
	bool show_edit = false;
	bool add_files = false;
	bool show_about = false;
	if (ImGui::BeginMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("Add file(s)...")) {
				add_files = true;
			}
			if (ImGui::MenuItem("Save pre...", 0, false, files_ready())) {
				create_popup = true;
			}

			ImGui::Separator();
			if (global.show_debug) {
				if (ImGui::MenuItem("Hide ImGui debug log")) {
					global.show_debug = false;
				}
			}
			else {
				if (ImGui::MenuItem("Show ImGui debug log")) {
					global.show_debug = true;
				}
			}

			ImGui::Separator();
			if (ImGui::MenuItem("Quit")) {
				global.quit = true;
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Mode")) {
			if (ImGui::MenuItem("Extract", 0, global.open_mode)) {
				global.open_mode = true;
			}
			if (ImGui::MenuItem("Create", 0, !global.open_mode)) {
				global.open_mode = false;
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Help")) {
			if (ImGui::MenuItem("About nspre-gui...")) {
				show_about = true;
			}
			ImGui::EndMenu();
		}

		ImGui::EndMenuBar();
	}

	if (create_popup) {
		ImGui::OpenPopup("Save");
	}

	if (show_about) {
		ImGui::OpenPopup("About");
	}

	if (files.size()) {
		if (ImGui::BeginTable("files_to_add", 3, ImGuiTableFlags_Borders)) {
			ImGui::TableSetupColumn(" ", ImGuiTableColumnFlags_WidthFixed);
			ImGui::TableSetupColumn("File", ImGuiTableColumnFlags_WidthFixed);
			ImGui::TableSetupColumn("Internal Path");
			ImGui::TableHeadersRow();

			bool delete_element = false;
			int element_to_delete = 0;

			for (int i = 0; i < files.size(); ++i) {
				ImGui::TableNextColumn();
				ImGui::PushID(i);
				if (ImGui::Button("-")) {
					delete_element = true;
					element_to_delete = i;
				}
				if (ImGui::IsItemHovered(ImGuiHoveredFlags_Stationary)) {
					ImGui::SetTooltip("Remove file");
				}
				ImGui::SameLine();
				ImGui::TableNextColumn();
				ImGui::Text("%s", files[i].first.filename().c_str());
				if (ImGui::IsItemHovered()) {
					ImGui::SetTooltip("%s", files[i].first.c_str());
				}

				ImGui::TableNextColumn();
				if (ImGui::Button("\"")) {
					edit_index = i;
					show_edit = true;
				}
				if (ImGui::IsItemHovered()) {
					ImGui::SetTooltip("Edit internal path");
				}
				ImGui::PopID();
				ImGui::SameLine();
				ImGui::Dummy({0,0});
				ImGui::SameLine();
				if (files[i].second.empty()) {
					ImGui::TextColored({255,0,0,255}, "(empty)");
				}
				else {
					ImGui::Text("%s", files[i].second.c_str());
				}

				if (ImGui::IsItemHovered()) {
					ImGui::SetTooltip("Double click to edit internal path");
					if (ImGui::IsMouseDoubleClicked(0)) {
						edit_index = i;
						show_edit = true;
					}
				}
			}

			ImGui::EndTable();

			if (delete_element) {
				files.erase(files.begin() + element_to_delete);
			}
		}
	}

	if (show_edit && edit_index > -1) {
		ImGui::OpenPopup("Edit");
	}

	ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x / 2) - (ImGui::CalcTextSize(" + ").x / 2));
	if (ImGui::Button(" + ")) {
		add_files = true;
	}
	if (ImGui::IsItemHovered()) {
		ImGui::SetTooltip("Add file(s)");
	}

	if (add_files) {
		ImGui::OpenPopup("Add file(s)");
	}

}

CreateWindow::CreateWindow() : fb_save(out_file, do_create) {}

}

