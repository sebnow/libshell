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
#include "util.h"

#define SH_BUFFER_SIZE 4096

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
			asmt_name = malloc(sizeof(*asmt_name) * (fpc - mark + 1));
			strncpy(asmt_name, mark, fpc - mark);
			asmt_name[fpc - mark] = '\0';
			/* Default to a null/empty assignment */
			asmt_value = NULL;
			mark = NULL;
		}
	}

	action assign_value {
		if(mark != NULL) {
			/* Ignore quotes.
			 * TODO: Different quotes will have to be interpreted
			 * differently (e.g. interpolation)
			 */
			mark_end = fpc;
			if(*mark == '\'' || *mark == '"') {
				mark++;
				mark_end--;
			}
			asmt_value = malloc(sizeof(*asmt_value) * (mark_end - mark + 1));
			strncpy(asmt_value, mark, mark_end - mark);
			asmt_value[mark_end - mark] = '\0';
			mark = NULL;
			mark_end = NULL;
		}
	}

	whitespace = space - [\n];

	name = (alpha | '_') (alnum | '_')*;
	terminator = [\n;];

	partial_string = '"' (extend - '"' | '\\' . extend - '"')* '"';
	string = "'" (extend - "'")* "'";
	unquoted_string = (extend - whitespace - terminator)+;

	assignment_value = string | partial_string | unquoted_string;

	assignment = name >mark %set_name '='
		(assignment_value >mark %assign_value)?
		terminator;

	comment = '#' whitespace* (any - '\n')+ >mark whitespace* '\n';

	main := |*
		assignment => {
			if(scanner->cb.assign != NULL) {
				scanner->cb.assign(asmt_name, asmt_value, scanner->user_data);
			}
			free(asmt_name);
			free(asmt_value);
			fbreak;
		};
		comment => {
			if(mark != NULL && scanner->cb.comment != NULL) {
				
				str = malloc(sizeof(*str) * (fpc - mark + 1));
				strncpy(str, mark, fpc - mark);
				str[fpc - mark] = '\0';
				scanner->cb.comment(str, scanner->user_data);
				free(str);
				str = NULL;
				mark = NULL;
				fbreak;
			}
		};
		[ \t\r\n];
		0 => { scanner->done = 1; fbreak; };
	*|;
}%%

/** Determine whether the scanner's input has been exhausted.
 * \param scanner The scanner to be checked.
 * \return True on success, false otherwise.
 * */
static int is_input_exhausted(struct sh_scanner const*);

SH_SYMEXPORT
int sh_scanner_init(struct sh_scanner *scanner,
	struct sh_scanner_callbacks const *cb, void *ctx)
{
	if(scanner == NULL || cb == NULL) {
		return 1;
	}
	memset(scanner, 0, sizeof(*scanner));
	scanner->line = 1;
	scanner->column = 1;
	memcpy(&scanner->cb, cb, sizeof(*cb));
	scanner->user_data = ctx;
	scanner->buffer = g_string_sized_new(SH_BUFFER_SIZE);
	if(scanner->buffer == NULL) {
		return 1;
	}
	scanner->p = scanner->pe = scanner->buffer->str;
	%% write init;
	return 0;
}

SH_SYMEXPORT
enum sh_scan_status sh_scan(struct sh_scanner *scanner)
{
	enum sh_scan_status status;
	char *input = NULL;
	char *asmt_name = NULL;
	char *asmt_value = NULL;
	char *mark = NULL;
	char *mark_end = NULL;
	char *str = NULL;
	GString *const buf = scanner->buffer;

	assert(scanner);
	assert(scanner->buffer);

	if(scanner->ts != NULL) {
		assert(buf->str <= scanner->ts && scanner->ts <= buf->str + buf->len);
	}

	/* Get more input if required */
	if(is_input_exhausted(scanner)) {
		input = scanner->cb.scan(scanner->user_data);
		/* There is nothing left to scan. */
		if(input == NULL) {
			scanner->eof = scanner->pe;
		} else {
			/* Free some space if capacity is low */
			if(scanner->ts && strlen(input) + buf->len > buf->allocated_len) {
				g_string_erase(buf, 0, scanner->ts - buf->str);
			}
			/* The scanner state needs to be reset with the new information. */
			g_string_append(buf, input);
			scanner->p = buf->str;
			scanner->pe = scanner->p + buf->len;
			/* "Shift" over the token */
			scanner->te = buf->str + (scanner->te - scanner->ts);
			scanner->ts = buf->str;
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
	g_string_free(scanner->buffer, TRUE);
}

static inline int is_input_exhausted(struct sh_scanner const *scanner)
{
	return scanner->p == scanner->pe;
}

