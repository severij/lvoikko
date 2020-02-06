/***
@module lvoikko
@author Severi Jääskeläinen
@licence GPLv3
*/
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

/* typedef struct opt_info { */
/* 	const int opt_id; */
/* 	const int opt_type; */
/* } opt_info_t; */

/* typedef struct opt_map { */
/* 	const char* opt_str; */
/* 	const opt_info_t opt_info; */
/* } opt_map_t; */

/* static const opt_map_t options[] = { */
/* 	{"accept_all_uppercase", {VOIKKO_OPT_ACCEPT_ALL_UPPERCASE, 0}}, */
/* 	{"accept_bulleted_lists", {VOIKKO_OPT_ACCEPT_BULLETED_LISTS_IN_GC, 0}}, */
/* 	{"accept_extra_hyphens", {VOIKKO_OPT_ACCEPT_EXTRA_HYPHENS, 0}}, */
/* 	{"accept_first_uppercase", {VOIKKO_OPT_ACCEPT_FIRST_UPPERCASE, 0}}, */
/* 	{"accept_missing_hyphens", {VOIKKO_OPT_ACCEPT_MISSING_HYPHENS, 0}}, */
/* 	{"hyphenate_unknown_words", {VOIKKO_OPT_HYPHENATE_UNKNOWN_WORDS, 0}}, */
/* 	{"ignore_dot", {VOIKKO_OPT_IGNORE_DOT, 0}}, */
/* 	{"ignore_nonwords", {VOIKKO_OPT_IGNORE_NONWORDS, 0}}, */
/* 	{"ignore_numbers", {VOIKKO_OPT_IGNORE_NUMBERS, 0}}, */
/* 	{"ignore_uppercase", {VOIKKO_OPT_IGNORE_UPPERCASE, 0}}, */
/* 	{"min_hyphenated_word_length", {VOIKKO_MIN_HYPHENATED_WORD_LENGTH, 1}}, */
/* 	{"no_ugly_hyphenation", {VOIKKO_OPT_NO_UGLY_HYPHENATION, 0}}, */
/* 	{"ocr_suggestions", {VOIKKO_OPT_OCR_SUGGESTIONS, 0}}, */
/* 	{"speller_cache_size", {VOIKKO_SPELLER_CACHE_SIZE, 1}}, */
/* }; */

/* const opt_info_t *get_option_info(const char *opt_str) { */
/* 	int i; */
/* 	for (i=0; i < N_OPTIONS; i++) { */
/* 		if (strcmp(opt_str, options[i].opt_str)) { */
/* 			return &options[i].opt_info; */
/* 		} */
/* 	} */
/* 	return NULL; */
/* } */

/* /1* int set_option(lua_State *L) { *1/ */

/* 	/1* luaL_checkoption *1/ */

/* 	struct VoikkoHandle *handle; */
/* 	const char *key = lua_tostring(L, 1); */
/* 	int value; */

/* 	const opt_info_t *opt_info = get_option_info(key); */

/* 	if (!opt_info) { */
/* 		luaL_error(L, "TODO: Error"); */
/* 		return 1; */
/* 	} */

/* 	handle = get_handle(L); */

/* 	switch ((*opt_info).opt_type) { */
/* 		case 0: */
/* 			value = lua_toboolean(L, 3); */
/* 			voikkoSetBooleanOption(handle, (*opt_info).opt_id, value); */
/* 			break; */
/* 		case 1: */
/* 			value = lua_tointeger(L, 3); */
/* 			voikkoSetIntegerOption(handle, (*opt_info).opt_id, value); */
/* 			break; */
/* 		default: */
/* 			luaL_error(L, "TODO: Error"); */
/* 			return 1; */
/* 	} */
	
/* 	return 0; */
/* } */


struct VoikkoHandle *lvoikko_checkhandle(lua_State *L, int index) {
	
	/* Get handle reference id */
	lua_pushstring(L, "__handle");
	lua_gettable(L, index);

	/* Get the referenced handle */
	const int ref = lua_tointeger(L, -1);
	lua_rawgeti(L, index, ref);

	/* Return the handle */
	return lua_touserdata(L, -1);
}

/***
A Voikko class
@type Voikko
*/

/***
Hyphenates the given word in UTF-8 encoding.
@function hyphenate
@tparam string word word to hyphenate.
@treturn[1] string containing the hyphenation using the following notation:

- `' '` = no hyphenation at this character,
- `'-'` = hyphenation point (character at this position
        is preserved in the hyphenated form),
- `'='` = hyphenation point (character at this position
        is replaced by the hyphen.)
@treturn[2] nil when error occurs during hyphenation.
*/
int hyphenate(lua_State *L) {

	/* Arguments */
	struct VoikkoHandle *handle = lvoikko_checkhandle(L, 1);
	const char *word = luaL_checkstring(L, 2);

	char *hyphenated = voikkoHyphenateCstr(handle, word);
	
	/* Check for errors */
	if (!hyphenated) {
		lua_pushstring(L, "Error occured during hyphenation");
		voikkoFreeCstr(hyphenated);
		/* Return error */
		return lua_error(L);
	}

	lua_pushstring(L, hyphenated);

	/* Free the resources */
	voikkoFreeCstr(hyphenated);

	/* Return hyphenation */
	return 1;
}

