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

#include "buffer.h"

#include <assert.h>
#include <string.h>
#include <stdio.h> /* BUFSIZ */

#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

#define DEFAULT_CAPACITY 1024
#define grow_factor(n) (((n) + 16) * 3 / 2)

char sh_null_buffer[1] = "";

static size_t new_capacity(size_t capacity)
{
	return capacity == 0 ? DEFAULT_CAPACITY : grow_factor(capacity);
}

int sh_buffer_init(struct sh_buffer *buffer, size_t capacity)
{
	memset(buffer, 0, sizeof(*buffer));
	buffer->data = sh_null_buffer;
	if(capacity > 0) {
		if(!sh_buffer_grow(buffer, capacity)) {
			return 1;
		}
	}
	return 0;
}

int sh_buffer_grow(struct sh_buffer *buffer, size_t hint)
{
	size_t wanted_cap = buffer->length + hint + 1;
	size_t grown;
	if(hint > buffer->capacity) {
		/* Integer overflow */
		assert(wanted_cap > buffer->length);
		if(buffer->capacity == 0) {
			buffer->data = NULL;
		}
		/* Allocate at least the wanted amount */
		grown = new_capacity(buffer->capacity);
		buffer->capacity = MAX(wanted_cap, grown);
		buffer->data = realloc(buffer->data, buffer->capacity * sizeof(*buffer->data));
	}
	return 1;
}

int sh_buffer_append(struct sh_buffer *buffer, char *data)
{
	size_t length = strlen(data);
	size_t total_length = (buffer->pos - buffer->data) + buffer->length;
	/* Not enough space at the end of the buffer. Attempt to shift data to the
	 * beginning of the buffer. */
	if(buffer->pos != NULL && total_length + length > buffer->capacity) {
		memmove(buffer->data, buffer->pos, buffer->length);
		buffer->data[buffer->length] = '\0';
		buffer->pos = buffer->data;
	}

	/* Allocate additional memory if capacity has been reached. */
	total_length = (buffer->pos - buffer->data) + buffer->length;
	if(buffer == NULL || total_length > buffer->capacity) {
		if(!sh_buffer_grow(buffer, length)) {
			return 0;
		}
	}
	/* Append the string to the buffer. */
	if(buffer->data != NULL) {
		buffer->data = strcat(buffer->data, data);
		buffer->length += length;
		if(buffer->pos == NULL) {
			buffer->pos = buffer->data;
		}
		return 1;
	} else {
		return 0;
	}
}

size_t sh_buffer_consume(struct sh_buffer *buffer, size_t n, char *data)
{
	size_t consumed = 0;
	if(buffer->length > 0) {
		if(data != NULL) {
			strncpy(data, buffer->pos, n + 1);
		}
		consumed = buffer->length > n ? n : buffer->length;
		buffer->pos += consumed;
		buffer->length -= consumed;
		if(buffer->length == 0) {
			buffer->pos = NULL;
		}
	} else if(data != NULL) {
		*data = '\0';
	}
	return consumed;
}

char *sh_buffer_data(struct sh_buffer *buffer)
{
	return buffer->pos;
}

size_t sh_buffer_length(struct sh_buffer *buffer)
{
	return buffer->length;
}

size_t sh_buffer_capacity(struct sh_buffer *buffer)
{
	return buffer->capacity;
}

void sh_buffer_release(struct sh_buffer *buffer)
{
	free(buffer->data);
}

