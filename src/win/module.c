/*
 *  ----------------------------------
 * |         libmem - by rdbo         |
 * |      Memory Hacking Library      |
 *  ----------------------------------
 */

/*
 * Copyright (C) 2023    Rdbo
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License version 3
 * as published by the Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 * 
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include <libmem/libmem.h>

LM_API lm_bool_t LM_CALL
LM_EnumModulesEx(const lm_process_t *process,
		 lm_bool_t (LM_CALL *callback)(lm_module_t *module,
					       lm_void_t   *arg),
		 lm_void_t          *arg)
{
	lm_bool_t result = LM_FALSE;
	HANDLE hsnap;
	MODULEENTRY32W entry;
	lm_module_t module;

	if (!process || !callback)
		return result;
	
	hsnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, process->pid);
	if (hsnap == INVALID_HANDLE_VALUE)
		return result;

	entry.dwSize = sizeof(entry);
	if (!Module32FirstW(hsnap, &entry))
		goto CLEAN_EXIT;

	do {
		if (!wcstoutf8(entry.szExePath, module.path, sizeof(module.path)))
			continue;

		if (!wcstoutf8(entry.szModule, module.name, sizeof(module.name)))
			continue;

		module.base = (lm_address_t)entry.modBaseAddr;
		module.size = (lm_address_t)entry.modBaseSize;
		module.end = module.base + module.size;
		
		if (callback(&module, arg) == LM_FALSE)
			break;
	} while (Module32NextW(hsnap, &entry));

	result = LM_TRUE;

CLEAN_EXIT:
	CloseHandle(hsnap);
	return result;
}

/********************************/

LM_API lm_bool_t LM_CALL
LM_LoadModule(lm_string_t  path,
	      lm_module_t *module_out)
{
	WCHAR wpath[LM_PATH_MAX];

	if (!path)
		return LM_FALSE;

	if (!utf8towcs(path, wpath, LM_ARRLEN(wpath)))
		return LM_FALSE;
	
	if (!LoadLibraryW(wpath))
		return LM_FALSE;

	/* TODO: Get library information through the HMODULE returned by LoadLibraryW (should be faster) */

	if (module_out)
		return LM_FindModule(path, module_out);

	return LM_TRUE;
}

/********************************/

LM_API lm_bool_t LM_CALL
LM_UnloadModule(const lm_module_t *module)
{
	HMODULE hmod;
	WCHAR wpath[LM_PATH_MAX];

	if (!module)
		return LM_FALSE;

	if (!utf8towcs(module->path, wpath, LM_ARRLEN(wpath)))
		return LM_FALSE;

	hmod = GetModuleHandleW(wpath); /* Increases the reference count by 1 */
	if (!hmod)
		return LM_FALSE;

	/* Decrease the reference count by 2 */
	/* NOTE: This does not ensure that the module was actually unloaded */
	CloseHandle(hmod);
	CloseHandle(hmod);

	return LM_TRUE;
}
