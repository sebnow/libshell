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

#define NTESTS 14

struct udata {
	char *input;
	char *last_name;
	char *last_value;
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

int main()
{
	struct sh_scanner_callbacks cb;
	cb.scan = scan_cb;
	cb.assign = assign_cb;

	plan_tests(NTESTS);

	test_init(&cb);
	test_assign_null(&cb);
	test_assign_string(&cb);

	return exit_status();
}

