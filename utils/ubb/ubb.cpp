/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#include <sqlite.hpp>
#include <js_sqlite.hpp>

#include <string>
#include <cerrno>
#include <cstdlib>

#include <readline/history.h>
#include <readline/readline.h>

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

bool stop_and_exit = false;

void interactive_shell(JSContext *ctx, JSObject *obj)
{
	char *str_in(nullptr);
	std::string str_buf;
	jsval rv;

	while (!stop_and_exit) {
		str_in = readline("ubb> ");
		if (!str_in)
			break;

		if (!str_buf.empty())
			str_buf.push_back(' ');

		str_buf.append(str_in);
		free(str_in);

		while (JS_TRUE != JS_BufferIsCompilableUnit(
			ctx, JS_TRUE, obj, str_buf.c_str(), str_buf.size()
		)) {
			str_in = readline("");
			if (!str_in)
				return;

			if (!str_buf.empty())
				str_buf.push_back(' ');

			str_buf.append(str_in);
			free(str_in);
		}

		if (JS_EvaluateScript(
			ctx, obj, str_buf.c_str(), str_buf.size(),
			nullptr, 0, &rv
		) && !JSVAL_IS_VOID(rv)) {
			JS::RootedString str_out(
				ctx, JS_ValueToString(ctx, rv)
			);
			auto p_str(JS_EncodeString(ctx, str_out));
			printf("%s\n", p_str);
			JS_free(ctx, p_str);
		}
		add_history(str_buf.c_str());
		str_buf.clear();
	}
}

}
/*
 * base 85

 0 - 9: 0 1 2 3 4 5 6 7 8 9
10 - 19: a b c d e f g h i j
20 - 29: k l m n o p q r s t
30 - 39: u v w x y z A B C D
40 - 49: E F G H I J K L M N
50 - 59: O P Q R S T U V W X
60 - 69: Y Z . - : + = ^ ! /
70 - 79: * ? & < > ( ) [ ] {
80 - 84: } @ % $ #


0x86 | 0x4F | 0xD2 | 0x6F | 0xB5 | 0x59 | 0xF7 | 0x5B

H | e | l | l | o | W | o | r | l | d

*/
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
	//ubb::test();
	interactive_shell(app.ctx, app.g_obj);
	return 0;
}
