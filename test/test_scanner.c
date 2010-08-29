
#include <stdlib.h>
#include <string.h>

#include <tap.h>
#include <shell.h>

#define NTESTS 11

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
	data.input = "foo='bar'\n";
	sh_scanner_init(&scnr, cb, &data);

	ok(sh_scan(&scnr) == sh_scan_in_progress,
		"Given a string assignment, scanning should succeed");
	ok(data.last_name && strcmp(data.last_name, "foo") == 0,
		"Given a string assignment, the name should be parsed");
	ok(data.last_value != NULL && strcmp(data.last_value, "bar") == 0,
		"Given a string assignment, the value should be parsed");

	sh_scanner_release(&scnr);
	free(data.last_name);
	free(data.last_value);
}

void test_assign_number(struct sh_scanner_callbacks *cb)
{
	struct sh_scanner scnr;
	struct udata data;

	memset(&data, 0, sizeof(data));
	data.input = "bar=1\n";
	sh_scanner_init(&scnr, cb, &data);

	ok(sh_scan(&scnr) == sh_scan_in_progress,
		"Given a number assignment, scanning should succeed");
	ok(data.last_name && strcmp(data.last_name, "bar") == 0,
		"Given a number assignment, the name should be parsed");
	ok(data.last_value != NULL && strcmp(data.last_value, "1") == 0,
		"Given a number assignment, the value should be the number");

	sh_scanner_release(&scnr);
	free(data.last_name);
	free(data.last_value);
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
	test_assign_number(&cb);

	return exit_status();
}

