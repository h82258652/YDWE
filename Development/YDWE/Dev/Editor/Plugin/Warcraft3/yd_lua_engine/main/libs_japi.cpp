#include "../lua/helper.h"
#include "../lua/jassbind.h"
#include <ydwe/util/console.h>
#include <ydwe/warcraft3/native_function.h>
#include <map>
#include <string>

namespace ydwe { namespace warcraft3 { namespace lua_engine {

	int  jass_call_closure(lua_State* L);

	int japi_get(lua_State* L)
	{
		lua::jassbind* lj = (lua::jassbind*)L;

		const char* name = lj->tostring(2);

		native_function::native_function const* nf = native_function::japi_func(name);
		if (nf)
		{
			lj->pushunsigned((uint32_t)(uintptr_t)nf);
			lj->pushcclosure((lua::state::cfunction)jass_call_closure, 1);
			return 1;
		}

		lj->pushnil();
		return 1;
	}

	int open_japi(lua::state* ls)
	{
		ls->newtable();
		{
			ls->newtable();
			{
				ls->pushstring("__index");
				ls->pushcclosure((lua::state::cfunction)japi_get, 0);
				ls->rawset(-3);
			}
			ls->setmetatable(-2);
		}
		ls->setglobal("japi");
		return 1;
	}
}}}
