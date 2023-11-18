cmake_minimum_required (VERSION 3.13)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project (Orbit VERSION 0.1)
configure_file(src/config.h.in config.h)

add_executable(${PROJECT_NAME}
	"src/main.cpp"
	"src/convertfloat.cpp"
	"src/convertfloat.h"
	"src/env_32_64.h"
	"src/oscillator.cpp"
	"src/oscillator.h"
	"src/util.cpp"
	"src/util.h"
	"src/wavplayer.cpp"
	"src/wavplayer.h"
)

#ASIO
set(ASIO_sdk "asiosdk_2.3.3_2019-06-14")
add_subdirectory(${ASIO_sdk})

#INTERNAL
add_subdirectory(src)
target_link_libraries(${PROJECT_NAME}
	engine
	driver
	wav
	wavetable
)

target_include_directories(${PROJECT_NAME}
	PUBLIC
		"${PROJECT_BINARY_DIR}"
		"${PROJECT_SOURCE_DIR}/src"
		"${PROJECT_SOURCE_DIR}/Walnut/src"
		"${PROJECT_SOURCE_DIR}/vendor/imgui"
		"${PROJECT_SOURCE_DIR}/vendor/glfw/include"
		"$ENV{VULKAN_SDK}/Include"
		"${PROJECT_SOURCE_DIR}/vendor/glm/"
		"$ENV{CPAL_ASIO_DIR}/common"
		"$ENV{CPAL_ASIO_DIR}/host"
		"$ENV{CPAL_ASIO_DIR}/host/pc"
)


#Walnut
include_external_msproject(Walnut 
	"${PROJECT_SOURCE_DIR}/Walnut/Walnut.vcxproj"
	GUID C038E8D9-ACDA-12B0-9595-260481D69900
	PLATFORM x64
)
add_dependencies(${PROJECT_NAME} Walnut)
target_link_libraries(${PROJECT_NAME}
		"${PROJECT_SOURCE_DIR}/Walnut/bin/Debug-windows-x86_64/Walnut/Walnut.lib"
		"${PROJECT_SOURCE_DIR}/vendor/imgui/bin/Debug-windows-x86_64/ImGui/ImGui.lib"
		"${PROJECT_SOURCE_DIR}/vendor/GLFW/bin/Debug-windows-x86_64/GLFW/GLFW.lib"
)

# Windows
set(WIN_INCLUDE "C:\\Program Files (x86)\\Windows Kits\\10\\Include\\10.0.19041.0\\um")
target_include_directories(${PROJECT_NAME} PUBLIC "${WIN_INCLUDE}")

find_library(winmm_LIB_PATH winmm)
target_link_libraries(${PROJECT_NAME} ${winmm_LIB_PATH})

find_library(audioeng_LIB_PATH audioeng)
target_link_libraries(${PROJECT_NAME} ${audioeng_LIB_PATH})

find_library(avrt_LIB_PATH avrt)
target_link_libraries(${PROJECT_NAME} ${avrt_LIB_PATH})