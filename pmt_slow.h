#ifndef __PMT_H__
#define __PMT_H__


///////////////////////////////////FUNCTION DECLARE/////////////////////////////////////////////////////////

static int pmt_adc_setup(WDC_DEVICE_HANDLE hDev, int imod_fem, int iframe, int mode, FILE *outinfo);
static int pmt_link_setup(WDC_DEVICE_HANDLE hDev, int imod_xmit, int imod_fem, int supernova);

//////////////////////TPC functions///////////////////
static int tpc_adc_setup(WDC_DEVICE_HANDLE *hDev, WDC_DEVICE_HANDLE *hDev2, int imod_fem, int imod_xmit, int iframe_length, int imod_trig, int timesize);

///////////////////////////////////FUNCTION DEFINE/////////////////////////////////////////////////////////

/*Function which configures PMT ADC+FEM Board*/
static int pmt_adc_setup(WDC_DEVICE_HANDLE hDev, int imod_fem, int iframe, int mode, FILE *outinfo)
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

  static UINT32 u32Data;
  static unsigned short u16Data;
  static long imod, ichip;
  unsigned short *buffp;

  /*
    WDC_ADDR_MODE mode;
    WDC_ADDR_RW_OPTIONS options;
  */
  static UINT32 i, j, k, ifr, nread, iprint, iwrite, ik, il, is, checksum;
  static UINT32 send_array[40000], read_array[1000], read_array_compare[40000000];
  static UINT32 nmask, index, itmp, nword_tot, nevent, iv, ijk, islow_read;
  static UINT32 imod_p, imod_trig, imod_shaper;
  static int icomp_l, comp_s, ia, ic;
  DWORD dwStatus;
  DWORD dwOptions = DMA_FROM_DEVICE;
  UINT32 iread, icheck, izero;
  UINT32 buf_send[40000];
  static int count, num, counta, nword, ireadback, nloop, ierror;
  static int ij, nsend, iloop, inew, idma_readback, iadd, jevent;
  static int itest, irun, ichip_c, dummy1, itrig_c;
  static int idup, ihold, idouble, ihold_set, istatus_read;
  static int idone, tr_bar, t_cs_reg, r_cs_reg, dma_tr;
  static int timesize, ipulse, ibase, a_id, itrig_delay;
  static int iset, ncount, nsend_f, nwrite, itrig_ext;
  static int imod_xmit, idiv, isample;
  static int iframe_length, itrig, idrift_time, ijtrig;
  static int idelay0, idelay1, threshold0, threshold1hg, threshold1lg, pmt_words, pmt_wordshg, pmt_wordslg;
  static int threshold3hg, threshold3lg;
  static int threshold1, threshold3;
  static int cos_mult, cos_thres, beam_mult, beam_thres, en_top, en_upper, en_lower, hg, lg, lg_ch, hg_ch, trig_ch, bg_ch;
  static int irise, ifall, istart_time, use_pmt, pmt_testpulse;
  static int ich_head, ich_sample, ich_frm, idebug, ntot_rec, nred;
  static int ineu, ibusy_send, ibusy_test, ihold_word, ndma_loop;
  static int irawprint, ifem_fst, ifem_lst, ifem_loop;
  static int pmt_deadtimehg, pmt_deadtimelg, pmt_mich_window, bg, bge, tre, beam_size, pmt_width;
  static int pmt_deadtime;
  static int oframe, osample, odiv, cframe, csample, cdiv;
  static int idac_shaper, pmt_dac_scan, pmt_precount;
  unsigned char charchannel;
  unsigned char carray[4000];
  struct timespec tim, tim2;
  tim.tv_sec = 0;
  tim.tv_nsec = 128000;

  PVOID pbuf_rec;
  WD_DMA *pDma_rec;
  DWORD dwOptions_send = DMA_TO_DEVICE | DMA_ALLOW_CACHE;
  DWORD dwOptions_rec = DMA_FROM_DEVICE | DMA_ALLOW_64BIT_ADDRESS;

  UINT32 *px, *py, *py1;
  FILE *outf, *inpf;

  printf("\nIs there a BNB gate physically input to the FEM?\t"); //
  //  scanf("%d",&bg);
  bg = 0;
  fprintf(outinfo, "Hardware config: Gate input = %d\n", bg);
  if (bg == 0 && (mode == 4 || mode == 5 || mode == 7))
  {
    printf("\nWarning: The PMT Beam Trigger is disabled because the BNB gate is not physically input to the FEM\n");
    printf("Enter 1 to acknowledge this:\t");
    scanf("%d", &ik); // enter anything; this will be overwritten next
  }
  // do not change! see docdb-2819
  hg_ch = 7;
  lg_ch = 3;
  bg_ch = 2;
  trig_ch = 0;
  printf("\nThis is the assumed (default) channel input config:\n");
  printf("\tLG_ch = %d, HG_ch = %d, TRIG_ch = %d, BG_ch = %d\n", lg_ch, hg_ch, trig_ch, bg_ch);
  printf("Refer to docdb-2819 for more details.\n");
  fprintf(outinfo, "Hardware config: LG_ch = %d, HG_ch = %d, TRIG_ch = %d, BG_ch = %d\n", lg_ch, hg_ch, trig_ch, bg_ch);
  printf("\n\nEnter 1 to enable discriminators for each channel:\n");
  printf("\n\tEnable high gain channel?\t");
  //  scanf("%d",&hg);
  hg = 0;
  printf("\tEnable low gain channel?\t");
  //  scanf("%d",&lg);
  lg = 0;
  if (bg == 1)
  {
    printf("\n\tEnable BNB gate channel (for readout)?\t");
    scanf("%d", &bge);
  }
  printf("\n\tEnable EXT trigger (LED or COSMIC Paddles) channel (for readout)?\t");
  //  scanf("%d",&tre);
  tre = 1;
  pmt_mich_window = 0; // disable by hand
  threshold0 = 10;     // discriminator 0 threshold
  printf("\nReadout parameter definitions:\n");
  printf("Discriminator thresholds assume 20 ADC/pe for HG, 2 ADC/pe for LG.\n");
  printf("\nThreshold for discr 0 = %d ADC counts\n", threshold0);
  if (hg == 1)
  {
    printf("\nEnter threshold for discr 3 (ADC) for HG (default=20):\t");
    scanf("%d", &threshold3hg);
    printf("\nEnter threshold for discr 1 (ADC) for HG (default=4):\t");
    scanf("%d", &threshold1hg);
    printf("Enter number of readout words for HG (default=20):\t");
    scanf("%d", &pmt_wordshg);
    printf("Enter PMT deadtime (64MHz samples) for HG (default=256; minimum recommended is %d):\t", pmt_wordshg + 4);
    scanf("%d", &pmt_deadtimehg);
  }
  if (lg == 1)
  {
    printf("\nEnter threshold for discr 3 (ADC) for LG (default=2):\t");
    scanf("%d", &threshold3lg);
    printf("\nEnter threshold for discr 1 (ADC) for LG (default=8):\t");
    scanf("%d", &threshold1lg);
    printf("Enter number of readout words for LG (default=20):\t");
    scanf("%d", &pmt_wordslg);
    printf("\nEnter PMT deadtime (64MHz samples) for LG (default=24; minimum recommended is %d):\t", pmt_wordslg + 4);
    scanf("%d", &pmt_deadtimelg);
  }
  // for all other channels
  pmt_words = 20;
  pmt_deadtime = 240;
  threshold1 = 40;
  threshold3 = 4095;

  if ((mode == 3) || (mode == 5) || (mode == 7))
  {
    printf("\nTriggering on PMT Cosmic Trigger:");
    printf("\nEnter cosmic trigger threshold (ADC); default is 800 (HG):\t");
    scanf("%d", &cos_thres);
    printf("Enter cosmic multiplicity; default is 1:\t");
    scanf("%d", &cos_mult);
  }
  else
  { // doesn't matter
    cos_thres = 100;
    cos_mult = 100;
  }
  if ((mode == 4) || (mode == 5) || (mode == 7))
  {
    printf("\nTriggering on PMT Beam Trigger:");
    printf("\nEnter beam trigger threshold (ADC); default is 40 (HG):\t");
    scanf("%d", &beam_thres);
    printf("Enter beam multiplicity; default is 1:\t");
    scanf("%d", &beam_mult);
  }
  else
  { // doesn't matter
    beam_thres = 100;
    beam_mult = 100;
  }
  pmt_precount = 2; // default
  pmt_width = 5;    // default
  fprintf(outinfo, "RUN MODE = %d\n", mode);
  fprintf(outinfo, "pmt_deadtime HG,LG = %d,%d\n", pmt_deadtimehg, pmt_deadtimelg);
  fprintf(outinfo, "pmt_deadtime BG,TRIG channels = %d\n", pmt_deadtime);
  fprintf(outinfo, "pmt_width = %d\n", pmt_width);
  fprintf(outinfo, "pmt_mich_window = %d\n", pmt_mich_window);
  fprintf(outinfo, "threshold0 = %d\n", threshold0);
  fprintf(outinfo, "threshold3 HG,LG = %d,%d\n", threshold3hg, threshold3lg);
  fprintf(outinfo, "threshold1 HG,LG = %d,%d\n", threshold1hg, threshold1lg);
  fprintf(outinfo, "threshold3 BG,TRIG channels = %d\n", threshold3);
  fprintf(outinfo, "threshold1 BG,TRIG channels = %d\n", threshold1);
  fprintf(outinfo, "cos_mult = %d\n", cos_mult);
  fprintf(outinfo, "cos_thres = %d\n", cos_thres);
  fprintf(outinfo, "beam_mult = %d\n", beam_mult);
  fprintf(outinfo, "beam_thres = %d\n", beam_thres);
  fprintf(outinfo, "pmt_precount = %d\n", pmt_precount);
  fprintf(outinfo, "pmt_words HG,LG = %d,%d\n", pmt_wordshg, pmt_wordslg);
  fprintf(outinfo, "pmt_words BG,TRIG channels = %d\n", pmt_words);
  // turn on the Stratix III power supply
  px = &buf_send[0];
  py = &read_array[0];

  imod = imod_fem;
  ichip = 1;
  buf_send[0] = (imod << 11) + (ichip << 8) + mb_feb_power_add + (0x0 << 16); // turn module 11 power on
  i = 1;
  k = 1;
  i = pcie_send(hDev, i, k, px);
  usleep(200000); // wait for 200 ms
  //  inpf = fopen("/home/uboonedaq/pmt_fpga_code/module1x_pmt_64MHz_new_head_07162013.rbf","r");
  inpf = fopen("/home/ub/WinDriver/wizard/GRAMS_project/uBooNE_Nominal_Firmware/pmt_fem/module1x_pmt_64MHz_new_head_07162013.rbf", "r");
  //  inpf = fopen("/home/ub/module1x_pmt_64MHz_new_head_12192013_allch.rbf","r");
  // inpf = fopen("/home/ub/WinDriver/wizard/GRAMS_project_am/uBooNE_Nominal_Firmware/xmit/readcontrol_110601_v3_play_header_hist_l1block_9_21_2018.rbf", "r");
  //  fprintf(outinfo,"PMT FEM FPGA: /home/uboonedaq/pmt_fpga_code/module1x_pmt_64MHz_new_head_07162013.rbf\n");
  fprintf(outinfo, "PMT FEM FPGA: /home/ub/WinDriver/wizard/GRAMS_project/uBooNE_Nominal_Firmware/pmt_fem/module1x_pmt_64MHz_new_head_07162013.rbf");
  //  fprintf(outinfo,"PMT FEM FPGA: /home/ub/module1x_pmt_64MHz_new_head_12192013_allch.rbf\n ");

  imod = imod_fem;
  ichip = mb_feb_conf_add;
  buf_send[0] = (imod << 11) + (ichip << 8) + 0x0 + (0x0 << 16); // turn conf to be on
  i = 1;
  k = 1;
  i = pcie_send(hDev, i, k, px);

  printf("\nLoading FEM FPGA...\n");
  usleep(1000); // wait for a while
  nsend = 500;
  count = 0;
  counta = 0;
  ichip_c = 7; // set ichip_c to stay away from any other command in the
  dummy1 = 0;
  while (fread(&charchannel, sizeof(char), 1, inpf) == 1)
  {
    carray[count] = charchannel;
    count++;
    counta++;
    if ((count % (nsend * 2)) == 0)
    {
      buf_send[0] = (imod << 11) + (ichip_c << 8) + (carray[0] << 16);
      send_array[0] = buf_send[0];
      if (dummy1 <= 5)
        printf(" counta = %d, first word = %x, %x, %x %x %x \n", counta, buf_send[0], carray[0], carray[1], carray[2], carray[3]);
      for (ij = 0; ij < nsend; ij++)
      {
        if (ij == (nsend - 1))
          buf_send[ij + 1] = carray[2 * ij + 1] + (0x0 << 16);
        else
          buf_send[ij + 1] = carray[2 * ij + 1] + (carray[2 * ij + 2] << 16);
        send_array[ij + 1] = buf_send[ij + 1];
      }
      nword = nsend + 1;
      i = 1;
      ij = pcie_send(hDev, i, nword, px);
      nanosleep(&tim, &tim2);
      dummy1 = dummy1 + 1;
      count = 0;
    }
  }
  if (feof(inpf))
  {
    printf("\tend-of-file word count= %d %d\n", counta, count);
    buf_send[0] = (imod << 11) + (ichip_c << 8) + (carray[0] << 16);
    if (count > 1)
    {
      if (((count - 1) % 2) == 0)
      {
        ik = (count - 1) / 2;
      }
      else
      {
        ik = (count - 1) / 2 + 1;
      }
      ik = ik + 2; // add one more for safety
      printf("\tik= %d\n", ik);
      for (ij = 0; ij < ik; ij++)
      {
        if (ij == (ik - 1))
          buf_send[ij + 1] = carray[(2 * ij) + 1] + (((imod << 11) + (ichip << 8) + 0x0) << 16);
        else
          buf_send[ij + 1] = carray[(2 * ij) + 1] + (carray[(2 * ij) + 2] << 16);
        send_array[ij + 1] = buf_send[ij + 1];
      }
    }
    else
      ik = 1;

    for (ij = ik - 10; ij < ik + 1; ij++)
    {
      printf("\tlast data = %d, %x\n", ij, buf_send[ij]);
    }

    nword = ik + 1;
    i = 1;
    i = pcie_send(hDev, i, nword, px);
  }
  usleep(2000); // wait for 2ms to cover the packet time plus fpga init time
  fclose(inpf);

  printf("FEM FPGA configuration done\n");
  ik = 1;
  if (ik == 1)
  {
    imod = imod_fem;
    ichip = 3;
    buf_send[0] = (imod << 11) + (ichip << 8) + 31 + (0x1 << 16); // turm the DRAM reset on
    i = 1;
    k = 1;
    i = pcie_send(hDev, i, k, px);
    imod = imod_fem;
    ichip = 3;
    buf_send[0] = (imod << 11) + (ichip << 8) + 31 + (0x0 << 16); // turm the DRAM reset off
    i = 1;
    k = 1;
    i = pcie_send(hDev, i, k, px);

    usleep(5000); // wait for 5 ms for DRAM to be initialized
    imod = imod_fem;
    ichip = 3;
    buf_send[0] = (imod << 11) + (ichip << 8) + 6 + (imod << 16); // set module number
    i = 1;
    k = 1;
    i = pcie_send(hDev, i, k, px);
  }

  nword = 1;
  i = pcie_rec(hDev, 0, 1, nword, iprint, py); // init the receiver

  imod = imod_fem;
  ichip = 3;
  buf_send[0] = (imod << 11) + (ichip << 8) + 20 + (0x0 << 16); // read out status
  i = 1;
  k = 1;
  i = pcie_send(hDev, i, k, px);
  py = &read_array[0];
  i = pcie_rec(hDev, 0, 2, nword, iprint, py); // read out 2 32 bits words
  printf("\nFEM STATUS:\n receive data word = %x, %x \n\n", read_array[0], read_array[1]);

  for (ik = 0; ik < 40; ik++)
  {

    printf("Configuration for channel %d in progress...\n", ik);
    imod = imod_fem;
    ichip = 3;
    buf_send[0] = (imod << 11) + (ichip << 8) + mb_feb_pmt_ch_set + (ik << 16); // set channel number for configuration
    i = 1;
    k = 1;
    i = pcie_send(hDev, i, k, px);

    usleep(100);

    imod = imod_fem;
    ichip = 3;
    idelay0 = 4;
    if (ik == 0)
      fprintf(outinfo, "\nidelay0 = %d\n", idelay0);
    buf_send[0] = (imod << 11) + (ichip << 8) + mb_feb_pmt_delay0 + (idelay0 << 16); // set delay0
    i = 1;
    k = 1;
    i = pcie_send(hDev, i, k, px);
    // set PMT delay1 to 12 (default)
    imod = imod_fem;
    ichip = 3;
    idelay1 = 12;
    if (ik == 0)
      fprintf(outinfo, "idelay1 = %d\n", idelay1);
    buf_send[0] = (imod << 11) + (ichip << 8) + mb_feb_pmt_delay1 + (idelay1 << 16); // set delay 1
    i = 1;
    k = 1;
    i = pcie_send(hDev, i, k, px);
    // set PMT precount
    imod = imod_fem;
    ichip = 3;
    buf_send[0] = (imod << 11) + (ichip << 8) + mb_feb_pmt_precount + (pmt_precount << 16); // set pmt precount
    i = 1;
    k = 1;
    i = pcie_send(hDev, i, k, px);
    // set PMT threshold 0
    imod = imod_fem;
    ichip = 3;
    buf_send[0] = (imod << 11) + (ichip << 8) + mb_feb_pmt_thresh0 + (threshold0 << 16); // set threshold 0
    i = 1;
    k = 1;
    i = pcie_send(hDev, i, k, px);
    // set PMT threshold 3
    imod = imod_fem;
    ichip = 3;
    //    if (ik==hg_ch) buf_send[0]=(imod<<11)+(ichip<<8)+mb_feb_pmt_thresh3+(threshold3hg<<16);
    //    else if (ik==lg_ch) buf_send[0]=(imod<<11)+(ichip<<8)+mb_feb_pmt_thresh3+(threshold3lg<<16);
    //    else
    buf_send[0] = (imod << 11) + (ichip << 8) + mb_feb_pmt_thresh3 + (threshold3 << 16); // set threshold 1
    i = 1;
    k = 1;
    i = pcie_send(hDev, i, k, px);
    // set PMT threshold 1
    imod = imod_fem;
    ichip = 3;
    //    if (ik==hg_ch) buf_send[0]=(imod<<11)+(ichip<<8)+mb_feb_pmt_thresh1+(threshold1hg<<16);
    //    else if (ik==lg_ch) buf_send[0]=(imod<<11)+(ichip<<8)+mb_feb_pmt_thresh1+(threshold1lg<<16);
    //    else
    buf_send[0] = (imod << 11) + (ichip << 8) + mb_feb_pmt_thresh1 + (threshold1 << 16); // set threshold 1
    i = 1;
    k = 1;
    i = pcie_send(hDev, i, k, px);
    // set PMT data words
    imod = imod_fem;
    ichip = 3;
    //    if (ik==hg_ch) buf_send[0]=(imod<<11)+(ichip<<8)+mb_feb_pmt_words+(pmt_wordshg<<16);  // set pmt_words
    //    else if (ik==lg_ch) buf_send[0]=(imod<<11)+(ichip<<8)+mb_feb_pmt_words+(pmt_wordslg<<16);
    //    else
    buf_send[0] = (imod << 11) + (ichip << 8) + mb_feb_pmt_words + (pmt_words << 16);
    i = 1;
    k = 1;
    i = pcie_send(hDev, i, k, px);

    // set PMT deadtime
    imod = imod_fem;
    ichip = 3;
    //    if (ik==hg_ch) buf_send[0]=(imod<<11)+(ichip<<8)+mb_feb_pmt_deadtime+(pmt_deadtimehg<<16);  // set pmt dead timr
    //    else if (ik==lg_ch) buf_send[0]=(imod<<11)+(ichip<<8)+mb_feb_pmt_deadtime+(pmt_deadtimelg<<16);
    //    else
    buf_send[0] = (imod << 11) + (ichip << 8) + mb_feb_pmt_deadtime + (pmt_deadtime << 16);
    i = 1;
    k = 1;
    i = pcie_send(hDev, i, k, px);
    // set PMT Michel window
    imod = imod_fem;
    ichip = 3;
    buf_send[0] = (imod << 11) + (ichip << 8) + mb_feb_pmt_window + (pmt_mich_window << 16); // set pmt Michel window
    i = 1;
    k = 1;
    i = pcie_send(hDev, i, k, px);
    // set PMT width (for trigger generation); default is supposed to be 100ns; here, ~78ns
    imod = imod_fem;
    ichip = 3;
    buf_send[0] = (imod << 11) + (ichip << 8) + mb_feb_pmt_width + (pmt_width << 16); // set pmt discr width
    i = 1;
    k = 1;
    i = pcie_send(hDev, i, k, px);
  }
  printf("\nConfiguring PMT Trigger parameters...\n");
  // set PMT cosmic ray trigger multiplicity
  imod = imod_fem;
  ichip = 3;
  buf_send[0] = (imod << 11) + (ichip << 8) + mb_feb_pmt_cos_mul + (cos_mult << 16); // set cosmic ray trigger mul
  i = 1;
  k = 1;
  i = pcie_send(hDev, i, k, px);
  // set PMT cosmic ray trigger pulse height
  imod = imod_fem;
  ichip = 3;
  buf_send[0] = (imod << 11) + (ichip << 8) + mb_feb_pmt_cos_thres + (cos_thres << 16); // set cosmic ray trigger peak
  i = 1;
  k = 1;
  i = pcie_send(hDev, i, k, px);
  // set PMT beam trigger multiplicity
  imod = imod_fem;
  ichip = 3;
  buf_send[0] = (imod << 11) + (ichip << 8) + mb_feb_pmt_beam_mul + (beam_mult << 16); // set beam trigger mul
  i = 1;
  k = 1;
  i = pcie_send(hDev, i, k, px);
  // set PMT beam trigger pulse height
  imod = imod_fem;
  ichip = 3;
  buf_send[0] = (imod << 11) + (ichip << 8) + mb_feb_pmt_beam_thres + (beam_thres << 16); // set beam trigger peak
  i = 1;
  k = 1;
  i = pcie_send(hDev, i, k, px);
  printf("\nEnabling/disabling channels...\n");
  // disable the top channels
  imod = imod_fem;
  ichip = 3;
  //  en_top=0xffff;
  en_top = 0x0;
  fprintf(outinfo, "en_top = 0x%x\n", en_top);
  buf_send[0] = (imod << 11) + (ichip << 8) + mb_feb_pmt_en_top + (en_top << 16); // enable/disable channel
  i = 1;
  k = 1;
  i = pcie_send(hDev, i, k, px);
  // disable the upper channels
  imod = imod_fem;
  ichip = 3;
  en_upper = 0xffff;
  fprintf(outinfo, "en_upper = 0x%x\n", en_upper);
  buf_send[0] = (imod << 11) + (ichip << 8) + mb_feb_pmt_en_upper + (en_upper << 16); // enable/disable channel
  i = 1;
  k = 1;
  i = pcie_send(hDev, i, k, px);
  // enable lower channel(s) as indicated above
  imod = imod_fem;
  ichip = 3;
  en_lower = 0x0;
  if (bg == 1)
  {
    if (bge == 1)
    {
      en_lower = en_lower + power(2, bg_ch);
    } // beam trigger readout
  }
  if (tre == 1)
  {
    en_lower = en_lower + power(2, trig_ch);
  } // ext trigger readout
  if (hg == 1)
  {
    en_lower = en_lower + power(2, hg_ch);
  }
  //  if (lg==1) {en_lower = en_lower+4;} //up to 08/15
  if (lg == 1)
  {
    en_lower = en_lower + power(2, lg_ch);
  }
  // 0xff00 channels not used in PAB shaper
  en_lower = 0xffff;
  fprintf(outinfo, "en_lower = 0x%x\n", en_lower);
  buf_send[0] = (imod << 11) + (ichip << 8) + mb_feb_pmt_en_lower + (en_lower << 16); // enable/disable channel
  i = 1;
  k = 1;
  i = pcie_send(hDev, i, k, px);
  // set maximum block size
  imod = imod_fem;
  ichip = 3;
  buf_send[0] = (imod << 11) + (ichip << 8) + mb_feb_pmt_blocksize + (0xffff << 16); // set max block size
  fprintf(outinfo, "pmt_blocksize = 0x%x\n", 0xffff);
  i = 1;
  k = 1;
  i = pcie_send(hDev, i, k, px);

  imod = imod_fem;
  ichip = 3;
  if (bg == 0)
    beam_size = 0x0; // this will actually output a few samples, doesn't completely disable it; that's why the request to unplug beam gate input from shaper, above
  else
  {
    printf("\nEnter beam size (1500 is nominal):\t");
    scanf("%d", &beam_size);
  }
  fprintf(outinfo, "gate_readout_size = %d\n", beam_size);
  buf_send[0] = (imod << 11) + (ichip << 8) + mb_feb_pmt_gate_size + (beam_size << 16); // set gate size = 1500
  i = 1;
  k = 1;
  i = pcie_send(hDev, i, k, px);
  // set beam delay
  imod = imod_fem;
  ichip = 3; // this inhibits the cosmic (discr 1) before the beam gate (64MHz clock)
  // 0x18=24 samples (should see no overlap)
  buf_send[0] = (imod << 11) + (ichip << 8) + mb_feb_pmt_beam_delay + (0x18 << 16); // set gate size
  fprintf(outinfo, "beam_delay = %d\n", 0x18);
  i = 1;
  k = 1;
  i = pcie_send(hDev, i, k, px);

  // set beam size window
  imod = imod_fem;
  ichip = 3;
  beam_size = 0x66;                                                                     // This is the gate size (1.6us in 64MHz corresponds to 102.5 time ticks; use 102=0x66)
  buf_send[0] = (imod << 11) + (ichip << 8) + mb_feb_pmt_beam_size + (beam_size << 16); // set gate size
  fprintf(outinfo, "beam_size = %d\n", beam_size);
  i = 1;
  k = 1;
  i = pcie_send(hDev, i, k, px);

  a_id = 0x20;
  imod = imod_fem;
  ichip = 3;
  buf_send[0] = (imod << 11) + (ichip << 8) + mb_feb_a_id + (a_id << 16); // set a_id
  i = 1;
  k = 1;
  i = pcie_send(hDev, i, k, px);
  // enable hold
  imod = imod_fem;
  ichip = 3;
  buf_send[0] = (imod << 11) + (ichip << 8) + mb_feb_hold_enable + (0x1 << 16); // enable the hold
  i = 1;
  k = 1;
  i = pcie_send(hDev, i, k, px);

  // work on the ADC -- set reset pulse
  imod = imod_fem;
  ichip = 5;
  buf_send[0] = (imod << 11) + (ichip << 8) + (mb_pmt_adc_reset) + (0x0 << 16); // reset goes high
  i = 1;
  k = 1;
  i = pcie_send(hDev, i, k, px);
  imod = imod_fem;
  ichip = 5;
  buf_send[0] = (imod << 11) + (ichip << 8) + (mb_pmt_adc_reset) + (0x1 << 16); // reset goes low
  i = 1;
  k = 1;
  i = pcie_send(hDev, i, k, px);
  imod = imod_fem;
  ichip = 5;
  buf_send[0] = (imod << 11) + (ichip << 8) + (mb_pmt_adc_reset) + (0x0 << 16); // reset goes high
  i = 1;
  k = 1;
  i = pcie_send(hDev, i, k, px);
  // enable ADC clock,
  imod = imod_fem;
  ichip = 5;
  buf_send[0] = (imod << 11) + (ichip << 8) + (mb_pmt_spi_add) + (0x7 << 16); // set spi address
  i = 1;
  k = 1;
  i = pcie_send(hDev, i, k, px);
  imod = imod_fem;
  ichip = 5;
  buf_send[0] = (imod << 11) + (ichip << 8) + (mb_pmt_adc_data_load) + (0xffff << 16); // load spi data, clock gate enable
  i = 1;
  k = 1;
  i = pcie_send(hDev, i, k, px);
  // load ADC sync data pattern  + set MSB 1st
  for (is = 1; is < 7; is++)
  {
    imod = imod_fem;
    ichip = 5;
    buf_send[0] = (imod << 11) + (ichip << 8) + (mb_pmt_spi_add) + ((is & 0xf) << 16); // set spi address
    i = 1;
    k = 1;
    i = pcie_send(hDev, i, k, px);
    imod = imod_fem;
    ichip = 5;
    buf_send[0] = (imod << 11) + (ichip << 8) + (mb_pmt_adc_data_load) + (0x0b00 << 16); // sync pattern, b for sync, 7 for skew, 3 for normal
    i = 1;
    k = 1;
    i = pcie_send(hDev, i, k, px);
    imod = imod_fem;
    ichip = 5;
    buf_send[0] = (imod << 11) + (ichip << 8) + (mb_pmt_adc_data_load) + (0x1400 << 16); // msb 1st
    i = 1;
    k = 1;
    i = pcie_send(hDev, i, k, px);
  }
  usleep(1000);

  // send FPGA ADC receiver reset
  imod = imod_fem;
  ichip = 3;
  buf_send[0] = (imod << 11) + (ichip << 8) + mb_feb_pmt_rxreset + (0x1 << 16); // FPGA ADC receiver reset on
  i = 1;
  k = 1;
  i = pcie_send(hDev, i, k, px);

  // readback status
  i = pcie_rec(hDev, 0, 1, nword, iprint, py); // init the receiver

  imod = imod_fem;
  ichip = 3;
  buf_send[0] = (imod << 11) + (ichip << 8) + 20 + (0x0 << 16); // read out status
  i = 1;
  k = 1;
  i = pcie_send(hDev, i, k, px);
  py = &read_array[0];
  i = pcie_rec(hDev, 0, 2, nword, iprint, py); // read out 2 32 bits words
  printf("\nFEM STATUS -- after reset = %x, %x \n", read_array[0], read_array[1]);
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

  // send FPGA ADC align
  imod = imod_fem;
  ichip = 3;
  buf_send[0] = (imod << 11) + (ichip << 8) + mb_feb_pmt_align_pulse + (0x0 << 16); // FPGA ADC receiver reset off
  i = 1;
  k = 1;
  i = pcie_send(hDev, i, k, px);

  usleep(5000); // wait for 5ms

  /**
  //readback status
  i = pcie_rec(hDev,0,1,nword,iprint,py);     // init the receiver

  imod=imod_fem;
  ichip=3;
  buf_send[0]=(imod<<11)+(ichip<<8)+20+(0x0<<16);  // read out status
  i=1;
  k=1;
  i = pcie_send(hDev, i, k, px);
  py = &read_array[0];
  i = pcie_rec(hDev,0,2,nword,iprint,py);     // read out 2 32 bits words
  printf("\nFEM STATUS -- after align = %x, %x \n", read_array[0], read_array[1]);
  printf(" module = %d, command = %d \n", ((read_array[0]>>11) & 0x1f), (read_array[0] &0xff));
  printf(" ADC right dpa lock     %d \n", ((read_array[0]>>17) & 0x1));
  printf(" ADC left  dpa lock     %d \n", ((read_array[0]>>18) & 0x1));
  printf(" block error 2          %d \n", ((read_array[0]>>19) & 0x1));
  printf(" block error 1          %d \n", ((read_array[0]>>20) & 0x1));
  printf(" pll locked             %d \n", ((read_array[0]>>21) & 0x1));
  printf(" superNova mem ready    %d \n", ((read_array[0]>>22) & 0x1));
  printf(" beam      mem ready    %d \n", ((read_array[0]>>23) & 0x1));
  printf(" ADC right PLL locked   %d \n", ((read_array[0]>>24) & 0x1));
  printf(" ADC left  PLL locked   %d \n", ((read_array[0]>>25) & 0x1));
  printf(" ADC align cmd right    %d \n", ((read_array[0]>>26) & 0x1));
  printf(" ADC align cmd left     %d \n", ((read_array[0]>>27) & 0x1));
  printf(" ADC align done right   %d \n", ((read_array[0]>>28) & 0x1));
  printf(" ADC align done left    %d \n", ((read_array[0]>>29) & 0x1));
  printf(" ADC align done left    %d \n", ((read_array[0]>>29) & 0x1));
  printf(" Neutrino data empty    %d \n", ((read_array[0]>>30) & 0x1));
  printf(" Neutrino Header empty  %d \n", ((read_array[0]>>31) & 0x1));
  **/

  for (is = 1; is < 7; is++)
  {
    imod = imod_fem;
    ichip = 5;
    buf_send[0] = (imod << 11) + (ichip << 8) + (mb_pmt_spi_add) + ((is & 0xf) << 16); // set spi address
    i = 1;
    k = 1;
    i = pcie_send(hDev, i, k, px);
    imod = imod_fem;
    ichip = 5;
    buf_send[0] = (imod << 11) + (ichip << 8) + (mb_pmt_adc_data_load) + (0x0300 << 16); // sync pattern, b for sync, 7 for skew, 3 for normal
    i = 1;
    k = 1;
    i = pcie_send(hDev, i, k, px);
  }

  return i;
}

