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

#include "posixutils.h"
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sys/utsname.h>

size_t
get_system_bits()
{
	size_t bits = sizeof(void *); /* Assumes system bits == pointer size by default */
	struct utsname utsbuf;
	const char *machines64[] = { "x86_64", "amd64", "aarch64" };
	size_t machines64_len = sizeof(machines64) / sizeof(machines64[0]);
	size_t i;

	if (uname(&utsbuf))
		return bits;

	for (i = 0; i < machines64_len; ++i) {
		if (!strcmp(utsbuf.machine, machines64[i])) {
			bits = 64;
			break;
		}
	}

	return bits;
}

size_t
get_name_from_path(char *path, char *namebuf, size_t namesize)
{
	char *last_separator;
	char *name;
	size_t namelen;

	assert(path != NULL && namebuf != NULL && namesize > 0);

	last_separator = strrchr(path, '/');
	name = &last_separator[1]; /* 'name' starts at 'last path separator + 1' */
	namelen = strlen(name);

	/* Truncate name if necessary */
	if (namelen >= namesize)
		namelen = namesize - 1;

	strncpy(namebuf, name, namelen);
	namebuf[namelen] = '\0';

	return namelen;
}