/***
Hyphenates the given word in UTF-8 encoding.
@function insert_hyphens
@tparam string word word to hyphenate
@tparam ?string hyphen character string to insert at hyphenation positions (default `'-'`).
@tparam ?bool allow_context_changes specifies wheter hyphens may be inserted even if they alter the word in unhyphenated form (default `true`).
@treturn string where hyphens are inserted in all hyphenation points
*/
int insert_hyphens(lua_State *L)
{
	/* Arguments */
	struct VoikkoHandle *handle = lvoikko_checkhandle(L, 1);
	const char *word = luaL_checkstring(L, 2);
	const char *hyphen = "-"; // Default value
	int allow_context_changes = 1; // Default value

	/* Check if optional arguments are provided */
	if (!lua_isnoneornil(L, 3))
		hyphen = luaL_checkstring(L, 3);
	if (!lua_isnoneornil(L, 4))
		allow_context_changes = luaL_checkinteger(L, 4);


	/* Get the hyphenated string */
	char *hyphenated = voikkoInsertHyphensCstr(handle, word, hyphen, allow_context_changes);

	/* Return the hyphenated string */
	lua_pushstring(L, hyphenated);
	return 1;
}

/***
Checks the spelling of an UTF-8 character string.
@function spell
@tparam string word word to check
@treturn int one of the spell checker return codes:

- 0 = spell failed
- 1 = spell ok
- 2 = internal error
- 3 = character conversion failed
*/
int spell(lua_State *L)
{
	/* Arguments */
	struct VoikkoHandle *handle = lvoikko_checkhandle(L, 1);
	const char *word = luaL_checkstring(L, 2);

	/* Get return code */
	int code = voikkoSpellCstr(handle, word);

	/* Return the code */
	lua_pushinteger(L, code);
	return 1;
}

/***
Finds suggested correct spellings for given UTF-8 encoded word.
@function suggest
@tparam string word word to find suggestions for
@treturn[1] table with zero or more strings containing the suggestions in UTF-8 encoding
@treturn[2] nil if no suggestions could be generated.
*/
int suggest(lua_State *L)
{
	/* Arguments */
	struct VoikkoHandle *handle = lvoikko_checkhandle(L, 1);
	const char *word = luaL_checkstring(L, 2);

	/* Get suggestions */
	char **suggestions = voikkoSuggestCstr(handle, word);

	if (!suggestions) return 0;

	/* Put suggestions in a table */
	lua_newtable(L);
	for (int i = 0; suggestions[i]; i++) {
		lua_pushstring(L, suggestions[i]);
		lua_rawseti(L, -2, i+1);
	}
	/* Free resources */
	voikkoFreeCstrArray(suggestions);

	/* Return the table containing the suggestions */
	return 1;
}


int raise_access_error(lua_State *L) {
	return luaL_error(L, "access error");
}

int terminate(lua_State *L)
{
	struct VoikkoHandle *handle = lvoikko_checkhandle(L, 1);
	voikkoTerminate(handle);
	return 0;
}

const luaL_Reg methods[] = {

	{"hyphenate", hyphenate},
	/* {"insert_hyphens", insert_hyphens}, */
	{"suggest", suggest},
	/* {"set_option", set_option}, */
	{"spell", spell},
#if LUA_VERSION_NUM < 502
	{"terminate", terminate},
#endif
	{NULL, NULL}
};

int new(lua_State *L) {

	const char *error = NULL;

	/* Arguments */
	const char *lang_code = lua_tostring(L, 1);
	const char *path = lua_tostring(L, 2);
	
	struct VoikkoHandle *handle = voikkoInit(&error, lang_code, path);

	if (!handle) {
		return luaL_error(L, error);
	}

	lua_newtable(L);
#if LUA_VERSION_NUM > 501
	luaL_setfuncs(L, methods, 0);
#else
	luaL_register(L, NULL, methods);
#endif
	lua_pushlightuserdata(L, handle);
	const int ref = luaL_ref(L, -2);
	lua_pushstring(L, "__handle");
	lua_pushinteger(L, ref);
	lua_settable(L, -3);

	lua_newtable(L);
#if LUA_VERSION_NUM > 501
	lua_pushstring(L, "__gc");
	lua_pushcfunction(L, terminate);
	lua_settable(L, -3);
#endif
	lua_pushstring(L, "__index");
	lua_rawgeti(L, -2, ref);
	lua_settable(L, -3);
	lua_pushstring(L, "__newindex");
	lua_pushcfunction(L, raise_access_error);
	lua_settable(L, -3);
	lua_pushstring(L, "__metatable");
	lua_pushboolean(L, 0);
	lua_settable(L, -3);

	lua_setmetatable(L, -2);

	voikkoFreeErrorMessageCstr(error);

	return 1;
}

LUALIB_API int luaopen_lvoikko(lua_State *L) {

	luaL_Reg new_func[] = { {"new", new}, {NULL, NULL} };

	lua_newtable(L);
	luaL_setfuncs(L, new_func, 0);

	return 1;
}
