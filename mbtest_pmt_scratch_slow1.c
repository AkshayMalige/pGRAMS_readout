#include "pmt_slow.h"

/*************************************************************
  General definitions
 *************************************************************/
/*************************************************************
  Functions implementation
 *************************************************************/
int main(void)
{

  struct timeval start;
  gettimeofday(&start, NULL);

  long seconds, useconds;
  seconds = start.tv_sec;
  useconds = start.tv_usec;

  printf("\nStart time of program: %ld sec %ld usec\n", seconds, useconds);

  WDC_DEVICE_HANDLE hDev = NULL;
  WDC_DEVICE_HANDLE hDev1 = NULL;
  WDC_DEVICE_HANDLE hDev2 = NULL;

  DWORD dwStatus;

  printf("\n");
  printf("PCIE diagnostic utility.\n");
  printf("Application accesses hardware using " WD_PROD_NAME ".\n");

  /* Initialize the PCIE library */
  dwStatus = PCIE_LibInit();
  if (WD_STATUS_SUCCESS != dwStatus)
  {
    PCIE_ERR("pcie_diag: Failed to initialize the PCIE library: %s",
             PCIE_GetLastErr());
    return dwStatus;
  }

  /* Find and open a PCIE device (by default ID) */
  if (PCIE_DEFAULT_VENDOR_ID)
    hDev = DeviceFindAndOpen(PCIE_DEFAULT_VENDOR_ID, PCIE_DEFAULT_DEVICE_ID);
  if (PCIE_DEFAULT_VENDOR_ID)
    hDev1 = DeviceFindAndOpen(PCIE_DEFAULT_VENDOR_ID, PCIE_DEFAULT_DEVICE_ID + 1);
  if (PCIE_DEFAULT_VENDOR_ID)
    hDev2 = DeviceFindAndOpen(PCIE_DEFAULT_VENDOR_ID, PCIE_DEFAULT_DEVICE_ID + 2);

  /* Display main diagnostics menu for communicating with the device */
  MenuMain(&hDev, &hDev2);

  /* Perform necessary cleanup before exiting the program */
  if (hDev)
    DeviceClose(hDev);
  DeviceClose(hDev1);
  DeviceClose(hDev2);

  dwStatus = PCIE_LibUninit();
  if (WD_STATUS_SUCCESS != dwStatus)
    PCIE_ERR("pcie_diag: Failed to uninit the PCIE library: %s", PCIE_GetLastErr());

  return dwStatus;
}

/* Plug-and-play and power management events handler routine */
static void DiagEventHandler(WDC_DEVICE_HANDLE hDev, DWORD dwAction)
{
  /* TODO: You can modify this function in order to implement your own
           diagnostics events handler routine */

  printf("\nReceived event notification (device handle 0x%p): ", hDev);
  switch (dwAction)
  {
  case WD_INSERT:
    printf("WD_INSERT\n");
    break;
  case WD_REMOVE:
    printf("WD_REMOVE\n");
    break;
  case WD_POWER_CHANGED_D0:
    printf("WD_POWER_CHANGED_D0\n");
    break;
  case WD_POWER_CHANGED_D1:
    printf("WD_POWER_CHANGED_D1\n");
    break;
  case WD_POWER_CHANGED_D2:
    printf("WD_POWER_CHANGED_D2\n");
    break;
  case WD_POWER_CHANGED_D3:
    printf("WD_POWER_CHANGED_D3\n");
    break;
  case WD_POWER_SYSTEM_WORKING:
    printf("WD_POWER_SYSTEM_WORKING\n");
    break;
  case WD_POWER_SYSTEM_SLEEPING1:
    printf("WD_POWER_SYSTEM_SLEEPING1\n");
    break;
  case WD_POWER_SYSTEM_SLEEPING2:
    printf("WD_POWER_SYSTEM_SLEEPING2\n");
    break;
  case WD_POWER_SYSTEM_SLEEPING3:
    printf("WD_POWER_SYSTEM_SLEEPING3\n");
    break;
  case WD_POWER_SYSTEM_HIBERNATE:
    printf("WD_POWER_SYSTEM_HIBERNATE\n");
    break;
  case WD_POWER_SYSTEM_SHUTDOWN:
    printf("WD_POWER_SYSTEM_SHUTDOWN\n");
    break;
  default:
    printf("0x%lx\n", dwAction);
    break;
  }
}

/* Main diagnostics menu */
static void MenuMain(WDC_DEVICE_HANDLE *phDev, WDC_DEVICE_HANDLE *phDev2)
{
  DWORD option;

  do
  {
    printf("\n");
    printf("PCIE main menu\n");
    printf("--------------\n");
    printf("%d. Scan PCI bus\n", MENU_MAIN_SCAN_PCI_BUS);
    printf("%d. Find and open a PCIE device\n", MENU_MAIN_FIND_AND_OPEN);
    if (*phDev)
    {
      printf("%d. Read/write memory and IO addresses on the device\n",
             MENU_MAIN_RW_ADDR);
      printf("%d. Read/write the device's configuration space\n",
             MENU_MAIN_RW_CFG_SPACE);
      if (PCIE_REGS_NUM)
      {
        printf("%d. Read/write the run-time registers\n",
               MENU_MAIN_RW_REGS);
      }
      printf("%d. Enable/disable the device's interrupts\n",
             MENU_MAIN_ENABLE_DISABLE_INT);
      printf("%d. Register/unregister plug-and-play and power management "
             "events\n",
             MENU_MAIN_EVENTS);
      printf("%d. MicroBoone test\n", MENU_MAIN_MB_TEST);
      printf("%d. Test loop for PCIe\n", MENU_MAIN_JSEBII_TEST);
    }
    printf("%d. Exit\n", MENU_MAIN_EXIT);

    /**
          if (DIAG_INPUT_FAIL == DIAG_GetMenuOption(&option,
              *phDev ? MENU_MAIN_EVENTS : MENU_MAIN_FIND_AND_OPEN))
          {
              continue;
          }
    **/

    if (DIAG_INPUT_FAIL == DIAG_GetMenuOption(&option,
                                              *phDev ? MENU_MAIN_JSEBII_TEST : MENU_MAIN_FIND_AND_OPEN))
    {
      continue;
    }

    switch (option)
    {
    case MENU_MAIN_EXIT: /* Exit menu */
      break;
    case MENU_MAIN_SCAN_PCI_BUS: /* Scan PCI bus */
      WDC_DIAG_PciDevicesInfoPrintAll(FALSE);
      break;
    case MENU_MAIN_FIND_AND_OPEN: /* Find and open a PCIE device */
      if (*phDev)
        DeviceClose(*phDev);
      *phDev = DeviceFindAndOpen(0, 0);
      break;
    case MENU_MAIN_RW_ADDR: /* Read/write memory and I/O addresses */
      MenuReadWriteAddr(*phDev);
      break;
    case MENU_MAIN_RW_CFG_SPACE: /* Read/Write the PCIE configuration space */
      MenuReadWriteCfgSpace(*phDev);
      break;
    case MENU_MAIN_RW_REGS: /* Read/write the run-time registers */
      if (PCIE_REGS_NUM)
        MenuReadWriteRegs(*phDev);
      else
        printf("Invalid selection\n");
      break;
    case MENU_MAIN_ENABLE_DISABLE_INT: /* Enable/disable interrupts */
      MenuInterrupts(*phDev);
      break;
    case MENU_MAIN_EVENTS: /* Register/unregister plug-and-play and power management events */
      MenuEvents(*phDev);
      break;
    case MENU_MAIN_MB_TEST: /* my test loop DMA */
      MenuMBtest(*phDev, *phDev2);
      break;
    }
  } while (MENU_MAIN_EXIT != option);
}

