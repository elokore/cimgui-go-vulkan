package glfwvulkanbackend

// #cgo CPPFLAGS: -DCIMGUI_GO_USE_GLFW
// #cgo linux LDFLAGS: -lvulkan
// #cgo windows LDFLAGS: -lvulkan-1
// #cgo darwin LDFLAGS: -framework Vulkan -framework Metal -framework QuartzCore
// #cgo !gles2,darwin LDFLAGS: -framework OpenGL
// #cgo gles2,darwin LDFLAGS: -lGLESv2
//
// #include <stdlib.h>
// #include <stdint.h>
// #include "glfw_vulkan_backend.h"
import "C"

import (
	"image"
	"image/draw"
	"unsafe"

	"github.com/AllenDang/cimgui-go/backend"
	"github.com/AllenDang/cimgui-go/imgui"
	"github.com/AllenDang/cimgui-go/internal"
	glfw "github.com/go-gl/glfw/v3.3/glfw"
	as "github.com/vulkan-go/asche"
	vk "github.com/vulkan-go/vulkan"
)

type voidCallbackFunc func()

type GLFWWindowFlags int

// Window flags
const (
	GLFWWindowFlagsNone             = GLFWWindowFlags(C.GLFWWindowNone)
	GLFWWindowFlagsResizable        = GLFWWindowFlags(C.GLFWWindowResizable)
	GLFWWindowFlagsMaximized        = GLFWWindowFlags(C.GLFWWindowMaximized)
	GLFWWindowFlagsDecorated        = GLFWWindowFlags(C.GLFWWindowDecorated)
	GLFWWindowFlagsTransparent      = GLFWWindowFlags(C.GLFWWindowTransparentFramebuffer)
	GLFWWindowFlagsVisible          = GLFWWindowFlags(C.GLFWWindowVisible)
	GLFWWindowFlagsFloating         = GLFWWindowFlags(C.GLFWWindowFloating)
	GLFWWindowFlagsFocused          = GLFWWindowFlags(C.GLFWWindowFocused)
	GLFWWindowFlagsIconified        = GLFWWindowFlags(C.GLFWWindowIconified)
	GLFWWindowFlagsAutoIconify      = GLFWWindowFlags(C.GLFWWindowAutoIconify)
	GLFWWindowFlagsMousePassthrough = GLFWWindowFlags(C.GLFWWindowMousePassthrough)
)

// SwapInterval values
const (
	GLFWSwapIntervalImmediate = GLFWWindowFlags(0)
	GLFWSwapIntervalVsync     = GLFWWindowFlags(1)
)

// Input modes
const (
	GLFWInputModeCursor         = GLFWWindowFlags(C.GLFW_CURSOR)
	GLFWInputModeCursorNormal   = GLFWWindowFlags(C.GLFW_CURSOR_NORMAL)
	GLFWInputModeCursorHidden   = GLFWWindowFlags(C.GLFW_CURSOR_HIDDEN)
	GLFWInputModeCursorDisabled = GLFWWindowFlags(C.GLFW_CURSOR_DISABLED)
	GLFWInputModeRawMouseMotion = GLFWWindowFlags(C.GLFW_RAW_MOUSE_MOTION)
)

type GLFWKey int

