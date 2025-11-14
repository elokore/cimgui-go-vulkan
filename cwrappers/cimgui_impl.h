#ifndef CIMGUI_IMPL_DEFINED
#define CIMGUI_IMPL_DEFINED
#ifdef CIMGUI_USE_GLFW
#ifdef CIMGUI_DEFINE_ENUMS_AND_STRUCTS

typedef struct GLFWwindow GLFWwindow;
typedef struct GLFWmonitor GLFWmonitor;
struct GLFWwindow;
struct GLFWmonitor;
#endif //CIMGUI_DEFINE_ENUMS_AND_STRUCTS
CIMGUI_API bool ImGui_ImplGlfw_InitForOpenGL(GLFWwindow* window,bool install_callbacks);
CIMGUI_API bool ImGui_ImplGlfw_InitForVulkan(GLFWwindow* window,bool install_callbacks);
CIMGUI_API bool ImGui_ImplGlfw_InitForOther(GLFWwindow* window,bool install_callbacks);
CIMGUI_API void ImGui_ImplGlfw_Shutdown(void);
CIMGUI_API void ImGui_ImplGlfw_NewFrame(void);
CIMGUI_API void ImGui_ImplGlfw_InstallCallbacks(GLFWwindow* window);
CIMGUI_API void ImGui_ImplGlfw_RestoreCallbacks(GLFWwindow* window);
CIMGUI_API void ImGui_ImplGlfw_SetCallbacksChainForAllWindows(bool chain_for_all_windows);
CIMGUI_API void ImGui_ImplGlfw_WindowFocusCallback(GLFWwindow* window,int focused);
CIMGUI_API void ImGui_ImplGlfw_CursorEnterCallback(GLFWwindow* window,int entered);
CIMGUI_API void ImGui_ImplGlfw_CursorPosCallback(GLFWwindow* window,double x,double y);
CIMGUI_API void ImGui_ImplGlfw_MouseButtonCallback(GLFWwindow* window,int button,int action,int mods);
CIMGUI_API void ImGui_ImplGlfw_ScrollCallback(GLFWwindow* window,double xoffset,double yoffset);
CIMGUI_API void ImGui_ImplGlfw_KeyCallback(GLFWwindow* window,int key,int scancode,int action,int mods);
CIMGUI_API void ImGui_ImplGlfw_CharCallback(GLFWwindow* window,unsigned int c);
CIMGUI_API void ImGui_ImplGlfw_MonitorCallback(GLFWmonitor* monitor,int event);
CIMGUI_API void ImGui_ImplGlfw_Sleep(int milliseconds);
CIMGUI_API float ImGui_ImplGlfw_GetContentScaleForWindow(GLFWwindow* window);
CIMGUI_API float ImGui_ImplGlfw_GetContentScaleForMonitor(GLFWmonitor* monitor);

#endif
#ifdef CIMGUI_USE_OPENGL3
CIMGUI_API bool ImGui_ImplOpenGL3_Init(const char* glsl_version);
CIMGUI_API void ImGui_ImplOpenGL3_Shutdown(void);
CIMGUI_API void ImGui_ImplOpenGL3_NewFrame(void);
CIMGUI_API void ImGui_ImplOpenGL3_RenderDrawData(ImDrawData* draw_data);
CIMGUI_API bool ImGui_ImplOpenGL3_CreateDeviceObjects(void);
CIMGUI_API void ImGui_ImplOpenGL3_DestroyDeviceObjects(void);
CIMGUI_API void ImGui_ImplOpenGL3_UpdateTexture(ImTextureData* tex);

#endif
#ifdef CIMGUI_USE_OPENGL2
CIMGUI_API bool ImGui_ImplOpenGL2_Init(void);
CIMGUI_API void ImGui_ImplOpenGL2_Shutdown(void);
CIMGUI_API void ImGui_ImplOpenGL2_NewFrame(void);
CIMGUI_API void ImGui_ImplOpenGL2_RenderDrawData(ImDrawData* draw_data);
CIMGUI_API bool ImGui_ImplOpenGL2_CreateDeviceObjects(void);
CIMGUI_API void ImGui_ImplOpenGL2_DestroyDeviceObjects(void);
CIMGUI_API void ImGui_ImplOpenGL2_UpdateTexture(ImTextureData* tex);

#endif

#ifdef CIMGUI_USE_VULKAN
#ifdef CIMGUI_DEFINE_ENUMS_AND_STRUCTS
struct ImGui_ImplVulkan_PipelineInfo
{
    // For Main viewport only
    VkRenderPass                    RenderPass;                     // Ignored if using dynamic rendering

    // For Main and Secondary viewports
    uint32_t                        Subpass;                        //
    VkSampleCountFlagBits           MSAASamples = {};               // 0 defaults to VK_SAMPLE_COUNT_1_BIT
#ifdef IMGUI_IMPL_VULKAN_HAS_DYNAMIC_RENDERING
    VkPipelineRenderingCreateInfoKHR PipelineRenderingCreateInfo;   // Optional, valid if .sType == VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR
#endif

    // For Secondary viewports only (created/managed by backend)
    VkImageUsageFlags               SwapChainImageUsage;            // Extra flags for vkCreateSwapchainKHR() calls for secondary viewports. We automatically add VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT. You can add e.g. VK_IMAGE_USAGE_TRANSFER_SRC_BIT if you need to capture from viewports.
};

