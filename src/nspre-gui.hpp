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

#pragma once
#include "imgui.h"
#include "nspre.hpp"
#include <filesystem>
#include <vector>

namespace ns {

static const size_t INPUTTEXT_BUFFER_SIZE = 256;

typedef std::vector<std::filesystem::path> PathList;
typedef std::pair<std::filesystem::path,std::string> FileEntry;
typedef std::pair<std::filesystem::directory_entry,bool> Selector;

class FileBrowserBase {
protected:
	std::filesystem::path m_current_path;
	std::filesystem::path m_previous_path;
	std::vector<Selector> m_dir_entries;
	std::vector<Selector> m_file_entries;
	char m_fname_buffer[INPUTTEXT_BUFFER_SIZE + 1] = {};
	bool m_show_hidden = false;
	bool m_sort_ascending = true;
	bool valid_selection = false;
	bool do_init = true;

	int compare_strings(const std::string& s0, const std::string& s1);
	void open_dir_base(std::filesystem::path path);
	void show_top_region();
	virtual void open_dir(const std::filesystem::path& path){}
	virtual void single_click(std::vector<Selector>& v, int i){}
	virtual void double_click(const std::vector<Selector>& v, int i){}
	virtual void init(){}
public:
	virtual void show(){}
	FileBrowserBase(){}
};

class FileBrowserOpenMulti : FileBrowserBase {
	ImGuiMultiSelectIO* msio;

	void open_dir(const std::filesystem::path& path);
	void single_click(std::vector<Selector>& v, int i);
	void double_click(const std::vector<Selector>& v, int i);
	void multi_select();
	void init();
public:
	void show();
};

class FileBrowserOpenOne : FileBrowserBase {
	bool filter = true;

	void open_dir(const std::filesystem::path& path);
	void single_click(std::vector<Selector>& v, int i);
	void double_click(const std::vector<Selector>& v, int i);
	void init();
public:
	void show();
};

class FileBrowserSaveMulti : FileBrowserBase {
	std::filesystem::path m_selected_path;

	void open_dir(const std::filesystem::path& path);
	void single_click(std::vector<Selector>& v, int i);
	void double_click(const std::vector<Selector>& v, int i);
	void init();
public:
	void show();
};

class FileBrowserSaveOne : FileBrowserBase {
	std::filesystem::path& out_file;
	bool& do_var;

	void open_dir(const std::filesystem::path& path);
	void single_click(std::vector<Selector>& v, int i);
	void double_click(const std::vector<Selector>& v, int i);
	void init(const std::filesystem::path& filename);
public:
	FileBrowserSaveOne(std::filesystem::path& path, bool& do_var_set);
	~FileBrowserSaveOne(){}
	void show(const std::filesystem::path& filename);
};

class ExtractWindow {
	FileBrowserOpenOne fb_open;
	FileBrowserSaveMulti fb_saveall;
	FileBrowserSaveOne fb_saveone;
	nspre::Reader pre_reader;
	std::filesystem::path in_file;
	std::filesystem::path old_in_file;
	std::filesystem::path out_dir;
	std::filesystem::path csv_out;
	bool do_open = false;
	bool do_extract = false;
	bool do_csv = false;

	void extract_files();
	void int_export_csv();
	void int_open_pre();
public:
	ExtractWindow();
	void show();
	bool pre_is_open();
	void open_pre(const std::filesystem::path& path);
	void extract_pre(const std::filesystem::path& path);
	void export_csv(const std::filesystem::path& path);
	void close_pre();
};

class CreateWindow {
	FileBrowserSaveOne fb_save;
	FileBrowserOpenMulti fb_openmulti;
	std::filesystem::path out_file;
	char ipath_buffer[INPUTTEXT_BUFFER_SIZE + 1] = {};
	std::vector<FileEntry> files;
	int edit_index = -1;
	bool do_create = false;
	bool edit_init = true;

	void create_pre();
	bool files_ready();
	void edit_popup();
public:
	CreateWindow();
	~CreateWindow(){}
	void show();
	void drop_files(const PathList& pathlist);
	void drop_file(const std::filesystem::path& path);
};

struct GlobalStruct {
	ImGuiIO* io;
	std::stringstream error_modal_text;
	bool show_demo_window = false;
	bool show_debug = false;
	bool open_mode = true;
	bool quit = false;
};

extern GlobalStruct global;
extern ExtractWindow extract_window;
extern CreateWindow create_window;

void open_pre(const std::filesystem::path& path);
void popup_proc();
}
