
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

void assign_cb(char *name, char *value, void *data) {
	struct udata *d = data;
	d->last_name = strdup(name);
	if(value != NULL) {
		d->last_value = strdup(value);
	} else {
		d->last_value = NULL;
	}
}

void test_init(struct sh_scanner_callbacks *cb)
{
	struct sh_scanner scnr;
	ok(sh_scanner_init(&scnr, cb, NULL) == 0, "Given a scanner, when "
		"callbacks are provided, then initialising should succeed");
	sh_scanner_release(&scnr);
	ok(sh_scanner_init(&scnr, NULL, NULL) != 0, "Given a scanner, when "
		"callbacks are not provided, then initialising should fail");
	sh_scanner_release(&scnr);
}

void test_assign_null(struct sh_scanner_callbacks *cb)
{
	struct sh_scanner scnr;
	struct udata data;

	memset(&data, 0, sizeof(data));
	data.input = "foo=\n";
	sh_scanner_init(&scnr, cb, &data);

	ok(sh_scan(&scnr) == sh_scan_in_progress,
		"Given a null assignment, scanning should succeed");
	ok(data.last_name && strcmp(data.last_name, "foo") == 0,
		"Given a null assignment, the name should be parsed");
	ok(data.last_value == NULL,
		"Given a null assignment, the value should be NULL");

	sh_scanner_release(&scnr);
	free(data.last_name);
}

void test_assign_string(struct sh_scanner_callbacks *cb)
{
	struct sh_scanner scnr;
	struct udata data;

	memset(&data, 0, sizeof(data));
	data.input = "foo='bar'; foo=\"bar\"; foo=bar;";
	sh_scanner_init(&scnr, cb, &data);

	ok(sh_scan(&scnr) == sh_scan_in_progress,
		"Given a literal string assignment, scanning should succeed");
	ok(data.last_name && strcmp(data.last_name, "foo") == 0,
		"Given a literal string assignment, the name should be parsed");
	ok(data.last_value != NULL && strcmp(data.last_value, "bar") == 0,
		"Given a literal string assignment, the value should be parsed");
	free(data.last_name);
	free(data.last_value);

	ok(sh_scan(&scnr) == sh_scan_in_progress,
		"Given a string assignment, scanning should succeed");
	ok(data.last_name && strcmp(data.last_name, "foo") == 0,
		"Given a string assignment, the name should be parsed");
	ok(data.last_value != NULL && strcmp(data.last_value, "bar") == 0,
		"Given a string assignment, the value should be parsed");
	free(data.last_name);
	free(data.last_value);

	ok(sh_scan(&scnr) == sh_scan_in_progress,
		"Given an unquoted string assignment, scanning should succeed");
	ok(data.last_name && strcmp(data.last_name, "foo") == 0,
		"Given an unquoted string assignment, the name should be parsed");
	ok(data.last_value != NULL && strcmp(data.last_value, "bar") == 0,
		"Given an unquoted string assignment, the value should be parsed");
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

