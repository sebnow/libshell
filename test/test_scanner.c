/* Copyright (c) 2010 Sebastian Nowicki <sebnow@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdlib.h>
#include <string.h>

#include <tap.h>
#include <shell.h>

#define NTESTS 19

struct udata {
	char *input;
	char *last_name;
	char *last_value;
	char *last_comment;
	char *last_command;
	char **last_command_args;
};

/* A string is used for the input, so it can be simply returned. */
char *scan_cb(void *data) {
	struct udata *d = data;
	return d->input;
}

void assign_cb(char const *name, char const *value, void *data) {
	struct udata *const d = data;
	d->last_name = NULL;
	d->last_value = NULL;
	if(name != NULL) {
		d->last_name = malloc(sizeof(*d->last_name) * strlen(name) + 1);
		if(d->last_name != NULL) {
			strcpy(d->last_name, name);
		}
	}
	if(value != NULL) {
		d->last_value = malloc(sizeof(*d->last_value) * strlen(value) + 1);
		if(d->last_value != NULL) {
			strcpy(d->last_value, value);
		}
	}
}

void comment_cb(char const *comment, void *data)
{
	struct udata *const d = data;
	d->last_comment = NULL;
	if(comment != NULL) {
		d->last_comment = malloc(sizeof(*d->last_comment) * strlen(comment) + 1);
		if(d->last_comment != NULL) {
			strcpy(d->last_comment, comment);
		}
	}
}

void command_cb(char const *command, char const **command_args, void *data)
{
	struct udata *const d = data;
	d->last_command = NULL;
	d->last_command_args = (char **)command_args;
	if(command != NULL) {
		d->last_command = malloc(sizeof(*d->last_command) * strlen(command) + 1);
		if(d->last_command != NULL) {
			strcpy(d->last_command, command);
		}
	}
}

void test_init(struct sh_scanner_callbacks const *cb)
{
	struct sh_scanner scnr;
	ok(sh_scanner_init(&scnr, cb, NULL) == 0, "Given a scanner, when "
		"callbacks are provided, then initialising should succeed", "");
	sh_scanner_release(&scnr);
	ok(sh_scanner_init(&scnr, NULL, NULL) != 0, "Given a scanner, when "
		"callbacks are not provided, then initialising should fail", "");
	sh_scanner_release(&scnr);
}

void test_assign_null(struct sh_scanner_callbacks const *cb)
{
	struct sh_scanner scnr;
	struct udata data;

	memset(&data, 0, sizeof(data));
	data.input = "foo=\n";
	sh_scanner_init(&scnr, cb, &data);

	ok(sh_scan(&scnr) == sh_scan_in_progress,
		"Given a null assignment, scanning should succeed", "");
	ok(data.last_name && strcmp(data.last_name, "foo") == 0,
		"Given a null assignment, the name should be parsed", "");
	ok(data.last_value == NULL,
		"Given a null assignment, the value should be NULL", "");

	sh_scanner_release(&scnr);
	free(data.last_name);
}

void test_assign_string(struct sh_scanner_callbacks const *cb)
{
	struct sh_scanner scnr;
	struct udata data;

	memset(&data, 0, sizeof(data));
	data.input = "foo='bar'; foo=\"bar\"; foo=bar;";
	sh_scanner_init(&scnr, cb, &data);

	ok(sh_scan(&scnr) == sh_scan_in_progress,
		"Given a literal string assignment, scanning should succeed", "");
	ok(data.last_name && strcmp(data.last_name, "foo") == 0,
		"Given a literal string assignment, the name should be parsed", "");
	ok(data.last_value != NULL && strcmp(data.last_value, "bar") == 0,
		"Given a literal string assignment, the value should be parsed", "");
	free(data.last_name);
	free(data.last_value);

	ok(sh_scan(&scnr) == sh_scan_in_progress,
		"Given a string assignment, scanning should succeed", "");
	ok(data.last_name && strcmp(data.last_name, "foo") == 0,
		"Given a string assignment, the name should be parsed", "");
	ok(data.last_value != NULL && strcmp(data.last_value, "bar") == 0,
		"Given a string assignment, the value should be parsed", "");
	free(data.last_name);
	free(data.last_value);

	ok(sh_scan(&scnr) == sh_scan_in_progress,
		"Given an unquoted string assignment, scanning should succeed", "");
	ok(data.last_name && strcmp(data.last_name, "foo") == 0,
		"Given an unquoted string assignment, the name should be parsed", "");
	ok(data.last_value != NULL && strcmp(data.last_value, "bar") == 0,
		"Given an unquoted string assignment, the value should be parsed", "");
	free(data.last_name);
	free(data.last_value);

	sh_scanner_release(&scnr);
}

void test_comment(struct sh_scanner_callbacks const *cb)
{
	struct sh_scanner scnr;
	struct udata data;

	memset(&data, 0, sizeof(data));
	data.input = "# Hello world!\n";
	sh_scanner_init(&scnr, cb, &data);

	ok(sh_scan(&scnr) == sh_scan_in_progress,
		"Given a comment, scanning should succeed", "");
	ok(data.last_comment && strcmp(data.last_comment, "Hello world!") == 0,
		"Given a comment, the content should be parsed", "");
	free(data.last_comment);

	sh_scanner_release(&scnr);
}

void test_command(struct sh_scanner_callbacks const *cb)
{
	struct sh_scanner scnr;
	struct udata data;
	char **args;
	int result;

	memset(&data, 0, sizeof(data));
	data.input = "echo Hello world\n";
	sh_scanner_init(&scnr, cb, &data);

	ok(sh_scan(&scnr) == sh_scan_in_progress,
		"Given a simple command, scanning should succeed", "");
	ok(data.last_command && strcmp(data.last_command, "echo") == 0,
		"Given a simple command, the content should be parsed", "");
	todo_start("Not implemented");
	args = data.last_command_args;
	result = 0;
	if(args != NULL) {
		result = 1;
		result = result && *args && strcmp(*args, "Hello");
		args++;
		result = result && *args && strcmp(*args, "world");
		args++;
		result = result && *args == NULL;
	}
	ok(result, "Given a simple command, the arguments should be parsed", "");
	todo_end();
	free(data.last_command);

	sh_scanner_release(&scnr);
}

int main()
{
	struct sh_scanner_callbacks cb;
	memset(&cb, 0, sizeof(cb));

	cb.scan = scan_cb;
	cb.assign = assign_cb;
	cb.comment = comment_cb;
	cb.command = command_cb;

	plan_tests(NTESTS);

	test_init(&cb);
	test_assign_null(&cb);
	test_assign_string(&cb);
	test_comment(&cb);
	test_command(&cb);

	return exit_status();
}

