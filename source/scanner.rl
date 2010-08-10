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

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "shell.h"

#include "buffer.h"
#include "util.h"

%%{
	machine Shell;
	access scanner->;
	variable p scanner->p;
	variable pe scanner->pe;
	variable eof scanner->eof;

	write data;

	action mark { mark = fpc; }

	action set_name {
		if(mark != NULL) {
			asmt_name = malloc(sizeof(*asmt_name) * (fpc - mark));
			strncpy(asmt_name, mark, fpc - mark);
			asmt_name[fpc - mark] = '\0';
			/* Default to a null/empty assignment */
			asmt_value.type = sh_value_type_scalar;
			asmt_value.scalar = NULL;
			mark = NULL;
		}
	}

	action assign_scalar {
		char *mark_end = fpc;
		if(mark != NULL) {
			asmt_value.type = sh_value_type_scalar;
			/* Ignore quotes.
			 * TODO: Different quotes will have to be interpreted
			 * differently (e.g. interpolation)
			 */
			if(*mark == '\'' || *mark == '"') {
				mark++;
				mark_end--;
			}
			asmt_value.scalar = malloc(sizeof(*asmt_value.scalar) * (mark_end - mark));
			strncpy(asmt_value.scalar, mark, mark_end - mark);
			asmt_value.scalar[mark_end - mark] = '\0';
			mark = NULL;
		}
	}

	whitespace = space - [\n];

	word = (alpha | '_') (alnum | '_')*;
	terminator = [\n;];

	partial_string = '"' (extend - '"' | '\\' . extend - '"')* '"';
	string = "'" (extend - "'")* "'";

	scalar = string | partial_string;

	assignment = word >mark %set_name '=' (scalar >mark %assign_scalar)* terminator;

	main := |*
		assignment => {
			if(scanner->cb->assign != NULL) {
				scanner->cb->assign(asmt_name, &asmt_value, scanner->user_data);
			}
			free(asmt_name);
			if(asmt_value.type == sh_value_type_scalar && asmt_value.scalar) {
				free(asmt_value.scalar);
			}
			fbreak;
		};
		[ \t\r\n];
		0 => { scanner->done = 1; fbreak; };
	*|;
}%%

/** Determine whether the scanner's input has been exhausted.
 * \param scanner The scanner to be checked.
 * \return True on success, false otherwise.
 * */
static int is_input_exhausted(struct sh_scanner *);

SH_SYMEXPORT
int sh_scanner_init(struct sh_scanner *scanner, struct sh_scanner_callbacks *cb, void *ctx)
{
	if(scanner == NULL || cb == NULL) {
		return 1;
	}
	memset(scanner, 0, sizeof(*scanner));
	scanner->line = 1;
	scanner->column = 1;
	scanner->cb = cb;
	scanner->user_data = ctx;
	scanner->buffer = malloc(sizeof(*scanner->buffer));
	if(scanner->buffer == NULL) {
		return 1;
	}
	memset(scanner->buffer, 0, sizeof(*scanner->buffer));
	if(sh_buffer_init(scanner->buffer, 0) != 0) {
		return 1;
	}
	scanner->p = scanner->pe = sh_buffer_data(scanner->buffer);
	%% write init;
	return 0;
}

SH_SYMEXPORT
enum sh_scan_status sh_scan(struct sh_scanner *scanner)
{
	enum sh_scan_status status;
	char *input = NULL;
	struct sh_buffer *buf = scanner->buffer;
	char *asmt_name;
	struct sh_value asmt_value;
	char *mark = NULL;

	assert(scanner);
	assert(scanner->buffer);

	/* Reset some values between scans */
	asmt_name = NULL;
	memset(&asmt_value, 0, sizeof(asmt_value));

	if(scanner->ts != NULL) {
		assert(sh_buffer_data(buf) <= scanner->ts);
		assert(scanner->ts < sh_buffer_data(buf) + sh_buffer_length(buf));
	}

	/* Get more input if required */
	if(is_input_exhausted(scanner)) {
		input = scanner->cb->scan(scanner->user_data);
		/* There is nothing left to scan. */
		if(input == NULL) {
			scanner->eof = scanner->pe;
		} else {
			/* Free some space if capacity is low */
			if(strlen(input) + sh_buffer_length(buf) > sh_buffer_capacity(buf)) {
				sh_buffer_consume(buf, scanner->ts - sh_buffer_data(buf), NULL);
			}
			/* The scanner state needs to be reset with the new information. */
			sh_buffer_append(buf, input);
			scanner->p = sh_buffer_data(buf);
			scanner->pe = scanner->p + sh_buffer_length(buf);
			/* "Shift" over the token */
			scanner->te = sh_buffer_data(buf) + (scanner->te - scanner->ts);
			scanner->ts = sh_buffer_data(buf);
		}
	}
	%% write exec;
	if(scanner->done) {
		status = sh_scan_complete;
	} else if(scanner->cs == Shell_error) {
		status = sh_scan_error;
	} else {
		status = sh_scan_in_progress;
	}
	return status;
}

SH_SYMEXPORT
void sh_scanner_release(struct sh_scanner *scanner) {
	sh_buffer_release(scanner->buffer);
}

static inline int is_input_exhausted(struct sh_scanner *scanner)
{
	return scanner->p == scanner->pe;
}
