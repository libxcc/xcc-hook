#include <xcc-hook/xcc.h>


// Dll main
#if defined(XCC_SYSTEM_WINDOWS)
BOOL WINAPI DllMain(HANDLE _HDllHandle, DWORD _Reason, LPVOID _Reserved)
{
	XCC_UNUSED(_HDllHandle);
	XCC_UNUSED(_Reserved);

	switch(_Reason)
	{
		case DLL_PROCESS_ATTACH:
			break;
		case DLL_PROCESS_DETACH:
			x_hook_mouse_release();
			break;
		default:
			break;
	}
	return TRUE;
}
#else
__attribute((constructor)) void xcc_hook_dynamic_library_init(void)
{
};

__attribute((destructor)) void xcc_hook_dynamic_library_fini(void)
{
	x_hook_mouse_release();
};
#endif
