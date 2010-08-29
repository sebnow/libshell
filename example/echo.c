#include <shell.h>
#include <stdio.h>
#include <string.h>

struct input {
	FILE *fp;
	char buf[1024];
};

char *get_input(void *ctx)
{
	struct input *data = ctx;
	size_t read;
	memset(data->buf, 0, sizeof(data->buf));
	read = fread(data->buf, sizeof(*data->buf), sizeof(data->buf), data->fp);
	if(read > 0) {
		return data->buf;
	} else {
		return 0;
	}
}

void assign(char const *name, char const *value, void *ctx) {
	char **ptr;
	printf("%s = %s\n", name, value);
}

int main(int argc, char **argv)
{
	struct input data;
	struct sh_scanner scanner;
	struct sh_scanner_callbacks cb;
	enum sh_scan_status status;

	/* First argument should be a shell script. */
	if(argc != 2) {
		fprintf(stderr, "usage: %s FILE\n", argv[0]);
		return 1;
	}

	memset(&data, 0, sizeof(data));
	data.fp = fopen(argv[1], "r");

	cb.scan = get_input;
	cb.assign = assign;
	sh_scanner_init(&scanner, &cb, &data);
	do {
		status = sh_scan(&scanner);
	} while(status == sh_scan_in_progress);

	if(status == sh_scan_error) {
		fprintf(stderr, "Error parsing!\n");
	}

	sh_scanner_release(&scanner);
	return status == sh_scan_error;
}

