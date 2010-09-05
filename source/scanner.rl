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

/** Determine whether the scanner's input has been exhausted.
 * \param scanner The scanner to be checked.
 * \return True on success, false otherwise.
 * */
static int is_input_exhausted(struct sh_scanner const*);

/** Call the assignment callback
 * \pre The \c markers list must be populated with the name, and optionally value,
 * strings.
 * \remarks The \c markers pointer will be modified to point to the new list.
 * \param scanner Scanner for which to call the callback.
 * \param markers A stack of captured strings.
 */
static void notify_assignment(struct sh_scanner const* scanner, GSList **markers);

/** Call the comment callback
 * \pre The \c markers list must be populated with the comment string.
 * \remarks The \c markers pointer will be modified to point to the new list.
 * \param scanner Scanner for which to call the callback.
 * \param markers Reference to a stack of captured strings.
 */
static void notify_comment(struct sh_scanner const* scanner, GSList **markers);

/** Call the command callback
 * \pre The \c markers list must be populated with the name, and any arguments.
 * \remarks The \c markers pointer will be modified to point to the new list.
 * \param scanner Scanner for which to call the callback.
 * \param markers A stack of captured strings.
 */
static void notify_command(struct sh_scanner const* scanner, GSList **markers);

/** Call the function callback
 * \pre The \c markers list must be populated with the function name.
 * \remarks The \c markers pointer will be modified to point to the new list.
 * \param scanner Scanner for which to call the callback.
 * \param markers A stack of captured strings.
 */
static void notify_function(struct sh_scanner const* scanner, GSList **markers);

%%{
	machine Shell;
	access scanner->;
	variable p scanner->p;
	variable pe scanner->pe;
	variable eof scanner->eof;

	write data;

	action mark { mark = fpc; }

	action mpush {
		if(mark != NULL) {
			str = g_string_new_len(mark, fpc - mark + 1);
			str->str[fpc - mark] = '\0';
			markers = g_slist_prepend(markers, str);
			mark = NULL;
		}
	}

	action mpush_value {
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
			str = g_string_new_len(mark, mark_end - mark + 1);
			str->str[mark_end - mark] = '\0';
			markers = g_slist_prepend(markers, str);
			mark = NULL;
			mark_end = NULL;
		}
	}

	whitespace = space - [\n];

	word = ([./] | alnum)+;
	name = (alpha | '_') (alnum | '_')*;
	fname = name;
	terminator = [\n;];

	partial_string = '"' (extend - '"' | '\\' . extend - '"')* '"';
	string = "'" (extend - "'")* "'";
	unquoted_string = (extend - whitespace - terminator)+;

	assignment_value = string | partial_string | unquoted_string;

	assignment = name >mark %mpush '='
		(assignment_value >mark %mpush_value)? terminator;

	comment = '#' whitespace* (print - '\n')+ >mark %mpush '\n';
	command = word >mark %mpush (whitespace+ word)* terminator;
	function = fname >mark %mpush '()';

	main := |*
		assignment => { notify_assignment(scanner, &markers); fbreak; };
		comment => { notify_comment(scanner, &markers); fbreak; };
		command => { notify_command(scanner, &markers); fbreak; };
		function => { notify_function(scanner, &markers); fbreak; };
		[ \t\r\n];
		0 => { scanner->done = 1; fbreak; };
	*|;
}%%

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
	char *mark = NULL;
	char *mark_end = NULL;
	GString *str = NULL;
	GString *const buf = scanner->buffer;
	GSList *markers = NULL; /* Stack of captured strings */

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

static void notify_assignment(struct sh_scanner const* scanner, GSList **markers)
{
	GString *name;
	GString *value;
	char *value_str = NULL;
	value = g_slist_nth_data(*markers, 0);
	assert(value != NULL);
	*markers = g_slist_remove(*markers, value);
	name = g_slist_nth_data(*markers, 0);
	/* Value can be NULL, in which case the value needs to be reassigned
	 * to the name. */
	if(name == NULL) {
		name = value;
		value = NULL;
	} else {
		*markers = g_slist_remove(*markers, name);
		value_str = value->str;
	}

	assert(name != NULL);

	if(scanner->cb.assign != NULL) {
		scanner->cb.assign(name->str, value_str, scanner->user_data);
	}
	g_string_free(name, TRUE);
	if(value != NULL) {
		g_string_free(value, TRUE);
	}
}

static void notify_comment(struct sh_scanner const* scanner, GSList **markers)
{
	GString *comment;
	comment = g_slist_nth_data(*markers, 0);
	assert(comment != NULL);
	*markers = g_slist_remove(*markers, comment);
	if(scanner->cb.comment != NULL) {
		scanner->cb.comment(comment->str, scanner->user_data);
	}
	g_string_free(comment, TRUE);
}

static void notify_command(struct sh_scanner const* scanner, GSList **markers)
{
	GString *name;
	name = g_slist_nth_data(*markers, 0);
	*markers = g_slist_remove(*markers, name);
	assert(name != NULL);
	if(scanner->cb.command != NULL) {
		scanner->cb.command(name->str, NULL, scanner->user_data);
	}
	g_string_free(name, TRUE);
}

static void notify_function(struct sh_scanner const* scanner, GSList **markers)
{
	GString *name;
	name = g_slist_nth_data(*markers, 0);
	*markers = g_slist_remove(*markers, name);
	assert(name != NULL);
	if(scanner->cb.function != NULL) {
		scanner->cb.function(name->str, scanner->user_data);
	}
	g_string_free(name, TRUE);
}

