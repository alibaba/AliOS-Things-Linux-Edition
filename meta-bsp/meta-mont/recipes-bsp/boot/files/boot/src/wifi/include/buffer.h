#ifndef __BUFFER_H__
#define __BUFFER_H__

void sched_lock(void);
void sched_unlock(void);
#define BHDR_LOCK()     sched_lock()
#define BHDR_UNLOCK()   sched_unlock()

static inline int bhdr_to_idx(MAC_INFO *info, buf_header *bhdr)
{
   return (bhdr - &info->buf_headers[0]);
}

static inline buf_header* idx_to_bhdr(MAC_INFO *info, int index)
{
   return (buf_header *)&info->buf_headers[index];
}

static inline buf_header* bhdr_find_tail(MAC_INFO *info, buf_header *bhdr)
{
   BHDR_LOCK();

   for (; bhdr; bhdr = (buf_header *)&info->buf_headers[bhdr->next_index]) if (bhdr->ep)
         break;
   BHDR_UNLOCK();

   return bhdr;
}

buf_header* bhdr_get_first(MAC_INFO *info);
void bhdr_insert_tail(MAC_INFO *info, int head, int tail);
int bhdr_tx_alloc_count(void);

#endif // __BUFFER_H__

