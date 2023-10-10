#pragma once

#define VK_USE_PLATFORM_METAL_EXT
#include <vulkan/vulkan.h>
#include <defines.h>

typedef enum vulkan_device_support_flag_bits {
    VULKAN_DEVICE_SUPPORT_FLAG_NONE_BIT = 0x00,

    /** @brief Indicates if the device supports native dynamic topology (i.e. using Vulkan API >= 1.3). */
    VULKAN_DEVICE_SUPPORT_FLAG_NATIVE_DYNAMIC_TOPOLOGY_BIT = 0x01,

    /** @brief Indicates if this device supports dynamic topology. If not, the renderer will need to generate a separate pipeline per topology type. */
    VULKAN_DEVICE_SUPPORT_FLAG_DYNAMIC_TOPOLOGY_BIT = 0x02,
    VULKAN_DEVICE_SUPPORT_FLAG_LINE_SMOOTH_RASTERISATION_BIT = 0x04,
    /** @brief Indicates if the device supports native dynamic front-face swapping (i.e. using Vulkan API >= 1.3). */
    VULKAN_DEVICE_SUPPORT_FLAG_NATIVE_DYNAMIC_FRONT_FACE_BIT = 0x08,
    /** @brief Indicates if the device supports extension-based dynamic front-face swapping. */
    VULKAN_DEVICE_SUPPORT_FLAG_DYNAMIC_FRONT_FACE_BIT = 0x10,
} vulkan_device_support_flag_bits;

/** @brief Bitwise flags for device support. @see vulkan_device_support_flag_bits. */
typedef u32 vulkan_device_support_flags;

typedef struct v_swapchain_support_info {
    /** @brief The surface capabilities. */
    VkSurfaceCapabilitiesKHR capabilities;
    /** @brief The number of available surface formats. */
    u32 format_count;
    /** @brief An array of the available surface formats. */
    VkSurfaceFormatKHR* formats;
    /** @brief The number of available presentation modes. */
    u32 present_mode_count;
    /** @brief An array of available presentation modes. */
    VkPresentModeKHR* present_modes;
}v_swapchain_support_info;


typedef struct vDevice {
     /** @brief The supported device-level api major version. */
    u32 api_major;

    /** @brief The supported device-level api minor version. */
    u32 api_minor;

    /** @brief The supported device-level api patch version. */
    u32 api_patch;

    /** @brief The Physical Device. This represents the GPU.*/
    VkPhysicalDevice physical_device;
    /** @brief The Logical Device. It is used for most vulkan operations*/
    VkDevice logical_device;

    v_swapchain_support_info swapchain_support;

    i32 graphics_queue_index;
    i32 present_queue_index;
    i32 transfer_queue_index;

    bool supports_device_local_host_visible;

    VkQueue graphics_queue;
    VkQueue present_queue;
    VkQueue transfer_queue;

    /** @brief A handle to a command pool for graphics operations. */
    VkCommandPool graphics_command_pool;
    
    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceFeatures features;
    VkPhysicalDeviceMemoryProperties memory;

     /** @brief The chosen supported depth format. */
    VkFormat depth_format;
     /** @brief The chosen depth format's number of channels.*/
    u8 depth_channel_count;

    /** @brief Indicates support for various features. */
    vulkan_device_support_flags support_flags;
}vDevice;

typedef struct vContext {
    VkInstance instance;
     /** @brief The internal Vulkan surface for the window to be drawn to. */
    VkSurfaceKHR surface;

    vDevice device;

    #ifdef _DEBUG 
    VkDebugUtilsMessengerEXT debug_messenger;
    #endif

    PFN_vkCmdSetPrimitiveTopologyEXT vkCmdSetPrimitiveTopologyEXT;
    PFN_vkCmdSetFrontFaceEXT vkCmdSetFrontFaceEXT;
}vContext;

typedef struct v_image{
    VkImage handle;
    VkDeviceMemory memory;
    VkImageView view;
    VkMemoryRequirements memory_requirements;
    VkMemoryPropertyFlags memory_flags;
    u32 width;
    u32 height;
    char* name;
}v_image;