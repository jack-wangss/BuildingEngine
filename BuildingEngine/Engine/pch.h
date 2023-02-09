#pragma once

// std
#include <array>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <set>

// glfw
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// vulkan
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

// imgui
#include <imgui.h>
#include <imgui_internal.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>