const (
	GLFWKeySpace        = GLFWKey(C.GLFWKeySpace)
	GLFWKeyApostrophe   = GLFWKey(C.GLFWKeyApostrophe)
	GLFWKeyComma        = GLFWKey(C.GLFWKeyComma)
	GLFWKeyMinus        = GLFWKey(C.GLFWKeyMinus)
	GLFWKeyPeriod       = GLFWKey(C.GLFWKeyPeriod)
	GLFWKeySlash        = GLFWKey(C.GLFWKeySlash)
	GLFWKey0            = GLFWKey(C.GLFWKey0)
	GLFWKey1            = GLFWKey(C.GLFWKey1)
	GLFWKey2            = GLFWKey(C.GLFWKey2)
	GLFWKey3            = GLFWKey(C.GLFWKey3)
	GLFWKey4            = GLFWKey(C.GLFWKey4)
	GLFWKey5            = GLFWKey(C.GLFWKey5)
	GLFWKey6            = GLFWKey(C.GLFWKey6)
	GLFWKey7            = GLFWKey(C.GLFWKey7)
	GLFWKey8            = GLFWKey(C.GLFWKey8)
	GLFWKey9            = GLFWKey(C.GLFWKey9)
	GLFWKeySemicolon    = GLFWKey(C.GLFWKeySemicolon)
	GLFWKeyEqual        = GLFWKey(C.GLFWKeyEqual)
	GLFWKeyA            = GLFWKey(C.GLFWKeyA)
	GLFWKeyB            = GLFWKey(C.GLFWKeyB)
	GLFWKeyC            = GLFWKey(C.GLFWKeyC)
	GLFWKeyD            = GLFWKey(C.GLFWKeyD)
	GLFWKeyE            = GLFWKey(C.GLFWKeyE)
	GLFWKeyF            = GLFWKey(C.GLFWKeyF)
	GLFWKeyG            = GLFWKey(C.GLFWKeyG)
	GLFWKeyH            = GLFWKey(C.GLFWKeyH)
	GLFWKeyI            = GLFWKey(C.GLFWKeyI)
	GLFWKeyJ            = GLFWKey(C.GLFWKeyJ)
	GLFWKeyK            = GLFWKey(C.GLFWKeyK)
	GLFWKeyL            = GLFWKey(C.GLFWKeyL)
	GLFWKeyM            = GLFWKey(C.GLFWKeyM)
	GLFWKeyN            = GLFWKey(C.GLFWKeyN)
	GLFWKeyO            = GLFWKey(C.GLFWKeyO)
	GLFWKeyP            = GLFWKey(C.GLFWKeyP)
	GLFWKeyQ            = GLFWKey(C.GLFWKeyQ)
	GLFWKeyR            = GLFWKey(C.GLFWKeyR)
	GLFWKeyS            = GLFWKey(C.GLFWKeyS)
	GLFWKeyT            = GLFWKey(C.GLFWKeyT)
	GLFWKeyU            = GLFWKey(C.GLFWKeyU)
	GLFWKeyV            = GLFWKey(C.GLFWKeyV)
	GLFWKeyW            = GLFWKey(C.GLFWKeyW)
	GLFWKeyX            = GLFWKey(C.GLFWKeyX)
	GLFWKeyY            = GLFWKey(C.GLFWKeyY)
	GLFWKeyZ            = GLFWKey(C.GLFWKeyZ)
	GLFWKeyLeftBracket  = GLFWKey(C.GLFWKeyLeftBracket)
	GLFWKeyBackslash    = GLFWKey(C.GLFWKeyBackslash)
	GLFWKeyRightBracket = GLFWKey(C.GLFWKeyRightBracket)
	GLFWKeyGraveAccent  = GLFWKey(C.GLFWKeyGraveAccent)
	GLFWKeyWorld1       = GLFWKey(C.GLFWKeyWorld1)
	GLFWKeyWorld2       = GLFWKey(C.GLFWKeyWorld2)

	/* Function keys */
	GLFWKeyEscape       = GLFWKey(C.GLFWKeyEscape)
	GLFWKeyEnter        = GLFWKey(C.GLFWKeyEnter)
	GLFWKeyTab          = GLFWKey(C.GLFWKeyTab)
	GLFWKeyBackspace    = GLFWKey(C.GLFWKeyBackspace)
	GLFWKeyInsert       = GLFWKey(C.GLFWKeyInsert)
	GLFWKeyDelete       = GLFWKey(C.GLFWKeyDelete)
	GLFWKeyRight        = GLFWKey(C.GLFWKeyRight)
	GLFWKeyLeft         = GLFWKey(C.GLFWKeyLeft)
	GLFWKeyDown         = GLFWKey(C.GLFWKeyDown)
	GLFWKeyUp           = GLFWKey(C.GLFWKeyUp)
	GLFWKeyPageUp       = GLFWKey(C.GLFWKeyPageUp)
	GLFWKeyPageDown     = GLFWKey(C.GLFWKeyPageDown)
	GLFWKeyHome         = GLFWKey(C.GLFWKeyHome)
	GLFWKeyEnd          = GLFWKey(C.GLFWKeyEnd)
	GLFWKeyCapsLock     = GLFWKey(C.GLFWKeyCapsLock)
	GLFWKeyScrollLock   = GLFWKey(C.GLFWKeyScrollLock)
	GLFWKeyNumLock      = GLFWKey(C.GLFWKeyNumLock)
	GLFWKeyPrintScreen  = GLFWKey(C.GLFWKeyPrintScreen)
	GLFWKeyPause        = GLFWKey(C.GLFWKeyPause)
	GLFWKeyF1           = GLFWKey(C.GLFWKeyF1)
	GLFWKeyF2           = GLFWKey(C.GLFWKeyF2)
	GLFWKeyF3           = GLFWKey(C.GLFWKeyF3)
	GLFWKeyF4           = GLFWKey(C.GLFWKeyF4)
	GLFWKeyF5           = GLFWKey(C.GLFWKeyF5)
	GLFWKeyF6           = GLFWKey(C.GLFWKeyF6)
	GLFWKeyF7           = GLFWKey(C.GLFWKeyF7)
	GLFWKeyF8           = GLFWKey(C.GLFWKeyF8)
	GLFWKeyF9           = GLFWKey(C.GLFWKeyF9)
	GLFWKeyF10          = GLFWKey(C.GLFWKeyF10)
	GLFWKeyF11          = GLFWKey(C.GLFWKeyF11)
	GLFWKeyF12          = GLFWKey(C.GLFWKeyF12)
	GLFWKeyF13          = GLFWKey(C.GLFWKeyF13)
	GLFWKeyF14          = GLFWKey(C.GLFWKeyF14)
	GLFWKeyF15          = GLFWKey(C.GLFWKeyF15)
	GLFWKeyF16          = GLFWKey(C.GLFWKeyF16)
	GLFWKeyF17          = GLFWKey(C.GLFWKeyF17)
	GLFWKeyF18          = GLFWKey(C.GLFWKeyF18)
	GLFWKeyF19          = GLFWKey(C.GLFWKeyF19)
	GLFWKeyF20          = GLFWKey(C.GLFWKeyF20)
	GLFWKeyF21          = GLFWKey(C.GLFWKeyF21)
	GLFWKeyF22          = GLFWKey(C.GLFWKeyF22)
	GLFWKeyF23          = GLFWKey(C.GLFWKeyF23)
	GLFWKeyF24          = GLFWKey(C.GLFWKeyF24)
	GLFWKeyF25          = GLFWKey(C.GLFWKeyF25)
	GLFWKeyKp0          = GLFWKey(C.GLFWKeyKp0)
	GLFWKeyKp1          = GLFWKey(C.GLFWKeyKp1)
	GLFWKeyKp2          = GLFWKey(C.GLFWKeyKp2)
	GLFWKeyKp3          = GLFWKey(C.GLFWKeyKp3)
	GLFWKeyKp4          = GLFWKey(C.GLFWKeyKp4)
	GLFWKeyKp5          = GLFWKey(C.GLFWKeyKp5)
	GLFWKeyKp6          = GLFWKey(C.GLFWKeyKp6)
	GLFWKeyKp7          = GLFWKey(C.GLFWKeyKp7)
	GLFWKeyKp8          = GLFWKey(C.GLFWKeyKp8)
	GLFWKeyKp9          = GLFWKey(C.GLFWKeyKp9)
	GLFWKeyKpDecimal    = GLFWKey(C.GLFWKeyKpDecimal)
	GLFWKeyKpDivide     = GLFWKey(C.GLFWKeyKpDivide)
	GLFWKeyKpMultiply   = GLFWKey(C.GLFWKeyKpMultiply)
	GLFWKeyKpSubtract   = GLFWKey(C.GLFWKeyKpSubtract)
	GLFWKeyKpAdd        = GLFWKey(C.GLFWKeyKpAdd)
	GLFWKeyKpEnter      = GLFWKey(C.GLFWKeyKpEnter)
	GLFWKeyKpEqual      = GLFWKey(C.GLFWKeyKpEqual)
	GLFWKeyLeftShift    = GLFWKey(C.GLFWKeyLeftShift)
	GLFWKeyLeftControl  = GLFWKey(C.GLFWKeyLeftControl)
	GLFWKeyLeftAlt      = GLFWKey(C.GLFWKeyLeftAlt)
	GLFWKeyLeftSuper    = GLFWKey(C.GLFWKeyLeftSuper)
	GLFWKeyRightShift   = GLFWKey(C.GLFWKeyRightShift)
	GLFWKeyRightControl = GLFWKey(C.GLFWKeyRightControl)
	GLFWKeyRightAlt     = GLFWKey(C.GLFWKeyRightAlt)
	GLFWKeyRightSuper   = GLFWKey(C.GLFWKeyRightSuper)
	GLFWKeyMenu         = GLFWKey(C.GLFWKeyMenu)
)

