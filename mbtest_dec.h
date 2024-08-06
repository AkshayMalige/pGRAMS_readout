#ifndef __MBTEST_H__
#define __MBTEST_H__

static DWORD dwAddrSpace;
static UINT32 u32Data;
static unsigned short u16Data;
static unsigned long long u64Data, u64Data1;
static DWORD dwOffset;
static long imod, ichip;
unsigned short *buffp;
static UINT32 i, j, k, ifr, iprint, iwrite, ik, il, is, checksum, writedebug;
static UINT32 istop, newcmd, irand, ioffset, kword, lastchnl, ib;
static UINT32 read_array1[40000], read_array_compare[40000000], read_array_1st[40000000];
static UINT32 read_array_c[40000];
static UINT32 read_comp[8000];
static UINT32 nmask, itmp, nword_tot, nevent, iv, ijk, islow_read, ch;
static UINT32 imod_p, imod_trig, imod_shaper;
unsigned short idcm_read_array[40000], read_array_s[1600000];
static UINT32 idcm_read_array32[40000];
static UINT32 idcm_send_array[400000];
static UINT32 idcm_verify_array[400000];
static int icomp_l, comp_s, ia, ic, ihuff, sample_b, dis;
static UINT32 iwrited;
UINT32 *idcm_send_p, *idcm_verify_p, *pbuffp_rec;
DWORD dwDMABufSize;
PVOID pbuf;
WD_DMA *pDma;
DWORD dwStatus;
DWORD dwOptions = DMA_FROM_DEVICE;
UINT32 iread, icheck, izero;
UINT32 buf_send[40000];
static int count, num, counta, nword, ireadback, nloop, ierror;
static int ij, nsend, iloop, inew, idma_readback, iadd, jevent;
static int itest, iframe, irun, ichip_c, dummy1, itrig_c;
static int idup, ihold, idouble, ihold_set, istatus_read;
static int idone, tr_bar, t_cs_reg, r_cs_reg, dma_tr;
static int timesize, ipulse, ibase, a_id, itrig_delay;
static int iset, ncount, nsend_f, nwrite, itrig_ext;
static int imod_xmit, idiv, isample;
static int iframe_length, itrig, idrift_time, ijtrig;
static int idelay0, idelay1, threshold0, threshold1, pmt_words;
static int cos_mult, cos_thres, en_top, en_upper, en_lower, hg, lg;
static int irise, ifall, istart_time, use_pmt, pmt_testpulse;
static int ich_head, ich_sample, ich_frm, idebug, ntot_rec, nred;
static int ineu, ibusy_send, ibusy_test, ihold_word, ndma_loop;
static int irawprint, nwrite_byte, idis_c, idis_c1;
static int icomp_index, nword_comp, nk, ilast_check;
static int ic_ev, ic_fr, event_save, frame_save, frame_ev, event_ev;
static int imod_fem, imod_st, imod_last, itrig_type, last_dma_loop_size;
static int n_read, n_write, pt_trig_wdone, pt_snova_wdone;
static int pt_trig_dmastart;
static int nremain, nread_dma, nremain_tran1, nremain_tran2, ig, nremain_dma;
static int rc_pt, nword_n, nwrite_byte_n, is1;
static int wait_min, wait_max;
static int iwait_usec, iwait_loop;
static int icom_factor, ifr_c2, ith_fr;
static int pmt_dac_scan, idac_shaper;
static int itrig_pulse, p1_delay, p1_width, p2_delay, p2_width, pulse_trig_delay;
static int icont, ibytec, n_trig, imulti;
static int mask1, mask8, readtype;
static UINT32 iwritem;
static UINT32 dmasizewrite;
void *status_pt;
size_t stacksize;
pthread_t threads[3];
pthread_attr_t attr_pt_trig_dma, attr_pt_sn_dma;
pthread_attr_t attr_pt_tr, attr_pt_sn;
unsigned char carray[4000];
static UINT32 send_array[40000];


static int   idelay0, idelay1, threshold0, threshold1hg, threshold1lg, pmt_words, pmt_wordshg, pmt_wordslg;
static int   threshold3hg, threshold3lg;
static int   threshold1,threshold3;
static int   cos_mult, cos_thres, beam_mult, beam_thres, en_top, en_upper, en_lower, hg, lg, lg_ch, hg_ch, trig_ch, bg_ch;
static int   irise, ifall, istart_time, use_pmt, pmt_testpulse, pmt_precount;
//static int   ich_head, ich_sample, ich_frm,idebug,ntot_rec,nred;
//static int   ineu,ibusy_send,ibusy_test,ihold_word,ndma_loop;
//static int   irawprint,ifem_fst,ifem_lst,ifem_loop;
static int   pmt_deadtimehg,pmt_deadtimelg,pmt_mich_window,bg,bge,tre,beam_size,pmt_width;
static int pmt_deadtime;
 
unsigned char charchannel;
struct timespec tim, tim2;
static long tnum;
char name[150];

PVOID pbuf_rec;
WD_DMA *pDma_rec;
PVOID pbuf_rec1;
WD_DMA *pDma_rec1;
PVOID pbuf_rec2;
WD_DMA *pDma_rec2;

DWORD dwOptions_send = DMA_TO_DEVICE | DMA_ALLOW_CACHE;
DWORD dwOptions_rec = DMA_FROM_DEVICE | DMA_ALLOW_64BIT_ADDRESS;

static UINT64 *buffp_rec64;
static UINT32 *buffp_rec32, *buffp_rec32_n;
UINT32 *px, *py, *py1;

FILE *outf, *inpf, *outinfo;

struct thread_data
{
    int id;
    WDC_DEVICE_HANDLE hdev;
    WDC_DEVICE_HANDLE hdevc;
};

struct thread_data thread_data_n;
struct thread_data thread_data_sn;

static int fd, fd2;
static int iter;

static int imod_pmt, imod_tpc;
int ntrans = 1;
static int nread = 4096 * 2 + 6;
//ifr = 0;
//iwrite = 1;
//iprint = 0;
//icheck = 0;
//istop = 0;
//iwrited = 0;
//dwDMABufSize = 4000000;

#endif
