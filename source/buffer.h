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

#ifndef SH_BUFFER_H
#define SH_BUFFER_H

#include <stdlib.h>

/** \defgroup Buffer
 * Functions for manipulating buffers.
 *
 * Buffers dynamically grows as required. Strings can be consumed from
 * the beginning of the buffers, and appended to the end. This allows
 * for a * sliding "window" into an input stream.
 *
 * \code
 * struct sh_buffer buf;
 * sh_buffer_init(buf, 1024);
 * sh_buffer_append(buf, "hello");
 * sh_buffer_length(buf); // 5
 * sh_buffer_data(buf); // "hello"
 *
 * char *data;
 * sh_buffer_consume(buf, 3, data);
 * // data = "hel"
 * sh_buffer_release(buf);
 * \endcode
 * \{
 */

/** A structure containing the state of a string buffer. */
struct sh_buffer {
    /** Pointer to the underlying string data. */
    char *data;
    /** Buffer position indicator within \c data */
    char *pos;
    /** Length of the string. */
    size_t length;
    /** Total capacity of the buffer. */
    size_t capacity;
};

/** Initialise a sh_buffer structure.
 * Memory must be allocated prior to initialisation.
 * \param buffer Memory block allocated for the sh_buffer structure.
 * \param capacity Initial capacity of the buffer.
 * \return 0 if initialised, 1 otherwise.
 */
int sh_buffer_init(struct sh_buffer *, size_t);

/** Increase the capacity of the buffer by at least \c hint bytes.
 * \remarks The buffer allocates memory as required. This should only be used
 * if the required size is known.
 * \param buffer Buffer to be grown.
 * \param hint Amount of additional memory that should be allocated.
 * \return True if grown successfully, false otherwise.
 */
int sh_buffer_grow(struct sh_buffer *buffer, size_t hint);

/** Append a string to the end of the buffer.
 * \param buffer The buffer being appended to.
 * \param data String data to be appended.
 * \return True (1) if initialised, false (0) otherwise.
 * \remarks If an error occurs, \c errno may be set, for instance with \c ENOMEM.
 */
int sh_buffer_append(struct sh_buffer *, char *);

/** Consume \c n amount of bytes from the beginning of the buffer.
 *
 * The buffer consumes and copies at most \c n bytes to \c data, and an
 * additional terminating null byte ('\\0'). The string \c data must be
 * large enough to recieve the copy, i.e. \c data must be at least \c n + 1
 * bytes.
 *
 * If the buffered string is less than \c n, the remainder of \c data will be
 * padded with null bytes.
 *
 * If \c data is \c NULL, the consumed string will not be copied.
 *
 * \param buffer The buffer being consumed from.
 * \param n Amount of bytes to consume.
 * \param data A string to which \c n bytes will be copied, or \c NULL.
 * \return The amount of bytes consumed.
 */
size_t sh_buffer_consume(struct sh_buffer *, size_t, char *);

/** Retrieve data from the buffer.
 *
 * The buffer retains management of allocated memory, and the returned
 * data should be copied. If sh_buffer_consume() is subsequently invoked
 * the validity of the returned pointer is undefined.
 *
 * \param buffer The buffer from which to retrieve data.
 * \return Pointer to the string contained within the buffer, or \c NULL
 * if the buffer is empty.
 */
char *sh_buffer_data(struct sh_buffer *);

/** Return the length of string contained within the buffer.
 * \param buffer The buffer from which to retrieve the length.
 * \return The length of string contained within the buffer.
 */
size_t sh_buffer_length(struct sh_buffer *);

/** Return the amount of allocated memory.
 * \param buffer The buffer to be checked.
 * \return The amount of allocated memory for the buffer.
 */
size_t sh_buffer_capacity(struct sh_buffer *);

/** Release allocated resources by the buffer.
 * \param buffer The buffer to be released.
 */
void sh_buffer_release(struct sh_buffer *);

/** \} */

#endif

