
#include <stdlib.h>
#include <string.h>

#include <tap.h>
#include <shell.h>

#define NTESTS 19

char *scan_cb(void *);
void assign_cb(char *, struct sh_value *, void *);

struct udata {
	char *input;
	char *last_name;
	struct sh_value *last_value;
};

/* A string is used for the input, so it can be simply returned. */
char *scan_cb(void *data) {
	struct udata *d = data;
	return d->input;
}

void assign_cb(char *name, struct sh_value *value, void *data) {
	struct udata *d = data;
	d->last_name = strdup(name);
	d->last_value->type = value->type;
	if(value->type == sh_value_type_scalar && value->scalar) {
		d->last_value->scalar = strdup(value->scalar);
	} else {
		/* TODO: array */
		d->last_value->array = NULL;
	}
}

void test_init(struct sh_scanner_callbacks *cb)
{
	struct sh_scanner scnr;
	ok(sh_scanner_init(&scnr, cb, NULL) == 0, "Given a scanner, when "
		"callbacks are provided, then initialising should succeed");
	ok(sh_scanner_init(&scnr, NULL, NULL) != 0, "Given a scanner, when "
		"callbacks are not provided, then initialising should fail");
}

void test_assign_null(struct sh_scanner_callbacks *cb)
{
	struct sh_scanner scnr;
	struct sh_value val;
	struct udata data;

	memset(&data, 0, sizeof(data));
	memset(&val, 0, sizeof(val));
	data.input = "foo=\n";
	data.last_value = &val;
	sh_scanner_init(&scnr, cb, &data);

	ok(sh_scan(&scnr) == sh_scan_in_progress,
		"Given a null assignment, scanning should succeed");
	ok(data.last_name && strcmp(data.last_name, "foo") == 0,
		"Given a null assignment, the name should be parsed");
	ok(data.last_value->type == sh_value_type_scalar,
		"Given a null assignment, the value type should be scalar");
	ok(data.last_value->scalar == NULL,
		"Given a null assignment, the value should be NULL");

	sh_scanner_release(&scnr);
	free(data.last_name);
}

void test_assign_string(struct sh_scanner_callbacks *cb)
{
	struct sh_scanner scnr;
	struct sh_value val;
	struct udata data;

	memset(&data, 0, sizeof(data));
	memset(&val, 0, sizeof(val));
	data.input = "foo='bar'\n";
	data.last_value = &val;
	sh_scanner_init(&scnr, cb, &data);

	ok(sh_scan(&scnr) == sh_scan_in_progress,
		"Given a string assignment, scanning should succeed");
	ok(data.last_name && strcmp(data.last_name, "foo") == 0,
		"Given a string assignment, the name should be parsed");
	ok(data.last_value && data.last_value->type == sh_value_type_scalar,
		"Given a string assignment, the value type should be scalar");
	ok(data.last_value->scalar && strcmp(data.last_value->scalar, "bar") == 0,
		"Given a string assignment, the value should be parsed");

	sh_scanner_release(&scnr);
	free(data.last_name);
	free(data.last_value->scalar);
}

void test_assign_number(struct sh_scanner_callbacks *cb)
{
	struct sh_scanner scnr;
	struct sh_value val;
	struct udata data;

	memset(&data, 0, sizeof(data));
	memset(&val, 0, sizeof(val));
	data.input = "bar=1\n";
	data.last_value = &val;
	sh_scanner_init(&scnr, cb, &data);

	ok(sh_scan(&scnr) == sh_scan_in_progress,
		"Given a number assignment, scanning should succeed");
	ok(data.last_name && strcmp(data.last_name, "bar") == 0,
		"Given a number assignment, the name should be parsed");
	ok(data.last_value && data.last_value->type == sh_value_type_scalar,
		"Given a number assignment, the value type should be scalar");
	ok(data.last_value->scalar && strcmp(data.last_value->scalar, "1") == 0,
		"Given a number assignment, the value should be the number");

	sh_scanner_release(&scnr);
	free(data.last_name);
	free(data.last_value->scalar);
}

void test_assign_array(struct sh_scanner_callbacks *cb)
{
	struct sh_scanner scnr;
	struct sh_value val;
	struct udata data;
	char **array;
	int result;

	memset(&data, 0, sizeof(data));
	memset(&val, 0, sizeof(val));
	data.input = "eggs=(a foo bar)\n";
	data.last_value = &val;
	sh_scanner_init(&scnr, cb, &data);

	ok(sh_scan(&scnr) == sh_scan_in_progress,
		"Given an array assignment, scanning should succeed");
	ok(data.last_name && strcmp(data.last_name, "eggs") == 0,
		"Given an array assignment, the name should be parsed");
	ok(data.last_value && data.last_value->type == sh_value_type_array,
		"Given an array assignment, the value type should be array");

	array = data.last_value->array;
	result = 0;
	if(array != NULL) {
		result = result && *array && strcmp(*array, "a");
		array++;
		result = result && *array && strcmp(*array, "foo");
		array++;
		result = result && *array && strcmp(*array, "bar");
		array++;
	}
	ok(result, "Given an array assignment, the value should be parsed");
	ok(array && *array == NULL,
		"Given an array assignment, the array should be NULL terminated");

	sh_scanner_release(&scnr);
	free(data.last_name);
	free(array);
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

	/* TODO: */
	todo_start("%s", "Not implemented");
	test_assign_number(&cb);
	test_assign_array(&cb);
	todo_end();

	return exit_status();
}

