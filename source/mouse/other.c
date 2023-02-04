#include <source/mouse/common.h>



/// initialize mouse hook
_XHOOKAPI_ bool __xcall__ x_hook_mouse_initialize()
{
	return false;
}

// release mouse hook
_XHOOKAPI_ void __xcall__ x_hook_mouse_release()
{
}



/// Install a mouse hook
_XHOOKAPI_ bool __xcall__ x_hook_mouse_attach(x_hook_context_t* _Context, x_hook_mouse_event_cb_t _Callback, void* _UserData)
{
	XCC_UNUSED(_Context);
	XCC_UNUSED(_Callback);
	XCC_UNUSED(_UserData);

	return false;
}

/// Uninstall a mouse hook
_XHOOKAPI_ bool __xcall__ x_hook_mouse_remove(x_hook_context_t _Context)
{
	XCC_UNUSED(_Context);

	return false;
}
