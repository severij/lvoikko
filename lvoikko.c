#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <lua.h>
#include <lauxlib.h>
#include <libvoikko/voikko.h>

#define MT_NAME "VOIKKO_HANDLE"
#define HANDLE_REGISTRY_KEY "VOIKKO_HANDLE_REGISTRY_KEY"
#define N_OPTIONS 16
#define MAX_N_SUGGESTIONS 100

typedef struct opt_info {
	const int opt_id;
	const int opt_type;
} opt_info_t;

typedef struct opt_map {
	const char* opt_str;
	const opt_info_t opt_info;
} opt_map_t;

static const opt_map_t options[] = {
	{"accept_all_uppercase", {VOIKKO_OPT_ACCEPT_ALL_UPPERCASE, 0}},
	{"accept_bulleted_lists", {VOIKKO_OPT_ACCEPT_BULLETED_LISTS_IN_GC, 0}},
	{"accept_extra_hyphens", {VOIKKO_OPT_ACCEPT_EXTRA_HYPHENS, 0}},
	{"accept_first_uppercase", {VOIKKO_OPT_ACCEPT_FIRST_UPPERCASE, 0}},
	{"accept_missing_hyphens", {VOIKKO_OPT_ACCEPT_MISSING_HYPHENS, 0}},
	{"hyphenate_unknown_words", {VOIKKO_OPT_HYPHENATE_UNKNOWN_WORDS, 0}},
	{"ignore_dot", {VOIKKO_OPT_IGNORE_DOT, 0}},
	{"ignore_nonwords", {VOIKKO_OPT_IGNORE_NONWORDS, 0}},
	{"ignore_numbers", {VOIKKO_OPT_IGNORE_NUMBERS, 0}},
	{"ignore_uppercase", {VOIKKO_OPT_IGNORE_UPPERCASE, 0}},
	{"min_hyphenated_word_length", {VOIKKO_MIN_HYPHENATED_WORD_LENGTH, 1}},
	{"no_ugly_hyphenation", {VOIKKO_OPT_NO_UGLY_HYPHENATION, 0}},
	{"ocr_suggestions", {VOIKKO_OPT_OCR_SUGGESTIONS, 0}},
	{"speller_cache_size", {VOIKKO_SPELLER_CACHE_SIZE, 1}},
};

struct VoikkoHandle *get_handle(lua_State *L) {
	lua_pushlightuserdata(L, HANDLE_REGISTRY_KEY);
	lua_gettable(L, LUA_REGISTRYINDEX);
	return lua_touserdata(L, -1);
}

void set_handle(lua_State *L, struct VoikkoHandle *handle) {
	lua_pushlightuserdata(L, HANDLE_REGISTRY_KEY);
	lua_pushlightuserdata(L, handle);
	lua_settable(L, LUA_REGISTRYINDEX);
}

const opt_info_t *get_option_info(const char *opt_str) {
	int i;
	for (i=0; i < N_OPTIONS; i++) {
		if (strcmp(opt_str, options[i].opt_str)) {
			return &options[i].opt_info;
		}
	}
	return NULL;
}

int set_option(lua_State *L) {

	struct VoikkoHandle *handle;
	const char *key = lua_tostring(L, 1);
	int value;

	const opt_info_t *opt_info = get_option_info(key);

	if (!opt_info) {
		luaL_error(L, "TODO: Error");
		return 1;
	}

	handle = get_handle(L);

	switch ((*opt_info).opt_type) {
		case 0:
			value = lua_toboolean(L, 3);
			voikkoSetBooleanOption(handle, (*opt_info).opt_id, value);
			break;
		case 1:
			value = lua_tointeger(L, 3);
			voikkoSetIntegerOption(handle, (*opt_info).opt_id, value);
			break;
		default:
			luaL_error(L, "TODO: Error");
			return 1;
	}
	
	return 0;
}

int terminate(lua_State *L) {

	struct VoikkoHandle *handle;

	lua_pushstring(L, HANDLE_REGISTRY_KEY);
	lua_rawget(L, LUA_REGISTRYINDEX);

	if (lua_isnil(L, -1)) return 0;

	handle = get_handle(L);

	if (!handle) {
		voikkoTerminate(handle);
	}

	return 0;
}

/* int spell_check(lua_State *L) { */
/* 	return 0; */
/* } */

int hyphenate(lua_State *L) {

	struct VoikkoHandle *handle;
	char *hyphenated;

	const char *word = luaL_checkstring(L, 1);

	handle = get_handle(L);

	hyphenated = voikkoHyphenateCstr(handle, word);

	if (!hyphenated) {
		/* TODO */
	}
	
	lua_pushstring(L, hyphenated);
	return 1;
}

int insert_hyphens(lua_State *L) {

	struct VoikkoHandle *handle;
	char *hyphenated;
	int context_change;

	const char *word = luaL_checkstring(L, 1);
	const char *hyphen = luaL_checkstring(L, 2);

	if (!lua_isboolean(L, 3)) {
		return 0;
	}

	context_change = lua_toboolean(L, -1);

	handle = get_handle(L);

	hyphenated = voikkoInsertHyphensCstr(handle, word, hyphen, context_change);

	lua_pushstring(L, hyphenated);

	return 1;
}

int suggest(lua_State *L) {

	struct VoikkoHandle *handle;
	char **suggestions;
	int i;

	const char *word = lua_tostring(L, 1);

	handle = get_handle(L);

	suggestions = voikkoSuggestCstr(handle, word);

	if (!suggestions) {
		return 0;
	}

	lua_newtable(L);

	for (i = 0; suggestions[i]; i++) {
		lua_pushstring(L, suggestions[i]);
		lua_rawseti(L, -2, i+1);
	}

	voikkoFreeCstrArray(suggestions);

	return 1;
}

const luaL_Reg voikko_funcs[] = {

	{"suggest", suggest},
	{"hyphenate", hyphenate},
	{"set_option", set_option},
	/* {"spell_check", spell_check}, */
	{NULL, NULL}
};

int init(lua_State *L) {

	static struct VoikkoHandle *handle;
	const char *error_msg_ptr[128];

	/* Arguments */
	const char *lang_code = lua_tostring(L, 1);
	const char *path = lua_tostring(L, 2);
	
	handle = voikkoInit(error_msg_ptr, lang_code, path);

	if (!handle) return luaL_error(L, *error_msg_ptr);

	set_handle(L, handle);

	lua_newtable(L);
	luaL_setfuncs(L, voikko_funcs, 0);

	lua_newtable(L);
	lua_pushstring(L, "__gc");
	lua_pushcfunction(L, terminate);
	lua_settable(L, -3);
	lua_setmetatable(L, -2);

	return 1;
}

LUALIB_API int luaopen_lvoikko(lua_State *L) {

	luaL_Reg init_f[] = { {"init", init}, {NULL, NULL} };
	luaL_newmetatable(L, MT_NAME);

	lua_newtable(L);
	luaL_setfuncs(L, init_f, 0);

	return 1;
}
