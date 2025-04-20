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

bool ExtractWindow::pre_is_open() {
	return pre_reader.error() == 0;
}

void ExtractWindow::close_pre() {
	pre_reader.close();
}

void ExtractWindow::open_pre(const std::filesystem::path& path) {
	in_file = path;
	do_open = true;
}

void ExtractWindow::int_export_csv() {
	std::ofstream stream(csv_out);
	if (stream.fail()) {
		global.error_modal_text.str("Can't create file \"");
		global.error_modal_text << csv_out.string() << "\"";
		std::fprintf(stderr, "%s\n", global.error_modal_text.str().c_str());
		return;
	}

	for (int i = 0; i < pre_reader.files().size(); ++i) {
		stream << std::string(pre_reader.files()[i].filename().data()) << ",";
		stream << pre_reader.files()[i].cmp_size() << ",";
		stream << pre_reader.files()[i].size() << ",";
		stream << std::string(pre_reader.files()[i].prepath().data()) << ",";
		if (i + 1 < pre_reader.files().size()) {
			stream << std::endl;
		}
		if (stream.fail()) {
			global.error_modal_text.str("Can't write to file \"");
			global.error_modal_text << csv_out.string() << "\"";
			std::fprintf(stderr, "%s\n", global.error_modal_text.str().c_str());
			return;
		}
	}
}

void ExtractWindow::export_csv(const std::filesystem::path& path) {
	csv_out = path;
	do_csv = true;
}

void ExtractWindow::int_open_pre() {
	if (pre_reader.error() == 0 && in_file == old_in_file) {
		global.error_modal_text.str("");
		global.error_modal_text << "File \"" << old_in_file.c_str() << "\" is already open";
		std::fprintf(stderr, "%s\n", global.error_modal_text.str().c_str());
		ImGui::OpenPopup("Error");
		return;
	}

	old_in_file = in_file;
	int err = pre_reader.open(in_file);
	if (err) {
		pre_reader.close();
		global.error_modal_text.str("");
		if (err == nspre::Error::FILE_OPEN) {
			global.error_modal_text << "Can't open file \"" << std::string(in_file) << "\"";
		}
		else {
			global.error_modal_text << "File \"" << std::string(in_file) << "\" is corrupted or not a pre/prx file";
		}
		std::fprintf(stderr, "%s\n", global.error_modal_text.str().c_str());
		ImGui::OpenPopup("Error");
		return;
	}

	std::printf("File \"%s\" opened, containing %zu files\n", in_file.c_str(), pre_reader.files().size());
}

void ExtractWindow::extract_pre(const std::filesystem::path& path) {
	out_dir = path;
	do_extract = true;
}

void ExtractWindow::extract_files() {
	if ((pre_reader.error() != 0) || pre_reader.files().size() == 0) {
		global.error_modal_text.str("No file open");
		std::fprintf(stderr, "%s\n", global.error_modal_text.str().c_str());
		ImGui::OpenPopup("Error");
		return;
	}

	int err;
	for (int i = 0; i < pre_reader.files().size(); ++i) {
		if ((err = pre_reader.files()[i].extract(out_dir / pre_reader.files()[i].filename()))) {
			if (err == nspre::Error::FILE_OPEN_OUTPUT) {
				global.error_modal_text.str("Can't create file \"");
				global.error_modal_text << std::string(out_dir / pre_reader.files()[i].filename()) << "\"";
			}
			else {
				global.error_modal_text.str("Error extracting file \"");
				global.error_modal_text << pre_reader.files()[i].filename() << "\"";
			}

			std::fprintf(stderr, "%s\n", global.error_modal_text.str().c_str());
			ImGui::OpenPopup("Error");
			return;
		}
	}

	std::printf("%zu files extracted from file \"%s\" to location \"%s\"\n", pre_reader.files().size(), in_file.c_str(), out_dir.c_str());
}

void ExtractWindow::show() {
	if (do_open) {
		int_open_pre();
		do_open = false;
	}

	if (do_csv) {
		int_export_csv();
		do_csv = false;
	}

	if (do_extract) {
		extract_files();
		do_extract = false;
	}

	ImGui::SetNextWindowSizeConstraints({400,400}, {global.io->DisplaySize.x - 24,global.io->DisplaySize.y - 24});
	if (ImGui::BeginPopupModal("Open", 0, ImGuiWindowFlags_NoScrollbar)) {
		fb_open.show();
		ImGui::EndPopup();
	}

	ImGui::SetNextWindowSizeConstraints({400,400}, {global.io->DisplaySize.x - 24,global.io->DisplaySize.y - 24});
	if (ImGui::BeginPopupModal("Select directory...", 0, ImGuiWindowFlags_NoScrollbar)) {
		fb_saveall.show();
		ImGui::EndPopup();
	}

	ImGui::SetNextWindowSizeConstraints({400,400}, {global.io->DisplaySize.x - 24,global.io->DisplaySize.y - 24});
	if (ImGui::BeginPopupModal("Export csv", 0, ImGuiWindowFlags_NoScrollbar)) {
		fb_saveone.show(fs::path(in_file.filename().string() + ".csv"));
		ImGui::EndPopup();
	}

	bool open_file = false;
	bool select_dir = false;
	bool export_csv = false;
	bool show_about = false;

	if (ImGui::BeginMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("Open pre/prx...")) {
				open_file = true;
			}
			if (ImGui::MenuItem("Extract...", 0, false, extract_window.pre_is_open())) {
				select_dir = true;
			}
			if (ImGui::MenuItem("Export csv...", 0, false, extract_window.pre_is_open())) {
				export_csv = true;
			}
			if (ImGui::MenuItem("Close", 0, false, extract_window.pre_is_open())) {
				extract_window.close_pre();
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

	if (show_about) {
		ImGui::OpenPopup("About");
	}

	if (pre_is_open()) {
		ImGui::Text("%s", in_file.c_str());
	}
	else {
		if (ImGui::Button("Open pre/prx...")) {
			open_file = true;
		}
	}
	if (pre_reader.files().size()) {
		if (ImGui::BeginTable("extract_table", 4, ImGuiTableFlags_Borders)) {
			ImGui::TableSetupColumn("File", ImGuiTableColumnFlags_WidthFixed);
			ImGui::TableSetupColumn("Compressed Size", ImGuiTableColumnFlags_WidthFixed);
			ImGui::TableSetupColumn("Size", ImGuiTableColumnFlags_WidthFixed);
			ImGui::TableSetupColumn("Path");
			ImGui::TableHeadersRow();

			for (auto& file : pre_reader.files()) {
				ImGui::TableNextColumn();
				ImGui::Text("%s", file.filename().c_str());
				ImGui::TableNextColumn();
				ImGui::Text("%d", file.cmp_size());
				ImGui::TableNextColumn();
				ImGui::Text("%d", file.size());
				ImGui::TableNextColumn();
				ImGui::Text("%s", file.prepath().c_str());
			}

			ImGui::EndTable();
		}
	}

	if (open_file) ImGui::OpenPopup("Open");
	if (select_dir) ImGui::OpenPopup("Select directory...");
	if (export_csv) ImGui::OpenPopup("Export csv");
}

ExtractWindow::ExtractWindow() : fb_saveone(csv_out, do_csv) {

}

}

