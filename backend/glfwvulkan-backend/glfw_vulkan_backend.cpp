#if defined(CIMGUI_GO_USE_GLFW)
#define GL_SILENCE_DEPRECATION
#define CIMGUI_USE_GLFW
#define CIMGUI_USE_VULKAN

#include "glfw_backend.h"
#include "../../cwrappers/cimgui.h"
#include "../../cwrappers/cimgui_impl.h"
#include "../../thirdparty/glfw/include/GLFW/glfw3.h" // Will drag system OpenGL headers
#include "../../cwrappers/imgui/backends/imgui_impl_vulkan.h"
#include <cstdlib>
#include <thread>
#include <chrono>
#include <iostream>
#include <vector>


// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to
// maximize ease of testing and compatibility with old VS compilers. To link
// with VS2010-era libraries, VS2015+ requires linking with
// legacy_stdio_definitions.lib, which we do using this pragma. Your own project
// should not be affected, as you are likely to link with a newer binary of GLFW
// that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

#define MAX_EXTRA_FRAME_COUNT 15
unsigned int glfw_target_fps = 30;
int extra_frame_count = MAX_EXTRA_FRAME_COUNT;

ImVec4 clear_color = *ImVec4_ImVec4_Float(0.45, 0.55, 0.6, 1.0);
VkDevice current_device;
VkDescriptorPool descriptor_pool;
VkRenderPass render_pass;
VkCommandPool command_pool;
VkQueue current_graphics_queue;
std::vector<VkFramebuffer> framebuffers;
std::vector<VkCommandBuffer> command_buffers;
std::vector<VkImage> swapchain_images;
std::vector<VkFence> fences;

void glfw_render(GLFWwindow *window, int image_index, VoidCallback renderLoop);

void igSetBgColor(ImVec4 color) { clear_color = color; }

void igSetTargetFPS(unsigned int fps) { glfw_target_fps = fps; }

static void glfw_error_callback(int error, const char *description) {
  fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

void glfw_window_refresh_callback(GLFWwindow *window) {
  VoidCallback loopFunc = (VoidCallback)(glfwGetWindowUserPointer(window));
  glfw_render(window, loopFunc);
}

int igInitGLFW() {
    glfwSetErrorCallback(glfw_error_callback);
    return glfwInit();
}

bool createRenderPass(VkDevice device, VkFormat swapchain_format) {
  VkAttachmentDescription color_attachment = {};
      color_attachment.format = swapchain_format;
      color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
      color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
      color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
      color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
      color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
      color_attachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
      color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  VkAttachmentReference color_attachment_ref = {};
      color_attachment_ref.attachment = 0;
      color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass = {};
      subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
      subpass.colorAttachmentCount = 1;
      subpass.pColorAttachments = &color_attachment_ref;

  VkSubpassDependency dependency = {};
      dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
      dependency.dstSubpass = 0;
      dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
      dependency.srcAccessMask = 0;
      dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
      dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

  VkRenderPassCreateInfo render_pass_info = {};
      render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
      render_pass_info.attachmentCount = 1;
      render_pass_info.pAttachments = &color_attachment;
      render_pass_info.subpassCount = 1;
      render_pass_info.pSubpasses = &subpass;
      render_pass_info.dependencyCount = 1;
      render_pass_info.pDependencies = &dependency;

  VkResult result = vkCreateRenderPass(device, &render_pass_info, nullptr, &render_pass);
  return result == VK_SUCCESS;
}

bool createCommandBuffers(VkDevice device, uint32_t graphics_queue_family, uint32_t swapchain_image_count) {
  VkCommandPoolCreateInfo pool_info = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
      .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
      .queueFamilyIndex = graphics_queue_family
  };

  VkResult result = vkCreateCommandPool(device, &pool_info, nullptr, &command_pool);
  if (result != VK_SUCCESS) {
      printf("IMGUI Command pool allocation failed!\n");
      return false;
  }

  VkCommandBufferAllocateInfo alloc_info = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .commandPool = command_pool,
      .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandBufferCount = swapchain_image_count
  };

  command_buffers.resize(swapchain_image_count);
  result = vkAllocateCommandBuffers(device, &alloc_info, command_buffers.data());

  if (result != VK_SUCCESS) {
      printf("IMGUI Command buffer allocations failed!\n");
      return false;
  }

  // Create fences
  fences.resize(swapchain_image_count);
  VkFenceCreateInfo fence_info = {};
  fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  for (int i = 0; i < swapchain_image_count; i++) {
      result = vkCreateFence(device, &fence_info, nullptr, &fences[i]);
      
      if (result != VK_SUCCESS) {
          printf("ERROR: Failed to create fence %u! Result: %d\n", i, result);
          return false;
      }
  }

  return true;
}

