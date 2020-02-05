rockspec_format = "3.0"
package = "lvoikko"
version = "dev-1"
source = {
   url = "git+https://github.com/severij/lvoikko"
}
description = {
   summary = "Lua bindings for Voikko",
   homepage = "https://github.com/severij/lvoikko",
   issues_url = "https://github.com/severij/lvoikko/issues",
   maintainer = "Severi Jääskeläinen",
   license = "GPLv3"
}
dependencies = {
   "lua >= 5.1"
}
test_dependencies = { "luafilesystem", "luacov" } 
test = { type = "busted" }
build = {
   type = "builtin",
   modules = {
      lvoikko = {
         sources = { "lvoikko.c" },
         libraries = { "voikko" }
      }
   }
}
