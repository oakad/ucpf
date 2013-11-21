/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#include <jsapi.h>

namespace {

struct ubb_main {
	static JSClass global_class;

	ubb_main()
	: runtime(JS_NewRuntime(1 << 24)),
	  ctx(runtime ? JS_NewContext(runtime, 8192) : nullptr),
	  
	  error(0)
	{
		if (!runtime)
			error = -1;
		else if (!ctx)
			error = -2;
	}

	~ubb_main()
	{
		if (ctx)
			JS_DestroyContext(ctx);
		if (runtime)
			JS_DestroyRuntime(runtime);
		JS_ShutDown();
	}

	JSRuntime *runtime;
	JSContext *ctx;
	int error;
};

JSClass ubb_main::global_class = {
	"global",
	JSCLASS_NEW_RESOLVE | JSCLASS_GLOBAL_FLAGS,
	JS_PropertyStub, JS_PropertyStub,
	JS_PropertyStub, JS_StrictPropertyStub,
	JS_EnumerateStub, JS_ResolveStub,
	JS_ConvertStub, nullptr,
	JSCLASS_NO_OPTIONAL_MEMBERS
};

}

int main(int argc, char **argv)
{
	ubb_main app;
	if (app.error)
		return app.error;

	JSAutoRequest app_req(app.ctx);
	auto g_obj(JS_NewGlobalObject(
		app.ctx, &ubb_main::global_class, nullptr
	));
	if (!g_obj)
		return -3;

	JSAutoCompartment app_cmp(app.ctx, g_obj);
	JS_SetGlobalObject(app.ctx, g_obj);
	if (!JS_InitStandardClasses(app.ctx, g_obj))
		return -4;

	auto msg(JS_NewStringCopyZ(app.ctx, "Message da orbi gratulata!\n"));
	jsval args[] = { STRING_TO_JSVAL(msg) };
	jsval rv = JSVAL_VOID;
	JS_CallFunctionName(app.ctx, g_obj, "print", 1, args, &rv);
	printf("aaa\n");
	return 0;
}
