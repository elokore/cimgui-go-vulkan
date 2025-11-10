package main

import (
	"runtime"
)

func init() {
	runtime.LockOSThread()
}

func main() {
	runApplication()
	// currentBackend.SetAfterCreateContextHook(common.AfterCreateContext)
	// currentBackend.SetBeforeDestroyContextHook(common.BeforeDestroyContext)

	//currentBackend.SetBgColor(imgui.NewVec4(0.45, 0.55, 0.6, 1.0))

	// currentBackend.CreateWindow("Hello from cimgui-go", 1200, 900)

	// currentBackend.SetDropCallback(func(p []string) {
	// 	fmt.Printf("drop triggered: %v", p)
	// })

	// currentBackend.SetCloseCallback(func() {
	// 	fmt.Println("window is closing")
	// })

	// currentBackend.SetIcons(common.Image())

	// currentBackend.Run(common.Loop)
}
