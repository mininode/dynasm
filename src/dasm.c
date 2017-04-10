#include <mininode/duktape.h>
#include <mininode/mininode.h>

const duk_function_list_entry dasm_funcs[] = {
	{"ActionList",        dasm_action_list,             DUK_VARARGS},
	{"build",             dasm_build,                   DUK_VARARGS},
	{"dump",              dasm_dump,                    0           },
	{"globals",           dasm_globals,                 0           },
	{ NULL, NULL, 0 }
};

const duk_function_list_entry dst_funcs[] = {
	{ NULL, NULL, 0 }
};

const duk_function_list_entry action_list_funcs[] = {
	{ NULL, NULL, 0 }
};

/* 
 * Ensure a constructor call.
 * Example: 
 *    var actions = new dasm.ActionList();
 *    var dst = new dasm(actions);
 */
inline duk_ret_t
ensure_constructor_call(duk_context *ctx) {
	if (!duk_is_constructor_call(ctx)) {
		return DUK_RET_TYPE_ERROR;
	}
}

/*
 * The action list lookup table is just an object in the global stash
 * that contains key:value pairs mapping strings to action lists.
 */
inline void
create_action_list_lookup_table(duk_context *ctx) {
	duk_idx_t stash_idx = duk_push_global_stash(ctx);
	duk_push_string(ctx, "actionlists")
	duk_idx_t obj_idx = duk_push_bare_object(ctx);
	duk_put_prop(ctx, stash_idx);
}

/*
 * Find an action list in the list lookup table.
 */
inline dasm_State *
find_action_list(duk_context *ctx, const char *list) {
}

duk_ret_t
action_list_constructor(duk_context *ctx) {
	const char *buf;

	ensure_constructor_call(ctx);

	buf = duk_require_string(ctx, 1);

	/* FIXME: http://duktape.org/guide.html#internalproperties */
	/* This should have an immutable internal property. */
	duk_idx_t stash_idx = duk_push_global_stash(ctx);
	duk_get_prop_string(ctx, -3, "actionlists");
	duk_push_string(ctx, buf);
	duk_push_pointer(ctx, (void *)dasm_ActList);
	duk_put_prop(ctx, stash_idx);
	duk_pop(ctx); /* We're done with the global stash. */

	duk_idx_t list_obj_idx = duk_push_this(ctx);
	duk_push_string(ctx, 'name');
	duk_push_string(ctx, buf);
	duk_put_prop(ctx, list_obj_idx);
	
	return 1;
}

duk_ret_t
dasm_constructor(duk_context *ctx) {
	/* Requires an ActionList object on the stack on entry. */
	dasm_State *ds = NULL;
	unsigned npc = 8;
	unsigned nextpc = 0;
	void *labels[lbl__MAX];

	ensure_constructor_call(ctx);

	duk_push_global_stash(ctx);
	ds = (dasm_State *)duk_push_fixed_buffer(ctx, sizeof(dasm_State));

	dasm_init(&ds, DASM_MAXSECTION);
	dasm_setupglobal(&ds, labels, lbl__MAX);
	dasm_setup(&ds, actions);
	/* Initialize npc dynamic labels. */
	dasm_growpc(&ds, npc);

	duk_push_this(ctx);

	/* TODO: Install function properties and make ourself immutable. */
}

/*
 * Here we call dasm_free() on our dasm_State struct 
 * and remove the action list from the lookup table.
 */
duk_ret_t
dasm_finalizer(duk_context *ctx) {
	
}

static void *
link_and_encode(dasm_State **ds) {
	size_t sz;
	void *buf;
	dasm_link(ds, &sz);
	buf = mmap(0, sz, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	dasm_encode(ds, buf);
	mprotect(buf, sz, PROT_READ | PROT_EXEC);
	return buf;
}