static int pmt_link_setup(WDC_DEVICE_HANDLE hDev, int imod_xmit, int imod_st, int supernova)
{
#include "wdc_defs.h"
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
  static UINT32 u32Data;
  static unsigned short u16Data;
  static long imod, ichip;
  unsigned short *buffp;

  static UINT32 i, j, k, ifr, nread, iprint, iwrite, ik, il, is, checksum;
  static UINT32 send_array[40000], read_array[1000], read_array_compare[40000000];
  static UINT32 nmask, index, itmp, nword_tot, nevent, iv, ijk, islow_read;
  static UINT32 imod_p, imod_trig, imod_shaper;
  static int icomp_l, comp_s, ia, ic, ihuff;
  DWORD dwStatus;
  DWORD dwOptions = DMA_FROM_DEVICE;
  UINT32 iread, icheck, izero;
  UINT32 buf_send[40000];
  static int count, num, counta, nword, ireadback, nloop, ierror;
  static int ij, nsend, iloop, inew, idma_readback, iadd, jevent;
  static int itest, irun, ichip_c, dummy1, itrig_c;
  static int idup, ihold, idouble, ihold_set, istatus_read;
  static int idone, tr_bar, t_cs_reg, r_cs_reg, dma_tr;
  static int timesize, ipulse, ibase, a_id, itrig_delay;
  static int iset, ncount, nsend_f, nwrite, itrig_ext;
  static int imod_fem, idiv, isample;
  static int iframe_length, itrig, idrift_time, ijtrig;
  static int idelay0, idelay1, threshold0, threshold1, pmt_words;
  static int cos_mult, cos_thres, en_top, en_upper, en_lower, hg, lg;
  static int irise, ifall, istart_time, use_pmt, pmt_testpulse;
  static int ich_head, ich_sample, ich_frm, idebug, ntot_rec, nred;
  static int ineu, ibusy_send, ibusy_test, ihold_word, ndma_loop;
  static int irawprint, ifem_fst, ifem_lst, ifem_loop;
  static int pmt_deadtime, pmt_mich_window;
  static int oframe, osample, odiv, cframe, csample, cdiv;
  static int idac_shaper, pmt_dac_scan;
  unsigned char charchannel;
  unsigned char carray[4000];
  struct timespec tim, tim2;
  tim.tv_sec = 0;
  tim.tv_nsec = 128000;

  PVOID pbuf_rec;
  WD_DMA *pDma_rec;
  DWORD dwOptions_send = DMA_TO_DEVICE | DMA_ALLOW_CACHE;
  DWORD dwOptions_rec = DMA_FROM_DEVICE | DMA_ALLOW_64BIT_ADDRESS;

  UINT32 *px, *py, *py1;
  px = &buf_send[0];
  py = &read_array[0];
  iprint = 1;
  printf("\nLink setup -- imod_st = %d, imod_xmit = %d \n", imod_st, imod_xmit);
  // reset all the link port receiver PLL
  for (imod_fem = (imod_st - 1); imod_fem > imod_xmit; imod_fem--)
  {
    printf("\n\n WE SHOULD NOT BE HERE! \n\n");
    printf("PMT SETUP\n\n");
    imod = imod_fem;
    printf("\nReset the link PLL for module %x \n", imod);
    ichip = 4;
    buf_send[0] = (imod << 11) + (ichip << 8) + mb_feb_pll_reset + (0x0 << 16); // reset LINKIN PLL
    i = 1;
    k = 1;
    i = pcie_send(hDev, i, k, px);
    usleep(1000); // give PLL time to reset
  }

  for (imod_fem = (imod_xmit + 1); imod_fem < (imod_st + 1); imod_fem++)
  {
    nword = 1;
    i = pcie_rec(hDev, 0, 1, nword, iprint, py); // init the receiver
    imod = imod_fem;
    ichip = 3;
    buf_send[0] = (imod << 11) + (ichip << 8) + 20 + (0x0 << 16); // read out status
    i = 1;
    k = 1;
    i = pcie_send(hDev, i, k, px);
    py = &read_array[0];
    i = pcie_rec(hDev, 0, 2, nword, iprint, py); // read out 2 32 bits words
    if (iprint == 1)
      printf("FEM module %d status word after PLL reset = %x, %x \n", imod, read_array[0], read_array[1]);
  }

  // rx reset?? LINKIN DPA??

  for (imod_fem = (imod_st - 1); imod_fem > imod_xmit; imod_fem--)
  {
    printf("\n\n WE SHOULD NOT BE HERE! \n\n");
    printf("PMT SETUP \n\n");
    imod = imod_fem;
    printf(" reset the link for module %d \n", imod);
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
  }

  //<----

  imod = imod_xmit;
  ichip = 3;
  // -- number of FEM modules -1, counting starts at 0
  buf_send[0] = (imod << 11) + (ichip << 8) + mb_xmit_modcount + ((imod_st - imod_xmit) << 16);
  i = 1;
  k = 1;
  i = pcie_send(hDev, i, k, px);

  // optical
  imod = imod_xmit;
  ichip = 3;
  buf_send[0] = (imod << 11) + (ichip << 8) + mb_opt_dig_reset + (0x1 << 16); // set optical reset on
  i = 1;
  k = 1;
  i = pcie_send(hDev, i, k, px);

  // enable Neutrino/superNova Token Passing
  imod = imod_xmit;
  ichip = 3;
  buf_send[0] = (imod << 11) + (ichip << 8) + mb_xmit_enable_1 + (0x1 << 16); // enable token 1 passing (neutrino)
  //      if(supernova == 0) buf_send[0]=(imod<<11)+(ichip<<8)+mb_xmit_enable_1+(0x1<<16);  // enable token 1 pass
  //      else buf_send[0]=(imod<<11)+(ichip<<8)+mb_xmit_enable_1+(0x0<<16);  // edisable token 1 pass
  i = 1;
  k = 1;
  i = pcie_send(hDev, i, k, px);

  printf("supernova? %i", supernova);
  if (supernova == 0)
    buf_send[0] = (imod << 11) + (ichip << 8) + mb_xmit_enable_2 + (0x0 << 16); // disable token 2 pass
  else
    buf_send[0] = (imod << 11) + (ichip << 8) + mb_xmit_enable_2 + (0x1 << 16); // enable token 2 pass
  i = 1;
  k = 1;
  i = pcie_send(hDev, i, k, px);

  // reset XMIT LINK PLL
  imod = imod_xmit;
  ichip = 3;
  buf_send[0] = (imod << 11) + (ichip << 8) + mb_xmit_link_pll_reset + (0x1 << 16); //  reset XMIT LINK PLL
  i = 1;
  k = 1;
  i = pcie_send(hDev, i, k, px);
  usleep(1000);

  // reset XMIT LINK IN DPA
  imod = imod_xmit;
  ichip = 3;
  buf_send[0] = (imod << 11) + (ichip << 8) + mb_xmit_link_reset + (0x1 << 16); //  reset XMIT LINK IN DPA
  i = 1;
  k = 1;
  i = pcie_send(hDev, i, k, px);

  // wait for 10ms
  usleep(10000);

  printf("\nXMIT FIFO reset \n");
  // reset XMIT FIFO
  imod = imod_xmit;
  ichip = 3;
  buf_send[0] = (imod << 11) + (ichip << 8) + mb_xmit_dpa_fifo_reset + (0x1 << 16); //  reset XMIT LINK IN DPA
  i = 1;
  k = 1;
  i = pcie_send(hDev, i, k, px);

  // set up last module to module left of XMIT
  imod = imod_xmit + 1;
  printf("Setting last module on, module address %d\n", imod);
  ichip = 4;
  buf_send[0] = (imod << 11) + (ichip << 8) + mb_feb_lst_on + (0x0 << 16); // set last module on
  i = 1;
  k = 1;
  i = pcie_send(hDev, i, k, px);

  for (is = 0; is < 1; is++)
  {
    // test re-align circuit
    imod = imod_xmit;
    ichip = 3;
    buf_send[0] = (imod << 11) + (ichip << 8) + mb_xmit_dpa_word_align + (0x1 << 16); //  send alignment pulse
    i = 1;
    k = 1;
    i = pcie_send(hDev, i, k, px);
  }
  usleep(5000); // wait for 5 ms
  printf("XMIT re-align done \n");

  nword = 1;
  i = pcie_rec(hDev, 0, 1, nword, iprint, py); // init the receiver
  imod = imod_xmit;
  ichip = 3;
  buf_send[0] = (imod_xmit << 11) + (ichip << 8) + mb_xmit_rdstatus + (0x0 << 16); // read out status

  i = 1;
  k = 1;
  i = pcie_send(hDev, i, k, px);
  py = &read_array[0];
  i = pcie_rec(hDev, 0, 2, nword, iprint, py); // read out 2 32 bits words
  printf("XMIT status word = %x, %x \n", read_array[0], read_array[1]);
  usleep(10000);

  /**
  // set up last module to module left of XMIT
  imod=imod_xmit+1;
  printf("Setting last module off, module address %d\n", imod);
  ichip=4;
  buf_send[0]=(imod<<11)+(ichip<<8)+mb_feb_lst_off+(0x0<<16);    // set last module off
  i=1;
  k=1;
  i = pcie_send(hDev, i, k, px);
  **/
  printf("Finishing XMIT alignment \n");
  // reset all the link port receiver
  imod_fem = imod_xmit + 1;
  while (imod_fem < imod_st)
  {
    printf("\n\n WE SHOULD NOT BE HERE! \n\n");

    imod = imod_fem + 1;
    ichip = 4;
    buf_send[0] = (imod << 11) + (ichip << 8) + mb_feb_lst_on + (0x0 << 16); // set last module on
    i = 1;
    k = 1;
    i = pcie_send(hDev, i, k, px);
    printf(" set last module, module address -- alignment %d\n", imod);
    imod = imod_fem;
    printf(" reset link for module %d \n", imod);
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
    //       printf(" finish FEM module %d alignment \n", imod);
    imod = imod_fem + 1;
    if (imod != imod_st)
    {
      ichip = 4;
      buf_send[0] = (imod << 11) + (ichip << 8) + mb_feb_lst_off + (0x0 << 16); // set last module off
      i = 1;
      k = 1;
      i = pcie_send(hDev, i, k, px);
      printf(" set last module off, module address -- alignment%d\n", imod);
    }
    imod_fem = imod_fem + 1;
  }

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
    usleep(100);
    i = pcie_rec(hDev, 0, 2, nword, iprint, py); // read out 2 32 bits words
    if (iprint == 1)
      printf("FEM module %d status word = %x, %x \n", imod, read_array[0], read_array[1]);
  }
  return i;
}

