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

#include "imgui.h"
#include "nspre-gui.hpp"
#include <filesystem>
#include <vector>
#include <cstring>

namespace fs = std::filesystem;

namespace ns {

// Sort strings 0-9,Aa-Zz instead of 0-9,A-Z,a-z
int FileBrowserBase::compare_strings(const std::string& s0, const std::string& s1) {
	int len = s0.size();
	if (s1.size() < len) len = s1.size();
	int c0;
	int c1;
	for (int i = 0; i < len; ++i) {
		c0 = s0[i];
		c1 = s1[i];

		if (c0 > 122) {
			c0 += 133;
		}
		else if (c0 < 91 && c0 > 64) {
			c0 = (c0 + 58) + (c0 - 65);
		}
		else if (c0 < 123 && c0 > 96) {
			c0 = (c0 + 27) + (c0 - 97);
		}

		if (c1 > 122) {
			c1 += 133;
		}
		else if (c1 < 91 && c1 > 64) {
			c1 = (c1 + 58) + (c1 - 65);
		}
		else if (c1 < 123 && c1 > 96) {
			c1 = (c1 + 27) + (c1 - 97);
		}

		if (c0 > c1) return 1;
		else if (c0 < c1) return -1;
	}

	if (s0.size() > s1.size()) return 1;
	else if (s0.size() < s1.size()) return -1;

	return 0;
}

void FileBrowserBase::open_dir_base(std::filesystem::path path) {
	m_dir_entries.clear();
	m_file_entries.clear();
	m_previous_path = m_current_path;
	m_current_path = fs::canonical(path);
	fs::directory_iterator di(path);
	std::vector<fs::directory_entry> files;
	std::vector<fs::directory_entry> dirs;
	int ecount = 0;
	for (auto entry : di) {
		++ecount;
		if (entry.is_directory()) {
			dirs.push_back(entry);
		}
		else if (entry.is_regular_file()) {
			files.push_back(entry);
		}
	}

	bool unsorted;
	do {
		unsorted = false;
		for (int i = 0; i < (int)dirs.size() - 1; ++i) {
			int r = compare_strings(dirs[i].path().filename().string(), dirs[i + 1].path().filename().string());
			if (r > 0) {
				std::swap(dirs[i], dirs[i + 1]);
				unsorted = true;
			}
		}
	}
	while (unsorted);

	do {
		unsorted = false;
		for (int i = 0; i < (int)files.size() - 1; ++i) {
			int r = compare_strings(files[i].path().filename().string(), files[i + 1].path().filename().string());
			if (r > 0) { 
				std::swap(files[i], files[i + 1]);
				unsorted = true;
			}
		}
	}
	while (unsorted);

	for (auto e : dirs) {
		m_dir_entries.push_back({e,false});
	}

	for (auto e : files) {
		m_file_entries.push_back({e,false});
	}
}

void FileBrowserBase::show_top_region() {
	ImGui::BeginDisabled(m_current_path == "/");
	if (ImGui::Button("Up")) {
		open_dir(m_current_path.parent_path());
	}
	ImGui::SameLine();
	ImGui::EndDisabled();
	ImGui::BeginDisabled(m_previous_path.empty());
	if (ImGui::Button("Back")) {
		open_dir(m_previous_path);
	}
	ImGui::SameLine();
	ImGui::EndDisabled();
	if (ImGui::Button("Refresh")) {
		open_dir(m_current_path);
	}
	ImGui::SameLine();
	ImGui::Checkbox("Show hidden", &m_show_hidden);
	ImGui::SameLine();
	ImGui::Checkbox("Ascending", &m_sort_ascending);

	ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
	ImGui::Text("%s", m_current_path.c_str());
	ImGui::PopItemWidth();
}

}
