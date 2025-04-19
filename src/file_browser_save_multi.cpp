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

namespace fs = std::filesystem;

namespace ns {

void FileBrowserSaveMulti::open_dir(const std::filesystem::path& path) {
	open_dir_base(path);
	m_selected_path = m_current_path;
	std::strncpy(m_fname_buffer, m_selected_path.filename().c_str(), INPUTTEXT_BUFFER_SIZE);
}

void FileBrowserSaveMulti::init() {
	open_dir(fs::current_path());
	std::strncpy(m_fname_buffer, m_current_path.filename().c_str(), INPUTTEXT_BUFFER_SIZE);
	do_init = false;
}

void FileBrowserSaveMulti::single_click(std::vector<Selector>& v, int i) {
	m_selected_path = v[i].first.path();
	std::strncpy(m_fname_buffer, v[i].first.path().filename().c_str(), INPUTTEXT_BUFFER_SIZE);
	for (auto& e : v) {
		e.second = false;
	}

	v[i].second = true;
}

void FileBrowserSaveMulti::double_click(const std::vector<Selector>& v, int i) {
	if (&v == &m_dir_entries) {
		open_dir(v[i].first.path() / fs::path(""));
	}
}

void FileBrowserSaveMulti::show() {
	if (do_init) {
		init();
	}

	show_top_region();

	ImVec2 list_size = ImGui::GetContentRegionAvail();
	list_size.y = ImGui::GetContentRegionAvail().y - ImGui::GetFrameHeight() * 2.5;

	if (ImGui::BeginListBox("###dirlist", list_size)) {
		if (m_current_path != "/") {
			if (ImGui::Selectable("..", false, ImGuiSelectableFlags_DontClosePopups | ImGuiSelectableFlags_AllowDoubleClick)) {
				if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
					open_dir(m_current_path.parent_path());
				}
			}
		}

		for (int i_ = 0; i_ < m_dir_entries.size(); ++i_) {
			int i;
			if (m_sort_ascending) i = i_;
			else i = (m_dir_entries.size() - 1) - i_;
			auto& e = m_dir_entries[i].first;

			if (e.path().filename().string()[0] == '.' && !m_show_hidden) {
				continue;
			}

			bool selected = m_dir_entries[i].second;

			std::stringstream epath;
			epath << e.path().filename().string() << "/";

			if (ImGui::Selectable(epath.str().c_str(), selected, ImGuiSelectableFlags_DontClosePopups | ImGuiSelectableFlags_AllowDoubleClick, {0,0})) {
				single_click(m_dir_entries, i);

				if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
					double_click(m_dir_entries, i);
				}
			} 
		}

		for (int i_ = 0; i_ < m_file_entries.size(); ++i_) {
			int i;
			if (m_sort_ascending) i = i_;
			else i = (m_file_entries.size() - 1) - i_;

			auto& e = m_file_entries[i].first;

			if (e.path().filename().string()[0] == '.' && !m_show_hidden) {
				continue;
			}

			if (ImGui::Selectable(e.path().filename().c_str(), false, ImGuiSelectableFlags_DontClosePopups | ImGuiSelectableFlags_AllowDoubleClick, {0,0})) {

			}
		}

		ImGui::EndListBox();
	}

	ImGui::Text("Directory");
	ImGui::SameLine();
	ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
	ImGui::InputText("###selectedpath", m_fname_buffer, INPUTTEXT_BUFFER_SIZE, ImGuiInputTextFlags_ReadOnly);
	ImGui::PopItemWidth();

	if (ImGui::Button("Extract")) {
		extract_window.extract_pre(m_selected_path);
		do_init = true;
		ImGui::CloseCurrentPopup();
	}

	ImGui::SameLine();

	if (ImGui::Button("Cancel")) {
		do_init = true;
		ImGui::CloseCurrentPopup();
	}
}

}