static int tpc_adc_setup(WDC_DEVICE_HANDLE *hDev, WDC_DEVICE_HANDLE *hDev2, int imod_fem, int imod_xmit, int iframe_length, int imod_trig, int timesize)
{

  // Variable declarations
  int imod_st, imod, ichip;
  static int i, k, count, counta, ichip_c, nsend, dummy1;
  unsigned char charchannel;
  unsigned char carray[4000];
  static UINT32 send_array[40000];
  static int ij, nword, ik, iprint, iframe, itrig_delay;
  struct timespec tim, tim2;
  static UINT32 read_array[40000];
  static int idebug, adcdata, is, fakeadcdata, ihuff, a_id, il, ijk;
  UINT32 u32Data, dwOffset, dwAddrSpace;
  FILE *inpf;
  imod_st = imod_xmit + 2;

  UINT32 buf_send[40000];
  buf_send[0] = 0x0;
  buf_send[1] = 0x0;
  UINT32 *px = &buf_send[0];
  UINT32 *py = &read_array[0];
  ihuff = 0;
  adcdata = 1;
  //******************************************************
  // turn on Stratix III power supply
  printf("\n\nBooting FEBs...\n");

  for (imod_fem = (imod_xmit + 2); imod_fem < (imod_st + 1); imod_fem++)
  { // loop over module numbers
    imod = imod_fem;
    printf("\n Booting module in slot %d \n", imod);

    // turn on the Stratix III power supply
    ichip = 1;
    buf_send[0] = (imod << 11) + (ichip << 8) + mb_feb_power_add + (0x0 << 16); // turn module 11 power on
    i = 1;
    k = 1;
    i = pcie_send(hDev, i, k, px);
    usleep(200000); // wait for 200 ms
    inpf = fopen("/home/ub/WinDriver/wizard/GRAMS_project_am/uBooNE_Nominal_Firmware/fem/module1x_140820_deb_fixbase_nf_8_31_2018.rbf", "r");
    ichip = mb_feb_conf_add;                                       // ichip=mb_feb_config_add(=2) is for configuration chip
    buf_send[0] = (imod << 11) + (ichip << 8) + 0x0 + (0x0 << 16); // turn conf to be on
    i = 1;
    k = 1;
    i = pcie_send(hDev, i, k, px);
    usleep(1000); // wait for a while

    count = 0;   // keeps track of config file data sent
    counta = 0;  // keeps track of total config file data read
    ichip_c = 7; // this chip number is actually a "ghost"; it doesn't exist; it's just there so the config chip
    // doesn't treat the first data word i'll be sending as a command designated for the config chip (had i used ichip_c=1)
    nsend = 500; // this defines number of 32bit-word-sized packets I'm allowed to use to send config file data
    dummy1 = 0;

    while (fread(&charchannel, sizeof(char), 1, inpf) == 1)
    {
      carray[count] = charchannel;
      count++;
      counta++;
      if ((count % (nsend * 2)) == 0)
      {
        buf_send[0] = (imod << 11) + (ichip_c << 8) + (carray[0] << 16);
        send_array[0] = buf_send[0];
        if (dummy1 <= 5)
          printf(" counta = %d, first word = %x, %x, %x %x %x \n", counta, buf_send[0], carray[0], carray[1], carray[2], carray[3]);
        for (ij = 0; ij < nsend; ij++)
        {
          if (ij == (nsend - 1))
            buf_send[ij + 1] = carray[2 * ij + 1] + (0x0 << 16);
          else
            buf_send[ij + 1] = carray[2 * ij + 1] + (carray[2 * ij + 2] << 16);
          send_array[ij + 1] = buf_send[ij + 1];
        }
        nword = nsend + 1;
        i = 1;
        ij = pcie_send(hDev, i, nword, px);
        nanosleep(&tim, &tim2);
        dummy1 = dummy1 + 1;
        count = 0;
      }
    }

    if (feof(inpf))
    {
      printf(" You have reached the end-of-file word count= %d %d\n", counta, count);
      buf_send[0] = (imod << 11) + (ichip_c << 8) + (carray[0] << 16);
      if (count > 1)
      {
        if (((count - 1) % 2) == 0)
        {
          ik = (count - 1) / 2;
        }
        else
        {
          ik = (count - 1) / 2 + 1;
        }
        ik = ik + 2; // add one more for safety
        printf(" ik= %d\n", ik);
        for (ij = 0; ij < ik; ij++)
        {
          if (ij == (ik - 1))
            buf_send[ij + 1] = carray[(2 * ij) + 1] + (((imod << 11) + (ichip << 8) + 0x0) << 16);
          else
            buf_send[ij + 1] = carray[(2 * ij) + 1] + (carray[(2 * ij) + 2] << 16);
          send_array[ij + 1] = buf_send[ij + 1];
        }
      }
      else
        ik = 1;

      for (ij = ik - 10; ij < ik + 1; ij++)
      {
        printf(" Last data = %d, %x\n", ij, buf_send[ij]);
      }

      nword = ik + 1;
      i = 1;
      i = pcie_send(hDev, i, nword, px);
    }
    usleep(2000); // wait for 2ms to cover the packet time plus fpga init time
    fclose(inpf);
    printf(" Configuration for module in slot %d COMPLETE.\n", imod);
  }
  printf("\n...FEB booting done \n");
  //******************************************************
  /* set tx mode register */
  u32Data = 0x00003fff; // set up number of words hold coming back from the XMIT module
  printf("\nNumber of words for hold be send back = %x\n", u32Data);
  dwOffset = 0x28;
  dwAddrSpace = 2;
  WDC_WriteAddr32(hDev2, dwAddrSpace, dwOffset, u32Data);
  //   set up hold
  printf("Setting up hold condition \n");
  dwAddrSpace = 2;
  u32Data = 0x8000000; // set up transmitter to return the hold -- upper transciever
  dwOffset = 0x18;
  WDC_WriteAddr32(hDev2, dwAddrSpace, dwOffset, u32Data);
  dwAddrSpace = 2;
  u32Data = 0x8000000; // set up transmitter to return the hold -- lower transciever
  dwOffset = 0x20;
  WDC_WriteAddr32(hDev2, dwAddrSpace, dwOffset, u32Data);
  //******************************************************

  imod = imod_trig;
  iframe = iframe_length;
  buf_send[0] = (imod << 11) + (mb_trig_frame_size) + ((iframe & 0xffff) << 16);
  i = 1;
  k = 1;
  i = pcie_send(hDev, i, k, px);

  // imod = 0;
  // ichip = 1;
  // iframe = iframe_length;                                                                        // 1023
  // buf_send[0] = (imod << 11) + (ichip << 8) + (mb_cntrl_load_frame) + ((iframe & 0xffff) << 16); // enable test mode
  // i = 1;
  // k = 1;
  // i = pcie_send(hDev, i, k, px);

  // load trig 1 position relative to the frame..
  imod = 0;
  ichip = 1;
  buf_send[0] = (imod << 11) + (ichip << 8) + (mb_cntrl_load_trig_pos) + ((itrig_delay & 0xffff) << 16); // enable test mode
  i = 1;
  k = 1;
  i = pcie_send(hDev, i, k, px);

  //******************************************************
  // FEB settings
  printf("\nSetting up FEB's...\n");

  for (imod_fem = (imod_xmit + 2); imod_fem < (imod_st + 1); imod_fem++)
  {
    imod = imod_fem;
    // reset FEB DRAM (step 1)
    ichip = 3;
    buf_send[0] = (imod << 11) + (ichip << 8) + mb_feb_dram_reset + (0x1 << 16); // turm the DRAM reset on
    i = 1;
    k = 1;
    i = pcie_send(hDev, i, k, px);
    // reset FEB DRAM (step 2)
    ichip = 3;
    buf_send[0] = (imod << 11) + (ichip << 8) + mb_feb_dram_reset + (0x0 << 16); // turm the DRAM reset off
    i = 1;
    k = 1;
    i = pcie_send(hDev, i, k, px);
    usleep(5000); // wait for 5 ms for DRAM to be initialized
    // set module number
    ichip = 3;
    buf_send[0] = (imod << 11) + (ichip << 8) + mb_feb_mod_number + (imod << 16); // set module number
    i = 1;
    k = 1;
    i = pcie_send(hDev, i, k, px);

    if (adcdata == 1)
    {
      printf("\nSetting up ADC, module %d\n", imod);
      // Set ADC address 1
      for (is = 0; is < 8; is++)
      {

        ichip = 5;                                                                         // ADC control
        buf_send[0] = (imod << 11) + (ichip << 8) + (mb_pmt_spi_add) + ((is & 0xf) << 16); // set ADC address (same op id =2 for PMT and TPC); second 16bit word is the address; the loop sets first 0-7 ADC addresses
        i = 1;
        k = 1;
        i = pcie_send(hDev, i, k, px);
        usleep(5000); // sleep for 2ms
        // The ADC spi stream: The 16 bits data before the last word is r/w, w1, w2 and 13 bits address;
        //                     the last 16 bits.upper byte = 8 bits data and lower 8 bits ignored.
        ichip = 5;
        buf_send[0] = (imod << 11) + (ichip << 8) + (mb_pmt_adc_data_load) + (0x0300 << 16); // op id = 3; first data word is 16bit 1st next word will be overwrite by the next next word //sync pattern, b for sync, 7 for skew, 3 for normal
        buf_send[1] = (((0x0) << 13) + (0xd)) + ((0xc) << 24) + ((0x0) << 16);               // 8 more bits to fill 24-bit data word; data is in <<24 operation; r/w op goes to 13 0xd is address and data is <<24; top 8 bits bottom 8 bits discarded; 0xc is mixed frequency pattern  (04/25/12)
        i = 1;
        k = 2;
        //	      i = pcie_send(hDev, i, k, px);
        usleep(5000); // sleep for 2ms

        ichip = 5;
        buf_send[0] = (imod << 11) + (ichip << 8) + (mb_pmt_adc_data_load) + (0x0300 << 16); // 1st next word will be overwrite by the next next word
        buf_send[1] = (((0x0) << 13) + (0xff)) + ((0x1) << 24) + ((0x0) << 16);              // write to transfer register:  set r/w =0, w1,w2 =0, a12-a0 = 0xff, data =0x1;
        i = 1;
        k = 2;
        //	      i = pcie_send(hDev, i, k, px);
        usleep(5000); // sleep for 2ms

        // double terminate ADC driver
        ichip = 5;                                                                         // ADC control
        buf_send[0] = (imod << 11) + (ichip << 8) + (mb_pmt_spi_add) + ((is & 0xf) << 16); // set ADC address (same op id =2 for PMT and TPC); second 16bit word is the address; the loop sets first 0-7 ADC addresses
        i = 1;
        k = 1;
        i = pcie_send(hDev, i, k, px);
        usleep(5000); // sleep for 2ms

      } // end ADC address set

      // Set ADC address 2
      for (is = 0; is < 8; is++)
      {
        ichip = 5;
        buf_send[0] = (imod << 11) + (ichip << 8) + (mb_pmt_spi_add) + ((is & 0xf) << 16); // set spi address
        i = 1;
        k = 1;
        i = pcie_send(hDev, i, k, px);
        usleep(5000); // sleep for 2ms
        //       printf(" spi port %d \n",is);
        //       scanf("%d",&ik);

        ichip = 5;
        buf_send[0] = (imod << 11) + (ichip << 8) + (mb_pmt_adc_data_load) + (0x0300 << 16); // 1st next word will be overwrite by the next next word
        //	      buf_send[1]=(((0x0)<<13)+(0xd))+((0x9)<<24)+((0x0)<<16);
        buf_send[1] = (((0x0) << 13) + (0xd)) + ((0xc) << 24) + ((0x0) << 16); // 0x0<<24 is for baseline //  set /w =0, w1,w2 =0, a12-a0 = 0xd, data =0xc (mixed frequency);
        i = 1;
        k = 2;
        i = pcie_send(hDev, i, k, px);
        usleep(5000); // sleep for 2ms
        //       printf(" spi port 2nd command %d \n",is);
        //       scanf("%d",&ik);

        ichip = 5;
        buf_send[0] = (imod << 11) + (ichip << 8) + (mb_pmt_adc_data_load) + (0x0300 << 16); // 1st next word will be overwrite by the next next word
        buf_send[1] = (((0x0) << 13) + (0xff)) + ((0x1) << 24) + ((0x0) << 16);              //  set /w =0, w1,w2 =0, a12-a0 = 0xff, data =0x1; //  write to transfer register
        i = 1;
        k = 2;
        i = pcie_send(hDev, i, k, px);
        usleep(5000); // sleep for 2ms

      } // end ADC address set

      // Reset ADC receiver
      ichip = 3;                                                                  // stratix III
      buf_send[0] = (imod << 11) + (ichip << 8) + mb_feb_adc_reset + (0x1 << 16); // FPGA ADC receiver reset on, cmd=1 on mb_feb_adc_reset=33
      i = 1;
      k = 1;
      i = pcie_send(hDev, i, k, px);
      usleep(5000);
      // Align ADC receiver
      ichip = 3;                                                                  // stratix III
      buf_send[0] = (imod << 11) + (ichip << 8) + mb_feb_adc_align + (0x0 << 16); // FPGA ADC align, cmd=0 on mb_feb_adc_align=1
      i = 1;
      k = 1;
      i = pcie_send(hDev, i, k, px);
      usleep(5000);

      // Set ADC address 3
      for (is = 0; is < 8; is++)
      {
        ichip = 5;
        buf_send[0] = (imod << 11) + (ichip << 8) + (mb_pmt_spi_add) + ((is & 0xf) << 16); // set spi address
        i = 1;
        k = 1;
        i = pcie_send(hDev, i, k, px);
        usleep(5000); // sleep for 2ms
        //       printf(" spi port %d \n",is);
        //       scanf("%d",&ik);

        ichip = 5;
        buf_send[0] = (imod << 11) + (ichip << 8) + (mb_pmt_adc_data_load) + (0x0300 << 16); // 1st next word will be overwrite by the next next word
        //	      buf_send[1]=(((0x0)<<13)+(0xd))+((0x9)<<24)+((0x0)<<16);
        buf_send[1] = (((0x0) << 13) + (0xd)) + ((0x0) << 24) + ((0x0) << 16); // 0x0<<24 is for baseline //  set /w =0, w1,w2 =0, a12-a0 = 0xd, data =0xc (mixed frequency);
        i = 1;
        k = 2;
        i = pcie_send(hDev, i, k, px);
        usleep(5000); // sleep for 2ms
                      //       printf(" spi port 2nd command %d \n",is);
        //       scanf("%d",&ik);

        ichip = 5;
        buf_send[0] = (imod << 11) + (ichip << 8) + (mb_pmt_adc_data_load) + (0x0300 << 16); // 1st next word will be overwrite by the next next word
        buf_send[1] = (((0x0) << 13) + (0xff)) + ((0x1) << 24) + ((0x0) << 16);              //  set /w =0, w1,w2 =0, a12-a0 = 0xff, data =0x1; //  write to transfer register
        i = 1;
        k = 2;
        i = pcie_send(hDev, i, k, px);
        usleep(5000); // sleep for 2ms
      } // end ADC address set

    } // end if ADC set

    nword = 1;
    i = pcie_rec(hDev, 0, 1, nword, iprint, py); // init the receiver
    // read out status
    ichip = 3;
    buf_send[0] = (imod << 11) + (ichip << 8) + mb_feb_rd_status + (0x0 << 16); // read out status
    i = 1;
    k = 1;
    i = pcie_send(hDev, i, k, px);
    py = &read_array[0];
    i = pcie_rec(hDev, 0, 2, nword, iprint, py); // read out 2 32 bits words
    printf("\nReceived FEB %d (slot %d) status data word = %x, %x \n", imod, imod, read_array[0], read_array[1]);
    if (idebug == 1)
    {
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

    //******************************************************
    // set test mode
    ichip = mb_feb_pass_add;

    buf_send[0] = (imod << 11) + (ichip << 8) + mb_feb_test_source + (0x0 << 16);
    i = 1;
    k = 1;
    i = pcie_send(hDev, i, k, px);

    //******************************************************
    // set compression state
    imod = imod_fem;
    ichip = 3;
    if (ihuff == 1)
      buf_send[0] = (imod << 11) + (ichip << 8) + mb_feb_b_nocomp + (0x0 << 16); // turn the compression
    else
      buf_send[0] = (imod << 11) + (ichip << 8) + mb_feb_b_nocomp + (0x1 << 16); // set b channel no compression
    i = 1;
    k = 1;
    i = pcie_send(hDev, i, k, px);

    // sn loop xxx
    imod = imod_fem;
    ichip = 3;
    if (ihuff == 1)
      buf_send[0] = (imod << 11) + (ichip << 8) + mb_feb_a_nocomp + (0x0 << 16); // turn the compression
    else
      buf_send[0] = (imod << 11) + (ichip << 8) + mb_feb_a_nocomp + (0x1 << 16); // set b channel no compression
    i = 1;
    k = 1;
    i = pcie_send(hDev, i, k, px);

    //******************************************************
    // set drift size
    ichip = 3;
    buf_send[0] = (imod << 11) + (ichip << 8) + mb_feb_timesize + (timesize << 16); // set drift time size
    i = 1;
    k = 1;
    i = pcie_send(hDev, i, k, px);

    // set id
    a_id = 0xf;
    ichip = 3;
    buf_send[0] = (imod << 11) + (ichip << 8) + mb_feb_b_id + (a_id << 16); // set a_id
    i = 1;
    k = 1;
    i = pcie_send(hDev, i, k, px);

    ichip = 4;
    buf_send[0] = (imod << 11) + (ichip << 8) + mb_feb_b_id + (a_id << 16); // set b_id
    i = 1;
    k = 1;
    i = pcie_send(hDev, i, k, px);

    // set max word in the pre-buffer memory
    ik = 8000;
    ichip = 3;
    buf_send[0] = (imod << 11) + (ichip << 8) + mb_feb_max + (ik << 16); // set pre-buffer max word
    i = 1;
    k = 1;
    i = pcie_send(hDev, i, k, px);

    // enable hold
    ichip = 3;
    buf_send[0] = (imod << 11) + (ichip << 8) + mb_feb_hold_enable + (0x1 << 16); // enable the hold
    i = 1;
    k = 1;
    i = pcie_send(hDev, i, k, px);

    // this was in SN loop
    if (imod == imod_st)
    {
      printf(" set last module, module address %d\n", imod);
      ichip = 4;
      buf_send[0] = (imod << 11) + (ichip << 8) + mb_feb_lst_on + (0x0 << 16); // set last module on
      i = 1;
      k = 1;
      i = pcie_send(hDev, i, k, px);
    }
    else
    {
      printf(" set last module off, module address %d\n", imod);
      ichip = 4;
      buf_send[0] = (imod << 11) + (ichip << 8) + mb_feb_lst_off + (0x0 << 16); // set last module on
      i = 1;
      k = 1;
      i = pcie_send(hDev, i, k, px);
    }
  } // end loop over FEMs

  printf("\n...FEB setup complete.\n\n");

  //******************************************************
  //     now reset all the link port receiver PLL
  for (imod_fem = (imod_st - 1); imod_fem > imod_xmit + 1; imod_fem--)
  {
    imod = imod_fem;
    printf("Resetting link PLL for module %x \n", imod);
    ichip = 4;
    buf_send[0] = (imod << 11) + (ichip << 8) + mb_feb_pll_reset + (0x0 << 16); // reset LINKIN PLL
    i = 1;
    k = 1;
    i = pcie_send(hDev, i, k, px);
    usleep(1000); // give PLL time to reset
  }

  for (imod_fem = (imod_st - 1); imod_fem > imod_xmit + 1; imod_fem--)
  {
    imod = imod_fem;
    printf(" reset the link for module %d \n", imod);
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
  }

  printf("\n...FEB setup complete.\n\n");
}


#endif
