#ifndef __TX_H__
#define __TX_H__

void lynx_tx_init(void);

#define ACQ_FLAG_DEFAULT_Q       0x01
#define ACQ_FLAG_CMD0_REQ        0x02
#define ACQ_FLAG_CMD1_REQ        0x04
#define ACQ_FLAG_ACTIVE          0x08
#define ACQ_FLAG_LOCK            0x10     /* do not switch it before all of the queued TX is done */

#define AC_BK_QID       0
#define AC_BE_QID       1
#define AC_VI_QID       2
#define AC_VO_QID       3
#define AC_LEGACY_QID   4
#define BCQ_QID         5

#define EMPTY_PKT_BUFH  0x3ff

#define ACQ_INTR_BIT(qid, cmdid)  (0x01UL << ((qid * 2) + cmdid))

int tid_to_qid(int tid);
#define DEF_ACQ(qid)    &info->def_acq[qid]
#define ACQE(q, ptr)    &q->acqe[ptr % q->queue_size]
#define ACQ_PENDING(q)  ((q->wptr >= q->rptr) ? (q->wptr - q->rptr) : \
                              ((((u16) q->wptr + 4096) - q->rptr)))
#define ACQ_FULL(q)     ((q->wptr >= q->rptr) ? ((q->wptr - q->rptr) == q->queue_size) : \
                              ((((u16) q->wptr + 4096) - q->rptr) == ((u16) q->queue_size)))
#define ACQ_EMPTY(q)    (q->wptr == q->rptr)

int mpdu_copy_to_buffer(mpdu *pkt, int use_eth_hdr, int *bhr_h, int *bhr_t);

u16 tx_rate_encoding(int format, int ch_offset, int retry_count, int sgi, int rate);

void lynx_acq_intr_handler(void);

void tx_single_acq_setup(acq *acq, int acqid, int spacing, int max_length, int win_size, int queue_size, int aidx, int tid, int ssn);
void tx_single_acq_start(acq *acq, int ssn);
void tx_mixed_acq_setup(acq *acq, int acqid, int queue_size);
void tx_acq_kick(acq *q, u32 esn);
int tx_acq_detach(acq *q);
int tx_acq_attach(acq *q);
acq *tx_acq_alloc(void);
void tx_acq_free(acq *q);

#endif // __TX_H__

