/*
 * Copyright (C) 2002-2005 Roman Zippel <zippel@linux-m68k.org>
 * Copyright (C) 2002-2005 Sam Ravnborg <sam@ravnborg.org>
 *
 * Released under the terms of the GNU GPL v2.0.
 */

#include <sys/stat.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "lkc.h"

/* file already present in list? If not add it */
struct file *file_lookup(const char *name)
{
	struct file *file;
	char *file_name = sym_expand_string_value(name);

	for (file = file_list; file; file = file->next) {
		if (!strcmp(name, file->name)) {
			free(file_name);
			return file;
		}
	}

	file = xmalloc(sizeof(*file));
	memset(file, 0, sizeof(*file));
	file->name = file_name;
	file->next = file_list;
	file_list = file;
	return file;
}

/* write a dependency file as used by kbuild to track dependencies */
int file_write_dep(const char *name)
{
	char *str;
	char buf[PATH_MAX+1], buf2[PATH_MAX+1], dir[PATH_MAX+1];
	struct symbol *sym, *env_sym;
	struct expr *e;
	struct file *file;
	FILE *out;

	if (!name)
		name = ".kconfig.d";

	strcpy(dir, conf_get_configname());
	str = strrchr(dir, '/');
	if (str)
		str[1] = 0;
	else
		dir[0] = 0;

	sprintf(buf, "%s..config.tmp", dir);
	out = fopen(buf, "w");
	if (!out)
		return 1;
	fprintf(out, "deps_config := \\\n");
	for (file = file_list; file; file = file->next) {
		if (file->next)
			fprintf(out, "\t%s \\\n", file->name);
		else
			fprintf(out, "\t%s\n", file->name);
	}
	fprintf(out, "\n%s: \\\n"
		     "\t$(deps_config)\n\n", conf_get_autoconfig_name());

	expr_list_for_each_sym(sym_env_list, e, sym) {
		struct property *prop;
		const char *value;

		prop = sym_get_env_prop(sym);
		env_sym = prop_get_symbol(prop);
		if (!env_sym)
			continue;
		value = getenv(env_sym->name);
		if (!value)
			value = "";
		fprintf(out, "ifneq \"$(%s)\" \"%s\"\n", env_sym->name, value);
		fprintf(out, "%s: FORCE\n", conf_get_autoconfig_name());
		fprintf(out, "endif\n");
	}

	fprintf(out, "\n$(deps_config): ;\n");
	fclose(out);
	sprintf(buf2, "%s%s", dir, name);
	rename(buf, buf2);
	return 0;
}


/* move a file, allows cross-device link */
int file_move(const char* src, const char* dst)
{
#ifndef BUF_SIZE
#define BUF_SIZE 4096
#endif

    int fd_input, fd_output;
    mode_t perm_file;
    char buf[BUF_SIZE];
    ssize_t num;

    fd_input = open(src, O_RDONLY);
    if (-1 == fd_input)
        return 1;

    perm_file = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
    fd_output = open(dst, O_CREAT | O_TRUNC | O_WRONLY, perm_file);
    if (-1 == fd_output)
        return 1;

    while (0 < (num = read(fd_input, buf, BUF_SIZE)))
        if (num != write(fd_output, buf, num))
            return 1;

    if (-1 == num)
        return 1;
    if (-1 == close(fd_input))
        return 1;
    if (-1 == close(fd_output))
        return 1;

    remove(src);

    return 0;
}


/* Allocate initial growable string */
struct gstr str_new(void)
{
	struct gstr gs;
	gs.s = xmalloc(sizeof(char) * 64);
	gs.len = 64;
	gs.max_width = 0;
	strcpy(gs.s, "\0");
	return gs;
}

/* Free storage for growable string */
void str_free(struct gstr *gs)
{
	if (gs->s)
		free(gs->s);
	gs->s = NULL;
	gs->len = 0;
}

/* Append to growable string */
void str_append(struct gstr *gs, const char *s)
{
	size_t l;
	if (s) {
		l = strlen(gs->s) + strlen(s) + 1;
		if (l > gs->len) {
			gs->s = xrealloc(gs->s, l);
			gs->len = l;
		}
		strcat(gs->s, s);
	}
}

/* Append printf formatted string to growable string */
void str_printf(struct gstr *gs, const char *fmt, ...)
{
	va_list ap;
	char s[10000]; /* big enough... */
	va_start(ap, fmt);
	vsnprintf(s, sizeof(s), fmt, ap);
	str_append(gs, s);
	va_end(ap);
}

/* Retrieve value of growable string */
const char *str_get(struct gstr *gs)
{
	return gs->s;
}

void *xmalloc(size_t size)
{
	void *p = malloc(size);
	if (p)
		return p;
	fprintf(stderr, "Out of memory.\n");
	exit(1);
}

void *xcalloc(size_t nmemb, size_t size)
{
	void *p = calloc(nmemb, size);
	if (p)
		return p;
	fprintf(stderr, "Out of memory.\n");
	exit(1);
}

void *xrealloc(void *p, size_t size)
{
	p = realloc(p, size);
	if (p)
		return p;
	fprintf(stderr, "Out of memory.\n");
	exit(1);
}

char *xstrdup(const char *s)
{
	char *p;

	p = strdup(s);
	if (p)
		return p;
	fprintf(stderr, "Out of memory.\n");
	exit(1);
}