bool createFrameBuffers(VkDevice device, std::vector<VkImageView> image_views, int width, int height) {
  framebuffers.resize(image_views.size());

  for (size_t i = 0; i < image_views.size(); i++) {
    VkImageView attachments[] = { image_views[i] };

    VkFramebufferCreateInfo framebuffer_info = {};
    framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebuffer_info.renderPass = render_pass;
    framebuffer_info.attachmentCount = 1;
    framebuffer_info.pAttachments = attachments;
    framebuffer_info.width = width;
    framebuffer_info.height = height;
    framebuffer_info.layers = 1;
    
    VkResult result = vkCreateFramebuffer(device, &framebuffer_info, nullptr, &framebuffers[i]);
    if (result != VK_SUCCESS) {
        return false;
    }
  }

  return true;
}

void igAttachToExistingWindow(GLFWwindow* window, VkInstance instance, VkDevice device, VkPhysicalDevice physical_device,
  VkQueue graphics_queue, VkPipelineCache pipeline_cache, uint32_t graphics_queue_family, VkImageView image_views[],
  VkImage swapchain_imgs[], VkFormat swapchain_format, uint32_t swapchain_image_count, int width, int height) {
  
  igCreateContext(0);
  ImGui_ImplGlfw_InitForVulkan(window, true);
    
  int swapchain_img_count = sizeof(swapchain_images) / sizeof(VkImage);
  swapchain_images.assign(swapchain_imgs, swapchain_imgs + swapchain_img_count);

  std::vector<VkImageView> views;
  views.assign(image_views, image_views + swapchain_img_count);

  current_device = device;
  current_graphics_queue = graphics_queue;

  createRenderPass(device, swapchain_format);
  createCommandBuffers(device, graphics_queue_family, swapchain_image_count);
  createFrameBuffers(device, views, width, height);

  // The pool is very oversized, an example I found said to do this, idk what else to set it to
	VkDescriptorPoolSize pool_sizes[] =
	{
		{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
	};

  VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 1000;
    pool_info.poolSizeCount = std::size(pool_sizes);
    pool_info.pPoolSizes = pool_sizes;

	vkCreateDescriptorPool(device, &pool_info, nullptr, &descriptor_pool);

  ImGui_ImplVulkan_PipelineInfo pipeline_info = {};
    pipeline_info.RenderPass = render_pass;
    pipeline_info.Subpass = 0;
    pipeline_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

  ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = instance;
    init_info.PhysicalDevice = physical_device;
    init_info.Device = device;
    init_info.Queue = graphics_queue;
    init_info.QueueFamily = graphics_queue_family;
    init_info.PipelineCache = pipeline_cache;
    init_info.DescriptorPool = descriptor_pool;
    init_info.PipelineInfoMain = pipeline_info;
    init_info.MinImageCount = 2;
    init_info.ImageCount = swapchain_image_count;
    init_info.Allocator = nullptr;
    init_info.CheckVkResultFn = nullptr;

  ImGui_ImplVulkan_Init(&init_info);
}

GLFWwindow *igCreateGLFWWindow(const char *title, int width, int height,
                               VoidCallback afterCreateContext) {
  // glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

  // // Create window with graphics context
  // GLFWwindow *window = glfwCreateWindow(width, height, title, NULL, NULL);
  // if (window == NULL)
  //   return NULL;
  // glfwMakeContextCurrent(window);

  // // Setup Dear ImGui context
  // igCreateContext(0);

  // if (afterCreateContext != NULL) {
  //   afterCreateContext();
  // }

  // ImGuiIO *io = igGetIO_Nil();
  // io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
  // // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad
  // // Controls
  // io->ConfigFlags |= ImGuiConfigFlags_DockingEnable;   // Enable Docking
  // io->ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport
  //                                                      // / Platform Windows
  // // io.ConfigViewportsNoAutoMerge = true;
  // // io.ConfigViewportsNoTaskBarIcon = true;

  // io->IniFilename = "";

  // // Setup Dear ImGui style
  // igStyleColorsDark(0);
  // // igStyleColorsLight();

  // // When viewports are enabled we tweak WindowRounding/WindowBg so platform
  // // windows can look identical to regular ones.
  // ImGuiStyle *style = igGetStyle();
  // if (io->ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
  //   style->WindowRounding = 0.0f;
  //   style->Colors[ImGuiCol_WindowBg].w = 1.0f;
  // }

  // // Setup Platform/Renderer backends
  // igAttachToExistingWindow(window);

  // // Install extra callback
  // glfwSetWindowRefreshCallback(window, glfw_window_refresh_callback);
  // glfwMakeContextCurrent(NULL);
  // return window;
  return NULL;
}

void igRefresh() { glfwPostEmptyEvent(); }

void glfw_render(GLFWwindow *window, int image_index, VoidCallback renderLoop) {
  VkCommandBuffer command_buffer = command_buffers[image_index];
  VkFence fence = fences[image_index];

  // Wait for command buffer to finish its last cycle
  vkWaitForFences(current_device, 1, &fence, VK_TRUE, UINT64_MAX);
  vkResetFences(current_device, 1, &fence);

  VkCommandBufferBeginInfo cmd_begin_info = {};
    cmd_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmd_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    cmd_begin_info.pInheritanceInfo = nullptr;
    cmd_begin_info.pNext = nullptr;

  vkResetCommandBuffer(command_buffer, 0);
  vkBeginCommandBuffer(command_buffer, &cmd_begin_info);

  // Start the Dear ImGui frame
  ImGui_ImplVulkan_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  igNewFrame();

  // Rendering
  igRender();
  ImDrawData* draw_data = igGetDrawData();

  VkImageMemoryBarrier barrier1 = {};
    barrier1.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier1.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    barrier1.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    barrier1.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    barrier1.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    barrier1.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier1.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier1.image = swapchain_images[image_index];
    barrier1.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier1.subresourceRange.baseMipLevel = 0;
    barrier1.subresourceRange.levelCount = 1;
    barrier1.subresourceRange.baseArrayLayer = 0;
    barrier1.subresourceRange.layerCount = 1;

  vkCmdPipelineBarrier(
      command_buffer,
      VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
      0,
      0, nullptr,
      0, nullptr,
      1, &barrier1
  );

  VkRenderPassBeginInfo render_pass_begin_info = {};
    render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_begin_info.renderPass = render_pass;
    render_pass_begin_info.framebuffer = framebuffers[image_index];
    render_pass_begin_info.renderArea.extent.width = static_cast<uint32_t>(draw_data->DisplaySize.x);
    render_pass_begin_info.renderArea.extent.height = static_cast<uint32_t>(draw_data->DisplaySize.y);

  VkClearValue clear_values[1] = {};
    clear_values[0].color = {{0.0f, 0.0f, 0.0f, 0.0f}};  // Transparent
    render_pass_begin_info.clearValueCount = 1;
    render_pass_begin_info.pClearValues = clear_values;

  vkCmdBeginRenderPass(command_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
  ImGui_ImplVulkan_RenderDrawData(draw_data, command_buffer); 
  vkCmdEndRenderPass(command_buffer);

  VkSubmitInfo submit_info = {};
  submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submit_info.commandBufferCount = 1;
  submit_info.pCommandBuffers = &command_buffer;
  
  vkEndCommandBuffer(command_buffer);
  VkResult result = vkQueueSubmit(current_graphics_queue, 1, &submit_info, fence);
}

void igGLFWRunLoop(GLFWwindow *window, VoidCallback loop, VoidCallback beforeRender, VoidCallback afterRender,
               VoidCallback beforeDestroyContext) {
  glfwMakeContextCurrent(window);
  ImGuiIO *io = igGetIO_Nil();

  // Load Fonts
  // - If no fonts are loaded, dear imgui will use the default font. You can
  // also load multiple fonts and use igPushFont()/PopFont() to select them.
  // - AddFontFromFileTTF() will return the ImFont* so you can store it if you
  // need to select the font among multiple.
  // - If the file cannot be loaded, the function will return NULL. Please
  // handle those errors in your application (e.g. use an assertion, or display
  // an error and quit).
  // - The fonts will be rasterized at a given size (w/ oversampling) and stored
  // into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which
  // ImGui_ImplXXXX_NewFrame below will call.
  // - Read 'docs/FONTS.md' for more instructions and details.
  // - Remember that in C/C++ if you want to include a backslash \ in a string
  // literal you need to write a double backslash \\ !
  // io.Fonts->AddFontDefault();
  // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
  // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
  // io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
  // io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
  // ImFont* font =
  // io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f,
  // NULL, io.Fonts->GetGlyphRangesJapanese()); IM_ASSERT(font != NULL);

  // Main loop

  // double lastFrameTime = glfwGetTime();
  // while (!glfwWindowShouldClose(window)) {
  //   double frameTime = 1.0 / glfw_target_fps;
  //   double frameDeltaTime = glfwGetTime() - lastFrameTime;

  //   if(frameDeltaTime >= frameTime) {
  //     glfwPollEvents();
  //     if (beforeRender != NULL) {
  //       beforeRender();
  //     }
  //     glfw_render(window, loop);
  //     if (afterRender != NULL) {
  //       afterRender();
  //     }
  //     lastFrameTime += frameTime;
  //   } else {
  //     double timeToSleep = frameTime - frameDeltaTime;
  //     std::this_thread::sleep_for(std::chrono::duration<double>(timeToSleep));
  //   }
  // }

  // // Cleanup
  // ImGui_ImplOpenGL3_Shutdown();
  // ImGui_ImplGlfw_Shutdown();

  // if (beforeDestroyContext != NULL) {
  //   beforeDestroyContext();
  // }

  // igDestroyContext(0);

  // glfwDestroyWindow(window);
  // glfwTerminate();

  return;
}

ImTextureID igCreateTexture(unsigned char *pixels, int width, int height) {
  // GLint last_texture;
  // GLuint texId;

  // glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
  // glGenTextures(1, &texId);
  // glBindTexture(GL_TEXTURE_2D, texId);
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

  // // Restore state
  // glBindTexture(GL_TEXTURE_2D, last_texture);

  return ImTextureID();
}

void igDeleteTexture(ImTextureID id) {
  // glBindTexture(GL_TEXTURE_2D, 0);
  // glDeleteTextures(1, (GLuint *)(&id));
}

void igGLFWWindow_GetDisplaySize(GLFWwindow *window, int *width, int *height) {
  glfwGetWindowSize(window, width, height);
}

void igGLFWWindow_GetContentScale(GLFWwindow *window, float *width, float *height) {
  glfwGetWindowContentScale(window, width, height);
}

void igGLFWWindow_SetWindowPos(GLFWwindow *window, int x, int y) { glfwSetWindowPos(window, x, y); }

void igGLFWWindow_GetWindowPos(GLFWwindow *window, int *x, int *y) { glfwGetWindowPos(window, x, y); }

void igGLFWWindow_SetShouldClose(GLFWwindow *window, bool value) { glfwSetWindowShouldClose(window, value); }

void igGLFWWindow_SetDropCallbackCB(GLFWwindow *wnd) { glfwSetDropCallback(wnd, (GLFWdropfun)dropCallback); }
void igGLFWWindow_SetKeyCallback(GLFWwindow *wnd) { glfwSetKeyCallback(wnd, (GLFWkeyfun)keyCallback); }
void igGLFWWindow_SetSizeCallback(GLFWwindow *wnd) { glfwSetWindowSizeCallback(wnd, (GLFWwindowsizefun)sizeCallback); }

void igGLFWWindow_SetCloseCallback(GLFWwindow *window) {
  glfwSetWindowCloseCallback(window, (GLFWwindowclosefun)closeCallback);
}

void igGLFWWindow_SetSize(GLFWwindow *window, int width, int height) { glfwSetWindowSize(window, width, height); }

void igGLFWWindow_SetTitle(GLFWwindow *window, const char *title) { glfwSetWindowTitle(window, title); }

void igGLFWWindow_SetSizeLimits(GLFWwindow *window, int minWidth, int minHeight, int maxWidth, int maxHeight) {
  glfwSetWindowSizeLimits(window, minWidth, minHeight, maxWidth, maxHeight);
}

void igWindowHint(int hint, int value) { glfwWindowHint(hint, value); }
void igGLFWInitHint(GLFWInitHint hint, int value) { glfwInitHint(hint, value); }

void igGLFWWindow_SetIcon(GLFWwindow *window, int count, CImage *images) {
    GLFWimage* glfwImages = (GLFWimage*)malloc(count * sizeof(GLFWimage));
    for (int i = 0; i < count; i++) {
        glfwImages[i].width = images[i].width;
        glfwImages[i].height = images[i].height;
        glfwImages[i].pixels = images[i].pixels;
    }

    glfwSetWindowIcon(window, count, glfwImages);
    free(glfwImages);
}

void iggImplGlfw_KeyCallback(GLFWwindow* w, int k,int s,int a,int m) { ImGui_ImplGlfw_KeyCallback(w,k,s,a,m); }

#endif