struct ImGui_ImplVulkan_InitInfo
{
    uint32_t                        ApiVersion;                 // Fill with API version of Instance, e.g. VK_API_VERSION_1_3 or your value of VkApplicationInfo::apiVersion. May be lower than header version (VK_HEADER_VERSION_COMPLETE)
    VkInstance                      Instance;
    VkPhysicalDevice                PhysicalDevice;
    VkDevice                        Device;
    uint32_t                        QueueFamily;
    VkQueue                         Queue;
    VkDescriptorPool                DescriptorPool;             // See requirements in note above; ignored if using DescriptorPoolSize > 0
    uint32_t                        DescriptorPoolSize;         // Optional: set to create internal descriptor pool automatically instead of using DescriptorPool.
    uint32_t                        MinImageCount;              // >= 2
    uint32_t                        ImageCount;                 // >= MinImageCount
    VkPipelineCache                 PipelineCache;              // Optional

    // Pipeline
    ImGui_ImplVulkan_PipelineInfo   PipelineInfoMain;           // Infos for Main Viewport (created by app/user)
    ImGui_ImplVulkan_PipelineInfo   PipelineInfoForViewports;   // Infos for Secondary Viewports (created by backend)
    //VkRenderPass                  RenderPass;                 // --> Since 2025/09/26: set 'PipelineInfoMain.RenderPass' instead
    //uint32_t                      Subpass;                    // --> Since 2025/09/26: set 'PipelineInfoMain.Subpass' instead
    //VkSampleCountFlagBits         MSAASamples;                // --> Since 2025/09/26: set 'PipelineInfoMain.MSAASamples' instead
    //VkPipelineRenderingCreateInfoKHR PipelineRenderingCreateInfo; // Since 2025/09/26: set 'PipelineInfoMain.PipelineRenderingCreateInfo' instead

    // (Optional) Dynamic Rendering
    // Need to explicitly enable VK_KHR_dynamic_rendering extension to use this, even for Vulkan 1.3 + setup PipelineInfoMain.PipelineRenderingCreateInfo and PipelineInfoViewports.PipelineRenderingCreateInfo.
    bool                            UseDynamicRendering;

    // (Optional) Allocation, Debugging
    const VkAllocationCallbacks*    Allocator;
    void                            (*CheckVkResultFn)(VkResult err);
    VkDeviceSize                    MinAllocationSize;          // Minimum allocation size. Set to 1024*1024 to satisfy zealous best practices validation layer and waste a little memory.

    // (Optional) Customize default vertex/fragment shaders.
    // - if .sType == VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO we use specified structs, otherwise we use defaults.
    // - Shader inputs/outputs need to match ours. Code/data pointed to by the structure needs to survive for whole during of backend usage.
    VkShaderModuleCreateInfo        CustomShaderVertCreateInfo;
    VkShaderModuleCreateInfo        CustomShaderFragCreateInfo;
};
#endif
CIMGUI_API bool ImGui_ImplVulkan_Init(ImGui_ImplVulkan_InitInfo* info);
CIMGUI_API void ImGui_ImplVulkan_Shutdown();
CIMGUI_API void ImGui_ImplVulkan_NewFrame();
CIMGUI_API void ImGui_ImplVulkan_RenderDrawData(ImDrawData* draw_data, VkCommandBuffer command_buffer, VkPipeline pipeline = VK_NULL_HANDLE);
CIMGUI_API void ImGui_ImplVulkan_SetMinImageCount(uint32_t min_image_count);
#endif

#ifdef CIMGUI_USE_SDL2
#ifdef CIMGUI_DEFINE_ENUMS_AND_STRUCTS

typedef struct SDL_Window SDL_Window;
typedef struct SDL_Renderer SDL_Renderer;
typedef struct _SDL_GameController _SDL_GameController;
struct SDL_Window;
struct SDL_Renderer;
struct _SDL_GameController;
typedef union SDL_Event SDL_Event;
typedef enum { ImGui_ImplSDL2_GamepadMode_AutoFirst, ImGui_ImplSDL2_GamepadMode_AutoAll, ImGui_ImplSDL2_GamepadMode_Manual }ImGui_ImplSDL2_GamepadMode;
#endif //CIMGUI_DEFINE_ENUMS_AND_STRUCTS
CIMGUI_API bool ImGui_ImplSDL2_InitForOpenGL(SDL_Window* window,void* sdl_gl_context);
CIMGUI_API bool ImGui_ImplSDL2_InitForVulkan(SDL_Window* window);
CIMGUI_API bool ImGui_ImplSDL2_InitForD3D(SDL_Window* window);
CIMGUI_API bool ImGui_ImplSDL2_InitForMetal(SDL_Window* window);
CIMGUI_API bool ImGui_ImplSDL2_InitForSDLRenderer(SDL_Window* window,SDL_Renderer* renderer);
CIMGUI_API bool ImGui_ImplSDL2_InitForOther(SDL_Window* window);
CIMGUI_API void ImGui_ImplSDL2_Shutdown(void);
CIMGUI_API void ImGui_ImplSDL2_NewFrame(void);
CIMGUI_API bool ImGui_ImplSDL2_ProcessEvent(const SDL_Event* event);
CIMGUI_API float ImGui_ImplSDL2_GetContentScaleForWindow(SDL_Window* window);
CIMGUI_API float ImGui_ImplSDL2_GetContentScaleForDisplay(int display_index);
CIMGUI_API void ImGui_ImplSDL2_SetGamepadMode(ImGui_ImplSDL2_GamepadMode mode,struct _SDL_GameController** manual_gamepads_array,int manual_gamepads_count);

#endif
#endif //CIMGUI_IMPL_DEFINED
