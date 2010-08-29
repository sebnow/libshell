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

#ifndef SH_SHELL_H
#define SH_SHELL_H

#include <glib.h>

/** \file shell.h
 * Header exporting the public interface of the library.
 */

/**
 * \mainpage A Shell parser library
 * This library provides an IEEE Std 1003.1 compliant shell parser.
 *
 * The library does not execute, nor interpret, shell code. It merely
 * \ref scanning "scans" an input stream and parses tokens, delegating
 * intepretation to the caller. An event driven approach is taken, using
 * callbacks to notify the caller of tokens.
 *
 * \section interface Interface
 * The interface is simple and minimal due to the event-driven nature of
 * the parser. The main function used is sh_scan() which is the entry
 * point of the scanner. The rest of the logic is handled by user-defined
 * callbacks. This means the interface is not cluttered with data
 * representation.
 */

/** \defgroup scanning Scanning
 * Functions and structures used for scanning Shell code.
 *
 * The scanner will parse an input stream (provided by the \ref
 * sh_scanner_callbacks::scan "scan callback"). When a token has been
 * parsed, the appropriate \ref callbacks "callback" will be called, which
 * delegates intepretation of the token.
 *
 * For example, in order to process assignments, something like the
 * following might be used:
 * \include echo.c
 *
 * \{
 */

/** \defgroup callbacks Callbacks
 * Callbacks are used to delegate various actions, such as retrieving
 * more input for scanning, or intepreting parsed tokens.
 *
 * \{
 */

/** Callback function used to retrieve more data for scanning.
 *
 * The callback should retrieve additional data to be scanned. If
 * additional data is not available (e.g. end of file) \c NULL should be
 * returned.
 *
 * \param user_data User-specified data, set in sh_scanner_init().
 * \return An input buffer ready for scanning or \c NULL.
 */
typedef char *(*sh_scan_func)(void *);

/** Callback function called when an asignment occurs.
 *
 * The \c name and \c value should be copied. Validity of the
 * memory pointed to be these pointers is undefined after the callback
 * is executed.
 *
 * \param name Name of the assigned variable.
 * \param value Value to be assigned.
 * \param user_data User-specified data, set in sh_scanner_init().
 */
typedef void (*sh_assign_func)(char *, char *, void *);

/** \} */

/** Conveniance structure containing all callbacks used by a scanner */
struct sh_scanner_callbacks {
	/** Callback function used to retrieve more input. */
	sh_scan_func scan;
	/** Callback function notifying of assignment. */
	sh_assign_func assign;
};

/** A structure containing the state of the scanner. */
struct sh_scanner {
	/* Ragel specific variables for keeping state, etc. */
	/** Current state of the ragel state machine */
	int cs;
	/** Action state of the ragel state machine */
	int act;
	/** Start of token. */
	char *ts;
	/** End of token. */
	char *te;
	/** Pointer to the start of data. */
	char *p;
	/** Pointer to the end of data. */
	char *pe;
	/** Points to \c pe if EOF is reached, otherwise NULL */
	char *eof;
	/** Whether scanning has been completed */
	int done;
	/** Current line number being scanned. */
	unsigned int line;
	/** Current column number being scanned. */
	unsigned int column;
	/** Buffer for input. */
	GString *buffer;
	/** Container for callbacks used by the scanner */
	struct sh_scanner_callbacks cb;
	/** User-supplied data to be passed when scan_callback is called. */
	void *user_data;
};

/** Possible scanner statuses */
enum sh_scan_status {
	/** Scanning has completed and there is no input left to scan. */
	sh_scan_complete = 0,
	/** Scanning is still in progress (EOF hasn't been reached). */
	sh_scan_in_progress,
	/** An error occurred during scanning. */
	sh_scan_error,
};

/** Initialise a scanner for use.
 *
 * Memory should be allocated prior to calling this function.
 *
 * \code
 * struct sh_scanner scanner;
 * sh_scanner_init(&scanner, &callbacks, NULL);
 * sh_scan(&scanner);
 * sh_scanner_release(&scanner);
 * \endcode
 *
 * \param scanner Allocated memory to be initialised.
 * \param callbacks Callback functions.
 * \param user_data Data to be passed as a parameter to callbacks.
 * \return 0 on success, 1 otherwise.
 */
int sh_scanner_init(struct sh_scanner *, struct sh_scanner_callbacks *, void *);

/** Release any resources used by the scanner.
 * \param scanner Scanner to be released.
 */
void sh_scanner_release(struct sh_scanner *);

/** Scan for another token using \c scan_callback for input.
 *
 * The input stream will be scanned for another token, calling the appropriate
 * callback to process the token. If input is exhausted the \ref
 * sh_scanner_callbacks::scan scan callback will be called.
 *
 * \param scanner An initialised scanner object.
 * \return A scan status indicating the scan's success.
 */
enum sh_scan_status sh_scan(struct sh_scanner *);

/** Free resources used by the scanner.
 * \param scanner The scanner to be released.
 */
void sh_scanner_release(struct sh_scanner *);

/** \} */

#endif /* SH_SHELL_H */