static void MenuMBtest(WDC_DEVICE_HANDLE hDev, WDC_DEVICE_HANDLE hDev2)
{
#include "wdc_defs.h"
#define poweroff 0x0
#define poweron 0x1
#define configure_s30 0x2
#define configure_s60 0x3
#define configure_cont 0x20
#define rdstatus 0x80
#define loopback 0x04
#define dcm2_run_off 254
#define dcm2_run_on 255
#define dcm2_online 2
#define dcm2_setmask 3
#define dcm2_offline_busy 4
#define dcm2_load_packet_a 10
#define dcm2_load_packet_b 11
#define dcm2_offline_load 9
#define dcm2_status_read 20
#define dcm2_led_sel 29
#define dcm2_buffer_status_read 30
#define dcm2_status_read_inbuf 21
#define dcm2_status_read_evbuf 22
#define dcm2_status_read_noevnt 23
#define dcm2_zero 12
#define dcm2_compressor_hold 31
#define dcm2_5_readdata 4
#define dcm2_5_firstdcm 8
#define dcm2_5_lastdcm 9
#define dcm2_5_status_read 5
#define dcm2_5_source_id 25
#define dcm2_5_lastchnl 24
#define dcm2_packet_id_a 25
#define dcm2_packet_id_b 26
#define dcm2_hitformat_a 27
#define dcm2_hitformat_b 28
#define part_run_off 254
#define part_run_on 255
#define part_online 2
#define part_offline_busy 3
#define part_offline_hold 4
#define part_status_read 20
#define part_source_id 25
#define t1_tr_bar 0
#define t2_tr_bar 4
#define cs_bar 2
/**  command register location **/
#define tx_mode_reg 0x28
#define t1_cs_reg 0x18
#define r1_cs_reg 0x1c
#define t2_cs_reg 0x20
#define r2_cs_reg 0x24
#define tx_md_reg 0x28
#define cs_dma_add_low_reg 0x0
#define cs_dma_add_high_reg 0x4
#define cs_dma_by_cnt 0x8
#define cs_dma_cntrl 0xc
#define cs_dma_msi_abort 0x10
/** define status bits **/
#define cs_init 0x20000000
#define cs_mode_p 0x8000000
#define cs_mode_n 0x0
#define cs_start 0x40000000
#define cs_done 0x80000000
#define dma_tr1 0x100000
#define dma_tr2 0x200000
#define dma_tr12 0x300000
#define dma_3dw_trans 0x0
#define dma_4dw_trans 0x0
#define dma_3dw_rec 0x40
#define dma_4dw_rec 0x60
#define dma_in_progress 0x80000000
#define dma_abort 0x2
#define mb_cntrl_add 0x1
#define mb_cntrl_test_on 0x1
#define mb_cntrl_test_off 0x0
#define mb_cntrl_set_run_on 0x2
#define mb_cntrl_set_run_off 0x3
#define mb_cntrl_set_trig1 0x4
#define mb_cntrl_set_trig2 0x5
#define mb_cntrl_load_frame 0x6
#define mb_cntrl_load_trig_pos 0x7
#define mb_feb_power_add 0x1
#define mb_feb_conf_add 0x2
#define mb_feb_pass_add 0x3
#define mb_feb_lst_on 1
#define mb_feb_lst_off 0
#define mb_feb_rxreset 2
#define mb_feb_align 3
#define mb_feb_pll_reset 5
#define mb_feb_adc_align 1
#define mb_feb_a_nocomp 2
#define mb_feb_b_nocomp 3
#define mb_feb_blocksize 4
#define mb_feb_timesize 5
#define mb_feb_mod_number 6
#define mb_feb_a_id 7
#define mb_feb_b_id 8
#define mb_feb_max 9
#define mb_feb_test_source 10
#define mb_feb_test_sample 11
#define mb_feb_test_frame 12
#define mb_feb_test_channel 13
#define mb_feb_test_ph 14
#define mb_feb_test_base 15
#define mb_feb_test_ram_data 16
#define mb_feb_a_test 17
#define mb_feb_b_test 18
#define mb_feb_rd_status 20
#define mb_feb_a_rdhed 21
#define mb_feb_a_rdbuf 22
#define mb_feb_b_rdhed 23
#define mb_feb_b_rdbuf 24
#define mb_feb_read_probe 30
#define mb_feb_dram_reset 31
#define mb_feb_adc_reset 33
#define mb_a_buf_status 34
#define mb_b_buf_status 35
#define mb_a_ham_status 36
#define mb_b_ham_status 37
#define mb_feb_a_maxwords 40
#define mb_feb_b_maxwords 41
#define mb_feb_hold_enable 42
#define mb_pmt_adc_reset 1
#define mb_pmt_spi_add 2
#define mb_pmt_adc_data_load 3
#define mb_xmit_conf_add 0x2
#define mb_xmit_pass_add 0x3
#define mb_xmit_modcount 0x1
#define mb_xmit_enable_1 0x2
#define mb_xmit_enable_2 0x3
#define mb_xmit_test1 0x4
#define mb_xmit_test2 0x5
#define mb_xmit_testdata 10
#define mb_xmit_rdstatus 20
#define mb_xmit_rdcounters 21
#define mb_xmit_link_reset 22
#define mb_opt_dig_reset 23
#define mb_xmit_dpa_fifo_reset 24
#define mb_xmit_dpa_word_align 25
#define mb_xmit_link_pll_reset 26
#define mb_trig_run 1
#define mb_trig_frame_size 2
#define mb_trig_deadtime_size 3
#define mb_trig_active_size 4
#define mb_trig_delay1_size 5
#define mb_trig_delay2_size 6
#define mb_trig_calib_delay 8
#define mb_trig_prescale0 10
#define mb_trig_prescale1 11
#define mb_trig_prescale2 12
#define mb_trig_prescale3 13
#define mb_trig_prescale4 14
#define mb_trig_prescale5 15
#define mb_trig_prescale6 16
#define mb_trig_prescale7 17
#define mb_trig_prescale8 18
#define mb_trig_mask0 20
#define mb_trig_mask1 21
#define mb_trig_mask2 22
#define mb_trig_mask3 23
#define mb_trig_mask4 24
#define mb_trig_mask5 25
#define mb_trig_mask6 26
#define mb_trig_mask7 27
#define mb_trig_mask8 28
#define mb_trig_rd_param 30
#define mb_trig_pctrig 31
#define mb_trig_rd_status 32
#define mb_trig_reset 33
#define mb_trig_calib 34
#define mb_trig_rd_gps 35
#define mb_trig_sel1 40
#define mb_trig_sel2 41
#define mb_trig_sel3 42
#define mb_trig_sel4 43
#define mb_trig_p1_delay 50
#define mb_trig_p1_width 51
#define mb_trig_p2_delay 52
#define mb_trig_p2_width 53
#define mb_trig_p3_delay 54
#define mb_trig_p3_width 55
#define mb_trig_pulse_delay 58
#define mb_trig_pulse1 60
#define mb_trig_pulse2 61
#define mb_trig_pulse3 62
#define mb_shaper_pulsetime 1
#define mb_shaper_dac 2
#define mb_shaper_pattern 3
#define mb_shaper_write 4
#define mb_shaper_pulse 5
#define mb_shaper_entrig 6
#define mb_feb_pmt_gate_size 47
#define mb_feb_pmt_beam_delay 48
#define mb_feb_pmt_beam_size 49
#define mb_feb_pmt_ch_set 50
#define mb_feb_pmt_delay0 51
#define mb_feb_pmt_delay1 52
#define mb_feb_pmt_precount 53
#define mb_feb_pmt_thresh0 54
#define mb_feb_pmt_thresh1 55
#define mb_feb_pmt_thresh2 56
#define mb_feb_pmt_thresh3 57
#define mb_feb_pmt_width 58
#define mb_feb_pmt_deadtime 59
#define mb_feb_pmt_window 60
#define mb_feb_pmt_words 61
#define mb_feb_pmt_cos_mul 62
#define mb_feb_pmt_cos_thres 63
#define mb_feb_pmt_mich_mul 64
#define mb_feb_pmt_mich_thres 65
#define mb_feb_pmt_beam_mul 66
#define mb_feb_pmt_beam_thres 67
#define mb_feb_pmt_en_top 68
#define mb_feb_pmt_en_upper 69
#define mb_feb_pmt_en_lower 70
#define mb_feb_pmt_blocksize 71
#define mb_feb_pmt_test 80
#define mb_feb_pmt_clear 81
#define mb_feb_pmt_test_data 82
#define mb_feb_pmt_pulse 83
#define mb_feb_pmt_rxreset 84
#define mb_feb_pmt_align_pulse 85
#define mb_feb_pmt_rd_counters 86

#define dma_buffer_size 10000000

  static DWORD dwAddrSpace;
  static UINT32 u32Data;
  static unsigned short u16Data;
  static unsigned long long u64Data, u64Data1;
  static DWORD dwOffset;
  static long imod, ichip;
  unsigned short *buffp;

  /*
    WDC_ADDR_MODE mode;
    WDC_ADDR_RW_OPTIONS options;
  */
  static UINT32 i, j, k, ifr, nread, iprint, iwrite, ik, il, is, checksum, writedebug;
  static UINT32 istop, newcmd, irand, ioffset, kword, lastchnl, ib;
  static UINT32 read_array[dma_buffer_size], read_array1[40000], read_array_compare[40000000], read_array_1st[40000000];
  ;
  static UINT32 read_array_c[40000];
  static UINT32 read_comp[8000];
  static UINT32 nmask, index, itmp, nword_tot, nevent, iv, ijk, islow_read, ch;
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

  unsigned char charchannel;
  struct timespec tim, tim2;
  static long tnum;
  char name[150];

  ///////////////////////
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
  nread = 4096 * 2 + 6;
  ifr = 0;
  iwrite = 1;
  iprint = 0;
  icheck = 0;
  istop = 0;
  iwrited = 0;
  dwDMABufSize = 4000000;

  newcmd = 1;
  printf("\n\n");
  printf("Note: At this time, there is no way to disable the 1500-sample readout\n");
  printf("      which occurs whenever the FEM receives a beam gate (BG). \n");
  printf("      To disable this feature, you must physically unplug the beam\n");
  printf("      gate input to ch 7 of the shaper (top-most of lower set of 8 LEMO\n");
  printf("      inputs to the shaper). If you do not remove the BG input, any\n");
  printf("      time the BG pulse falls within the 4 frames which are read\n");
  printf("      out as part of an event, the event will contain a set of\n");
  printf("      1500 samples from all 40 PMT channels.\n");
  printf("      Enter 1 to acknowledge this:\t");
  ik = 1;

  printf("\n\nIf you are using the Bo PMT Readout configuration (HG and LG channel\n");
  printf("readout from single PMT, make sure this is the hardware configuration\n");
  printf("you are using:\n");
  printf("docdb-2819");
  if (newcmd == 2)
  {
    printf("\tPlease remove the LED trigger from the EXT input to the trigger\n");
    printf("\tboard and replace it with the COSMIC (paddles) trigger\n");
    printf("\tEnter 1 when you are done:\t");
    scanf("%d", &ik);
  }
  printf("\n\n");
  if (newcmd == 1)
  {
    mask1 = 0x0;
    mask8 = 0x2;
  } // EXT
  else if (newcmd == 2)
  {
    mask1 = 0x0;
    mask8 = 0x2;
  } // EXT
  else if (newcmd == 3)
  {
    mask1 = 0x4;
    mask8 = 0x0;
  } // PMT cosmic //xxx
  else if (newcmd == 4)
  {
    mask1 = 0x1;
    mask8 = 0x0;
  } // PMT beam //xxx
  else if (newcmd == 5)
  {
    mask1 = 0x8;
    mask8 = 0x0;
  } // PMT all
  else if (newcmd == 6)
  {
    mask1 = 0x0;
    mask8 = 0xfd;
  } // BNB
  else if (newcmd == 7)
  {
    mask1 = 0x8;
    mask8 = 0xfd;
  } // PMT all or BNB

  static time_t start, end;
  struct timeval starttest18, endtest18;
  long mytime18, seconds18, useconds18;
  time_t rawtime;
  char timestr[500];

  static int trig_deadtime;
  time(&rawtime);
  //      strftime(timestr, sizeof(timestr), "%Y_%m_%d_%H:%M:%S", localtime(&rawtime));
  strftime(timestr, sizeof(timestr), "%Y_%m_%d", localtime(&rawtime));
  printf("\n ######## PMT XMIT Readout (keyboard interrupt)\n\n");
  printf("\n Enter SUBRUN NUMBER or NAME:\t");
  char subrun[50];
  scanf("%s", &subrun);
  //    printf("Output in /home/uboonedaq/pmt_data_temp/xmit_subrun_%s_%s.bin and .info \n",timestr,subrun);
  //      printf("\nEnter 1 for DMA size = Buf Size = %d (recommended for high trigger/data rates and long runs)\t",dwDMABufSize);
  //      scanf("%d", &ibytec);
  ibytec = 0;
  if (ibytec == 1)
    ibytec = dwDMABufSize;
  else
  {
    printf("\nEnter desired DMA size (<%d)\t", dwDMABufSize);
    scanf("%d", &ibytec);
  }
  dwDMABufSize = ibytec;

  char _buf[200];
  //      sprintf(_buf,"/home/uboonedaq/pmt_data_temp/xmit_subrun_%s_%s_dma_no_1.bin",timestr,subrun);
  sprintf(_buf, "/home/ub/WinDriver/wizard/GRAMS_project_am/data/pmt/xmit_subrun_%s_%s_dma_no_1.bin", timestr, subrun);
  outf = fopen(_buf, "wb");
  fd = creat("test_file.dat", 0755);
  // fd = creat(_buf,0755);
  //       sprintf(_buf,"/home/uboonedaq/pmt_data_temp/xmit_subrun_%s_%s.info",timestr,subrun);
  sprintf(_buf, "/home/ub/WinDriver/wizard/GRAMS_project_am/data/pmt/xmit_subrun_%s_%s.info", timestr, subrun);
  outinfo = fopen(_buf, "w");
  fprintf(outinfo, "nevispcie_pmt_scratch_nokbhit.cpp\n\n");

  idebug = 1;
  irawprint = 0;
  icont = 1;
  iwrite = 1; // grams

  if (iwrite == 1)
  {
    iwritem = 0; // grams
    if (iwritem == 0)
    {
      dmasizewrite = 1; // grams
      sprintf(name, "xmit_trig_bin_grams_%s.dat", subrun);
      fd = creat(name, 0755);
      printf("\n\tOutput file: %s\n", name);
    }
  }

  writedebug = 1;
  // see docdb-2819 for hardware config details

  imod_xmit = 6; // 10;
  imod_pmt = 7;
  imod_st = 8;
  // imod_xmit=7;//10;
  // imod_st=8;

  imod_tpc = 8;
  imod_trig = 16;  // 16;
  imod_shaper = 4; // 6;
  fprintf(outinfo, "imod_xmit = %d\n", imod_xmit);
  fprintf(outinfo, "imod_st   = %d\n", imod_st);
  fprintf(outinfo, "imod_trig = %d\n", imod_trig);
  fprintf(outinfo, "imod_shaper = %d\n", imod_shaper);
  printf("\n");
  printf(" XMIT module address = %d\n", imod_xmit);
  printf(" PMT ADC module address = %d \n", imod_pmt);
  printf(" Shaper address = %d \n", imod_shaper);
  printf(" Trigger module address = %d \n", imod_trig);

  printf("\nFrame length (16MHz time ticks)?\n\t1.6ms --> Enter 25599 (nominal)\n\t512us --> Enter 8191 (if LED calibration)\n\t");
  //      scanf("%d",&iframe_length);
  iframe_length = 1599;
  iframe = iframe_length;
  itrig_delay = 10; // 10
  timesize = 199;
  fprintf(outinfo, "iframe_length (16MHz) = %d\n", iframe_length);
  fprintf(outinfo, "itrig_delay = %d\n", itrig_delay);
  printf("\tframe size = %d\n", iframe_length);
  fprintf(outinfo, "ibytec = %d (DMA size)\n", ibytec);
  fprintf(outinfo, "dwDMABufSize = %d\n", dwDMABufSize);
  fprintf(outinfo, "dma_buffer_size (read_array) = %d\n", ibytec);
  icheck = 0;
  ifr = 0;
  irand = 0;
  islow_read = 0;
  iprint = 1;
  nsend = 500;

  // XMIT PCIe
  dwAddrSpace = 2;
  u32Data = 0x20000000; // initial transmitter, no hold
  dwOffset = 0x18;
  WDC_WriteAddr32(hDev2, dwAddrSpace, dwOffset, u32Data);
  dwAddrSpace = 2;
  u32Data = 0x20000000; // initial transmitter, no hold
  dwOffset = 0x20;
  WDC_WriteAddr32(hDev2, dwAddrSpace, dwOffset, u32Data);

  dwAddrSpace = 2;
  u32Data = 0x20000000; // initial receiver
  dwOffset = 0x1c;
  WDC_WriteAddr32(hDev2, dwAddrSpace, dwOffset, u32Data);
  dwAddrSpace = 2;
  u32Data = 0x20000000; // initial receiver
  dwOffset = 0x24;
  WDC_WriteAddr32(hDev2, dwAddrSpace, dwOffset, u32Data);

  dwAddrSpace = 2;
  u32Data = 0xfff; // set mode off with 0xfff...
  dwOffset = 0x28;
  WDC_WriteAddr32(hDev2, dwAddrSpace, dwOffset, u32Data);

  // RUN INITIALIZATION
  px = &buf_send[0];
  py = &read_array[0];
  imod = 0; /* controller module */
  /** initialize **/
  buf_send[0] = 0x0;
  buf_send[1] = 0x0;
  i = 1;
  k = 1;
  i = pcie_send(hDev, i, k, px);
  // set offline test
  imod = 0;
  ichip = 1;
  buf_send[0] = (imod << 11) + (ichip << 8) + (mb_cntrl_test_on) + (0x0 << 16); // enable offline run on
  i = 1;
  k = 1;
  i = pcie_send(hDev, i, k, px);

  // set trigger module run off
  imod = imod_trig;
  buf_send[0] = (imod << 11) + (mb_trig_run) + ((0x0) << 16); // set up run off
  i = 1;
  k = 1;
  i = pcie_send(hDev, i, k, px);

  // printf("\nEnter trigger deadtime (in units of 32us); minumum is 250 (8ms):\t");
  //       scanf("%d",&trig_deadtime);
  trig_deadtime = 250;
  //  set trigger module dead time size
  imod = imod_trig;
  buf_send[0] = (imod << 11) + (mb_trig_deadtime_size) + ((trig_deadtime & 0xff) << 16); // set trigger module deadtime size
  fprintf(outinfo, "trig_deadtime_size = 0x%x\n", trig_deadtime & 0xff);
  i = 1;
  k = 1;
  i = pcie_send(hDev, i, k, px);

  // set offline test
  imod = 0;
  ichip = 1;
  buf_send[0] = (imod << 11) + (ichip << 8) + (mb_cntrl_test_off) + (0x0 << 16); // set controller test off
  i = 1;
  k = 1;
  i = pcie_send(hDev, i, k, px);

  imod = imod_xmit;
  ichip = mb_xmit_conf_add;
  buf_send[0] = (imod << 11) + (ichip << 8) + 0x0 + (0x0 << 16); // turn conf to be on
  i = 1;
  k = 1;
  i = pcie_send(hDev, i, k, px);

  // fprintf(outinfo,"XMIT FPGA: /home/ub/readcontrol_11601_v3_play_header_6_21_2013.rbf");

  boot_xmit(hDev, imod_xmit);

  printf("\nXMIT done, booting FEM...\n");

  //    both FEM and XMIT bootted.

  // check FEM status again
  // readback status
  // nword=1;
  // i = pcie_rec(hDev,0,1,nword,iprint,py);     // init the receiver

  // imod_fem=15;
  // imod=imod_fem;
  // ichip=3;
  // buf_send[0]=(imod<<11)+(ichip<<8)+20+(0x0<<16);  // read out status
  // i=1;
  // k=1;
  // i = pcie_send(hDev, i, k, px);
  // py = &read_array[0];
  // i = pcie_rec(hDev,0,2,nword,iprint,py);     // read out 2 32 bits words

  nword = 1;
  i = pcie_rec(hDev, 0, 1, nword, iprint, py); // init the receiver

  // imod_fem=15;
  imod = imod_xmit;
  ichip = 3;
  buf_send[0] = (imod << 11) + (ichip << 8) + mb_xmit_rdstatus + (0x0 << 16); // read out status
  i = 1;
  k = 1;
  i = pcie_send(hDev, i, k, px);
  py = &read_array[0];
  i = pcie_rec(hDev, 0, 2, nword, iprint, py); // read out 2 32 bits words
  printf("XMIT status word = %x, %x \n", read_array[0], read_array[1]);

  printf("\nDo you want to read out TPC or PMT? Enter 0 for TPC, 1 for PMT, 2 for both\n");
  scanf("%i", &readtype);

  if (readtype == 0)
  {
    tpc_adc_setup(hDev, hDev2, imod_fem, imod_xmit, iframe_length, imod_trig, timesize);
  }
  else if (readtype == 1)
  {
    imod_fem = imod_pmt;
    printf("Booting FEM in slot %d...\n", imod_fem);
    ik = pmt_adc_setup(hDev, imod_fem, iframe, newcmd, outinfo);
  }
  else if (readtype == 2)
  {
    printf("Booting FEM in slot %d...\n", imod_pmt);
    ik = pmt_adc_setup(hDev, imod_pmt, iframe, newcmd, outinfo);
    tpc_adc_setup(hDev, hDev2, imod_tpc, imod_xmit, iframe_length, imod_trig, timesize);
  }

  printf("\nFEM booting done\n");

  printf("\nFEM STATUS -- after config = %x, %x \n", read_array[0], read_array[1]);
  printf(" module = %d, command = %d \n", ((read_array[0] >> 11) & 0x1f), (read_array[0] & 0xff));
  printf(" ADC right dpa lock     %d \n", ((read_array[0] >> 17) & 0x1));
  printf(" ADC left  dpa lock     %d \n", ((read_array[0] >> 18) & 0x1));
  printf(" block error 2          %d \n", ((read_array[0] >> 19) & 0x1));
  printf(" block error 1          %d \n", ((read_array[0] >> 20) & 0x1));
  printf(" pll locked             %d \n", ((read_array[0] >> 21) & 0x1));
  printf(" supernova mem ready    %d \n", ((read_array[0] >> 22) & 0x1));
  printf(" beam      mem ready    %d \n", ((read_array[0] >> 23) & 0x1));
  printf(" ADC right PLL locked   %d \n", ((read_array[0] >> 24) & 0x1));
  printf(" ADC left  PLL locked   %d \n", ((read_array[0] >> 25) & 0x1));
  printf(" ADC align cmd right    %d \n", ((read_array[0] >> 26) & 0x1));
  printf(" ADC align cmd left     %d \n", ((read_array[0] >> 27) & 0x1));
  printf(" ADC align done right   %d \n", ((read_array[0] >> 28) & 0x1));
  printf(" ADC align done left    %d \n", ((read_array[0] >> 29) & 0x1));
  printf(" Neutrino data empty    %d \n", ((read_array[0] >> 30) & 0x1));
  printf(" Neutrino header empty  %d \n", ((read_array[0] >> 31) & 0x1));

  /**
  imod=imod_shaper;
  buf_send[0] = (imod << 11) + mb_shaper_dac + (0xfff << 16);
  i = 1;
  k = 1;
  i = pcie_send(hDev, i, k, px);
  usleep(10);
  imod = imod_shaper; // write to the DAC
  buf_send[0] = (imod << 11) + mb_shaper_write + ((0xfff) << 16);
  i = 1;
  k = 1;
  i = pcie_send(hDev, i, k, px);
  usleep(1000);
  //
  imod = imod_shaper; // write pulse time
  idiv = 1;
  isample = 1;
  buf_send[0] = (imod << 11) + mb_shaper_pulsetime + (((idiv & 0x7) + ((isample & 0xfff) << 3)) << 16); // all bits on
  i = 1;
  k = 1;
  i = pcie_send(hDev, i, k, px);
  usleep(1000);
  imod = imod_shaper;                                             //write to test pattern register
  buf_send[0] = (imod << 11) + mb_shaper_pattern + ((0xf) << 16); // all bits on
  i = 1;
  k = 1;
  i = pcie_send(hDev, i, k, px);
  usleep(1000);
  imod = imod_shaper;
  is = 0;
  buf_send[0] = (imod << 11) + mb_shaper_entrig + ((is) << 16); // all bits on
  i = 1;
  k = 1;
  i = pcie_send(hDev, i, k, px);
  usleep(1000);
  **/

  /*if(readtype==1 || readtype==2){

    //    set tx mode register
    u32Data = 0x00003fff;  // set up number of words hold coming back from the XMIT module
    printf("\nNumber of words for hold to be send back = %x\n",u32Data);
    fprintf(outinfo,"nwords for hold = 0x%x\n",u32Data);
    dwOffset = 0x28;
    dwAddrSpace =2;
    WDC_WriteAddr32(hDev2, dwAddrSpace, dwOffset, u32Data);
  //   set up hold
    printf("Setting up the hold condition \n");
    dwAddrSpace=2;
    u32Data = 0x8000000;    // set up transmitter to return the hold -- upper transciever
    dwOffset = 0x18;
    WDC_WriteAddr32(hDev2, dwAddrSpace, dwOffset, u32Data);
    dwAddrSpace =2;
    u32Data = 0x8000000;    // set up transmitter to return the hold -- lower transciever
    dwOffset = 0x20;
    WDC_WriteAddr32(hDev2, dwAddrSpace, dwOffset, u32Data);

    printf("finished setting up hold condition!\n");

    //Frame size
    imod=imod_trig;
    buf_send[0]=(imod<<11)+(mb_trig_frame_size)+((iframe & 0xffff)<<16); //set up frame size.
    i=1;
    k=1;
    i = pcie_send(hDev, i, k, px);
    //PMT TRIGGER
    imod=imod_trig;
    buf_send[0]=(imod<<11)+(mb_trig_mask1)+((mask1&0xf)<<16); //set mask1[3] on.
    i=1;
    k=1;
    i = pcie_send(hDev, i, k, px);
    fprintf(outinfo,"trig_mask1 = 0x%x\n",mask1);
    //set prescale1 to 0
    imod=imod_trig;
    buf_send[0]=(imod<<11)+(mb_trig_prescale1)+(0x0<<16); //set prescale1 0
    fprintf(outinfo,"trig_prescale1 = 0x%x\n",0x0);
    i=1;
    k=1;
    i = pcie_send(hDev, i, k, px);

    //EXT & BEAM TRIGGERS
    imod=imod_trig;
    buf_send[0]=(imod<<11)+(mb_trig_mask8)+((mask8&0xff)<<16);
    fprintf(outinfo,"trig_mask8 = 0x%x\n",mask8);
    i=1;
    k=1;
    i = pcie_send(hDev, i, k, px);
    //set prescale8 to 0
    imod=imod_trig;
    buf_send[0]=(imod<<11)+(mb_trig_prescale8)+(0x0<<16); //set prescale8 0
    fprintf(outinfo,"trig_prescale8 = 0x%x\n",0x0);
    i=1;
    k=1;
    i = pcie_send(hDev, i, k, px);

    printf("\nLINK setup \n");
    //imod_fem = imod_xmit+1;
    //for(imod = imod_xmit+1; imod < imod_st+1; imod++){
    ik = pmt_link_setup(hDev,imod_xmit,imod_st,0);      // no supernova
    //}
  }
  else if(readtype == 0){*/
  printf("reached here at least\n");
  //******************************************************
  printf("\n...FEB setup complete.\n\n");
  //******************************************************

  // XMIT setup
  printf("Setting up XMIT module\n");
  //     set up xmit module  -- module count
  imod = imod_xmit;
  ichip = 3;
  //                  -- number of FEM module -1, counting start at 0
  buf_send[0] = (imod << 11) + (ichip << 8) + mb_xmit_modcount + ((imod_st - imod_xmit - 1) << 16);
  i = 1;
  k = 1;
  i = pcie_send(hDev, i, k, px);

  //     reset optical
  imod = imod_xmit;
  ichip = 3;
  buf_send[0] = (imod << 11) + (ichip << 8) + mb_opt_dig_reset + (0x1 << 16); // set optical reset on
  i = 1;
  k = 1;
  i = pcie_send(hDev, i, k, px);

  //     enable Neutrino Token Passing
  imod = imod_xmit;
  ichip = 3;
  buf_send[0] = (imod << 11) + (ichip << 8) + mb_xmit_enable_1 + (0x1 << 16); // enable token 1 pass
  i = 1;
  k = 1;
  i = pcie_send(hDev, i, k, px);

  buf_send[0] = (imod << 11) + (ichip << 8) + mb_xmit_enable_2 + (0x0 << 16); // disable token 2 pass
  i = 1;
  k = 1;
  i = pcie_send(hDev, i, k, px);

  //       reset XMIT LINK IN DPA
  imod = imod_xmit;
  ichip = 3;
  buf_send[0] = (imod << 11) + (ichip << 8) + mb_xmit_link_pll_reset + (0x1 << 16); //  reset XMIT LINK IN DPA
  i = 1;
  k = 1;
  i = pcie_send(hDev, i, k, px);
  usleep(1000);

  //     reset XMIT LINK IN DPA
  imod = imod_xmit;
  ichip = 3;
  buf_send[0] = (imod << 11) + (ichip << 8) + mb_xmit_link_reset + (0x1 << 16); //  reset XMIT LINK IN DPA
  i = 1;
  k = 1;
  i = pcie_send(hDev, i, k, px);

  //     wait for 10ms just in case
  usleep(10000);

  //     reset XMIT FIFO reset
  imod = imod_xmit;
  ichip = 3;
  buf_send[0] = (imod << 11) + (ichip << 8) + mb_xmit_dpa_fifo_reset + (0x1 << 16); //  reset XMIT LINK IN DPA
  i = 1;
  k = 1;
  i = pcie_send(hDev, i, k, px);

  for (is = 0; is < 1; is++)
  {
    //      test re-align circuit
    imod = imod_xmit;
    ichip = 3;
    buf_send[0] = (imod << 11) + (ichip << 8) + mb_xmit_dpa_word_align + (0x1 << 16); //  send alignment pulse
    i = 1;
    k = 1;
    i = pcie_send(hDev, i, k, px);
  }
  usleep(5000); // wait for 5 ms

  nword = 1;
  i = pcie_rec(hDev, 0, 1, nword, iprint, py); // init the receiver
  imod = imod_xmit;
  ichip = 3;
  buf_send[0] = (imod << 11) + (ichip << 8) + mb_xmit_rdstatus + (0x0 << 16); // read out status

  i = 1;
  k = 1;
  i = pcie_send(hDev, i, k, px);
  py = &read_array[0];
  i = pcie_rec(hDev, 0, 2, nword, iprint, py); // read out 2 32 bits words
  printf("XMIT status word = %x, %x \n", read_array[0], read_array[1]);
  usleep(10000);

  //******************************************************
  //     now reset all the link port receiver
  imod_fem = imod_xmit + 1;
  while (imod_fem < imod_st)
  {
    imod = imod_fem + 1;
    ichip = 4;
    buf_send[0] = (imod << 11) + (ichip << 8) + mb_feb_lst_on + (0x0 << 16); // set last module on
    i = 1;
    k = 1;
    i = pcie_send(hDev, i, k, px);
    //	printf(" set last modulen, module address %d\n", imod);
    imod = imod_fem;
    //	printf(" reset the link for module %d \n", imod);
    ichip = 4;
    buf_send[0] = (imod << 11) + (ichip << 8) + mb_feb_rxreset + (0x0 << 16); // reset LINKIN DPA
    i = 1;
    k = 1;
    i = pcie_send(hDev, i, k, px);

    ichip = 4;
    buf_send[0] = (imod << 11) + (ichip << 8) + mb_feb_align + (0x0 << 16); // send alignment command
    i = 1;
    k = 1;
    i = pcie_send(hDev, i, k, px);
    usleep(1000);

    imod = imod_fem + 1;
    if (imod != imod_st)
    {
      ichip = 4;
      buf_send[0] = (imod << 11) + (ichip << 8) + mb_feb_lst_off + (0x0 << 16); // set last module off
      i = 1;
      k = 1;
      i = pcie_send(hDev, i, k, px);
      //	  printf(" set last module off, module address %d\n", imod);
    }
    imod_fem = imod_fem + 1;
  }
  printf("\n...XMIT setup complete\n");
  //******************************************************
  // XMIT setup
  printf("Setting up XMIT module\n");
  //     set up xmit module  -- module count
  imod = imod_xmit;
  ichip = 3;
  //                  -- number of FEM module -1, counting start at 0
  buf_send[0] = (imod << 11) + (ichip << 8) + mb_xmit_modcount + ((imod_st - imod_xmit - 1) << 16);
  i = 1;
  k = 1;
  i = pcie_send(hDev, i, k, px);

  //     reset optical
  imod = imod_xmit;
  ichip = 3;
  buf_send[0] = (imod << 11) + (ichip << 8) + mb_opt_dig_reset + (0x1 << 16); // set optical reset on
  i = 1;
  k = 1;
  i = pcie_send(hDev, i, k, px);

  //     enable Neutrino Token Passing
  imod = imod_xmit;
  ichip = 3;
  buf_send[0] = (imod << 11) + (ichip << 8) + mb_xmit_enable_1 + (0x1 << 16); // enable token 1 pass
  i = 1;
  k = 1;
  i = pcie_send(hDev, i, k, px);

  buf_send[0] = (imod << 11) + (ichip << 8) + mb_xmit_enable_2 + (0x0 << 16); // disable token 2 pass
  i = 1;
  k = 1;
  i = pcie_send(hDev, i, k, px);

  //       reset XMIT LINK IN DPA
  imod = imod_xmit;
  ichip = 3;
  buf_send[0] = (imod << 11) + (ichip << 8) + mb_xmit_link_pll_reset + (0x1 << 16); //  reset XMIT LINK IN DPA
  i = 1;
  k = 1;
  i = pcie_send(hDev, i, k, px);
  usleep(1000);

  //     reset XMIT LINK IN DPA
  imod = imod_xmit;
  ichip = 3;
  buf_send[0] = (imod << 11) + (ichip << 8) + mb_xmit_link_reset + (0x1 << 16); //  reset XMIT LINK IN DPA
  i = 1;
  k = 1;
  i = pcie_send(hDev, i, k, px);

  //******************************************************
  // read back status
  for (imod_fem = (imod_xmit + 1); imod_fem < (imod_st + 1); imod_fem++)
  {
    i = pcie_rec(hDev, 0, 1, nword, iprint, py); // init the receiver
    imod = imod_fem;
    ichip = 3;
    buf_send[0] = (imod << 11) + (ichip << 8) + 20 + (0x0 << 16); // read out status
    i = 1;
    k = 1;
    i = pcie_send(hDev, i, k, px);
    py = &read_array[0];
    i = pcie_rec(hDev, 0, 2, nword, iprint, py); // read out 2 32 bits words
    printf("\nReceived FEB %d (slot %d) status data word = %x, %x \n", imod, imod, read_array[0], read_array[1]);
    printf("----------------------------\n");
    printf("FEB %d (slot %d) status \n", imod, imod);
    printf("----------------------------\n");
    printf("cmd return (20)       : %d\n", (read_array[0] & 0xFF));               // bits 7:0
    printf("check bits 10:8 (0)   : %d\n", ((read_array[0] >> 8) & 0x7));         // bits 10:8
    printf("module number (%d)    : %d\n", imod, ((read_array[0] >> 11) & 0x1F)); // bits 15:11
    printf("----------------------------\n");
    printf("check bit  0 (0)      : %d\n", (read_array[0] >> 16) & 0x1);
    printf("Right ADC DPA locked  : %d\n", (read_array[0] >> 17) & 0x1);
    printf("Left  ADC DPA locked  : %d\n", (read_array[0] >> 18) & 0x1);
    printf("SN pre-buf err        : %d\n", (read_array[0] >> 19) & 0x1);
    printf("Neutrino pre-buf err  : %d\n", (read_array[0] >> 20) & 0x1);
    printf("PLL locked            : %d\n", (read_array[0] >> 21) & 0x1);
    printf("SN memory ready       : %d\n", (read_array[0] >> 22) & 0x1);
    printf("Neutrino memory ready : %d\n", (read_array[0] >> 23) & 0x1);
    printf("ADC lock right        : %d\n", (read_array[0] >> 24) & 0x1);
    printf("ADC lock left         : %d\n", (read_array[0] >> 25) & 0x1);
    printf("ADC align right       : %d\n", (read_array[0] >> 26) & 0x1);
    printf("ADC align left        : %d\n", (read_array[0] >> 27) & 0x1);
    printf("check bits 15:12 (0)  : %d\n", (read_array[0] >> 28) & 0xf);
    printf("----------------------------\n");
  }

  //
  //    set mask1 bit 3 high
  //
  imod = imod_trig;
  buf_send[0] = (imod << 11) + (mb_trig_mask8) + (0x2 << 16); // set mask1[3] on.
  i = 1;
  k = 1;
  i = pcie_send(hDev, i, k, px);
  //
  //    set prescale1 to 0
  //
  imod = imod_trig;
  buf_send[0] = (imod << 11) + (mb_trig_prescale8) + (0x0 << 16); // set prescale1 0
  i = 1;
  k = 1;
  i = pcie_send(hDev, i, k, px);

  nword = (((64 * timesize * 3) / 2 + 5) * (imod_st - imod_xmit)) + 2; // total number of 32 bits word

  // }
  // ik = pmt_link_setup(hDev,imod_xmit,imod_xmit+1,0);      // no supernova

  ik = pmt_link_setup(hDev, imod_xmit, imod_xmit + 1, 0); // no supernova

  printf(" Enter number of events to read out and save:\n ");
  scanf("%d", &nevent);

  // turn the run on to start data flow
  printf("\nEnter 1 to set the RUN on \n");
  scanf("%d", &ik);

  /***
nword = 100;//this is the assumed event size
last_dma_loop_size = (nword*4)%dwDMABufSize;
ndma_loop = (nword*4)/dwDMABufSize;
  ***/
  // if(readtype==0){
  nword = (((64 * timesize * 3) / 2 + 5) * (imod_st - imod_xmit)) + 2; // total number of 32 bits word

  last_dma_loop_size = (nword * 4) % dwDMABufSize;
  ndma_loop = (nword * 4) / dwDMABufSize;
  //}
  // else{
  // ndma_loop = 1;
  // last_dma_loop_size = dwDMABufSize;
  // nword = (((1 * timesize * 3) / 2 + 5) * (nmod)) + 2; // total number of 32 bits word
  // ndma_loop = (nword *4) / dwDMABufSize;

  //}

  ifr = 0;
  printf(" DMA will run %d loop(s) per event\n", (ndma_loop + 1));
  printf("\t%d loops with %d words and %d loop with %d words\n\n", ndma_loop, dwDMABufSize / 4, 1, dwDMABufSize / 4);

  // variables for data checking
  /**
     nc = 0;
     iset = 0;
     ncount = 0;
     nwords = 0;
     ibin = 1;
     ic = 0;
     ih = 0;
     set = 0;
  **/

  /** begin timer **/
  struct timeval starttest3312, endtest3312;
  gettimeofday(&starttest3312, NULL);
  long mytime3312, seconds3312, useconds3312;
  seconds3312 = starttest3312.tv_sec;
  useconds3312 = starttest3312.tv_usec;
  printf("\n\nStart time of test: %ld sec %ld usec\n", seconds3312, useconds3312);
  // get a fresh timestamp
  gettimeofday(&starttest3312, NULL);

  // nevent = 100;
  idebug = 1;

  //******************************************************
  for (ijk = 0; ijk < nevent; ijk++)
  {
    if (ijk % 10 == 0)
      printf("\n===================> EVENT No. %i\n\n", ijk);
    ntot_rec = 0; // keeps track of number of words received in event
    for (iv = 0; iv < (ndma_loop + 1); iv++)
    {
      if (ifr == 0)
      { // first dma

        if (ntrans == 1)
        {
          if (idebug == 1)
            printf(" First DMA: buffer allocation 1\n");
          dwStatus = WDC_DMAContigBufLock(hDev2, &pbuf_rec, dwOptions_rec, dwDMABufSize, &pDma_rec);
          if (WD_STATUS_SUCCESS != dwStatus)
          {
            printf("Failed locking a rec 1 Contiguous DMA buffer. Error 0x%lx - %s\n", dwStatus, Stat2Str(dwStatus));
            printf("enter 1 to continue \n");
            scanf("%d", &is);
          }
          else
          {
            u32Data = pDma_rec->Page->pPhysicalAddr & 0xffffffff;
            if (idebug == 1)
              printf(" First DMA: buffer allocation lower address = %x\n", u32Data);
            u32Data = (pDma_rec->Page->pPhysicalAddr >> 32) & 0xffffffff;
            if (idebug == 1)
              printf(" First DMA: buffer allocation higher address = %x\n", u32Data);
          }
        }
        else if (ntrans == 2)
        {
          if (idebug == 1)
            printf(" First DMA: buffer allocation 1\n");
          dwStatus = WDC_DMAContigBufLock(hDev2, &pbuf_rec1, dwOptions_rec, dwDMABufSize, &pDma_rec1);
          if (WD_STATUS_SUCCESS != dwStatus)
          {
            printf("Failed locking a rec 1 Contiguous DMA buffer. Error 0x%lx - %s\n", dwStatus, Stat2Str(dwStatus));
            printf("enter 1 to continue \n");
            scanf("%d", &is);
          }
          else
          {
            u32Data = pDma_rec1->Page->pPhysicalAddr & 0xffffffff;
            if (idebug == 1)
              printf(" First DMA: buffer allocation lower address = %x\n", u32Data);
            u32Data = (pDma_rec1->Page->pPhysicalAddr >> 32) & 0xffffffff;
            if (idebug == 1)
              printf(" First DMA: buffer allocation higher address = %x\n", u32Data);
          }

          if (idebug == 1)
            printf(" First DMA: buffer allocation 2\n");
          dwStatus = WDC_DMAContigBufLock(hDev2, &pbuf_rec2, dwOptions_rec, dwDMABufSize, &pDma_rec2);
          if (WD_STATUS_SUCCESS != dwStatus)
          {
            printf("Failed locking a rec 2 Contiguous DMA buffer. Error 0x%lx - %s\n", dwStatus, Stat2Str(dwStatus));
            printf("enter 1 to continue \n");
            scanf("%d", &is);
          }
          else
          {
            u32Data = pDma_rec2->Page->pPhysicalAddr & 0xffffffff;
            if (idebug == 1)
              printf(" First DMA: buffer allocation 2 lower address = %x\n", u32Data);
            u32Data = (pDma_rec2->Page->pPhysicalAddr >> 32) & 0xffffffff;
            if (idebug == 1)
              printf(" First DMA: buffer allocation 2 higher address = %x\n", u32Data);
          }
        }
        /* set tx mode register */
        u32Data = 0x00002000;
        dwOffset = tx_md_reg;
        dwAddrSpace = cs_bar;
        WDC_WriteAddr32(hDev2, dwAddrSpace, dwOffset, u32Data);

        /* write this will abort previous DMA */
        dwAddrSpace = 2;
        dwOffset = cs_dma_msi_abort;
        u32Data = dma_abort;
        WDC_WriteAddr32(hDev2, dwAddrSpace, dwOffset, u32Data);

        /* clear DMA register after the abort */
        dwAddrSpace = 2;
        dwOffset = cs_dma_msi_abort;
        u32Data = 0;
        WDC_WriteAddr32(hDev2, dwAddrSpace, dwOffset, u32Data);
        if (idebug == 1)
          printf(" First DMA: initial abort finished \n");
      } // end if first dma

      if (ntrans == 1)
      {
        buffp_rec32 = (UINT32 *)pbuf_rec;
        WDC_DMASyncCpu(pDma_rec); // IS_709
      }
      else if (ntrans == 2)
      {

        if ((iv % 2) == 0)
          buffp_rec32 = (UINT32 *)pbuf_rec1;
        else
          buffp_rec32 = (UINT32 *)pbuf_rec2;

        if ((iv % 2) == 0)
          WDC_DMASyncCpu(pDma_rec1);
        else
          WDC_DMASyncCpu(pDma_rec2);
      }
      /* synch cache */
      // not needed in SL6?
      if (idebug == 1)
        printf(" synch CPU \n");

      if (iv != ndma_loop)
        nwrite_byte = dwDMABufSize;
      else
        nwrite_byte = last_dma_loop_size;

      if (idebug == 1)
        printf(" nwrite_byte is set to : %d\n", nwrite_byte);

      // if(ntrans==1){
      // for (is = 0; is < nwrite_byte/4; is++){  //IS_709
      //     buffp_rec32[is]=0;  //zero out buffer
      // }
      // }
      for (is = 1; is < 3; is++)
      {
        tr_bar = t1_tr_bar;
        r_cs_reg = r1_cs_reg;
        dma_tr = dma_tr1;
        if (is == 2)
        {
          tr_bar = t2_tr_bar;
          r_cs_reg = r2_cs_reg;
          dma_tr = dma_tr2;
        }
        if (idebug == 1)
          printf(" is = %d\n", is);
        /** initialize the receiver ***/
        u32Data = cs_init;
        dwOffset = r_cs_reg;
        dwAddrSpace = cs_bar;

        // receiver only gets initialize for the 1st time
        if (ifr == 0)
        {
          if (idebug == 1)
            printf(" First DMA: initialize the input fifo\n");
          WDC_WriteAddr32(hDev2, dwAddrSpace, dwOffset, u32Data);
        }

        /** start the receiver **/
        dwAddrSpace = cs_bar;
        u32Data = cs_start + nwrite_byte; /* 32 bits mode == 4 bytes per word *2 fibers **/
        if (idebug == 1)
          printf(" DMA loop %d with DMA data length %d \n", iv, nwrite_byte);
        dwOffset = r_cs_reg;
        WDC_WriteAddr32(hDev2, dwAddrSpace, dwOffset, u32Data);
      }

      ifr = 1;

      /** set up DMA for both transceiver together **/
      if (ntrans == 1)
      {
        dwAddrSpace = cs_bar;
        dwOffset = cs_dma_add_low_reg;
        u32Data = pDma_rec->Page->pPhysicalAddr & 0xffffffff;
        WDC_WriteAddr32(hDev2, dwAddrSpace, dwOffset, u32Data);

        dwAddrSpace = cs_bar;
        dwOffset = cs_dma_add_high_reg;
        u32Data = (pDma_rec->Page->pPhysicalAddr >> 32) & 0xffffffff;
        WDC_WriteAddr32(hDev2, dwAddrSpace, dwOffset, u32Data);
      }
      else if (ntrans == 2)
      {
        dwAddrSpace = cs_bar;
        dwOffset = cs_dma_add_low_reg;
        if ((iv % 2) == 0)
          u32Data = pDma_rec1->Page->pPhysicalAddr & 0xffffffff;
        else
          u32Data = pDma_rec2->Page->pPhysicalAddr & 0xffffffff;
        WDC_WriteAddr32(hDev2, dwAddrSpace, dwOffset, u32Data);

        dwAddrSpace = cs_bar;
        dwOffset = cs_dma_add_high_reg;
        if ((iv % 2) == 0)
          u32Data = (pDma_rec1->Page->pPhysicalAddr >> 32) & 0xffffffff;
        else
          u32Data = (pDma_rec2->Page->pPhysicalAddr >> 32) & 0xffffffff;
        WDC_WriteAddr32(hDev2, dwAddrSpace, dwOffset, u32Data);
      }

      /** set up DMA for both transceiver together **/
      /*dwAddrSpace = cs_bar;
      dwOffset = cs_dma_add_low_reg;
      //if ((iv % 2) == 0)u32Data = pDma_rec1->Page->pPhysicalAddr & 0xffffffff;
      //else u32Data = pDma_rec2->Page->pPhysicalAddr & 0xffffffff;
            u32Data = pDma_rec->Page->pPhysicalAddr & 0xffffffff;
      WDC_WriteAddr32(hDev2, dwAddrSpace, dwOffset, u32Data);

      dwAddrSpace = cs_bar;
      dwOffset = cs_dma_add_high_reg;
      //if ((iv % 2) == 0)u32Data = (pDma_rec1->Page->pPhysicalAddr >> 32) & 0xffffffff;
      //else u32Data = (pDma_rec2->Page->pPhysicalAddr >> 32) & 0xffffffff;
            u32Data = (pDma_rec->Page->pPhysicalAddr >> 32) & 0xffffffff;
      WDC_WriteAddr32(hDev2, dwAddrSpace, dwOffset, u32Data);
            */
      /* byte count */
      dwAddrSpace = cs_bar;
      dwOffset = cs_dma_by_cnt;
      u32Data = nwrite_byte;
      WDC_WriteAddr32(hDev2, dwAddrSpace, dwOffset, u32Data);

      if (ntrans == 1)
      {
        /* write this will start DMA */
        dwAddrSpace = 2;
        dwOffset = cs_dma_cntrl;
        is = (pDma_rec->Page->pPhysicalAddr >> 32) & 0xffffffff;
      }
      else if (ntrans == 2)
      {
        dwAddrSpace = 2;
        dwOffset = cs_dma_cntrl;
        if ((iv % 2) == 0)
          is = (pDma_rec1->Page->pPhysicalAddr >> 32) & 0xffffffff;
        else
          is = (pDma_rec2->Page->pPhysicalAddr >> 32) & 0xffffffff;
      }
      if (is == 0)
      {
        if (idebug == 1)
          printf(" use 3dw \n");
        u32Data = dma_tr12 + dma_3dw_rec;
      }
      else
      {
        u32Data = dma_tr12 + dma_4dw_rec;
        if (idebug == 1)
          printf(" use 4dw \n");
      }
      WDC_WriteAddr32(hDev2, dwAddrSpace, dwOffset, u32Data);
      if (idebug == 1)
        printf(" DMA set up done, byte count = %d\n", nwrite_byte);

      itrig_c = 0;
      itrig_ext = 1;

      // send trigger
      if (iv == 0 && ijk == 0)
      {
        if (itrig_c == 1)
        {
          imod = imod_trig; /* trigger module */
          buf_send[0] = (imod << 11) + mb_trig_pctrig + ((0x0) << 16);
          i = 1;
          k = 1;
          i = pcie_send(hDev, i, k, px);
          usleep(5000); // 3x frame size
        }
        else if (itrig_ext == 1)
        {
          //
          //     only need to restart the run if the we use the test data or 1st run
          //
          printf("Run turning on....\n");
          imod = imod_trig;
          buf_send[0] = (imod << 11) + (mb_trig_run) + ((0x1) << 16); // set up run
          i = 1;
          k = 1;
          i = pcie_send(hDev, i, k, px);
          usleep(5000);
        }
        else
        {
          imod = 0;
          ichip = 1;
          buf_send[0] = (imod << 11) + (ichip << 8) + mb_cntrl_set_trig1 + (0x0 << 16); // send trigger
          i = 1;
          k = 1;
          i = pcie_send(hDev, i, k, px);
          if (idebug == 1)
            printf(" trigger send  \n");
          usleep(5000);
        }
      }

      // extra wait just to be certain --- WK
      // usleep(20000);
      /***    check to see if DMA is done or not **/
      idone = 0;
      // iwritem=0;
      for (is = 0; is < 6000000; is++)
      {
        dwAddrSpace = cs_bar;
        u64Data = 0;
        dwOffset = cs_dma_cntrl;
        WDC_ReadAddr32(hDev2, dwAddrSpace, dwOffset, &u32Data);
        // if (idebug == 1)printf(" receive DMA status word %d %X \n", is, u32Data);
        if ((u32Data & dma_in_progress) == 0)
        {
          idone = 1;
          if (idebug == 1)
            printf(" receive DMA complete %d \n", is);
        }
        if ((u32Data & dma_in_progress) == 0)
          break;
      }
      if (idone == 0)
      {
        printf("\n loop %d, DMA is not finished \n", iv);
        dwAddrSpace = cs_bar;
        dwOffset = cs_dma_by_cnt;
        WDC_ReadAddr64(hDev2, dwAddrSpace, dwOffset, &u64Data);
        printf(" DMA word count = %x, %x \n", (u64Data >> 32), (u64Data & 0xffff));
        if (ntrans == 1)
          WDC_DMASyncIo(pDma_rec);
        else if (ntrans == 2)
        {
          if ((iv % 2) == 0)
            WDC_DMASyncIo(pDma_rec1);
          else
            WDC_DMASyncIo(pDma_rec2);
        }
        nred = (nwrite_byte - (u64Data & 0xffff)) / 4;
        printf(" number of 32-bit words received: %d (this DMA) + %d (total before this DMA)\n", nred, ntot_rec);
        iwrite = 1;
        irawprint = 0;
        if (iwrite == 1 || irawprint == 1)
        {
          for (is = 0; is < nred; is++)
          {
            read_array[is] = *buffp_rec32++;
            printf(" %8x", read_array[is]);
            if (irawprint == 1)
              read_array_compare[ntot_rec + is] = read_array[is];
            if (irawprint == 1 && ijk == 0)
              read_array_1st[ntot_rec + is] = read_array[is];
            if (iwrited == 1)
            {
              fprintf(outf, " %8x", read_array[is]);
              if ((((ntot_rec + is + 1) % 8) == 0))
                fprintf(outf, "\n");
            }
          }
          // n_write = write(fd,read_array,nwrite_byte);

          //	      scanf("%d",&ik);
        }
        ntot_rec = ntot_rec + nred;

        // iwritem=1;
        // iwrite=1;//1
        // dmasizewrite=1;//1

        dwAddrSpace = cs_bar;
        u64Data = 0;
        dwOffset = t1_cs_reg;
        WDC_ReadAddr64(hDev2, dwAddrSpace, dwOffset, &u64Data);
        printf(" Status word for channel 1 after read = %x, %x \n", (u64Data >> 32), (u64Data & 0xffff));
        dwAddrSpace = cs_bar;
        u64Data = 0;
        dwOffset = t2_cs_reg;
        WDC_ReadAddr64(hDev2, dwAddrSpace, dwOffset, &u64Data);
        printf(" status word for channel 2 after read = %x, %x \n", (u64Data >> 32), (u64Data & 0xffff));
        /* write this will abort previous DMA */
        dwAddrSpace = 2;
        dwOffset = cs_dma_msi_abort;
        u32Data = dma_abort;
        WDC_WriteAddr32(hDev2, dwAddrSpace, dwOffset, u32Data);

        /* clear DMA register after the abort */
        dwAddrSpace = 2;
        dwOffset = cs_dma_msi_abort;
        u32Data = 0;
        WDC_WriteAddr32(hDev2, dwAddrSpace, dwOffset, u32Data);
      }

      if (idone == 0)
        break;

      /* synch DMA i/O cache **/
      // SL6
      if (ntrans == 1)
        WDC_DMASyncIo(pDma_rec);
      else if (ntrans == 2)
      {
        if ((iv % 2) == 0)
          WDC_DMASyncIo(pDma_rec1);
        else
          WDC_DMASyncIo(pDma_rec2);
      }
      if (idebug == 1)
      {
        dwAddrSpace = cs_bar;
        u64Data = 0;
        dwOffset = t1_cs_reg;
        WDC_ReadAddr64(hDev2, dwAddrSpace, dwOffset, &u64Data);
        printf(" status word for Channel 1 after read = %x, %x \n", (u64Data >> 32), (u64Data & 0xffff));
        dwAddrSpace = cs_bar;
        u64Data = 0;
        dwOffset = t2_cs_reg;
        WDC_ReadAddr64(hDev2, dwAddrSpace, dwOffset, &u64Data);
        printf(" status word for channel 2 after read = %x, %x \n", (u64Data >> 32), (u64Data & 0xffff));
      }

      /*for (is=0; is < (nwrite_byte/4); is++) {

          if((is%8) ==0) printf("%4d", is);
          printf(" %8X", buffp_rec32[is]);
          if((is+1)%8 == 0) printf("\n");
      }*/

      nwrite = nwrite_byte / 4;
      if (iwrite == 1 || irawprint == 1)
      {
        for (is = 0; is < nwrite; is++)
        {
          read_array[is] = *buffp_rec32++;
          if (irawprint == 1)
            read_array_compare[ntot_rec + is] = read_array[is];
          if (irawprint == 1 && ijk == 0)
            read_array_1st[ntot_rec + is] = read_array[is];
          if (iwrited == 1)
          {

            // n_write = write(outf,read_array,nwrite_byte);
            fprintf(outf, " %8x", read_array[is]);
            if ((((ntot_rec + is + 1) % 8) == 0))
              fprintf(outf, "\n");
          }
        }
      }

      if (iwrite == 1 && dmasizewrite == 1)
      {

        //           for (is=0; is < (nwrite_byte/4); is++) {
        //
        //              if((is%8) ==0) printf("%4d", is);
        //              printf(" %8X", read_array[is]);
        //              if((is+1)%8 == 0) printf("\n");
        //          }

        n_write = write(fd, read_array, nwrite * 4);
      }

      if (iwritem == 1)
      {
        sprintf(name, "./xmit_trig_bin_grams_dma_%s_%i_%i.dat", subrun, ijk, iv);
        fd2 = creat(name, 0755);
        n_write = write(fd2, read_array, nwrite * 4);
        close(fd2);
      }

      ntot_rec = ntot_rec + nwrite;

      //        if (iwritem == 1)
      //          {
      //		    sprintf(name, "./xmit_trig_bin_grams_dma_%s_%i_%i.dat", subrun, ijk, iv);
      //		    fd = creat(name, 0755);
      // n_write = write(fd, read_array, nred * 4);
      //		    close(fd);
      //		  }
      //		else if (iwrite == 1 && dmasizewrite == 1)
      //		  {
      //		    n_write = write(fd, read_array, nred * 4);
      //		  }

    } // end dma loop

    /*	    nwrite = nwrite_byte / 4;
    if (iwrite == 1 || irawprint == 1)
      {
              printf("\n condition 1\n");
  for (is = 0; is < nwrite; is++)
    {
      read_array[is] = *buffp_rec32++;
      if (irawprint == 1)
        read_array_compare[ntot_rec + is] = read_array[is];
      if (irawprint == 1 && ijk == 0)
        read_array_1st[ntot_rec + is] = read_array[is];
      if (iwrited == 1)
        {
        fprintf(outf, " %8x", read_array[is]);
        if ((((ntot_rec + is + 1) % 8) == 0))
        fprintf(outf, "\n");
        }
    }
      }
    if (iwritem == 1)
      {
              printf("condition 2 \n");
  sprintf(name, "./data/pmt/xmit_trig_bin_grams_dma_%i_%i_%i.dat", tnum, ijk, iv);
  fd = creat(name, 0755);
              n_write = write(fd, read_array, nwrite * 4);
  close(fd);
      }
    else if (iwrite == 1 && dmasizewrite == 1)
      {
              printf("condition 3 \n");
  n_write = write(fd, read_array, nwrite * 4);
      }
    ntot_rec = ntot_rec + nwrite;
  } // end dma loop
      */
    // write to file: full event
    // if (iwrite == 1 && dmasizewrite == 1)
    //  {

    //          n_write = write(fd, read_array_compare, ntot_rec * 4);
    //	  }

  } // end loop over events

  gettimeofday(&endtest3312, NULL);
  seconds3312 = endtest3312.tv_sec;
  useconds3312 = endtest3312.tv_usec;
  printf("\nEnd time of test:   %ld sec %ld usec\n", seconds3312, useconds3312);
  seconds3312 = endtest3312.tv_sec - starttest3312.tv_sec;
  useconds3312 = endtest3312.tv_usec - starttest3312.tv_usec;
  mytime3312 = seconds3312 * 1000000. + useconds3312 * 1.; // elapsed time in usec
  printf("\nElapsed time:\t\t\t %ld usec\n\n", mytime3312);

  if (itrig_c == 1)
  {
    //******************************************************
    // stop run
    imod = imod_trig;
    buf_send[0] = (imod << 11) + (mb_trig_run) + ((0x0) << 16); // set up run off
    i = 1;
    k = 1;
    i = pcie_send(hDev, i, k, px);
  }
  else if ((itrig_ext == 1))
  {
    //
    //  set trigger module run off
    //
    imod = imod_trig;
    buf_send[0] = (imod << 11) + (mb_trig_run) + ((0x0) << 16); // set up run off
    i = 1;
    k = 1;
    i = pcie_send(hDev, i, k, px);
  }

  /**
     ifr=0;
     iv=0;
     iter=1;
     changemode(1);
     printf("\nStarting DMA loop. Enter q to quit run whenever\n");
     idebug=1;

     while (!kbhit()){

     iv++;
     if(ifr ==0) {//first dma
     if (idebug==1) printf(" buffer allocation 1\n");
     dwStatus = WDC_DMAContigBufLock(hDev2, &pbuf_rec, dwOptions_rec, dwDMABufSize, &pDma_rec);
     if (WD_STATUS_SUCCESS != dwStatus) {
     printf("Failed locking a rec Contiguous DMA buffer. Error 0x%lx - %s\n", dwStatus, Stat2Str(dwStatus));
     printf("enter 1 to continue \n");
     scanf("%d",&is);
     }
     else {
     u32Data = pDma_rec->Page->pPhysicalAddr & 0xffffffff;
     if (idebug==1) printf(" buffer allocation lower address = %x\n", u32Data);
     u32Data = (pDma_rec->Page->pPhysicalAddr >> 32) & 0xffffffff;
     if (idebug==1) printf(" buffer allocation higher address = %x\n", u32Data);
     }

     // set tx mode register
     u32Data = 0x00002000;
     dwOffset = tx_md_reg;
     dwAddrSpace =cs_bar;
     WDC_WriteAddr32(hDev2, dwAddrSpace, dwOffset, u32Data);

     // write this will abort previous DMA
     dwAddrSpace =2;
     dwOffset = cs_dma_msi_abort;
     u32Data = dma_abort;
     WDC_WriteAddr32(hDev2, dwAddrSpace, dwOffset, u32Data);
     // clear DMA register after the abort
     dwAddrSpace =2;
     dwOffset = cs_dma_msi_abort;
     u32Data = 0;
     WDC_WriteAddr32(hDev2, dwAddrSpace, dwOffset, u32Data);
     if (idebug==1) printf(" initial abort finished \n");

     }//end if first dma

     buffp_rec32 = (UINT32*)pbuf_rec;
     nwrite_byte = ibytec;
     for (is=0; is< nwrite_byte/4; is++) {
     buffp_rec32[is]=0; //zero out buffer
     }

     for (is=1; is<3; is++) {
     tr_bar = t1_tr_bar;
     r_cs_reg = r1_cs_reg;
     dma_tr = dma_tr1;
     if(is == 2) {
     tr_bar = t2_tr_bar;
     r_cs_reg = r2_cs_reg;
     dma_tr = dma_tr2;
     }
     // initialize the receiver
     u32Data = cs_init;
     dwOffset = r_cs_reg;
     dwAddrSpace =cs_bar;

     // receiver only gets initialize if this is the 1st dma
     if(ifr ==0) {
     WDC_WriteAddr32(hDev2, dwAddrSpace, dwOffset, u32Data);
     }
     // start the receiver
     dwAddrSpace = cs_bar;
     u32Data = cs_start+nwrite_byte;
     dwOffset = r_cs_reg;
     WDC_WriteAddr32(hDev2, dwAddrSpace, dwOffset, u32Data);
     }

     ifr=1;

     // set up DMA
     dwAddrSpace =cs_bar;
     dwOffset = cs_dma_add_low_reg;
     u32Data = pDma_rec->Page->pPhysicalAddr & 0xffffffff;
     WDC_WriteAddr32(hDev2, dwAddrSpace, dwOffset, u32Data);

     dwAddrSpace =cs_bar;
     dwOffset = cs_dma_add_high_reg;
     u32Data = (pDma_rec->Page->pPhysicalAddr >> 32) & 0xffffffff;
     WDC_WriteAddr32(hDev2, dwAddrSpace, dwOffset, u32Data);

     // byte count
     dwAddrSpace =cs_bar;
     dwOffset = cs_dma_by_cnt;
     u32Data = nwrite_byte;
     WDC_WriteAddr32(hDev2, dwAddrSpace, dwOffset, u32Data);


     //write this will start DMA
     dwAddrSpace =2;
     dwOffset = cs_dma_cntrl;
     is = (pDma_rec->Page->pPhysicalAddr >> 32) & 0xffffffff;
     if(is == 0) {
     u32Data = dma_tr12+dma_3dw_rec;
     }
     else {
     u32Data = dma_tr12+dma_4dw_rec;
     }
     WDC_WriteAddr32(hDev2, dwAddrSpace, dwOffset, u32Data);
     //     only need to restart the run if the we use the test data or 1st run
     if((iv == 1)) {
     //get the time
     // begin timer
     gettimeofday(&starttest18,NULL);
     seconds18 = starttest18.tv_sec;
     useconds18 = starttest18.tv_usec;

     printf("\n\nStart time of test: %ld sec %ld usec\n",seconds18,useconds18);
     //get a fresh timestamp
     gettimeofday(&starttest18,NULL);

     printf("RUN ON!\n");
     imod=imod_trig;
     buf_send[0]=(imod<<11)+(mb_trig_run)+((0x1)<<16); //set up run
     i=1;
     k=1;
     i = pcie_send(hDev, i, k, px);
     }//end if first loop

     printf(".....loop %d\n",iv);
     if (icont == 1) {//continuous dma without termination
     idone = 0;
     is=0;
     while(is<100000){ //run until dma is filled
     dwAddrSpace =cs_bar;
     u64Data =0;
     dwOffset = cs_dma_cntrl;
     WDC_ReadAddr32(hDev2, dwAddrSpace, dwOffset, &u32Data);
     if((u32Data & dma_in_progress) == 0) {
     idone =1;
     break;
     } //else sleep(10);
     is++;
     }

     if(idone ==0) printf("DMA is not done \n");//this shouldn't happen for while(1)
     for (is=0; is < (nwrite_byte/4); is++) {
     if(!(outf)){
     if((is%8) ==0) printf("%4d",is);
     printf("  %8X", buffp_rec32[is]);
     if((is+1)%8 ==0) printf("\n");
     }
     }
     if(idone==1) {
     if (outf){
     //disk write
     for (is=0; is<(nwrite_byte/4); is++) {
     read_array[is]= *buffp_rec32++;
     }
     if (writedebug==1){
     start = time(NULL);
     n_write = write(fd,read_array,nwrite_byte);
     end = time(NULL);
     if (iv%100==0){//update file every 100 dma's
     iter++;
     close(fd);
     sprintf(_buf,"/home/uboonedaq/pmt_data_temp/xmit_subrun_%s_%s_dma_no_%d.bin",timestr,subrun,iter);
     fd = creat(_buf,0755);
     }
     }
     }
     }

     }//end if icont
     else
     {
     printf("\nYou can't run this routine with icont==0!!!\n");
     }
     }
     ch = getchar();
     printf("Quit DMA\n");
     printf("iv=%d\n",iv);
     changemode(0);
     printf("\nRun complete.\n");

     gettimeofday(&endtest18,NULL);
     seconds18 = endtest18.tv_sec;
     useconds18 = endtest18.tv_usec;

     printf("\nEnd time of test:   %ld sec %ld usec\n",seconds18,useconds18);

     seconds18 = endtest18.tv_sec - starttest18.tv_sec;
     useconds18 = endtest18.tv_usec - starttest18.tv_usec;

     mytime18 = seconds18*1. + useconds18*0.000001; //elapsed time in sec

     printf("\nElapsed time:\t\t\t %ld sec\n\n", mytime18);
     fprintf(outinfo,"Run time = %ld sec\n\n", mytime18);
     printf("Run data in %s set of files\n\n",_buf);

  **/

  idebug = 0;

  // check FEM status again
  // readback status
  nword = 1;
  i = pcie_rec(hDev, 0, 1, nword, iprint, py); // init the receiver

  imod_fem = 15;
  imod = imod_fem;
  ichip = 3;
  buf_send[0] = (imod << 11) + (ichip << 8) + 20 + (0x0 << 16); // read out status
  i = 1;
  k = 1;
  i = pcie_send(hDev, i, k, px);
  py = &read_array[0];
  i = pcie_rec(hDev, 0, 2, nword, iprint, py); // read out 2 32 bits words
  printf("\nFEM STATUS -- after run = %x, %x \n", read_array[0], read_array[1]);
  printf(" module = %d, command = %d \n", ((read_array[0] >> 11) & 0x1f), (read_array[0] & 0xff));
  printf(" ADC right dpa lock     %d \n", ((read_array[0] >> 17) & 0x1));
  printf(" ADC left  dpa lock     %d \n", ((read_array[0] >> 18) & 0x1));
  printf(" block error 2          %d \n", ((read_array[0] >> 19) & 0x1));
  printf(" block error 1          %d \n", ((read_array[0] >> 20) & 0x1));
  printf(" pll locked             %d \n", ((read_array[0] >> 21) & 0x1));
  printf(" supernova mem ready    %d \n", ((read_array[0] >> 22) & 0x1));
  printf(" beam      mem ready    %d \n", ((read_array[0] >> 23) & 0x1));
  printf(" ADC right PLL locked   %d \n", ((read_array[0] >> 24) & 0x1));
  printf(" ADC left  PLL locked   %d \n", ((read_array[0] >> 25) & 0x1));
  printf(" ADC align cmd right    %d \n", ((read_array[0] >> 26) & 0x1));
  printf(" ADC align cmd left     %d \n", ((read_array[0] >> 27) & 0x1));
  printf(" ADC align done right   %d \n", ((read_array[0] >> 28) & 0x1));
  printf(" ADC align done left    %d \n", ((read_array[0] >> 29) & 0x1));
  printf(" Neutrino data empty    %d \n", ((read_array[0] >> 30) & 0x1));
  printf(" Neutrino header empty  %d \n", ((read_array[0] >> 31) & 0x1));

  if (outf)
    fclose(outf);
  if (outinfo)
    fclose(outinfo);
  close(fd);
}

