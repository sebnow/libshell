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

#include <string.h>

#include <tap.h>

#include "buffer.h"

#define NTESTS 7

int main()
{
	struct sh_buffer buffer;
	size_t consumed = 0;
	char data[256];
	size_t length;

	plan_tests(NTESTS);

	ok(sh_buffer_init(&buffer, 8) == 0,
		"Given a new buffer, initialising should suceeed");
	ok(sh_buffer_length(&buffer) == 0, "Given a new buffer, it should be empty");

	sh_buffer_append(&buffer, "Hello");
	ok(strcmp(sh_buffer_data(&buffer), "Hello") == 0
		&& sh_buffer_length(&buffer) == 5, "Given a new buffer, when a string is "
		"appended, it should contain the string");

	consumed = sh_buffer_consume(&buffer, 2, data);
	ok(consumed == 2 && sh_buffer_length(&buffer) == 3, "Given a buffer, when "
		"consumed, its length should decrease");
	ok(strcmp(sh_buffer_data(&buffer), "llo") == 0, "Given a buffer, when "
		"consumed, the old data should not be accessible");
	memset(data, 0, sizeof(*data));

	consumed = sh_buffer_consume(&buffer, 4, data);
	ok(consumed == 3 && sh_buffer_length(&buffer) == 0, "Given a buffer, when "
		"consumed beyond its length, it should consume at most its length");
	memset(data, 0, sizeof(*data));

	consumed = sh_buffer_consume(&buffer, 10, data);
	ok(consumed == 0 && sh_buffer_length(&buffer) == 0 && strlen(data) == 0,
		"Given an empty buffer, when consumed, nothing should be returned");
	memset(data, 0, sizeof(*data));

	length = buffer.capacity;
	sh_buffer_append(&buffer, "Hello world!");

	return exit_status();
}

