#include <source/mouse/common.h>


// Mouse event message ID
#define		X_HOOK_MESSAGE_MOUSE			(WM_USER + 0x1000)


// System-level shared data
#pragma data_seg("x_hook_mouse_shared_data")
HGLOBAL					x_hook_mouse_shared_memory = NULL;
HGLOBAL					x_hook_mouse_shared_data = NULL;
HWND 					x_hook_mouse_shared_window = NULL;
HHOOK					x_hook_mouse_shared_handle = NULL;
#pragma data_seg()

// Application-level shared data
static x_mutex_t 			x_hook_mouse_process_mutex;
static x_hook_mouse_context*		x_hook_mouse_process_array = NULL;



// System hook callback processing
static LRESULT CALLBACK x_hook_mouse_system_callback(int _Code, WPARAM wParam, LPARAM lParam)
{
	XCC_UNUSED(wParam);

	/*
	WM_LBUTTONDOWN;
	WM_LBUTTONUP;
	WM_MOUSEMOVE;
	WM_MOUSEWHEEL;
	WM_MOUSEHWHEEL;
	WM_RBUTTONDOWN;
	WM_RBUTTONUP;
	*/

	if(x_hook_mouse_shared_window)
	{
		x_hook_mouse_data_t*	vMouseData = (x_hook_mouse_data_t*) GlobalLock(x_hook_mouse_shared_data);
		MSLLHOOKSTRUCT*		vMouseStruct = (MSLLHOOKSTRUCT*)lParam;
		switch(wParam)
		{
			case WM_LBUTTONDOWN:
				vMouseData->event = X_HOOK_MOUSE_PRESS_L;
				break;
			case WM_LBUTTONUP:
				vMouseData->event = X_HOOK_MOUSE_RELEASE_L;
				break;
			case WM_MOUSEMOVE:
				vMouseData->event = X_HOOK_MOUSE_MOVE;
				break;
			case WM_RBUTTONDOWN:
				vMouseData->event = X_HOOK_MOUSE_PRESS_R;
				break;
			case WM_RBUTTONUP:
				vMouseData->event = X_HOOK_MOUSE_RELEASE_R;
				break;
			default:
				vMouseData->event = X_HOOK_MOUSE_UNKNOWN;
				break;
		}
		vMouseData->x = vMouseStruct->pt.x;
		vMouseData->y = vMouseStruct->pt.y;
		GlobalUnlock(x_hook_mouse_shared_data);
		PostMessageW(x_hook_mouse_shared_window, X_HOOK_MESSAGE_MOUSE, (WPARAM)x_hook_mouse_shared_memory, (LPARAM)x_hook_mouse_shared_data);

		int*			vGlobalMemory = (int*) GlobalLock(x_hook_mouse_shared_memory);
		if(vGlobalMemory && *vGlobalMemory == X_HOOK_EVENT_ACCEPT)
		{
			GlobalUnlock(x_hook_mouse_shared_memory);
			return TRUE;
		}
		GlobalUnlock(x_hook_mouse_shared_memory);
	}
	return CallNextHookEx(x_hook_mouse_shared_handle, _Code, wParam, lParam);
}

// Listen for event window callbacks
static LRESULT CALLBACK x_hook_mouse_window_callback(HWND _Hwnd, UINT _Message, WPARAM wParam, LPARAM lParam)
{
	if(_Message == WM_CREATE)
	{
		x_hook_mouse_shared_handle = SetWindowsHookExW(WH_MOUSE_LL, x_hook_mouse_system_callback, NULL, 0);
	}
	else if(_Message == WM_CLOSE)
	{
		DestroyWindow(_Hwnd);
	}
	else if(_Message == WM_DESTROY)
	{
		UnhookWindowsHookEx(x_hook_mouse_shared_handle);
		x_hook_mouse_shared_handle = NULL;
		PostQuitMessage(0);
	}
	else if(_Message == X_HOOK_MESSAGE_MOUSE)
	{
		x_mutex_lock(&x_hook_mouse_process_mutex);

		int*			vMouseStatus = (int*) GlobalLock((HGLOBAL)wParam);
		x_hook_mouse_data_t*	vMouseData = (x_hook_mouse_data_t*) GlobalLock((HGLOBAL)lParam);
		x_hook_mouse_context*	vNode = x_hook_mouse_process_array;
		if(vMouseStatus)
		{
			(*vMouseStatus) = X_HOOK_EVENT_IGNORE;
			while (vNode && (*vMouseStatus) == X_HOOK_EVENT_IGNORE)
			{
				(*vMouseStatus) = vNode->cb_address(vMouseData, vNode->cb_user_data);
				vNode = vNode->next;
			}
		}

		x_mutex_unlock(&x_hook_mouse_process_mutex);
		GlobalUnlock((HGLOBAL)wParam);
	}
	return DefWindowProcW(_Hwnd, _Message, wParam, lParam);
}

