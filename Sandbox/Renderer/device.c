#include "device.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../Containers/dynlist.h"

typedef struct v_physical_device_requirements {

    bool graphics;
    bool present;
    bool compute;
    bool transfer;
    //list
    const char** device_extension_names;

    bool sampler_anisotropy;
    bool discrete_gpu;

}v_physical_device_requirements;

typedef struct v_physical_device_queue_family_info {

    i32 graphics_family_index;
    i32 present_family_index;
    i32 compute_family_index;
    i32 transfer_family_index;

}v_physical_device_queue_family_info;

static bool physical_device_meets_requirements(
    VkPhysicalDevice device,
    VkSurfaceKHR surface,
    const VkPhysicalDeviceProperties* properties,
    const VkPhysicalDeviceFeatures* features,
    const v_physical_device_requirements* requirements,
    v_physical_device_queue_family_info* out_queue_info,
    v_swapchain_support_info* out_swapchain_support){

    // Evaluate device properties to determine if it meets the needs of our applcation.
    out_queue_info->graphics_family_index = -1;
    out_queue_info->present_family_index = -1;
    out_queue_info->compute_family_index = -1;
    out_queue_info->transfer_family_index = -1;

    // Discrete GPU?
    if (requirements->discrete_gpu) {
        if (properties->deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            printf("Device is not a discrete GPU, and one is required. Skipping.\n");
            return false;
        }
    }

    u32 queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, 0);
    VkQueueFamilyProperties queue_families[32];
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families);

    // Look at each queue and see what queues it supports
    printf("Graphics | Present | Compute | Transfer | Name\n");
    u8 min_transfer_score = 255;
    for (u32 i = 0; i < queue_family_count; ++i) {
        u8 current_transfer_score = 0;

        // Graphics queue?
        if (out_queue_info->graphics_family_index == -1 && queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            out_queue_info->graphics_family_index = i;
            ++current_transfer_score;

            // If also a present queue, this prioritizes grouping of the 2.
            VkBool32 supports_present = VK_FALSE;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &supports_present);
            if (supports_present) {
                out_queue_info->present_family_index = i;
                ++current_transfer_score;
            }
        }

        // Compute queue?
        if (queue_families[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
            out_queue_info->compute_family_index = i;
            ++current_transfer_score;
        }

        // Transfer queue?
        if (queue_families[i].queueFlags & VK_QUEUE_TRANSFER_BIT) {
            // Take the index if it is the current lowest. This increases the
            // liklihood that it is a dedicated transfer queue.
            if (current_transfer_score <= min_transfer_score) {
                min_transfer_score = current_transfer_score;
                out_queue_info->transfer_family_index = i;
            }
        }
    }

    // If a present queue hasn't been found, iterate again and take the first one.
    // This should only happen if there is a queue that supports graphics but NOT
    // present.
    if (out_queue_info->present_family_index == -1) {
        for (u32 i = 0; i < queue_family_count; ++i) {
            VkBool32 supports_present = VK_FALSE;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &supports_present);
            if (supports_present) {
                out_queue_info->present_family_index = i;

                // If they differ, bleat about it and move on. This is just here for troubleshooting
                // purposes.
                if (out_queue_info->present_family_index != out_queue_info->graphics_family_index) {
                    printf("Warning: Different queue index used for present vs graphics: %u.\n", i);
                }
                break;
            }
        }
    }

     // Print out some info about the device
    printf("       %d |       %d |       %d |        %d | %s\n",
          out_queue_info->graphics_family_index != -1,
          out_queue_info->present_family_index != -1,
          out_queue_info->compute_family_index != -1,
          out_queue_info->transfer_family_index != -1,
          properties->deviceName);

    if (
        (!requirements->graphics || (requirements->graphics && out_queue_info->graphics_family_index != -1)) &&
        (!requirements->present || (requirements->present && out_queue_info->present_family_index != -1)) &&
        (!requirements->compute || (requirements->compute && out_queue_info->compute_family_index != -1)) &&
        (!requirements->transfer || (requirements->transfer && out_queue_info->transfer_family_index != -1))) {
        printf("Device meets queue requirements.\n");
        printf("Graphics Family Index: %i\n", out_queue_info->graphics_family_index);
        printf("Present Family Index:  %i\n", out_queue_info->present_family_index);
        printf("Transfer Family Index: %i\n", out_queue_info->transfer_family_index);
        printf("Compute Family Index:  %i\n", out_queue_info->compute_family_index);

        // Query swapchain support.
        v_device_query_swapchain_support(
            device,
            surface,
            out_swapchain_support);

        if (out_swapchain_support->format_count < 1 || out_swapchain_support->present_mode_count < 1) {
            if (out_swapchain_support->formats) {
                // free(out_swapchain_support->formats);
            }
            if (out_swapchain_support->present_modes) {
                // free(out_swapchain_support->present_modes);
            }
            printf("Required swapchain support not present, skipping device.\n");
            return false;
        }

        // Device extensions.
        if (requirements->device_extension_names) {
            u32 available_extension_count = 0;
            VkExtensionProperties* available_extensions = 0;
            vkEnumerateDeviceExtensionProperties(
                device,
                0,
                &available_extension_count,
                0);
            if (available_extension_count != 0) {
                available_extensions = malloc(sizeof(VkExtensionProperties) * available_extension_count);
                memset(available_extensions, 0, sizeof(VkExtensionProperties) * available_extension_count);
                vkEnumerateDeviceExtensionProperties(
                    device,
                    0,
                    &available_extension_count,
                    available_extensions);

                u32 required_extension_count = dynlist_length(requirements->device_extension_names);
                for (u32 i = 0; i < required_extension_count; ++i) {
                    bool found = false;
                    for (u32 j = 0; j < available_extension_count; ++j) {
                        if ((strcmp(requirements->device_extension_names[i], available_extensions[j].extensionName) == 0)) {
                            found = true;
                            break;
                        }
                    }

                    if (!found) {
                        printf("Required extension not found: '%s', skipping device.\n", requirements->device_extension_names[i]);
                        free(available_extensions);
                        return false;
                    }
                }
            }
            free(available_extensions);
        }

        // Sampler anisotropy
        if (requirements->sampler_anisotropy && !features->samplerAnisotropy) {
            printf("Device does not support samplerAnisotropy, skipping.\n");
            return false;
        }

        // Device meets all requirements.
        return true;
    }
    
    return false;
}

