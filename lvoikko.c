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

struct VoikkoHandle *lvoikko_checkhandle(lua_State *L, int index) {
	
	/* Get handle reference id */
	lua_pushstring(L, "__handle");
	lua_gettable(L, index);

	/* Get the referenced handle */
	const int ref = lua_tointeger(L, -1);
	/* Pop the integer value from the stack */
	lua_pop(L, 1);
	lua_rawgeti(L, index, ref);

	struct VoikkoHandle *handle = lua_touserdata(L, -1);

	/* Pop the light userdata value from the stack */
	lua_pop(L, 1);

	/* Return the handle */
	return handle;
}

/***
A Voikko class
@type Voikko
*/

/***
Get hyphenation pattern of the given word in UTF-8 encoding.
@function get_hyphenation_pattern
@tparam string word word from which the hyphenation pattern will be created.
@treturn[1] string containing the hyphenation using the following notation:

- `' '` = no hyphenation at this character,
- `'-'` = hyphenation point (character at this position
        is preserved in the hyphenated form),
- `'='` = hyphenation point (character at this position
        is replaced by the hyphen.)
@treturn[2] nil when error occurs during the creation of the pattern.
*/
int get_hyphenation_pattern(lua_State *L) {

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
int hyphenate(lua_State *L)
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
		allow_context_changes = lua_toboolean(L, 4);

	/* Get the hyphenated string */
	char *hyphenated = voikkoInsertHyphensCstr(handle, word, hyphen, allow_context_changes);

	lua_pushstring(L, hyphenated);

	/* Free the resources */
	voikkoFreeCstr(hyphenated);

	/* Return the hyphenated string */
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

	{"get_hyphenation_pattern", get_hyphenation_pattern},
	{"hyphenate", hyphenate},
	{"suggest", suggest},
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
