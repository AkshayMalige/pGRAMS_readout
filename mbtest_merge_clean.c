/* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ INCLUDES ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
#include "mbtest_merge_clean.h"

/* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ FUNCTION DEFINATIONS ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
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

    if (hDev) /* Perform necessary cleanup before exiting the program */
        DeviceClose(hDev);
    DeviceClose(hDev1);
    DeviceClose(hDev2);

    dwStatus = PCIE_LibUninit();
    if (WD_STATUS_SUCCESS != dwStatus)
        PCIE_ERR("pcie_diag: Failed to uninit the PCIE library: %s", PCIE_GetLastErr());

    return dwStatus;
}

static void MenuMain(WDC_DEVICE_HANDLE *phDev, WDC_DEVICE_HANDLE *phDev2) // Main diagnostics menu
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

static void DiagEventHandler(WDC_DEVICE_HANDLE hDev, DWORD dwAction) // Plug-and-play and power management events handler routine
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

static void MenuInterrupts(WDC_DEVICE_HANDLE hDev) // Enable/Disable interrupts menu
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

static void MenuReadWriteRegs(WDC_DEVICE_HANDLE hDev) // Display read/write run-time registers menu
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

static void MenuEvents(WDC_DEVICE_HANDLE hDev) // Register/unregister Plug-and-play and power management events
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

static void MenuReadWriteCfgSpace(WDC_DEVICE_HANDLE hDev) // Display read/write configuration space menu
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

static void MenuReadWriteAddr(WDC_DEVICE_HANDLE hDev) // Read/write memory or I/O space address menu
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

/* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ MBTEST  ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
static void MenuMBtest(WDC_DEVICE_HANDLE hDev, WDC_DEVICE_HANDLE hDev2)
{
#include "mbtest_dec.h"

    /* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ MISC...  ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */
    newcmd = 1;
    mask1 = 0x0;
    mask8 = 0x2;
    static time_t start, end;
    struct timeval starttest18, endtest18;
    long mytime18, seconds18, useconds18;
    time_t rawtime;
    char timestr[500];
    static int trig_deadtime;
    time(&rawtime);
    strftime(timestr, sizeof(timestr), "%Y_%m_%d", localtime(&rawtime));
    printf("\n ######## PMT XMIT Readout (keyboard interrupt)\n\n Enter SUBRUN NUMBER or NAME:\t");
    char subrun[50];
    scanf("%s", &subrun);
    printf("\nEnter desired DMA size (<%d)\t", dwDMABufSize);
    scanf("%d", &ibytec);
    dwDMABufSize = ibytec;
    char _buf[200];
    sprintf(_buf, "/home/ub/WinDriver/wizard/GRAMS_project_am/data/pmt/xmit_subrun_%s_%s_dma_no_1.bin", timestr, subrun);
    outf = fopen(_buf, "wb");
    fd = creat("test_file.dat", 0755);
    sprintf(_buf, "/home/ub/WinDriver/wizard/GRAMS_project_am/data/pmt/xmit_subrun_%s_%s.info", timestr, subrun);
    outinfo = fopen(_buf, "w");
    // fprintf(outinfo, "nevispcie_pmt_scratch_nokbhit.cpp\n\n");
    idebug = 1;
    irawprint = 0;
    icont = 1;
    iwrite = 1;     // grams
    writedebug = 1; // see docdb-2819 for hardware config details
    imod_xmit = 6;  // 10;
    imod_pmt = 7;
    imod_st = 8;
    imod_tpc = 8;
    imod_trig = 16;  // 16;
    imod_shaper = 4; // 6;
    iframe_length = 1599;
    iframe = iframe_length;
    itrig_delay = 10; // 10
    timesize = 199;
    icheck = 0;
    ifr = 0;
    irand = 0;
    islow_read = 0;
    iprint = 1;
    nsend = 500;

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
    fprintf(outinfo, "iframe_length (16MHz) = %d\n", iframe_length);
    fprintf(outinfo, "itrig_delay = %d\n", itrig_delay);
    printf("\tframe size = %d\n", iframe_length);
    fprintf(outinfo, "ibytec = %d (DMA size)\n", ibytec);
    fprintf(outinfo, "dwDMABufSize = %d\n", dwDMABufSize);
    fprintf(outinfo, "dma_buffer_size (read_array) = %d\n", ibytec);

    /* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ CONTROLLER-PCIE SETTUP  ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */
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

    px = &buf_send[0]; // RUN INITIALIZATION
    py = &read_array[0];
    imod = 0; // controller module

    buf_send[0] = 0x0; // INITIALIZE
    buf_send[1] = 0x0;
    i = 1;
    k = 1;
    i = pcie_send(hDev, i, k, px);

    /* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ XMIT BOOT  ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */

    boot_xmit(hDev, imod_xmit);
    printf("\nDo you want to read out TPC or PMT? Enter 0 for TPC, 1 for PMT, 2 for both\n");
    scanf("%i", &readtype);

    /* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ XMIT SETTUP  ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */

    setup_xmit(hDev, imod_xmit);

    /* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ FEM BOOT  ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */

    boot_fems(hDev, imod_xmit, imod_st, fem_type);

    /* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ FEM SETTUP  ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */

    setup_fems(hDev, fem_slot, true);  // TPC
    setup_fems(hDev, fem_slot, false); // PMT

    /* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ TRIGGER_SETUP  ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */

    setup_trig(hDev, trig_slot, fem_type)

    /* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ TRIGGER_SETUP  ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */

    link_fems(hDev, trig_slot, fem_type)

    /* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ DMA & EB  ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */
}