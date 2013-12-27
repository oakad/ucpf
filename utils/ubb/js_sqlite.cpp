/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#include "js_sqlite.hpp"
#include <jsfriendapi.h>

namespace {

struct js_sqlite {
	static JSClass sqlite_class;
	static JSFunctionSpec methods[];

	static JSBool create(JSContext *ctx, unsigned int argc, JS::Value *vp)
	{
		jschar *db_name, *vfs_name;
		uint32_t flags;

		if (!JS_ConvertArguments(
			ctx, argc, JS_ARGV(ctx, vp), "WuW", &db_name, &flags,
			&vfs_name
		))
			return JS_FALSE;

		JS::RootedObject obj(ctx, JS_NewObjectForConstructor(
			ctx, &sqlite_class, vp
		));
		if (!obj)
			return JS_FALSE;

		if (!JS_FreezeObject(ctx, obj))
			return JS_FALSE;

		//JS_ReportError

		js::SetReservedSlot(
			obj, 0, PRIVATE_TO_JSVAL((void *)0x1234)
		);

		 *vp = OBJECT_TO_JSVAL(obj);
		return JS_TRUE;
	}

	static void finalize(JSFreeOp *op, JSObject *obj)
	{
	}
};

JSClass js_sqlite::sqlite_class = {
	.name = "SQLITE",
	.flags = JSCLASS_NEW_RESOLVE | JSCLASS_HAS_RESERVED_SLOTS(1),
	.addProperty = JS_PropertyStub,
	.delProperty = JS_PropertyStub,
	.getProperty = JS_PropertyStub,
	.setProperty = JS_StrictPropertyStub,
	.enumerate = JS_EnumerateStub,
	.resolve = JS_ResolveStub,
	.convert = JS_ConvertStub,
	.finalize = &finalize,
	.checkAccess = nullptr,
	.call = nullptr,
	.hasInstance = nullptr,
	.construct = nullptr,
	.trace = nullptr
};

JSFunctionSpec js_sqlite::methods[] = {
	JS_FS_END
};


}

namespace ubb {


bool js_sqlite_class_init(JSContext *ctx, JSObject *obj)
{
	JSAutoRequest req(ctx);
	JS::RootedObject js_sqlite_proto(ctx, JS_InitClass(
		ctx, obj, nullptr, &js_sqlite::sqlite_class,
		js_sqlite::create, 3, nullptr, js_sqlite::methods,
		nullptr, nullptr
	));

	return js_sqlite_proto ? true : false;
}

}
