/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#include <sqlite.hpp>
#include <js_sqlite.hpp>

#include <cerrno>
#include <cstdlib>

namespace {

constexpr int jsvm_err_offset = -32;


struct ubb_js {
	static JSClass global_class;

	static void js_error_reporter(
		JSContext *cx, char const *message, JSErrorReport *report
	);

	static JSBool print(JSContext *cx, unsigned argc, jsval *vp);

	static JSFunctionSpec functions[];

	ubb_js()
	: runtime(JS_NewRuntime(1 << 24)),
	  ctx(runtime ? JS_NewContext(runtime, 8192) : nullptr),
	  error(0)
	{
		if (!runtime)
			error = jsvm_err_offset - 1;
		else if (!ctx)
			error = jsvm_err_offset - 2;

		JSAutoRequest req(ctx);
		g_obj = JS_NewGlobalObject(ctx, &global_class, nullptr);
		if (!g_obj) {
			error = jsvm_err_offset - 3;
			return;
		}

		JSAutoCompartment cmp(ctx, g_obj);
		JS_SetErrorReporter(ctx, &ubb_js::js_error_reporter);

		if (!JS_InitStandardClasses(ctx, g_obj))
			error = jsvm_err_offset - 4;

		if (!JS_DefineFunctions(ctx, g_obj, functions))
			error = jsvm_err_offset - 5;

		if (!ubb::js_sqlite_class_init(ctx, g_obj))
			error = jsvm_err_offset - 6;
	}

	~ubb_js()
	{
		if (ctx)
			JS_DestroyContext(ctx);

		if (runtime)
			JS_DestroyRuntime(runtime);

		JS_ShutDown();
	}

	JSRuntime *runtime;
	JSContext *ctx;
	JSObject *g_obj;
	int error;
};

JSClass ubb_js::global_class = {
	.name = "global",
	.flags = JSCLASS_NEW_RESOLVE | JSCLASS_GLOBAL_FLAGS,
	.addProperty = JS_PropertyStub,
	.delProperty = JS_PropertyStub,
	.getProperty = JS_PropertyStub,
	.setProperty = JS_StrictPropertyStub,
	.enumerate = JS_EnumerateStub,
	.resolve = JS_ResolveStub,
	.convert = JS_ConvertStub,
	.finalize = nullptr,
	.checkAccess = nullptr,
	.call = nullptr,
	.hasInstance = nullptr,
	.construct = nullptr,
	.trace = nullptr
};

void ubb_js::js_error_reporter(
	JSContext *ctx, char const *message, JSErrorReport *report
)
{
	printf("js: %s (%s)\n", message, report->linebuf);
}

JSBool ubb_js::print(JSContext *ctx, unsigned int argc, jsval *vp)
{
	jsval *args = JS_ARGV(cx, vp);
	JSString *str;

	for (decltype(argc) cnt(0); cnt < argc; ++cnt) {
		str = JS_ValueToString(ctx, args[cnt]);
		if (!str)
			return false;

		char *bytes(JS_EncodeString(ctx, str));
		if (!bytes)
			return false;
		printf("%s%s", cnt ? " " : "", bytes);
		JS_free(ctx, bytes);
	}

	JS_SET_RVAL(cx, vp, JSVAL_VOID);
	return true;
}

JSFunctionSpec ubb_js::functions[] = {
	JS_FN("print", &ubb_js::print, 0, 0),
	JS_FS_END
};

}

int main(int argc, char **argv)
{
	if (!ubb::sqlite_init())
		return -ELIBBAD;
	std::atexit(ubb::sqlite_shutdown);

	ubb_js app;
	if (app.error)
		return app.error;
	

	JSAutoRequest req(app.ctx);
	JSAutoCompartment cmp(app.ctx, app.g_obj);
	auto msg1(JS_NewStringCopyZ(app.ctx, "Message da orbi gratulata!\n"));
	auto msg2(JS_NewStringCopyZ(app.ctx, "Sequenudo huindo!\n"));
	jsval args[] = { STRING_TO_JSVAL(msg1), STRING_TO_JSVAL(msg2) };
	jsval rv = JSVAL_VOID;
	JS_CallFunctionName(app.ctx, app.g_obj, "print", 2, args, &rv);
	ubb::test();
	return 0;
}