type GLFWModifierKey int

const (
	GLFWModShift    = GLFWModifierKey(C.GLFWModShift)
	GLFWModControl  = GLFWModifierKey(C.GLFWModControl)
	GLFWModAlt      = GLFWModifierKey(C.GLFWModAlt)
	GLFWModSuper    = GLFWModifierKey(C.GLFWModSuper)
	GLFWModCapsLock = GLFWModifierKey(C.GLFWModCapsLock)
	GLFWModNumLock  = GLFWModifierKey(C.GLFWModNumLock)
)

var _ backend.Backend[GLFWWindowFlags] = &GLFWVulkanBackend{}

type GLFWVulkanBackend struct {
	afterCreateContext   voidCallbackFunc
	loop                 voidCallbackFunc
	beforeRender         voidCallbackFunc
	afterRender          voidCallbackFunc
	beforeDestroyContext voidCallbackFunc
	dropCB               backend.DropCallback
	closeCB              func(pointer unsafe.Pointer)
	keyCb                backend.KeyCallback
	sizeCb               backend.SizeChangeCallback
	window               uintptr
}

func NewGLFWBackend() *GLFWVulkanBackend {
	b := &GLFWVulkanBackend{}
	if C.igInitGLFW() == 0 {
		panic("Failed to initialize GLFW")
	}

	return b
}