/* Enable/Disable interrupts menu */
static void MenuInterrupts(WDC_DEVICE_HANDLE hDev)
{
  DWORD option, dwIntOptions;
  BOOL fIntEnable, fIsMsi;

  dwIntOptions = WDC_GET_INT_OPTIONS(hDev);
  fIsMsi = WDC_INT_IS_MSI(dwIntOptions);
  if (dwIntOptions & INTERRUPT_LEVEL_SENSITIVE)
  {
    /* TODO: You can remove this message after you have modified the
       implementation of PCIE_IntEnable() in pcie_lib.c to
       correctly acknowledge level-sensitive interrupts (see guidelines
       in PCIE_IntEnable()) */
    printf("\n");
    printf("WARNING!!!\n");
    printf("----------\n");
    printf("Your hardware has level sensitive interrupts.\n");
    printf("Before enabling the interrupts, %s first modify the source "
           "code of PCIE_IntEnable(), in the file pcie_lib.c, to "
           "correctly acknowledge\n%s interrupts when they occur (as dictated by "
           "the hardware's specifications)\n",
           fIsMsi ? "it's recommended that you" : "you must",
           fIsMsi ? "level sensitive" : "");
  }

  do
  {
    fIntEnable = !PCIE_IntIsEnabled(hDev);

    printf("\n");
    printf("Interrupts\n");
    printf("-----------\n");
    printf("%d. %s interrupts\n", MENU_INT_ENABLE_DISABLE,
           fIntEnable ? "Enable" : "Disable");
    printf("%d. Exit menu\n", MENU_INT_EXIT);
    printf("\n");

    if (DIAG_INPUT_FAIL == DIAG_GetMenuOption(&option,
                                              MENU_RW_ADDR_WRITE))
    {
      continue;
    }

    switch (option)
    {
    case MENU_INT_EXIT: /* Exit menu */
      break;
    case MENU_INT_ENABLE_DISABLE: /* Enable/disable interrupts */
      if (fIntEnable)
      {
        DWORD dwStatus = PCIE_IntEnable(hDev, DiagIntHandler);

        if (WD_STATUS_SUCCESS == dwStatus)
          printf("Interrupts enabled\n");
        else
        {
          PCIE_ERR("Failed enabling interrupts. Error 0x%lx - %s\n",
                   dwStatus, Stat2Str(dwStatus));
        }
      }
      else
      {
        if (WD_STATUS_SUCCESS == PCIE_IntDisable(hDev))
          printf("Interrupts disabled\n");
        else
          PCIE_ERR("Failed disabling interrupts: %s", PCIE_GetLastErr());
      }
      break;
    }
  } while (MENU_INT_EXIT != option);
}

