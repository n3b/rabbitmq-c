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

#include "amqp_hash.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>


void amqp_channel_table_init(amqp_channel_table_t *table)
{
    assert(table != NULL);
    
    memset(&table->entries, 0,
           sizeof(amqp_channel_table_entry_t) * AMQP_CHANNEL_TABLE_SIZE);
    init_amqp_pool(&table->entry_pool, sizeof(amqp_channel_table_entry_t) * 16);
}

void amqp_channel_table_destroy(amqp_channel_table_t *table)
{
    int i;
    assert(table != NULL);
    
    for (i = 0; i < AMQP_CHANNEL_TABLE_SIZE; ++i)
    {
        amqp_channel_table_entry_t *entry;
        for (entry = table->entries[i];
             entry != NULL; entry = entry->next)
        {
            amqp_pool_link_t *pool = entry->pool;
            while (NULL != pool)
            {
                amqp_pool_link_t *next;

                empty_amqp_pool(&pool->pool);

                next = pool->next;
                free(pool);
                pool = next;
            }
        }
    }
    
    empty_amqp_pool(&table->entry_pool);
}


int amqp_channel_table_add_pool(amqp_channel_table_t *table, amqp_channel_t channel, amqp_pool_link_t *pool)
{
  amqp_channel_table_entry_t *entry;
  
  assert(NULL != table);
  
  entry = amqp_channel_table_get_entry(table, channel);
  if (NULL == entry)
    return -1;
  
  pool->next = entry->pool;
  entry->pool = pool;

  return 0;
}

amqp_pool_link_t *amqp_channel_table_get_used_pools(amqp_channel_table_t *table, amqp_channel_t channel)
{
    amqp_channel_table_entry_t *entry;
    amqp_pool_link_t *pool;
    
    assert(NULL != table);
    
    entry = amqp_channel_table_get_entry(table, channel);
    
    pool = entry->pool;
    while (NULL != pool)
    {
        recycle_amqp_pool(&pool->pool);
        pool = pool->next;
    }
    
    pool = entry->pool;
    entry->pool = NULL;
    
    return pool;
}

amqp_pool_link_t *amqp_channel_table_get_all_used_pools(amqp_channel_table_t *table)
{
    amqp_pool_link_t *results;
    int i;
    
    assert(NULL != table);
    
    results = NULL;
    
    for (i = 0; i < AMQP_CHANNEL_TABLE_SIZE; ++i)
    {
        amqp_channel_table_entry_t *entry;
        for (entry = table->entries[i];
             entry != NULL; entry = entry->next)
        {
            amqp_pool_link_t *pool = entry->pool;
            while (NULL != pool)
            {
                amqp_pool_link_t *next;
                
                next = pool->next;
                pool->next = results;
                results = pool;
                
                pool = next;
            }
            entry->pool = NULL;
        }
    }
    
    return results;
}

static int amqp_channel_table_hash(amqp_channel_t key)
{
    return key % AMQP_CHANNEL_TABLE_SIZE;
}

amqp_channel_table_entry_t *amqp_channel_table_get_entry(amqp_channel_table_t *table,
                                                         amqp_channel_t key)
{
    int entry_key;
    amqp_channel_table_entry_t *entry;
    
    assert(table != NULL);
    
    entry_key = amqp_channel_table_hash(key);
    
    entry = table->entries[entry_key];
    while (NULL != entry)
    {
        if (key == entry->channel)
        {
            return entry;
        }
        entry = entry->next;
    }
    
    /* Entry doesn't exist, create it */
    entry = amqp_channel_entry_create(&table->entry_pool);
    if (NULL == entry)
    {
        return NULL;
    }
    
    entry->channel = key;
    entry->next = table->entries[entry_key];
    table->entries[entry_key] = entry;
    
    return entry;
}

static amqp_channel_table_entry_t *amqp_channel_entry_create(amqp_pool_t *from_pool)
{
    amqp_channel_table_entry_t *entry;
    
    assert(NULL != from_pool);
    
    entry = amqp_pool_alloc(from_pool, sizeof(amqp_channel_table_entry_t));
    if (NULL == entry)
    {
        return NULL;
    }
    
    entry->next = NULL;
    entry->pool = NULL;
    
    return entry;
}
