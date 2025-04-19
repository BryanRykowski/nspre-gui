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

void FileBrowserOpenMulti::open_dir(const std::filesystem::path& path) {
	open_dir_base(path);
}

void FileBrowserOpenMulti::single_click(std::vector<Selector>& v, int i) {

}

void FileBrowserOpenMulti::double_click(const std::vector<Selector>& v, int i) {
	if (&v == &m_file_entries) {
		create_window.drop_file(v[i].first.path());
		do_init = true;
		ImGui::CloseCurrentPopup();
	}
	else {
		open_dir(v[i].first.path() / fs::path(""));
	}
}

void FileBrowserOpenMulti::init() {
	open_dir(fs::current_path());
	std::strncpy(m_fname_buffer, "(none)", INPUTTEXT_BUFFER_SIZE);
	do_init = false;
}

void FileBrowserOpenMulti::multi_select() {
	for (auto& req : msio->Requests) {
		if (req.Type  == ImGuiSelectionRequestType_SetAll) {
			for (auto& s : m_file_entries) {
				s.second = req.Selected;
			}
		}
		else if (req.Type == ImGuiSelectionRequestType_SetRange) {
			if (req.RangeFirstItem < req.RangeLastItem) {
				for (int i = req.RangeFirstItem; i <= req.RangeLastItem; ++i) {
					m_file_entries[i].second = req.Selected;
				}
			}
			else {
				for (int i = req.RangeFirstItem; i >= req.RangeLastItem; --i) {
					m_file_entries[i].second = req.Selected;
				}
			}
		}
	}
}

void FileBrowserOpenMulti::show() {
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

			std::stringstream epath;
			epath << e.path().filename().string() << "/";

			if (ImGui::Selectable(epath.str().c_str(), false, ImGuiSelectableFlags_DontClosePopups | ImGuiSelectableFlags_AllowDoubleClick, {0,0})) {

				if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
					double_click(m_dir_entries, i);
				}
			} 
		}

		valid_selection = false;

		msio = ImGui::BeginMultiSelect(ImGuiMultiSelectFlags_ClearOnClickVoid);
		multi_select();

		for (int i_ = 0; i_ < m_file_entries.size(); ++i_) {
			int i;
			if (m_sort_ascending) i = i_;
			else i = (m_file_entries.size() - 1) - i_;

			auto& e = m_file_entries[i].first;

			if (m_file_entries[i].second) {
				valid_selection = true;
			}

			if (e.path().filename().string()[0] == '.' && !m_show_hidden) {
				continue;
			}

			bool& selected = m_file_entries[i].second;

			ImGui::SetNextItemSelectionUserData(i);
			if (ImGui::Selectable(e.path().filename().c_str(), (bool*)&selected, ImGuiSelectableFlags_DontClosePopups | ImGuiSelectableFlags_AllowDoubleClick)) {
				if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
					double_click(m_file_entries, i);
				}
			}
		}

		ImGui::EndMultiSelect();
		multi_select();

		ImGui::EndListBox();
	}

	int select_count = 0;
	for (auto& f : m_file_entries) {
		if (f.second) {
			++select_count;
		}
	}

	ImGui::Text("%d file%s selected", select_count, select_count == 1 ? "" : "s");

	ImGui::BeginDisabled(!valid_selection);
	if (ImGui::Button("Open")) {
		for (auto& f : m_file_entries) {
			if (f.second) {
				create_window.drop_file(f.first.path());
			}
		}

		do_init = true;
		ImGui::CloseCurrentPopup();
	}

	ImGui::SameLine();
	ImGui::EndDisabled();

	if (ImGui::Button("Cancel")) {
		do_init = true;
		ImGui::CloseCurrentPopup();
	}
}

}