/* Register/unregister Plug-and-play and power management events */
static void MenuEvents(WDC_DEVICE_HANDLE hDev)
{
  DWORD option;
  BOOL fRegister;

  do
  {
    fRegister = !PCIE_EventIsRegistered(hDev);

    printf("\n");
    printf("Plug-and-play and power management events\n");
    printf("------------------------------------------\n");
    printf("%d. %s events\n", MENU_EVENTS_REGISTER_UNREGISTER,
           fRegister ? "Register" : "Unregister");
    printf("%d. Exit menu\n", MENU_EVENTS_EXIT);
    printf("\n");

    if (DIAG_INPUT_FAIL == DIAG_GetMenuOption(&option,
                                              MENU_EVENTS_REGISTER_UNREGISTER))
    {
      continue;
    }

    switch (option)
    {
    case MENU_EVENTS_EXIT: /* Exit menu */
      break;
    case MENU_EVENTS_REGISTER_UNREGISTER: /* Register/unregister events */
      if (fRegister)
      {
        if (WD_STATUS_SUCCESS == PCIE_EventRegister(hDev, DiagEventHandler))
          printf("Events registered\n");
        else
          PCIE_ERR("Failed to register events. Last error:\n%s", PCIE_GetLastErr());
      }
      else
      {
        if (WD_STATUS_SUCCESS == PCIE_EventUnregister(hDev))
          printf("Events unregistered\n");
        else
          PCIE_ERR("Failed to unregister events. Last Error:\n%s", PCIE_GetLastErr());
      }
      break;
    }
  } while (MENU_EVENTS_EXIT != option);
}

