#include "renderer.h"
#include <stdio.h>
#include <stdlib.h>
#include <GWindow.h>
#include <string.h>
#include "device.h"
#include <GWindow.h>
#include "../Containers/dynlist.h"

#define ARRAY_SIZE(arr) sizeof(arr)/sizeof(arr[0])

static vContext* v_ctx;

static const char* VALIDATION_LAYERS[] = {
    "VK_LAYER_KHRONOS_validation",
};

#ifdef _DEBUG
    const bool enableValidationLayers = true;
#else
    const bool enableValidationLayers = false;
#endif

VKAPI_ATTR VkBool32 VKAPI_CALL vk_debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT message_types,
    const VkDebugUtilsMessengerCallbackDataEXT *callback_data, void *user_data);

/**
 * @brief Utility function to query and display supported extensions
 * 
 */
void query_for_available_Extensions_Support(){

    u32 extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(0, &extensionCount, 0);
    VkExtensionProperties* extensions = malloc(sizeof(VkExtensionProperties) * extensionCount);
    vkEnumerateInstanceExtensionProperties(0, &extensionCount, extensions);
    
    printf("Available Extensions[%d]: \n", extensionCount);

    for(int i = 0; i < extensionCount; i++){
        printf("[%d] %s\n", i, extensions[i].extensionName);
    }

    free(extensions);
}

/**
 * @brief Utility function to query and display supported validation layers
 * 
 */
void query_for_available_ValidationLayers_Support(){
    u32 layerCount = 0;
    vkEnumerateInstanceLayerProperties(&layerCount, 0);
    VkLayerProperties* availableLayers = malloc(sizeof(VkLayerProperties) * layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers);

    printf("Supported Validation Layers[%d]:\n", layerCount);

    for(int i = 0; i < layerCount; i++){
        printf("[%d] %s\n", i, availableLayers[i].layerName);
    }

    free(availableLayers);
}


/**
 * @brief Utility function to check if validation in array 'VALIDATION_LAYERS' are available or not
 * 
 * @return true if available
 * @return false if not available
 */
bool check_validation_layer_support(){
    u32 layerCount = 0;
    vkEnumerateInstanceLayerProperties(&layerCount, 0);
    VkLayerProperties* availableLayers = malloc(sizeof(VkLayerProperties) * layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers);

    size_t vlength = ARRAY_SIZE(VALIDATION_LAYERS);

    for (int i = 0; i < vlength; i++){
        bool layerFound = false;

        for (int j = 0; j < layerCount; j++){
            if (strcmp(VALIDATION_LAYERS[i], availableLayers[j].layerName) == 0){
                // printf("%s == %s\n", VALIDATION_LAYERS[i], availableLayers[j].layerName);
                layerFound = true;
                break;
            }
        }

        if(!layerFound){
            printf("Validation Layer not found!\n");
            free(availableLayers);
            return false;
        }

        printf("Validation Layer is found!\n");
        free(availableLayers);
        return true;
    }

    return false;
}

void populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT* createInfo){
    u32 log_severity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;  //|
                                                       //    VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;

    // createInfo = {VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};
    createInfo->sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo->messageSeverity = log_severity;
    createInfo->messageType =  VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    createInfo->pfnUserCallback = vk_debug_callback;
    createInfo->pNext = NULL;
    createInfo->flags = 0;
}

void setup_debug_messenger(){
    if (!enableValidationLayers) return;
    
    VkDebugUtilsMessengerCreateInfoEXT debug_create_info;

    populate_debug_messenger_create_info(&debug_create_info);

    PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(v_ctx->instance, "vkCreateDebugUtilsMessengerEXT");
    
    if (func != 0){
        printf("Vulkan debugger created.\n");
        func(v_ctx->instance, &debug_create_info, 0, &v_ctx->debug_messenger);
    }else{
        printf("Failed to create debug messenger!\n");
    }

    // printf("Vulkan debugger created.\n");
}


bool createInstance(){

    if(enableValidationLayers && !check_validation_layer_support()){
        printf("Validation layers requested but not found/available!\n");
        return false;
    }

    //Lets create our vulkan instance
    VkApplicationInfo appInfo = {0};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_2;

    VkInstanceCreateInfo createInfo = {0};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    const char** extensions = dynlist_create(const char*);
    GWindow_getRequiredVulkanExtensions(&extensions);
    #ifdef _DEBUG
    dynlist_push(extensions, &VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    #endif

    createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;

    createInfo.enabledExtensionCount = dynlist_length(extensions);
    createInfo.ppEnabledExtensionNames = extensions;

    VkDebugUtilsMessengerCreateInfoEXT debug_create_info = {0};
    if (enableValidationLayers){
        createInfo.enabledLayerCount = ARRAY_SIZE(VALIDATION_LAYERS);
        createInfo.ppEnabledLayerNames = VALIDATION_LAYERS;
        populate_debug_messenger_create_info(&debug_create_info);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debug_create_info;
    }else{
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = NULL;
    }
    
    if (vkCreateInstance(&createInfo, 0, &v_ctx->instance) != VK_SUCCESS) {
        dynlist_destroy(extensions);
        printf("Failed to create instance\n");
        return false;
    }
     
    printf("Instance created successfully!\n"); 

    dynlist_destroy(extensions);

    return true;
}

bool create_surface(){
    u64 size = 0;
    GWindow_get_handle_info(&size, 0);
    void* block = malloc(size);
    GWindow_get_handle_info(&size, block);

    VkMetalSurfaceCreateInfoEXT create_info = {VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT};
    create_info.pNext = NULL;
    create_info.flags = 0;
    create_info.pLayer = ((macos_handle_info*)block)->layer;

    VkResult result = vkCreateMetalSurfaceEXT(
        v_ctx->instance, 
        &create_info,
        0,
        &v_ctx->surface);
    if (result != VK_SUCCESS) {
        printf("Vulkan surface creation failed.\n");
        return false;
    }

    printf("Vulkan surface creation Succeeded!.\n");

    return true;
}

/**
 * @brief Initialises our Renderer
 * 
 * @return true if successful
 * @return false if unsuccessful
 */
bool renderer_initialize(){

    v_ctx = malloc(sizeof(vContext));
    memset(v_ctx, 0, sizeof(vContext));

    createInstance();
    setup_debug_messenger();
    create_surface();
    v_device_create(v_ctx);

    return true;
}

/**
 * @brief Shutsdown our renderer and deallocates memory
 * 
 */
void renderer_shutdown(){

    #if defined(_DEBUG)
    printf("Destroying Vulkan debugger...\n");
    if (v_ctx->debug_messenger) {
        PFN_vkDestroyDebugUtilsMessengerEXT func =
            (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
                v_ctx->instance, "vkDestroyDebugUtilsMessengerEXT");
        func(v_ctx->instance, v_ctx->debug_messenger, 0);
    }
    #endif

    vkDestroyInstance(v_ctx->instance, 0);


    free(v_ctx);
}

VKAPI_ATTR VkBool32 VKAPI_CALL
vk_debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
                  VkDebugUtilsMessageTypeFlagsEXT message_types,
                  const VkDebugUtilsMessengerCallbackDataEXT *callback_data,
                  void *user_data) {
    switch (message_severity) {
        default:
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            printf("%s", callback_data->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            printf("%s", callback_data->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            printf("%s", callback_data->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            printf("%s", callback_data->pMessage);
            break;
    }
    return VK_FALSE;
}