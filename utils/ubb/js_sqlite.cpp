/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#include "js_sqlite.hpp"
#include <jsfriendapi.h>

#include <sqlite3.h>

#include <memory>

namespace {

template <typename T>
struct js_deleter {
	JSContext *ctx = nullptr;

	void operator()(T *ptr) const
	{
		JS_free(ctx, ptr);
	}
};

struct js_sqlite {
	static JSClass sqlite_class;
	static JSFunctionSpec methods[];
	static JSPropertySpec static_properties[];

	static JSBool create(JSContext *ctx, unsigned int argc, JS::Value *vp)
	{
		std::unique_ptr<
			char, js_deleter<char>
		> db_name(nullptr), vfs_name(nullptr);
		uint32_t flags(0);

		db_name.get_deleter().ctx = ctx;
		vfs_name.get_deleter().ctx = ctx;

		printf("create %d\n", argc);
		switch (argc) {
		case 1: {
			JSString *db_name_;
			if (!JS_ConvertArguments(
				ctx, argc, JS_ARGV(ctx, vp), "S", &db_name_
			))
				return JS_FALSE;
			else {
				db_name.reset(JS_EncodeString(ctx, db_name_));
			}
			break;
		}
		case 2: {
			JSString *db_name_;
			if (!JS_ConvertArguments(
				ctx, argc, JS_ARGV(ctx, vp), "Su", &db_name,
				&flags
			))
				return JS_FALSE;
			else {
				db_name.reset(JS_EncodeString(ctx, db_name_));
			}
			break;
		}
		case 3: {
			JSString *db_name_, *vfs_name_;

			if (!JS_ConvertArguments(
				ctx, argc, JS_ARGV(ctx, vp), "SuS", &db_name,
				&flags, &vfs_name_
			))
				return JS_FALSE;
			else {
				db_name.reset(JS_EncodeString(ctx, db_name_));
				vfs_name.reset(JS_EncodeString(ctx, vfs_name_));
			}
			break;
		}
		};

		JS::RootedObject obj(ctx, JS_NewObjectForConstructor(
			ctx, &sqlite_class, vp
		));

		if (!obj)
			return JS_FALSE;

		if (!JS_FreezeObject(ctx, obj))
			return JS_FALSE;

		sqlite3 *db(nullptr);
		auto rc(sqlite3_open_v2(
			db_name.get(), &db, flags, vfs_name.get()
		));
		printf("open %d, %p\n", rc, db);
		if (rc != SQLITE_OK) {
			JS_ReportError(
				ctx, "sqlite: open failed with error %d", rc
			);
			return JS_FALSE;
		}

		js::SetReservedSlot(obj, 0, PRIVATE_TO_JSVAL(db));
		*vp = OBJECT_TO_JSVAL(obj);
		return JS_TRUE;
	}

	static void finalize(JSFreeOp *op, JSObject *obj)
	{
		sqlite3 *db(reinterpret_cast<sqlite3 *>(
			JSVAL_TO_PRIVATE(js::GetReservedSlot(obj, 0))
		));
		if (db)
			sqlite3_close(db);
	}

	static JSBool open_flags(
		JSContext *ctx, JSHandleObject obj, JSHandleId id,
		JSMutableHandleValue vp
	)
	{
		printf("open_flags %d\n", JSID_TO_INT(id.get()));
		*vp.address() = INT_TO_JSVAL(1 << (JSID_TO_INT(id.get()) - 1));
		return JS_TRUE;
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

JSPropertySpec js_sqlite::static_properties[] = {
	{
		"OPEN_READONLY", 1, JSPROP_READONLY | JSPROP_SHARED,
		&open_flags, nullptr
	},
	{
		"OPEN_READWRITE", 2, JSPROP_READONLY | JSPROP_SHARED,
		&open_flags, nullptr
	},
	{
		"OPEN_URI", 7, JSPROP_READONLY | JSPROP_SHARED,
		&open_flags, nullptr
	},
	{
		"OPEN_MEMORY", 8, JSPROP_READONLY | JSPROP_SHARED,
		&open_flags, nullptr
	},
	{nullptr, 0, 0, nullptr, nullptr}
};

}

namespace ubb {


bool js_sqlite_class_init(JSContext *ctx, JSObject *obj)
{
	JSAutoRequest req(ctx);
	JS::RootedObject js_sqlite_proto(ctx, JS_InitClass(
		ctx, obj, nullptr, &js_sqlite::sqlite_class,
		js_sqlite::create, 1, nullptr, js_sqlite::methods,
		js_sqlite::static_properties, nullptr
	));

	if (js_sqlite_proto) {
		js::SetReservedSlot(
			js_sqlite_proto, 0, PRIVATE_TO_JSVAL(nullptr)
		);
		return true;
	} else
		return false;
}

}