// ForceX11 is here, because some imgui features are not available on wayland.
// Call this BEFORE creating glfw backend!
func ForceX11() {
	C.glfwInitHint(C.GLFWInitHintPlatform, C.int(C.GLFWPlatformX11))
}

func (b *GLFWVulkanBackend) handle() *C.GLFWwindow {
	return (*C.GLFWwindow)(unsafe.Pointer(b.window))
}

func (b *GLFWVulkanBackend) SetAfterCreateContextHook(hook func()) {
	b.afterCreateContext = hook
}

func (b *GLFWVulkanBackend) AfterCreateHook() func() {
	return b.afterCreateContext
}

func (b *GLFWVulkanBackend) SetBeforeDestroyContextHook(hook func()) {
	b.beforeDestroyContext = hook
}

func (b *GLFWVulkanBackend) BeforeDestroyHook() func() {
	return b.beforeDestroyContext
}

func (b *GLFWVulkanBackend) SetBeforeRenderHook(hook func()) {
	b.beforeRender = hook
}

func (b *GLFWVulkanBackend) BeforeRenderHook() func() {
	return b.beforeRender
}

func (b *GLFWVulkanBackend) SetAfterRenderHook(hook func()) {
	b.afterRender = hook
}

func (b *GLFWVulkanBackend) AfterRenderHook() func() {
	return b.afterRender
}