// Listen for event window loop
static void __xcall__ x_hook_mouse_window_loop(void* _Param)
{
	XCC_UNUSED(_Param);

	wchar_t 	vWindowName[XCC_PATH_MAX] = {0};
	HMODULE		vModuleHandle = GetModuleHandleW(NULL);
	HWND		vWindowHandle = NULL;
	MSG		vMessage;
	WNDCLASSEXW	vWCEX;

	swprintf(vWindowName, XCC_PATH_MAX, L"x_hook_mouse_window_0x%016llX", (uint64_t)GetCurrentProcessId());

	vWCEX.cbSize = sizeof(WNDCLASSEX);
	vWCEX.style = CS_HREDRAW | CS_VREDRAW;
	vWCEX.lpfnWndProc = x_hook_mouse_window_callback;
	vWCEX.cbClsExtra = 0;
	vWCEX.cbWndExtra = 0;
	vWCEX.hInstance = vModuleHandle;
	vWCEX.hIcon = NULL;
	vWCEX.hCursor = LoadCursorA(NULL, IDC_ARROW);
	vWCEX.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	vWCEX.lpszMenuName = NULL;
	vWCEX.lpszClassName = vWindowName;
	vWCEX.hIconSm = NULL;
	RegisterClassExW(&vWCEX);

	vWindowHandle = CreateWindowExW(WS_EX_TOPMOST, vWindowName, vWindowName, WS_POPUP, 0, 0, 400, 300, NULL, NULL, vModuleHandle, NULL);
	if(vWindowHandle == NULL)
	{
		return;
	}

	ShowWindow(vWindowHandle, SW_HIDE);
	UpdateWindow(vWindowHandle);
	SetWindowTextW(vWindowHandle, vWindowName);

	x_hook_mouse_shared_window = vWindowHandle;
	while(GetMessageW(&vMessage, NULL, 0U, 0U) > 0)
	{
		TranslateMessage(&vMessage);
		DispatchMessageW(&vMessage);
	}
	x_hook_mouse_shared_window = NULL;
}



// initialize mouse hook
_XHOOKAPI_ bool __xcall__ x_hook_mouse_initialize()
{
	if(x_hook_mouse_shared_window)
	{
		return true;
	}

	// Step 1. Initialize various data
	x_hook_mouse_shared_memory = GlobalAlloc(GMEM_FIXED, sizeof(int));
	x_hook_mouse_shared_data = GlobalAlloc(GMEM_FIXED, sizeof(x_hook_mouse_data_t));
	x_mutex_init(&x_hook_mouse_process_mutex);
	x_hook_mouse_process_array = NULL;

	// Step 2. Create a shared window for listening to mouse events
	x_thread_create(x_hook_mouse_window_loop, NULL);

	// Step 3. Check for success and return
	for(int vIndex = 0; vIndex < 1000 && x_hook_mouse_shared_window == NULL; ++vIndex)
	{
		x_posix_msleep(1);
	}
	return x_hook_mouse_shared_window;
}

// release mouse hook
_XHOOKAPI_ void __xcall__ x_hook_mouse_release()
{
	if(x_hook_mouse_shared_window == NULL)
	{
		return;
	}

	// Step 1. Close the event listener window
	PostMessageW(x_hook_mouse_shared_window, WM_CLOSE, (WPARAM)NULL, (LPARAM)NULL);
	while(x_hook_mouse_shared_window)
	{
		x_posix_msleep(1);
	}

	// Step2. clean up resources
	x_mutex_destroy(&x_hook_mouse_process_mutex);
	GlobalFree(x_hook_mouse_shared_memory);
	GlobalFree(x_hook_mouse_shared_data);
	x_hook_mouse_shared_memory = NULL;
	while(x_hook_mouse_process_array)
	{
		x_hook_mouse_context*	vNode = x_hook_mouse_process_array->next;
		x_posix_free(x_hook_mouse_process_array);
		x_hook_mouse_process_array = vNode;
	}
}



/// Install a mouse hook
_XHOOKAPI_ bool __xcall__ x_hook_mouse_attach(x_hook_context_t* _Context, x_hook_mouse_event_cb_t _Callback, void* _UserData)
{
	if(_Context == NULL || _Callback == NULL)
	{
		return false;
	}

	x_hook_mouse_context*	vNode = (x_hook_mouse_context*) x_posix_malloc(sizeof(x_hook_mouse_context));
	if(vNode == NULL)
	{
		return false;
	}
	x_posix_memset(vNode, 0, sizeof(x_hook_mouse_context));
	vNode->cb_address = _Callback;
	vNode->cb_user_data = _UserData;

	x_mutex_lock(&x_hook_mouse_process_mutex);

	if(x_hook_mouse_process_array)
	{
		vNode->next = x_hook_mouse_process_array;
		x_hook_mouse_process_array->prev = vNode;
	}
	x_hook_mouse_process_array = vNode;
	*_Context = vNode;

	x_mutex_unlock(&x_hook_mouse_process_mutex);
	return true;
}

/// Uninstall a mouse hook
_XHOOKAPI_ bool __xcall__ x_hook_mouse_remove(x_hook_context_t _Context)
{
	if(_Context == NULL)
	{
		return false;
	}

	x_mutex_lock(&x_hook_mouse_process_mutex);

	x_hook_mouse_context*	vNode = (x_hook_mouse_context*)_Context;
	if(x_hook_mouse_process_array == vNode)
	{
		x_hook_mouse_process_array = x_hook_mouse_process_array->next;
	}
	else
	{
		vNode->prev->next = vNode->next;
		if(vNode->next)
		{
			vNode->next->prev = vNode->prev;
		}
	}
	x_posix_free(vNode);

	x_mutex_unlock(&x_hook_mouse_process_mutex);
	return true;
}