/* Display read/write run-time registers menu */
static void MenuReadWriteRegs(WDC_DEVICE_HANDLE hDev)
{
  DWORD option;

  if (!PCIE_REGS_NUM)
  {
    printf("There are currently no pre-defined run-time registers\n");
    return;
  }

  do
  {
    /* Display pre-defined registers' information */
    printf("\n");
    printf("PCIE run-time registers:\n");
    printf("--------------------------\n");
    WDC_DIAG_RegsInfoPrint(gpPCIE_Regs, PCIE_REGS_NUM, WDC_DIAG_REG_PRINT_ALL);

    printf("\n");
    printf("Read/write the PCIE run-time registers\n");
    printf("-----------------------------------------\n");
    printf("%d. Read all run-time registers defined for the device (see list above)\n",
           MENU_RW_REGS_READ_ALL);
    printf("%d. Read from a specific register\n", MENU_RW_REGS_READ_REG);
    printf("%d. Write to a specific register\n", MENU_RW_REGS_WRITE_REG);
    printf("%d. Exit menu\n", MENU_RW_REGS_EXIT);
    printf("\n");

    if (DIAG_INPUT_FAIL == DIAG_GetMenuOption(&option,
                                              MENU_RW_REGS_WRITE_REG))
    {
      continue;
    }

    switch (option)
    {
    case MENU_RW_REGS_EXIT: /* Exit menu */
      break;
    case MENU_RW_REGS_READ_ALL:
      WDC_DIAG_ReadRegsAll(hDev, gpPCIE_Regs, PCIE_REGS_NUM, FALSE);
      break;
    case MENU_RW_REGS_READ_REG: /* Read from a register */
      WDC_DIAG_ReadWriteReg(hDev, gpPCIE_Regs, PCIE_REGS_NUM, WDC_READ, FALSE);
      break;
    case MENU_RW_REGS_WRITE_REG: /* Write to a register */
      WDC_DIAG_ReadWriteReg(hDev, gpPCIE_Regs, PCIE_REGS_NUM, WDC_WRITE, FALSE);
      break;
    }
  } while (MENU_RW_REGS_EXIT != option);
}
/* Display read/write configuration space menu */
static void MenuReadWriteCfgSpace(WDC_DEVICE_HANDLE hDev)
{
  DWORD option;

  do
  {
    /* Display pre-defined registers' information */
    if (PCIE_CFG_REGS_NUM)
    {
      printf("\n");
      printf("Configuration registers:\n");
      printf("------------------------\n");
      WDC_DIAG_RegsInfoPrint(gpPCIE_CfgRegs, PCIE_CFG_REGS_NUM,
                             WDC_DIAG_REG_PRINT_ALL & ~WDC_DIAG_REG_PRINT_ADDR_SPACE);
    }

    printf("\n");
    printf("Read/write the device's configuration space\n");
    printf("--------------------------------------------\n");
    printf("%d. Read from an offset\n", MENU_RW_CFG_SPACE_READ_OFFSET);
    printf("%d. Write to an offset\n", MENU_RW_CFG_SPACE_WRITE_OFFSET);
    if (PCIE_CFG_REGS_NUM)
    {
      printf("%d. Read all configuration registers defined for the device (see list above)\n",
             MENU_RW_CFG_SPACE_READ_ALL_REGS);
      printf("%d. Read from a named register\n", MENU_RW_CFG_SPACE_READ_REG);
      printf("%d. Write to a named register\n", MENU_RW_CFG_SPACE_WRITE_REG);
    }
    printf("%d. Exit menu\n", MENU_RW_CFG_SPACE_EXIT);
    printf("\n");

    if (DIAG_INPUT_FAIL == DIAG_GetMenuOption(&option,
                                              PCIE_CFG_REGS_NUM ? MENU_RW_CFG_SPACE_WRITE_REG : MENU_RW_CFG_SPACE_WRITE_OFFSET))
    {
      continue;
    }

    switch (option)
    {
    case MENU_RW_CFG_SPACE_EXIT: /* Exit menu */
      break;
    case MENU_RW_CFG_SPACE_READ_OFFSET: /* Read from a configuration space offset */
      WDC_DIAG_ReadWriteBlock(hDev, WDC_READ, WDC_AD_CFG_SPACE);
      break;
    case MENU_RW_CFG_SPACE_WRITE_OFFSET: /* Write to a configuration space offset */
      WDC_DIAG_ReadWriteBlock(hDev, WDC_WRITE, WDC_AD_CFG_SPACE);
      break;
    case MENU_RW_CFG_SPACE_READ_ALL_REGS:
      WDC_DIAG_ReadRegsAll(hDev, gpPCIE_CfgRegs, PCIE_CFG_REGS_NUM, TRUE);
      break;
    case MENU_RW_CFG_SPACE_READ_REG: /* Read from a configuration register */
      WDC_DIAG_ReadWriteReg(hDev, gpPCIE_CfgRegs, PCIE_CFG_REGS_NUM, WDC_READ, TRUE);
      break;
    case MENU_RW_CFG_SPACE_WRITE_REG: /* Write to a configuration register */
      WDC_DIAG_ReadWriteReg(hDev, gpPCIE_CfgRegs, PCIE_CFG_REGS_NUM, WDC_WRITE, TRUE);
      break;
    }
  } while (MENU_RW_CFG_SPACE_EXIT != option);
}
/* Read/write memory or I/O space address menu */
static void MenuReadWriteAddr(WDC_DEVICE_HANDLE hDev)
{
  DWORD option;
  static DWORD dwAddrSpace = ACTIVE_ADDR_SPACE_NEEDS_INIT;
  static WDC_ADDR_MODE mode = WDC_MODE_32;
  static BOOL fBlock = FALSE;

  /* Initialize active address space */
  if (ACTIVE_ADDR_SPACE_NEEDS_INIT == dwAddrSpace)
  {
    DWORD dwNumAddrSpaces = PCIE_GetNumAddrSpaces(hDev);

    /* Find the first active address space */
    for (dwAddrSpace = 0; dwAddrSpace < dwNumAddrSpaces; dwAddrSpace++)
    {
      if (WDC_AddrSpaceIsActive(hDev, dwAddrSpace))
        break;
    }

    /* Sanity check */
    if (dwAddrSpace == dwNumAddrSpaces)
    {
      PCIE_ERR("MenuReadWriteAddr: Error - no active address spaces found\n");
      dwAddrSpace = ACTIVE_ADDR_SPACE_NEEDS_INIT;
      return;
    }
  }

  do
  {
    printf("\n");
    printf("Read/write the device's memory and IO ranges\n");
    printf("---------------------------------------------\n");
    printf("%d. Change active address space for read/write "
           "(currently: BAR %ld)\n",
           MENU_RW_ADDR_SET_ADDR_SPACE, dwAddrSpace);
    printf("%d. Change active read/write mode (currently: %s)\n",
           MENU_RW_ADDR_SET_MODE,
           (WDC_MODE_8 == mode) ? "8 bit" : (WDC_MODE_16 == mode) ? "16 bit"
                                        : (WDC_MODE_32 == mode)   ? "32 bit"
                                                                  : "64 bit");
    printf("%d. Toggle active transfer type (currently: %s)\n",
           MENU_RW_ADDR_SET_TRANS_TYPE,
           (fBlock ? "block transfers" : "non-block transfers"));
    printf("%d. Read from active address space\n", MENU_RW_ADDR_READ);
    printf("%d. Write to active address space\n", MENU_RW_ADDR_WRITE);
    printf("%d. Exit menu\n", MENU_RW_ADDR_EXIT);
    printf("\n");

    if (DIAG_INPUT_FAIL == DIAG_GetMenuOption(&option,
                                              MENU_RW_ADDR_WRITE))
    {
      continue;
    }

    switch (option)
    {
    case MENU_RW_ADDR_EXIT: /* Exit menu */
      break;
    case MENU_RW_ADDR_SET_ADDR_SPACE: /* Set active address space for read/write address requests */
    {
      SetAddrSpace(hDev, &dwAddrSpace);
      break;
    }
    case MENU_RW_ADDR_SET_MODE: /* Set active mode for read/write address requests */
      WDC_DIAG_SetMode(&mode);
      break;
    case MENU_RW_ADDR_SET_TRANS_TYPE: /* Toggle active transfer type */
      fBlock = !fBlock;
      break;
    case MENU_RW_ADDR_READ:  /* Read from a memory or I/O address */
    case MENU_RW_ADDR_WRITE: /* Write to a memory or I/O address */
    {
      WDC_DIRECTION direction =
          (MENU_RW_ADDR_READ == option) ? WDC_READ : WDC_WRITE;

      if (fBlock)
        WDC_DIAG_ReadWriteBlock(hDev, direction, dwAddrSpace);
      else
        WDC_DIAG_ReadWriteAddr(hDev, direction, dwAddrSpace, mode);

      break;
    }
    }
  } while (MENU_RW_ADDR_EXIT != option);
}