func (b *GLFWVulkanBackend) SetBgColor(color imgui.Vec4) {
	c := color.ToC()
	C.igSetBgColor(*((*C.ImVec4)(unsafe.Pointer(&c))))
}

func (b *GLFWVulkanBackend) Run(loop func()) {
	b.loop = loop
	C.igGLFWRunLoop(b.handle(), C.VoidCallback(backend.LoopCallback()), C.VoidCallback(backend.BeforeRender()), C.VoidCallback(backend.AfterRender()), C.VoidCallback(backend.BeforeDestroyContext()))
}

func (b *GLFWVulkanBackend) LoopFunc() func() {
	return b.loop
}

func (b *GLFWVulkanBackend) DropCallback() backend.DropCallback {
	return b.dropCB
}

func (b *GLFWVulkanBackend) CloseCallback() func(wnd unsafe.Pointer) {
	return b.closeCB
}

func (b *GLFWVulkanBackend) SetWindowPos(x, y int) {
	C.igGLFWWindow_SetWindowPos(b.handle(), C.int(x), C.int(y))
}

func (b *GLFWVulkanBackend) GetWindowPos() (x, y int32) {
	xArg, xFin := internal.WrapNumberPtr[C.int, int32](&x)
	defer xFin()

	yArg, yFin := internal.WrapNumberPtr[C.int, int32](&y)
	defer yFin()

	C.igGLFWWindow_GetWindowPos(b.handle(), xArg, yArg)

	return
}

func (b *GLFWVulkanBackend) SetWindowSize(width, height int) {
	C.igGLFWWindow_SetSize(b.handle(), C.int(width), C.int(height))
}

func (b *GLFWVulkanBackend) DisplaySize() (width int32, height int32) {
	widthArg, widthFin := internal.WrapNumberPtr[C.int, int32](&width)
	defer widthFin()

	heightArg, heightFin := internal.WrapNumberPtr[C.int, int32](&height)
	defer heightFin()

	C.igGLFWWindow_GetDisplaySize(b.handle(), widthArg, heightArg)

	return
}

func (b *GLFWVulkanBackend) ContentScale() (width, height float32) {
	widthArg, widthFin := internal.WrapNumberPtr[C.float, float32](&width)
	defer widthFin()

	heightArg, heightFin := internal.WrapNumberPtr[C.float, float32](&height)
	defer heightFin()

	C.igGLFWWindow_GetContentScale(b.handle(), widthArg, heightArg)

	return
}

func (b *GLFWVulkanBackend) SetWindowTitle(title string) {
	titleArg, titleFin := internal.WrapString[C.char](title)
	defer titleFin()

	C.igGLFWWindow_SetTitle(b.handle(), (*C.char)(titleArg))
}

// The minimum and maximum size of the content area of a windowed mode window.
// To specify only a minimum size or only a maximum one, set the other pair to -1
// e.g. SetWindowSizeLimits(640, 480, -1, -1)
func (b *GLFWVulkanBackend) SetWindowSizeLimits(minWidth, minHeight, maxWidth, maxHeight int) {
	C.igGLFWWindow_SetSizeLimits(b.handle(), C.int(minWidth), C.int(minHeight), C.int(maxWidth), C.int(maxHeight))
}

func (b *GLFWVulkanBackend) SetShouldClose(value bool) {
	C.igGLFWWindow_SetShouldClose(b.handle(), C.bool((value)))
}

