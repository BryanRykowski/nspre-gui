cmake_minimum_required (VERSION 3.18.4)
project (nspre_gui_proj VERSION 1.0.1)

find_package(SDL2 REQUIRED)

add_compile_definitions(NSPRE_GUI_VERSION="${CMAKE_PROJECT_VERSION}")

add_executable(nspre-gui)

target_sources(nspre-gui PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/file_browser_base.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/file_browser_save_multi.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/file_browser_open_multi.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/file_browser_save_one.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/file_browser_open_one.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/extract_window.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/create_window.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/nspre-gui.hpp
)

target_include_directories(nspre-gui PRIVATE ${SDL2_INCLUDE_DIRS})
target_link_libraries(nspre-gui PRIVATE ${SDL2_LIBRARIES} GL)

target_include_directories(nspre-gui PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/imgui
	${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends
	${CMAKE_CURRENT_SOURCE_DIR}/nspre
)

target_sources(nspre-gui PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_demo.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_draw.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_tables.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_widgets.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_opengl3.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_sdl2.cpp
)

