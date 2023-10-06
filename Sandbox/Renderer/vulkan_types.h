#pragma once

#include <vulkan/vulkan.h>
#include <defines.h>

typedef struct vDevice {
    /** @brief The Physical Device. This represents the GPU.*/
    VkPhysicalDevice physical_device;
    /** @brief The Logical Device. It is used for most vulkan operations*/
    VkDevice logical_device;
}vDevice;

typedef struct vContext {
    VkInstance instance;

    vDevice device;

    #ifdef _DEBUG 
    VkDebugUtilsMessengerEXT debug_messenger;
    #endif
}vContext;