func (b *GLFWVulkanBackend) CreateWindow(title string, width, height int) {
	titleArg, titleFin := internal.WrapString[C.char](title)
	defer titleFin()

	b.window = uintptr(unsafe.Pointer(C.igCreateGLFWWindow(
		(*C.char)(titleArg),
		C.int(width),
		C.int(height),
		C.VoidCallback(backend.AfterCreateContext()),
	)))
	if b.window == 0 {
		panic("Failed to create GLFW window")
	}
}

func (b *GLFWVulkanBackend) AttachToExistingWindow(window *glfw.Window, instance vk.Instance, device vk.Device, physical_device vk.PhysicalDevice,
	graphics_queue vk.Queue, pipeline_cache vk.PipelineCache, graphics_queue_family uint32, swapchainImageResources []*as.SwapchainImageResources, swapchainDimensions *as.SwapchainDimensions) {

	swapchainImageCount := len(swapchainImageResources)
	swapchainFormat := swapchainDimensions.Format
	width := swapchainDimensions.Width
	height := swapchainDimensions.Height

	imageViews := make([]unsafe.Pointer, swapchainImageCount)
	swapchainImages := make([]unsafe.Pointer, swapchainImageCount)

	for i, resource := range swapchainImageResources {
		imageViews[i] = unsafe.Pointer(resource.View())
		swapchainImages[i] = unsafe.Pointer(resource.Image())
	}

	// Convert image views to be C compatible
	cImageViews := make([]*C.void, swapchainImageCount)
	cSwapchainImages := make([]*C.void, swapchainImageCount)

	for i, view := range imageViews {
		cImageViews[i] = (*C.void)(view)
		cSwapchainImages[i] = (*C.void)(swapchainImages[i])
	}

	// C array of image views
	// imageViewsArray := (*unsafe.Pointer)(unsafe.Pointer(&cImageViews[0]))
	// swapchainImagesArray := (*unsafe.Pointer)(unsafe.Pointer(&cSwapchainImages[0]))

	// The actual C glfw window handle is hidden as a private member of glfw.Window, use reflection to retrieve it
	//v := reflect.ValueOf(window).Elem()
	//dataField := v.FieldByName("data")
	//cGlfwWindow := unsafe.Pointer(dataField.Pointer())

	C.igAttachToExistingWindow(
		(*C.GLFWwindow)(window.Handle()),
		(C.VkInstance)(unsafe.Pointer(instance)),
		(C.VkDevice)(unsafe.Pointer(device)),
		(C.VkPhysicalDevice)(unsafe.Pointer(physical_device)),
		(C.VkQueue)(unsafe.Pointer(graphics_queue)),
		(C.VkPipelineCache)(unsafe.Pointer(pipeline_cache)),
		C.uint32_t(graphics_queue_family),
		(*C.VkImageView)(unsafe.Pointer(&imageViews[0])),
		(*C.VkImage)(unsafe.Pointer(&swapchainImages[0])),
		C.VkFormat(swapchainFormat),
		C.uint32_t(swapchainImageCount),
		C.int(width),
		C.int(height),
	)
}

func (b *GLFWVulkanBackend) SetTargetFPS(fps uint) {
	C.igSetTargetFPS(C.uint(fps))
}

func (b *GLFWVulkanBackend) Refresh() {
	C.igRefresh()
}

func (b *GLFWVulkanBackend) CreateTexture(pixels unsafe.Pointer, width, height int) imgui.TextureRef {
	tex := C.igCreateTexture((*C.uchar)(pixels), C.int(width), C.int(height))
	return *imgui.NewTextureRefTextureID(*imgui.NewTextureIDFromC(&tex))
}

func (b *GLFWVulkanBackend) CreateTextureRgba(img *image.RGBA, width, height int) imgui.TextureRef {
	tex := C.igCreateTexture((*C.uchar)(&(img.Pix[0])), C.int(width), C.int(height))
	return *imgui.NewTextureRefTextureID(*imgui.NewTextureIDFromC(&tex))
}