static bool select_physical_device(vContext* context) {
    u32 physical_device_count = 0;
    vkEnumeratePhysicalDevices(context->instance, &physical_device_count, 0);
    if (physical_device_count == 0) {
        printf("No devices which support Vulkan were found.\n");
        return false;
    }

    // Setup requirements
    // TODO: These requirements should probably be driven by engine
    // configuration.
    v_physical_device_requirements requirements = {};
    requirements.graphics = true;
    requirements.present = true;
    requirements.transfer = true;
    // NOTE: Enable this if compute will be required.
    // requirements.compute = true;
    requirements.sampler_anisotropy = true;
#if __APPLE__
    requirements.discrete_gpu = false;
#else
    requirements.discrete_gpu = true;
#endif
    requirements.device_extension_names = dynlist_create(const char*);
    dynlist_push(requirements.device_extension_names, &VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    // Iterate physical devices to find one that fits the bill.
    VkPhysicalDevice physical_devices[32];
    vkEnumeratePhysicalDevices(context->instance, &physical_device_count, physical_devices);
    for (u32 i = 0; i < physical_device_count; ++i) {
        VkPhysicalDeviceProperties2 properties2 = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2};
        VkPhysicalDeviceDriverProperties driverProperties = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRIVER_PROPERTIES};
        properties2.pNext = &driverProperties;
        vkGetPhysicalDeviceProperties2(physical_devices[i], &properties2);
        VkPhysicalDeviceProperties properties = properties2.properties;

        VkPhysicalDeviceFeatures features;
        vkGetPhysicalDeviceFeatures(physical_devices[i], &features);

        VkPhysicalDeviceFeatures2 features2 = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};
        // Check for dynamic topology support via extension.
        VkPhysicalDeviceExtendedDynamicStateFeaturesEXT dynamic_state_next = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT};
        features2.pNext = &dynamic_state_next;
        // Check for smooth line rasterisation support via extension.
        VkPhysicalDeviceLineRasterizationFeaturesEXT smooth_line_next = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LINE_RASTERIZATION_FEATURES_EXT};
        dynamic_state_next.pNext = &smooth_line_next;
        // Perform the query.
        vkGetPhysicalDeviceFeatures2(physical_devices[i], &features2);

        VkPhysicalDeviceMemoryProperties memory;
        vkGetPhysicalDeviceMemoryProperties(physical_devices[i], &memory);

        printf("Evaluating device: '%s', index %u.\n", properties.deviceName, i);

        // Check if device supports local/host visible combo
        bool supports_device_local_host_visible = false;
        for (u32 i = 0; i < memory.memoryTypeCount; ++i) {
            // Check each memory type to see if its bit is set to 1.
            if (
                ((memory.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0) &&
                ((memory.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) != 0)) {
                supports_device_local_host_visible = true;
                break;
            }
        }

        v_physical_device_queue_family_info queue_info = {};
        bool result = physical_device_meets_requirements(
            physical_devices[i],
            context->surface,
            &properties,
            &features,
            &requirements,
            &queue_info,
            &context->device.swapchain_support);

        if (result) {
            printf("Selected device: '%s'.\n", properties.deviceName);
            // GPU type, etc.
            switch (properties.deviceType) {
                default:
                case VK_PHYSICAL_DEVICE_TYPE_OTHER:
                    printf("GPU type is Unknown.\n");
                    break;
                case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
                    printf("GPU type is Integrated.\n");
                    break;
                case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
                    printf("GPU type is Descrete.\n");
                    break;
                case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
                    printf("GPU type is Virtual.\n");
                    break;
                case VK_PHYSICAL_DEVICE_TYPE_CPU:
                    printf("GPU type is CPU.\n");
                    break;
            }

            printf("GPU Driver version: %s\n", driverProperties.driverInfo);

            // Save off the device-supported API version.
            context->device.api_major = VK_VERSION_MAJOR(properties.apiVersion);
            context->device.api_minor = VK_VERSION_MINOR(properties.apiVersion);
            context->device.api_patch = VK_VERSION_PATCH(properties.apiVersion);

            // Vulkan API version.
            printf(
                "Vulkan API version: %d.%d.%d\n",
                context->device.api_major,
                context->device.api_minor,
                context->device.api_minor);

            // Memory information
            for (u32 j = 0; j < memory.memoryHeapCount; ++j) {
                f32 memory_size_gib = (((f32)memory.memoryHeaps[j].size) / 1024.0f / 1024.0f / 1024.0f);
                if (memory.memoryHeaps[j].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
                    printf("Local GPU memory: %.2f GiB\n", memory_size_gib);
                } else {
                    printf("Shared System memory: %.2f GiB\n", memory_size_gib);
                }
            }

            context->device.physical_device = physical_devices[i];
            context->device.graphics_queue_index = queue_info.graphics_family_index;
            context->device.present_queue_index = queue_info.present_family_index;
            context->device.transfer_queue_index = queue_info.transfer_family_index;
            // NOTE: set compute index here if needed.

            // Keep a copy of properties, features and memory info for later use.
            context->device.properties = properties;
            context->device.features = features;
            context->device.memory = memory;
            context->device.supports_device_local_host_visible = supports_device_local_host_visible;

            // The device may or may not support this, so save that here.
            if (dynamic_state_next.extendedDynamicState) {
                context->device.support_flags |= VULKAN_DEVICE_SUPPORT_FLAG_DYNAMIC_TOPOLOGY_BIT;
                context->device.support_flags |= VULKAN_DEVICE_SUPPORT_FLAG_DYNAMIC_FRONT_FACE_BIT;
            }
            if (context->device.api_major > 1 && context->device.api_minor > 2) {
                context->device.support_flags |= VULKAN_DEVICE_SUPPORT_FLAG_NATIVE_DYNAMIC_TOPOLOGY_BIT;
                context->device.support_flags |= VULKAN_DEVICE_SUPPORT_FLAG_NATIVE_DYNAMIC_FRONT_FACE_BIT;
            }
            if (smooth_line_next.smoothLines) {
                context->device.support_flags |= VULKAN_DEVICE_SUPPORT_FLAG_LINE_SMOOTH_RASTERISATION_BIT;
            }
            break;
        }
    }

    // Clean up requirements.
    dynlist_destroy(requirements.device_extension_names);

    // Ensure a device was selected
    if (!context->device.physical_device) {
        printf("No physical devices were found which meet the requirements.\n");
        return false;
    }

    printf("Physical device selected.\n");
    return true;
}

