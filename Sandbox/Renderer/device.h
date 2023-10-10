#pragma once

#include "vulkan_types.h"

bool v_device_create(vContext* context);
void v_device_destroy(vContext* context);
void v_device_query_swapchain_support(
    VkPhysicalDevice physical_device,
    VkSurfaceKHR surface,
    v_swapchain_support_info* out_support_info);