func (b *GLFWVulkanBackend) DeleteTexture(id imgui.TextureRef) {
	C.igDeleteTexture(C.ImTextureID(unsafe.Pointer(uintptr(id.TexID()))))
}

// SetDropCallback sets the drop callback which is called when an object
// is dropped over the window.
func (b *GLFWVulkanBackend) SetDropCallback(cbfun backend.DropCallback) {
	b.dropCB = cbfun
	C.igGLFWWindow_SetDropCallbackCB(b.handle())
}

func (b *GLFWVulkanBackend) SetCloseCallback(cbfun backend.WindowCloseCallback) {
	b.closeCB = func(_ unsafe.Pointer) {
		cbfun()
	}

	C.igGLFWWindow_SetCloseCallback(b.handle())
}

// SetWindowHint applies to next CreateWindow call
// so use it before CreateWindow call ;-)
func (b *GLFWVulkanBackend) SetWindowFlags(flag GLFWWindowFlags, value int) {
	C.igWindowHint(C.GLFWWindowFlags(flag), C.int(value))
}

// SetIcons sets icons for the window.
// THIS CODE COMES FROM https://github.com/go-gl/glfw (BSD-3 clause) - Copyright (c) 2012 The glfw3-go Authors. All rights reserved.
func (b *GLFWVulkanBackend) SetIcons(images ...image.Image) {
	count := len(images)
	cimages := make([]C.CImage, count)
	freePixels := make([]func(), count)

	for i, img := range images {
		var pixels []uint8
		b := img.Bounds()

		switch img := img.(type) {
		case *image.NRGBA:
			pixels = img.Pix
		default:
			m := image.NewNRGBA(image.Rect(0, 0, b.Dx(), b.Dy()))
			draw.Draw(m, m.Bounds(), img, b.Min, draw.Src)
			pixels = m.Pix
		}

		pix, free := func(origin []byte) (pointer *uint8, free func()) {
			n := len(origin)
			if n == 0 {
				return nil, func() {}
			}

			ptr := C.CBytes(origin)
			return (*uint8)(ptr), func() { C.free(ptr) }
		}(pixels)

		freePixels[i] = free

		cimages[i].width = C.int(b.Dx())
		cimages[i].height = C.int(b.Dy())
		cimages[i].pixels = (*C.uchar)(pix)
	}

	var p *C.CImage
	if count > 0 {
		p = &cimages[0]
	}
	C.igGLFWWindow_SetIcon(b.handle(), C.int(count), p)

	for _, v := range freePixels {
		v()
	}
}

func (b *GLFWVulkanBackend) SetKeyCallback(cbfun backend.KeyCallback) {
	b.keyCb = func(k, s, a, m int) {
		C.iggImplGlfw_KeyCallback(b.handle(), C.int(k), C.int(s), C.int(a), C.int(m))
		cbfun(k, s, a, m)
	}
	C.igGLFWWindow_SetKeyCallback(b.handle())
}

func (b *GLFWVulkanBackend) KeyCallback() backend.KeyCallback {
	return b.keyCb
}

func (b *GLFWVulkanBackend) SetSizeChangeCallback(cbfun backend.SizeChangeCallback) {
	b.sizeCb = cbfun
	C.igGLFWWindow_SetSizeCallback(b.handle())
}

func (b *GLFWVulkanBackend) SizeCallback() backend.SizeChangeCallback {
	return b.sizeCb
}

func (b *GLFWVulkanBackend) SetSwapInterval(interval GLFWWindowFlags) error {
	C.glfwSwapInterval(C.int(interval))
	return nil
}

func (b *GLFWVulkanBackend) SetCursorPos(x, y float64) {
	C.glfwSetCursorPos(b.handle(), C.double(x), C.double(y))
}

func (b *GLFWVulkanBackend) SetInputMode(mode GLFWWindowFlags, value GLFWWindowFlags) {
	C.glfwSetInputMode(b.handle(), C.int(mode), C.int(value))
}
