#ifndef			_XCC_HOOK_MOUSE_H_
#define			_XCC_HOOK_MOUSE_H_

#include <xcc-hook/header.h>

XCC_CXX_EXTERN_BEGIN



/// X-Hook mouse events
typedef enum x_hook_mouse_event
{
	X_HOOK_MOUSE_MOVE		= 0,
	X_HOOK_MOUSE_PRESS_L		= 1,
	X_HOOK_MOUSE_RELEASE_L		= 2,
	X_HOOK_MOUSE_PRESS_R		= 3,
	X_HOOK_MOUSE_RELEASE_R		= 4,
	X_HOOK_MOUSE_UNKNOWN		= 255
}x_hook_mouse_event_t;

/// X-Hook mouse point
typedef struct x_hook_mouse_data
{
	x_hook_mouse_event_t		event;
	int				x;
	int				y;
}x_hook_mouse_data_t;


/// Mouse event callback function type
/// \param _PointX : Mouse X coordinate
/// \param _PointY : Mouse Y coordinate
/// \param _UserData : Caller's custom parameters
/// \return : True is returned for continued downloading and false is returned for aborted transmission
typedef bool (__xcall__ * x_hook_mouse_event_cb_t)(const x_hook_mouse_data_t* _MouseData, void* _UserData);



/// initialize mouse hook
/// \return : Return true to indicate success and false to indicate failure
_XHOOKAPI_ bool __xcall__ x_hook_mouse_initialize();

// release mouse hook
_XHOOKAPI_ void __xcall__ x_hook_mouse_release();



/// Install a mouse hook
/// \param _Context : Context handle pointer to hook
/// \param _Callback : Mouse event callback function
/// \param _UserData : Caller's custom parameters
/// \return : Return true to indicate success and false to indicate failure
_XHOOKAPI_ bool __xcall__ x_hook_mouse_attach(x_hook_context_t* _Context, x_hook_mouse_event_cb_t _Callback, void* _UserData);

/// Uninstall a mouse hook
/// \param _Context : context handle
/// \return : Return true to indicate success and false to indicate failure
_XHOOKAPI_ bool __xcall__ x_hook_mouse_remove(x_hook_context_t _Context);



XCC_CXX_EXTERN_END

#endif
