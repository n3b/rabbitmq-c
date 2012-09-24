/*
 * ***** BEGIN LICENSE BLOCK *****
 * Version: MIT
 *
 * Portions created by VMware are Copyright (c) 2007-2012 VMware, Inc.
 * All Rights Reserved.
 *
 * Portions created by Tony Garnock-Jones are Copyright (c) 2009-2010
 * VMware, Inc. and Tony Garnock-Jones. All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * ***** END LICENSE BLOCK *****
 */

#ifndef AMQP_HASH_H
#define AMQP_HASH_H

#include "amqp.h"

#define AMQP_CHANNEL_TABLE_SIZE 256

typedef struct amqp_pool_link_t_ {
    amqp_pool_t pool;
    struct amqp_pool_link_t_ *next;
} amqp_pool_link_t;

typedef struct amqp_channel_table_entry_t_ {
    amqp_pool_link_t *pool;
    struct amqp_channel_table_entry_t_ *next;
    amqp_channel_t channel;
} amqp_channel_table_entry_t;

typedef struct amqp_channel_table_t_ {
    amqp_channel_table_entry_t *entries[AMQP_CHANNEL_TABLE_SIZE];
    amqp_pool_t entry_pool;
} amqp_channel_table_t;

void amqp_channel_table_init(amqp_channel_table_t *table);
void amqp_channel_table_destroy(amqp_channel_table_t *table);

int amqp_channel_table_add_pool(amqp_channel_table_t *table, amqp_channel_t channel, amqp_pool_link_t *pool);

amqp_pool_link_t *amqp_channel_table_get_used_pools(amqp_channel_table_t *table, amqp_channel_t channel);
amqp_pool_link_t *amqp_channel_table_get_all_used_pools(amqp_channel_table_t *table);


static amqp_channel_table_entry_t *amqp_channel_table_get_entry(amqp_channel_table_t *table, amqp_channel_t key);


static amqp_channel_table_entry_t *amqp_channel_entry_create(amqp_pool_t *from_pool);

#endif /* AMQP_HASH_H */