bool v_device_create(vContext* context){
    if (!select_physical_device(context)){
        return false;
    }

    printf("Creating Logical Device...\n");
    //NOTE: Do not create additional Queues for shared indices
    bool present_shares_graphics_queue = context->device.graphics_queue_index == context->device.present_queue_index;
    bool transfer_shares_graphics_queue = context->device.graphics_queue_index == context->device.transfer_queue_index;
    u32 index_count = 1;

    if(!present_shares_graphics_queue){
        printf("Doesnt Share present\n");
        index_count++;
    }

    if(!transfer_shares_graphics_queue){
         printf("Doesnt Share transfer\n");
        index_count++;
    }

    u32 indices[32];
    u8 index = 0;

    indices[index++] = context->device.graphics_queue_index;
    if (!present_shares_graphics_queue) {
        indices[index++] = context->device.present_queue_index;
    }
    if (!transfer_shares_graphics_queue) {
        indices[index++] = context->device.transfer_queue_index;
    }

    VkDeviceQueueCreateInfo queue_create_infos[32];
    f32 queue_priority = 1.0f;
    for (u32 i = 0; i < index_count; ++i) {
        queue_create_infos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_infos[i].queueFamilyIndex = indices[i];
        queue_create_infos[i].queueCount = 1;

        // TODO: Enable this for a future enhancement.
        // if (indices[i] == context->device.graphics_queue_index) {
        //     queue_create_infos[i].queueCount = 2;
        // }
        queue_create_infos[i].flags = 0;
        queue_create_infos[i].pNext = 0;
        queue_create_infos[i].pQueuePriorities = &queue_priority;
    }

    // Request device features.
    // TODO: should be config driven
    VkPhysicalDeviceFeatures device_features = {};
    device_features.samplerAnisotropy = VK_TRUE;  // Request anistrophy
    device_features.fillModeNonSolid = VK_TRUE;   // TODO: Check if supported?

    bool portability_required = false;
    u32 available_extension_count = 0;
    VkExtensionProperties* available_extensions = 0;
    vkEnumerateDeviceExtensionProperties(context->device.physical_device, 0, &available_extension_count, 0);
    if (available_extension_count != 0) {
        available_extensions = malloc(sizeof(VkExtensionProperties) * available_extension_count);
        memset(available_extensions, 0, sizeof(VkExtensionProperties) * available_extension_count);
        vkEnumerateDeviceExtensionProperties(context->device.physical_device, 0, &available_extension_count, available_extensions);
        for (u32 i = 0; i < available_extension_count; ++i) {
            if ((strcmp(available_extensions[i].extensionName, "VK_KHR_portability_subset") == 0)) {
                printf("Adding required extension 'VK_KHR_portability_subset'.\n");
                portability_required = true;
                break;
            }
        }
    }
    free(available_extensions);

    // Setup an array of 3, even if we don't use them all.
    const char* extension_names[4];
    u32 ext_idx = 0;
    extension_names[ext_idx] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
    ext_idx++;
    // If portability is required (i.e. mac), add it.
    if (portability_required) {
        extension_names[ext_idx] = "VK_KHR_portability_subset";
        ext_idx++;
    }

    bool dynamic_state_extension_included = false;
    // If dynamic topology isn't supported natively but *is* supported via extension,
    // include the extension. These may both be false in the event of macos.
    if (
        ((context->device.support_flags & VULKAN_DEVICE_SUPPORT_FLAG_NATIVE_DYNAMIC_TOPOLOGY_BIT) == 0) &&
        ((context->device.support_flags & VULKAN_DEVICE_SUPPORT_FLAG_DYNAMIC_TOPOLOGY_BIT) != 0)) {
        extension_names[ext_idx] = VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME;
        ext_idx++;
        dynamic_state_extension_included = true;
    }
    // If smooth lines are supported, load the extension.
    if ((context->device.support_flags & VULKAN_DEVICE_SUPPORT_FLAG_LINE_SMOOTH_RASTERISATION_BIT)) {
        extension_names[ext_idx] = VK_EXT_LINE_RASTERIZATION_EXTENSION_NAME;
        ext_idx++;
    }
    if (!dynamic_state_extension_included) {
        // If dynamic front-face isn't supported natively but *is* supported via extension,
        // include the extension. These may both be false in the event of macos.
        if (
            ((context->device.support_flags & VULKAN_DEVICE_SUPPORT_FLAG_NATIVE_DYNAMIC_FRONT_FACE_BIT) == 0) &&
            ((context->device.support_flags & VULKAN_DEVICE_SUPPORT_FLAG_DYNAMIC_FRONT_FACE_BIT) != 0)) {
            extension_names[ext_idx] = VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME;
            ext_idx++;
            dynamic_state_extension_included = true;
        }
    }
    VkDeviceCreateInfo device_create_info = {VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
    device_create_info.queueCreateInfoCount = index_count;
    device_create_info.pQueueCreateInfos = queue_create_infos;
    device_create_info.pEnabledFeatures = &device_features;
    device_create_info.enabledExtensionCount = ext_idx;
    device_create_info.ppEnabledExtensionNames = extension_names;

    // Deprecated and ignored, so pass nothing.
    device_create_info.enabledLayerCount = 0;
    device_create_info.ppEnabledLayerNames = 0;

    // VK_EXT_extended_dynamic_state
    VkPhysicalDeviceExtendedDynamicStateFeaturesEXT extended_dynamic_state = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT};
    extended_dynamic_state.extendedDynamicState = VK_TRUE;
    device_create_info.pNext = &extended_dynamic_state;

    // Smooth line rasterisation, if supported.
    VkPhysicalDeviceLineRasterizationFeaturesEXT line_rasterization_ext = {0};
    if (context->device.support_flags & VULKAN_DEVICE_SUPPORT_FLAG_LINE_SMOOTH_RASTERISATION_BIT) {
        line_rasterization_ext.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LINE_RASTERIZATION_FEATURES_EXT;
        line_rasterization_ext.smoothLines = VK_TRUE;
        extended_dynamic_state.pNext = &line_rasterization_ext;
    }

    // Create the device.
    vkCreateDevice(
        context->device.physical_device,
        &device_create_info,
        0,
        &context->device.logical_device);

    // VK_SET_DEBUG_OBJECT_NAME(context, VK_OBJECT_TYPE_DEVICE, context->device.logical_device, "Vulkan Logical Device");

    printf("Logical device created.\n");

     //Examine dynamic topology support and load function pointer if need be.
    if (
        !(context->device.support_flags & VULKAN_DEVICE_SUPPORT_FLAG_NATIVE_DYNAMIC_TOPOLOGY_BIT) &&
        (context->device.support_flags & VULKAN_DEVICE_SUPPORT_FLAG_DYNAMIC_TOPOLOGY_BIT)) {
        printf("Vulkan device doesn't support native dynamic topology, but does via extension. Using extension.\n");
        context->vkCmdSetPrimitiveTopologyEXT = (PFN_vkCmdSetPrimitiveTopologyEXT)vkGetInstanceProcAddr(context->instance, "vkCmdSetPrimitiveTopologyEXT");
    } else {
        if (context->device.support_flags & VULKAN_DEVICE_SUPPORT_FLAG_NATIVE_DYNAMIC_TOPOLOGY_BIT) {
            printf("Vulkan device supports native dynamic topology.\n");
        } else {
            printf("Vulkan device does not support native or extension dynamic topology.\n");
        }
    }

     //Examine dynamic front-face support and load function pointer if need be.
    if (
        !(context->device.support_flags & VULKAN_DEVICE_SUPPORT_FLAG_NATIVE_DYNAMIC_FRONT_FACE_BIT) &&
        (context->device.support_flags & VULKAN_DEVICE_SUPPORT_FLAG_DYNAMIC_FRONT_FACE_BIT)) {
        printf("Vulkan device doesn't support native dynamic front-face, but does via extension. Using extension.\n");
        context->vkCmdSetFrontFaceEXT = (PFN_vkCmdSetFrontFaceEXT)vkGetInstanceProcAddr(context->instance, "vkCmdSetFrontFaceEXT");
    } else {
        if (context->device.support_flags & VULKAN_DEVICE_SUPPORT_FLAG_NATIVE_DYNAMIC_FRONT_FACE_BIT) {
            printf("Vulkan device supports native dynamic front-face.\n");
        } else {
            printf("Vulkan device does not support native or extension dynamic front-face.\n");
        }
    }

    // Get queues.
    vkGetDeviceQueue(
        context->device.logical_device,
        context->device.graphics_queue_index,
        0,
        &context->device.graphics_queue);

    vkGetDeviceQueue(
        context->device.logical_device,
        context->device.present_queue_index,
        0,
        &context->device.present_queue);

    vkGetDeviceQueue(
        context->device.logical_device,
        context->device.transfer_queue_index,
        0,
        &context->device.transfer_queue);
    printf("Queues obtained.\n");

    // Create command pool for graphics queue.
    VkCommandPoolCreateInfo pool_create_info = {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
    pool_create_info.queueFamilyIndex = context->device.graphics_queue_index;
    pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    vkCreateCommandPool(
        context->device.logical_device,
        &pool_create_info,
        0,
        &context->device.graphics_command_pool);
    printf("Graphics command pool created.\n");


    return true;
}

void v_device_destroy(vContext* context){

}

void v_device_query_swapchain_support(
    VkPhysicalDevice physical_device,
    VkSurfaceKHR surface,
    v_swapchain_support_info* out_support_info){

    // Surface capabilities
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        physical_device,
        surface,
        &out_support_info->capabilities);

    // Surface formats
    vkGetPhysicalDeviceSurfaceFormatsKHR(
        physical_device,
        surface,
        &out_support_info->format_count,
        0);

    if (out_support_info->format_count != 0) {
        if (!out_support_info->formats) {
            out_support_info->formats = malloc(sizeof(VkSurfaceFormatKHR) * out_support_info->format_count);
            memset(out_support_info->formats, 0, sizeof(VkSurfaceFormatKHR) * out_support_info->format_count);
        }
        vkGetPhysicalDeviceSurfaceFormatsKHR(
            physical_device,
            surface,
            &out_support_info->format_count,
            out_support_info->formats);
    }

    // Present modes
    vkGetPhysicalDeviceSurfacePresentModesKHR(
        physical_device,
        surface,
        &out_support_info->present_mode_count,
        0);
    if (out_support_info->present_mode_count != 0) {
        if (!out_support_info->present_modes) {
            out_support_info->present_modes = malloc(sizeof(VkPresentModeKHR) * out_support_info->present_mode_count);
            memset(out_support_info->present_modes, 0, sizeof(VkPresentModeKHR) * out_support_info->present_mode_count);
        }
        vkGetPhysicalDeviceSurfacePresentModesKHR(
            physical_device,
            surface,
            &out_support_info->present_mode_count,
            out_support_info->present_modes);
    }

}