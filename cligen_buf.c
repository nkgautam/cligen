/*
  Copyright (C) 2001-2016 Olof Hagsand

  This file is part of CLIgen.

  CLIgen is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  CLIgen is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with CLIgen; see the file COPYING.
 *
 *
 * CLIgen dynamic buffers 
 * @code
 *   cbuf *cb;
 *   if ((cb = cbuf_new()) == NULL)
 *      err();
 *   cprintf(cb, "content %d", 42);
 *   if (write(f, cbuf_get(cb), cbuf_len(cb)) < 0)
 *      err();
 *   cbuf_free(cb);
 * @endcode
 */

/*
 * Constants
 */
#define CBUFLEN_START 8*1024   /* Start length */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cligen_buf.h"

/*! CLIgen buffer hidden from external view 
 */
struct cbuf {
    char  *cb_buffer;     
    size_t cb_buflen;  
    size_t cb_strlen;  
};

/*! Allocate cligen buffer. The handle returned can be used in  successive sprintf calls
 * which dynamically print a string.
 * The handle should be freed by cbuf_free()
 * Return the allocated object handle on success.
 * Returns NULL on error.
 */
cbuf *
cbuf_new(void)
{
    cbuf *cb;

    if ((cb = (cbuf*)malloc(sizeof(*cb))) == NULL)
	return NULL;
    memset(cb, 0, sizeof(*cb));
    if ((cb->cb_buffer = malloc(CBUFLEN_START)) == NULL)
	return NULL;
    cb->cb_buflen = CBUFLEN_START;
    memset(cb->cb_buffer, 0, cb->cb_buflen);
    cb->cb_strlen = 0;
    return cb;
}

/*! Free cligen buffer previously allocated with cbuf_new
 */
void
cbuf_free(cbuf *cb)
{
    if (cb->cb_buffer)
	free(cb->cb_buffer);
    free(cb);
}

/*! Return actual byte buffer of cligen buffer
 */
char*
cbuf_get(cbuf *cb)
{
    return cb->cb_buffer;
}

/*! Return length of cligen buffer
 */
int
cbuf_len(cbuf *cb)
{
    return cb->cb_strlen;
}

/*! Reset a cligen buffer. That is, restart it from scratch.
 */
void
cbuf_reset(cbuf *cb)
{
    cb->cb_strlen    = 0; 
    cb->cb_buffer[0] = '\0'; 
}

/*! Create cligen buf by printf like semantics
 * 
 * @param [in]  cb      cligen buffer allocated by cbuf_new(), may be reallocated.
 * @param [in]  format  arguments uses printf syntax.
 * @retval      See printf
 */
int
cprintf(cbuf *cb, const char *format, ...)
{
    va_list ap;
    int diff;
    int retval;

    va_start(ap, format);
  again:
    if (cb == NULL)
	return 0;
    retval = vsnprintf(cb->cb_buffer+cb->cb_strlen, 
		       cb->cb_buflen-cb->cb_strlen, 
		       format, ap);
    if (retval < 0)
	return -1;
    diff = cb->cb_buflen - (cb->cb_strlen + retval + 1);
    if (diff <= 0){
	cb->cb_buflen *= 2;
	if ((cb->cb_buffer = realloc(cb->cb_buffer, cb->cb_buflen)) == NULL)
	    return -1;
	va_end(ap);
	va_start(ap, format);
	goto again;
    }
    cb->cb_strlen += retval;

    va_end(ap);
    return retval;
}




