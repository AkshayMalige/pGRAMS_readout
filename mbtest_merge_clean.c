/* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ INCLUDES ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <termios.h>
#include <unistd.h>
#include "wdc_defs.h"
#include "wdc_lib.h"
#include "utils.h"
#include "status_strings.h"
#include "samples/shared/diag_lib.h"
#include "samples/shared/wdc_diag_lib.h"
#include "samples/shared/pci_regs.h"
#include "pcie_lib.h"
#include <stdbool.h>
#include <stdint.h>

#define PCIE_ERR printf

static CHAR gsInput[256];

const WDC_REG gPCIE_CfgRegs[] = {
    {WDC_AD_CFG_SPACE, PCI_VID, WDC_SIZE_16, WDC_READ_WRITE, "VID", "Vendor ID"},
    {WDC_AD_CFG_SPACE, PCI_DID, WDC_SIZE_16, WDC_READ_WRITE, "DID", "Device ID"},
    {WDC_AD_CFG_SPACE, PCI_CR, WDC_SIZE_16, WDC_READ_WRITE, "CMD", "Command"},
    {WDC_AD_CFG_SPACE, PCI_SR, WDC_SIZE_16, WDC_READ_WRITE, "STS", "Status"},
    {WDC_AD_CFG_SPACE, PCI_REV, WDC_SIZE_32, WDC_READ_WRITE, "RID_CLCD", "Revision ID & Class Code"},
    {WDC_AD_CFG_SPACE, PCI_CCSC, WDC_SIZE_8, WDC_READ_WRITE, "SCC", "Sub Class Code"},
    {WDC_AD_CFG_SPACE, PCI_CCBC, WDC_SIZE_8, WDC_READ_WRITE, "BCC", "Base Class Code"},
    {WDC_AD_CFG_SPACE, PCI_CLSR, WDC_SIZE_8, WDC_READ_WRITE, "CALN", "Cache Line Size"},
    {WDC_AD_CFG_SPACE, PCI_LTR, WDC_SIZE_8, WDC_READ_WRITE, "LAT", "Latency Timer"},
    {WDC_AD_CFG_SPACE, PCI_HDR, WDC_SIZE_8, WDC_READ_WRITE, "HDR", "Header Type"},
    {WDC_AD_CFG_SPACE, PCI_BISTR, WDC_SIZE_8, WDC_READ_WRITE, "BIST", "Built-in Self Test"},
    {WDC_AD_CFG_SPACE, PCI_BAR0, WDC_SIZE_32, WDC_READ_WRITE, "BADDR0", "Base Address 0"},
    {WDC_AD_CFG_SPACE, PCI_BAR1, WDC_SIZE_32, WDC_READ_WRITE, "BADDR1", "Base Address 1"},
    {WDC_AD_CFG_SPACE, PCI_BAR2, WDC_SIZE_32, WDC_READ_WRITE, "BADDR2", "Base Address 2"},
    {WDC_AD_CFG_SPACE, PCI_BAR3, WDC_SIZE_32, WDC_READ_WRITE, "BADDR3", "Base Address 3"},
    {WDC_AD_CFG_SPACE, PCI_BAR4, WDC_SIZE_32, WDC_READ_WRITE, "BADDR4", "Base Address 4"},
    {WDC_AD_CFG_SPACE, PCI_BAR5, WDC_SIZE_32, WDC_READ_WRITE, "BADDR5", "Base Address 5"},
    {WDC_AD_CFG_SPACE, PCI_CIS, WDC_SIZE_32, WDC_READ_WRITE, "CIS", "CardBus CIS Pointer"},
    {WDC_AD_CFG_SPACE, PCI_SVID, WDC_SIZE_16, WDC_READ_WRITE, "SVID", "Sub-system Vendor ID"},
    {WDC_AD_CFG_SPACE, PCI_SDID, WDC_SIZE_16, WDC_READ_WRITE, "SDID", "Sub-system Device ID"},
    {WDC_AD_CFG_SPACE, PCI_EROM, WDC_SIZE_32, WDC_READ_WRITE, "EROM", "Expansion ROM Base Address"},
    {WDC_AD_CFG_SPACE, PCI_CAP, WDC_SIZE_8, WDC_READ_WRITE, "NEW_CAP", "New Capabilities Pointer"},
    {WDC_AD_CFG_SPACE, PCI_ILR, WDC_SIZE_32, WDC_READ_WRITE, "INTLN", "Interrupt Line"},
    {WDC_AD_CFG_SPACE, PCI_IPR, WDC_SIZE_32, WDC_READ_WRITE, "INTPIN", "Interrupt Pin"},
    {WDC_AD_CFG_SPACE, PCI_MGR, WDC_SIZE_32, WDC_READ_WRITE, "MINGNT", "Minimum Required Burst Period"},
    {WDC_AD_CFG_SPACE, PCI_MLR, WDC_SIZE_32, WDC_READ_WRITE, "MAXLAT", "Maximum Latency"},
};
#define PCIE_CFG_REGS_NUM sizeof(gPCIE_CfgRegs) / sizeof(WDC_REG)
const WDC_REG *gpPCIE_CfgRegs = gPCIE_CfgRegs;
const WDC_REG *gpPCIE_Regs = NULL;

#define PCIE_REGS_NUM 0

/* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ FUNTION DEFINATIONS ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */
static void changemode(int); // keyboard interrupt data taking
static WDC_DEVICE_HANDLE DeviceFindAndOpen(DWORD dwVendorId, DWORD dwDeviceId);
static BOOL DeviceFind(DWORD dwVendorId, DWORD dwDeviceId, WD_PCI_SLOT *pSlot);
static WDC_DEVICE_HANDLE DeviceOpen(const WD_PCI_SLOT *pSlot);
static void DeviceClose(WDC_DEVICE_HANDLE hDev);
static void MenuMBtest(WDC_DEVICE_HANDLE hDev, WDC_DEVICE_HANDLE hDev2);
static void MenuReadWriteAddr(WDC_DEVICE_HANDLE hDev);
static void MenuEvents(WDC_DEVICE_HANDLE hDev);
static void MenuMain(WDC_DEVICE_HANDLE *phDev, WDC_DEVICE_HANDLE *phDev2);
static void MenuInterrupts(WDC_DEVICE_HANDLE hDev);
static void MenuReadWriteCfgSpace(WDC_DEVICE_HANDLE hDev);
static void MenuReadWriteRegs(WDC_DEVICE_HANDLE hDev);
static void SetAddrSpace(WDC_DEVICE_HANDLE hDev, PDWORD pdwAddrSpace);
static void DiagIntHandler(WDC_DEVICE_HANDLE hDev, PCIE_INT_RESULT *pIntResult);
static void DiagEventHandler(WDC_DEVICE_HANDLE hDev, DWORD dwAction);
static int pcie_send(WDC_DEVICE_HANDLE hDev, int mode, int nword, UINT32 *buff_send);
static int pcie_rec(WDC_DEVICE_HANDLE hDev, int mode, int istart, int nword, int ipr_status, UINT32 *buff_rec);
static int power(int a, int b); // enable/disable channels
static int pmt_adc_setup(WDC_DEVICE_HANDLE hDev, int imod_fem, int iframe, int mode, FILE *outinfo);

/*static void boot_xmit(WDC_DEVICE_HANDLE *hDev, int imod_xmit_in, int dmabufsize, bool print_debug);
static void setup_xmit(WDC_DEVICE_HANDLE *hDev, int imod_xmit_in, int dmabufsize, bool print_debug);
static int boot_fems(WDC_DEVICE_HANDLE hDev, int imod_xmit_in, int imod_st, bool fem_type, bool print_debug);
static int setup_fems(WDC_DEVICE_HANDLE hDev, int imod_xmit, int imod_st, int fem_slot, bool fem_type, FILE *outfile, int dmabufsize);
static int setup_tpc_fem(WDC_DEVICE_HANDLE hDev, int imod_xmit, int imod_st, int fem_slot, bool fem_type);
static int setup_pmt_fem(WDC_DEVICE_HANDLE hDev, int imod_fem, int iframe_length, int mode, FILE *outinfo, int dmabufsize);
static int setup_trig(WDC_DEVICE_HANDLE hDev, int trig_slot);
static int link_fems(WDC_DEVICE_HANDLE hDev, int imod_xmit, int imod_st, int dmabufsize);
*/
static WDC_DEVICE_HANDLE DeviceFindAndOpen(DWORD dwVendorId, DWORD dwDeviceId)
{
    WD_PCI_SLOT slot;

    if (!DeviceFind(dwVendorId, dwDeviceId, &slot))
        return NULL;

    return DeviceOpen(&slot);
}

static BOOL DeviceFind(DWORD dwVendorId, DWORD dwDeviceId, WD_PCI_SLOT *pSlot) // Find a PCIE device
{
    DWORD dwStatus;
    DWORD i, dwNumDevices;
    WDC_PCI_SCAN_RESULT scanResult;

    if (dwVendorId == 0)
    {
        if (DIAG_INPUT_SUCCESS != DIAG_InputDWORD((PVOID)&dwVendorId,
                                                  "Enter vendor ID", TRUE, 0, 0))
        {
            return FALSE;
        }

        if (DIAG_INPUT_SUCCESS != DIAG_InputDWORD((PVOID)&dwDeviceId,
                                                  "Enter device ID", TRUE, 0, 0))
        {
            return FALSE;
        }
    }

    BZERO(scanResult);
    dwStatus = WDC_PciScanDevices(dwVendorId, dwDeviceId, &scanResult);
    if (WD_STATUS_SUCCESS != dwStatus)
    {
        PCIE_ERR("DeviceFind: Failed scanning the PCI bus.\n"
                 "Error: 0x%lx - %s\n",
                 dwStatus, Stat2Str(dwStatus));
        return FALSE;
    }

    dwNumDevices = scanResult.dwNumDevices;
    if (!dwNumDevices)
    {
        PCIE_ERR("No matching device was found for search criteria "
                 "(Vendor ID 0x%lX, Device ID 0x%lX)\n",
                 dwVendorId, dwDeviceId);

        return FALSE;
    }

    printf("\n");
    printf("Found %ld matching device%s [ Vendor ID 0x%lX%s, Device ID 0x%lX%s ]:\n",
           dwNumDevices, dwNumDevices > 1 ? "s" : "",
           dwVendorId, dwVendorId ? "" : " (ALL)",
           dwDeviceId, dwDeviceId ? "" : " (ALL)");

    for (i = 0; i < dwNumDevices; i++)
    {
        printf("\n");
        printf("%2ld. Vendor ID: 0x%lX, Device ID: 0x%lX\n",
               i + 1,
               scanResult.deviceId[i].dwVendorId,
               scanResult.deviceId[i].dwDeviceId);

        WDC_DIAG_PciDeviceInfoPrint(&scanResult.deviceSlot[i], FALSE);
    }
    printf("\n");

    if (dwNumDevices > 1)
    {
        sprintf(gsInput, "Select a device (1 - %ld): ", dwNumDevices);
        i = 0;
        if (DIAG_INPUT_SUCCESS != DIAG_InputDWORD((PVOID)&i,
                                                  gsInput, FALSE, 1, dwNumDevices))
        {
            return FALSE;
        }
    }

    *pSlot = scanResult.deviceSlot[i - 1];

    return TRUE;
}

static WDC_DEVICE_HANDLE DeviceOpen(const WD_PCI_SLOT *pSlot) // Open a handle to a PCIE device
{
    WDC_DEVICE_HANDLE hDev;
    DWORD dwStatus;
    WD_PCI_CARD_INFO deviceInfo;

    /* Retrieve the device's resources information */
    BZERO(deviceInfo);
    deviceInfo.pciSlot = *pSlot;
    dwStatus = WDC_PciGetDeviceInfo(&deviceInfo);
    if (WD_STATUS_SUCCESS != dwStatus)
    {
        PCIE_ERR("DeviceOpen: Failed retrieving the device's resources information.\n"
                 "Error 0x%lx - %s\n",
                 dwStatus, Stat2Str(dwStatus));
        return NULL;
    }

    /* NOTE: You can modify the device's resources information here, if
       necessary (mainly the deviceInfo.Card.Items array or the items number -
       deviceInfo.Card.dwItems) in order to register only some of the resources
       or register only a portion of a specific address space, for example. */

    /* Open a handle to the device */
    hDev = PCIE_DeviceOpen(&deviceInfo);
    if (!hDev)
    {
        PCIE_ERR("DeviceOpen: Failed opening a handle to the device: %s",
                 PCIE_GetLastErr());
        return NULL;
    }

    return hDev;
}

static void DeviceClose(WDC_DEVICE_HANDLE hDev) // Close handle to a PCIE device
{
    if (!hDev)
        return;

    if (!PCIE_DeviceClose(hDev))
    {
        PCIE_ERR("DeviceClose: Failed closing PCIE device: %s",
                 PCIE_GetLastErr());
    }
}

static void SetAddrSpace(WDC_DEVICE_HANDLE hDev, PDWORD pdwAddrSpace)
{
    DWORD dwAddrSpace;
    DWORD dwNumAddrSpaces = PCIE_GetNumAddrSpaces(hDev);
    PCIE_ADDR_SPACE_INFO addrSpaceInfo;

    printf("\n");
    printf("Select an active address space:\n");
    printf("-------------------------------\n");

    for (dwAddrSpace = 0; dwAddrSpace < dwNumAddrSpaces; dwAddrSpace++)
    {
        BZERO(addrSpaceInfo);
        addrSpaceInfo.dwAddrSpace = dwAddrSpace;
        if (!PCIE_GetAddrSpaceInfo(hDev, &addrSpaceInfo))
        {
            PCIE_ERR("SetAddrSpace: Error - Failed to get address space information: %s",
                     PCIE_GetLastErr());
            return;
        }

        printf("%ld. %-*s %-*s %s\n",
               dwAddrSpace + 1,
               MAX_NAME_DISPLAY, addrSpaceInfo.sName,
               MAX_TYPE - 1, addrSpaceInfo.sType,
               addrSpaceInfo.sDesc);
    }
    printf("\n");

    if (DIAG_INPUT_SUCCESS != DIAG_InputDWORD((PVOID)&dwAddrSpace,
                                              "Enter option", FALSE, 1, dwNumAddrSpaces))
    {
        return;
    }

    dwAddrSpace--;
    if (!WDC_AddrSpaceIsActive(hDev, dwAddrSpace))
    {
        printf("You have selected an inactive address space\n");
        return;
    }

    *pdwAddrSpace = dwAddrSpace;
}

static void DiagIntHandler(WDC_DEVICE_HANDLE hDev, PCIE_INT_RESULT *pIntResult) // Diagnostics interrupt handler routine
{
    /* TODO: You can modify this function in order to implement your own
             diagnostics interrupt handler routine */

    printf("Got interrupt number %ld\n", pIntResult->dwCounter);
    printf("Interrupt Type: %s\n",
           WDC_DIAG_IntTypeDescriptionGet(pIntResult->dwEnabledIntType));
    if (WDC_INT_IS_MSI(pIntResult->dwEnabledIntType))
        printf("Message Data: 0x%lx\n", pIntResult->dwLastMessage);
}

static int pcie_send(WDC_DEVICE_HANDLE hDev, int mode, int nword, UINT32 *buff_send)
{
/* imode =0 single word transfer, imode =1 DMA */
#include "wdc_defs.h"
    static DWORD dwAddrSpace;
    static DWORD dwDMABufSize;

    static UINT32 *buf_send;
    static WD_DMA *pDma_send;
    static DWORD dwStatus;
    static DWORD dwOptions_send = DMA_TO_DEVICE;
    static DWORD dwOffset;
    static UINT32 u32Data;
    static PVOID pbuf_send;
    int nwrite, i, j, iprint;
    static int ifr = 0;

    if (ifr == 0)
    {
        ifr = 1;
        dwDMABufSize = 140000;
        dwStatus = WDC_DMAContigBufLock(hDev, &pbuf_send, dwOptions_send, dwDMABufSize, &pDma_send);
        if (WD_STATUS_SUCCESS != dwStatus)
        {
            printf("Failed locking a send Contiguous DMA buffer. Error 0x%lx - %s\n", dwStatus, Stat2Str(dwStatus));
        }
        buf_send = pbuf_send;
    }
    iprint = 0;
    if (mode == 1)
    {
        for (i = 0; i < nword; i++)
        {
            *(buf_send + i) = *buff_send++;
            /*	printf("%d \n",*(buf_send+i));   */
        }
    }
    if (mode == 0)
    {
        nwrite = nword * 4;
        /*setup transmiiter */
        dwAddrSpace = 2;
        u32Data = 0x20000000;
        dwOffset = 0x18;
        WDC_WriteAddr32(hDev, dwAddrSpace, dwOffset, u32Data);
        dwAddrSpace = 2;
        u32Data = 0x40000000 + nwrite;
        dwOffset = 0x18;
        WDC_WriteAddr32(hDev, dwAddrSpace, dwOffset, u32Data);
        for (j = 0; j < nword; j++)
        {
            dwAddrSpace = 0;
            dwOffset = 0x0;
            u32Data = *buff_send++;
            WDC_WriteAddr32(hDev, dwAddrSpace, dwOffset, u32Data);
        }
        for (i = 0; i < 20000; i++)
        {
            dwAddrSpace = 2;
            dwOffset = 0xC;
            WDC_ReadAddr32(hDev, dwAddrSpace, dwOffset, &u32Data);
            if (iprint == 1)
                printf(" status reed %d %X \n", i, u32Data);
            if (((u32Data & 0x80000000) == 0) && iprint == 1)
                printf(" Data Transfer complete %d \n", i);
            if ((u32Data & 0x80000000) == 0)
                break;
        }
    }
    if (mode == 1)
    {
        nwrite = nword * 4;
        WDC_DMASyncCpu(pDma_send);
        /*
    printf(" nwrite = %d \n", nwrite);
    printf(" pcie_send hDev = %d\n", hDev);
    printf(" buf_send = %X\n",*buf_send);
        */
        /*setup transmiiter */
        dwAddrSpace = 2;
        u32Data = 0x20000000;
        dwOffset = 0x18;
        WDC_WriteAddr32(hDev, dwAddrSpace, dwOffset, u32Data);
        dwAddrSpace = 2;
        u32Data = 0x40000000 + nwrite;
        dwOffset = 0x18;
        WDC_WriteAddr32(hDev, dwAddrSpace, dwOffset, u32Data);

        /* set up sending DMA starting address */

        dwAddrSpace = 2;
        u32Data = 0x20000000;
        dwOffset = 0x0;
        u32Data = pDma_send->Page->pPhysicalAddr & 0xffffffff;
        WDC_WriteAddr32(hDev, dwAddrSpace, dwOffset, u32Data);

        dwAddrSpace = 2;
        u32Data = 0x20000000;
        dwOffset = 0x4;
        u32Data = (pDma_send->Page->pPhysicalAddr >> 32) & 0xffffffff;
        WDC_WriteAddr32(hDev, dwAddrSpace, dwOffset, u32Data);

        /* byte count */
        dwAddrSpace = 2;
        dwOffset = 0x8;
        u32Data = nwrite;
        WDC_WriteAddr32(hDev, dwAddrSpace, dwOffset, u32Data);

        /* write this will start DMA */
        dwAddrSpace = 2;
        dwOffset = 0xc;
        u32Data = 0x00100000;
        WDC_WriteAddr32(hDev, dwAddrSpace, dwOffset, u32Data);

        for (i = 0; i < 20000; i++)
        {
            dwAddrSpace = 2;
            dwOffset = 0xC;
            WDC_ReadAddr32(hDev, dwAddrSpace, dwOffset, &u32Data);
            if (iprint == 1)
                printf(" DMA status reed %d %X \n", i, u32Data);
            if (((u32Data & 0x80000000) == 0) && iprint == 1)
                printf(" DMA complete %d \n", i);
            if ((u32Data & 0x80000000) == 0)
                break;
        }
        WDC_DMASyncIo(pDma_send);
    }
    return i;
}

static int pcie_rec(WDC_DEVICE_HANDLE hDev, int mode, int istart, int nword, int ipr_status, UINT32 *buff_rec)
{
/* imode =0 single word transfer, imode =1 DMA */
#include "wdc_defs.h"
    static DWORD dwAddrSpace;
    static DWORD dwDMABufSize;

    static UINT32 *buf_rec;
    static WD_DMA *pDma_rec;
    static DWORD dwStatus;
    static DWORD dwOptions_rec = DMA_FROM_DEVICE;
    static DWORD dwOffset;
    static UINT32 u32Data;
    static UINT64 u64Data;
    static PVOID pbuf_rec;
    int nread, i, j, iprint, icomp;
    static int ifr = 0;

    if (ifr == 0)
    {
        ifr = 1;
        dwDMABufSize = 140000;
        dwStatus = WDC_DMAContigBufLock(hDev, &pbuf_rec, dwOptions_rec, dwDMABufSize, &pDma_rec);
        if (WD_STATUS_SUCCESS != dwStatus)
        {
            printf("Failed locking a send Contiguous DMA buffer. Error 0x%lx - %s\n", dwStatus, Stat2Str(dwStatus));
        }
        buf_rec = pbuf_rec;
    }
    iprint = 0;
    //    printf(" istart = %d\n", istart);
    //   printf(" mode   = %d\n", mode);
    /** set up the receiver **/
    if ((istart == 1) | (istart == 3))
    {
        // initalize transmitter mode register...
        printf(" nword = %d \n", nword);
        /*
             if(ipr_status ==1) {
              dwAddrSpace =2;
              u64Data =0;
              dwOffset = 0x18;
              WDC_ReadAddr64(hDev, dwAddrSpace, dwOffset, &u64Data);
              printf (" status word before set = %x, %x \n",(u64Data>>32), (u64Data &0xffff));
             }
        */
        dwAddrSpace = 2;
        u32Data = 0xf0000008;
        dwOffset = 0x28;
        WDC_WriteAddr32(hDev, dwAddrSpace, dwOffset, u32Data);

        /*initialize the receiver */
        dwAddrSpace = 2;
        u32Data = 0x20000000;
        dwOffset = 0x1c;
        WDC_WriteAddr32(hDev, dwAddrSpace, dwOffset, u32Data);
        /* write byte count **/
        dwAddrSpace = 2;
        u32Data = 0x40000000 + nword * 4;
        dwOffset = 0x1c;
        WDC_WriteAddr32(hDev, dwAddrSpace, dwOffset, u32Data);
        if (ipr_status == 1)
        {
            dwAddrSpace = 2;
            u64Data = 0;
            dwOffset = 0x18;
            WDC_ReadAddr64(hDev, dwAddrSpace, dwOffset, &u64Data);
            printf(" status word before read = %I64u, %I64u \n", (u64Data >> 32), (u64Data & 0xffff)); // error, expects unsigned int but gets UINT64
        }

        return 0;
    }
    if ((istart == 2) | (istart == 3))
    {
        //     if(ipr_status ==1) {
        //      dwAddrSpace =2;
        //      u64Data =0;
        //      dwOffset = 0x18;
        //      WDC_ReadAddr64(hDev, dwAddrSpace, dwOffset, &u64Data);
        //      printf (" status word before read = %x, %x \n",(u64Data>>32), (u64Data &0xffff));
        //     }
        if (mode == 0)
        {
            nread = nword / 2 + 1;
            if (nword % 2 == 0)
                nread = nword / 2;
            for (j = 0; j < nread; j++)
            {
                dwAddrSpace = 0;
                dwOffset = 0x0;
                u64Data = 0xbad;
                WDC_ReadAddr64(hDev, dwAddrSpace, dwOffset, &u64Data);
                //       printf("u64Data = %16X\n",u64Data);
                *buff_rec++ = (u64Data & 0xffffffff);
                *buff_rec++ = u64Data >> 32;
                //       printf("%x \n",(u64Data &0xffffffff));
                //       printf("%x \n",(u64Data >>32 ));
                //       if(j*2+1 > nword) *buff_rec++ = (u64Data)>>32;
                //       *buff_rec++ = 0x0;
            }
            if (ipr_status == 1)
            {
                dwAddrSpace = 2;
                u64Data = 0;
                dwOffset = 0x18;
                WDC_ReadAddr64(hDev, dwAddrSpace, dwOffset, &u64Data);
                printf(" status word after read = %x, %x \n", (u64Data >> 32), (u64Data & 0xffff));
            }
            return 0;
        }
        if (mode == 1)
        {
            nread = nword * 4;
            WDC_DMASyncCpu(pDma_rec);
            /*
                  printf(" nwrite = %d \n", nwrite);
                  printf(" pcie_send hDev = %d\n", hDev);
                  printf(" buf_send = %X\n",*buf_send);
            */
            /*setup receiver
                  dwAddrSpace =2;
                  u32Data = 0x20000000;
                  dwOffset = 0x1c;
                  WDC_WriteAddr32(hDev, dwAddrSpace, dwOffset, u32Data);
                  dwAddrSpace =2;
                  u32Data = 0x40000000+nread;
                  dwOffset = 0x1c;
                  WDC_WriteAddr32(hDev, dwAddrSpace, dwOffset, u32Data);
            */
            /* set up sending DMA starting address */

            dwAddrSpace = 2;
            u32Data = 0x20000000;
            dwOffset = 0x0;
            u32Data = pDma_rec->Page->pPhysicalAddr & 0xffffffff;
            WDC_WriteAddr32(hDev, dwAddrSpace, dwOffset, u32Data);

            dwAddrSpace = 2;
            u32Data = 0x20000000;
            dwOffset = 0x4;
            u32Data = (pDma_rec->Page->pPhysicalAddr >> 32) & 0xffffffff;
            WDC_WriteAddr32(hDev, dwAddrSpace, dwOffset, u32Data);

            /* byte count */
            dwAddrSpace = 2;
            dwOffset = 0x8;
            u32Data = nread;
            WDC_WriteAddr32(hDev, dwAddrSpace, dwOffset, u32Data);

            /* write this will start DMA */
            dwAddrSpace = 2;
            dwOffset = 0xc;
            u32Data = 0x00100040;
            WDC_WriteAddr32(hDev, dwAddrSpace, dwOffset, u32Data);
            icomp = 0;
            for (i = 0; i < 20000; i++)
            {
                dwAddrSpace = 2;
                dwOffset = 0xC;
                WDC_ReadAddr32(hDev, dwAddrSpace, dwOffset, &u32Data);
                if (iprint == 1)
                    printf(" DMA status read %d %X \n", i, u32Data);
                if (((u32Data & 0x80000000) == 0))
                {
                    icomp = 1;
                    if (iprint == 1)
                        printf(" DMA complete %d \n", i);
                }
                if ((u32Data & 0x80000000) == 0)
                    break;
            }
            if (icomp == 0)
            {
                printf("DMA timeout\n");
                return 1;
            }
		    WDC_DMASyncIo(pDma_rec);
		    for (i = 0; i < nword; i++)
		    {
			*buff_rec++ = *(buf_rec + i);
			/*	printf("%d \n",*(buf_send+i));   */
		    }
		}
	    }
	    return 0;
	}

	enum
	{
	    MENU_RW_ADDR_SET_ADDR_SPACE = 1,
	    MENU_RW_ADDR_SET_MODE,
	    MENU_RW_ADDR_SET_TRANS_TYPE,
	    MENU_RW_ADDR_READ,
	    MENU_RW_ADDR_WRITE,
	    MENU_RW_ADDR_EXIT = DIAG_EXIT_MENU,
	};

	#define ACTIVE_ADDR_SPACE_NEEDS_INIT 0xFF

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

	static int power(int a, int b)
	{

	    if (b == 0)
		return 1;
	    if (a == 0)
		return 0;
	    if (b % 2 == 0)
	    {
		return power(a * a, b / 2);
	    }
	    else if (b % 2 == 1)
	    {
		return a * power(a * a, b / 2);
	    }
	    return 0;
	}

	static void changemode(int dir)
	{
	    static struct termios oldt, newt;

	    if (dir == 1)
	    {
		tcgetattr(STDIN_FILENO, &oldt);
		newt = oldt;
		newt.c_lflag &= ~(ICANON | ECHO);
		tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	    }
	    else
		tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	}

	/*static void boot_xmit(WDC_DEVICE_HANDLE *hDev, int imod_xmit_in, int dmabufsize, bool print_debug)
	{

	    printf("$$$$$ boot_xmit dmabufsize %i\n", dmabufsize);

	    ichip = 0x2;
	    buf_send[0] = 0x0;
	    buf_send[1] = 0x0;
	    px = &buf_send[0];
	    UINT32 read_array[dmabufsize];

	    inpf = fopen("/home/ub/WinDriver/wizard/GRAMS_project_am/uBooNE_Nominal_Firmware/xmit/readcontrol_110601_v3_play_header_hist_l1block_9_21_2018.rbf", "r"); // tpc readout akshay
	    // inpf = fopen("/home/ub/WinDriver/wizard/GRAMS_project_am/uBooNE_Nominal_Firmware/xmit/readcontrol_11601_v3_play_header_6_21_2013.rbf", "r");//tpc readout akshay

	    printf("\nBooting XMIT module...\n\n");
	    imod = imod_xmit_in;
	    ichip = mb_xmit_conf_add;
	    buf_send[0] = (imod << 11) + (ichip << 8) + 0x0 + (0x0 << 16); // turn conf to be on
	    i = 1;
	    k = 1;
	    i = pcie_send(hDev, i, k, px);

	    usleep(1000); // wait for a while
	    count = 0;
	    counta = 0;
	    nsend = 500;
	    ichip_c = 7; // set ichip_c to stay away from any other command in the
	    nword = 1;
	    imod = imod_xmit_in;
	    dummy1 = 0;

	    if (print_debug == true)
	    {
		printf("\n boot_xmit debug : \n");
		printf("dmabufsize %x\n", dmabufsize);
		printf("imod_xmit_in %x\n", imod_xmit_in);
		printf("mb_xmit_conf_add %x\n", mb_xmit_conf_add);
	    }

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
			printf("\tcounta = %d, first word = %x, %x, %x %x %x \n", counta, buf_send[0], carray[0], carray[1], carray[2], carray[3]);

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

	    nword = 1;
	    i = pcie_rec(hDev, 0, 1, nword, iprint, py); // init the receiver
	    nword = 1;
	    i = pcie_rec(hDev, 0, 1, nword, iprint, py); // init the receiver
	    imod = imod_xmit_in;
	    ichip = 3;
	    buf_send[0] = (imod << 11) + (ichip << 8) + mb_xmit_rdstatus + (0x0 << 16); // read out status

	    i = 1; // This is status read
	    k = 1;
	    i = pcie_send(hDev, i, k, px);
	    py = &read_array[0];
	    i = pcie_rec(hDev, 0, 2, nword, iprint, py); // read out 2 32 bits words
	    printf("XMIT status word = %x, %x \n", read_array[0], read_array[1]);
	}

	static void setup_xmit(WDC_DEVICE_HANDLE *hDev, int imod_xmit_in, int dmabufsize, bool print_debug)
	{

	    UINT32 read_array[dmabufsize];

	    imod_xmit = imod_xmit_in;
	    printf("Setting up XMIT module\n");
	    imod = imod_xmit; // XMIT setup
	    ichip = 3;
	    buf_send[0] = (imod << 11) + (ichip << 8) + mb_xmit_modcount + ((imod_st - imod_xmit - 1) << 16); //                  -- number of FEM module -1, counting start at 0
	    i = 1;
	    k = 1;
	    i = pcie_send(hDev, i, k, px);

	    imod = imod_xmit; //     reset optical
	    ichip = 3;
	    buf_send[0] = (imod << 11) + (ichip << 8) + mb_opt_dig_reset + (0x1 << 16); // set optical reset on
	    i = 1;
	    k = 1;
	    i = pcie_send(hDev, i, k, px);

	    imod = imod_xmit; //     enable Neutrino Token Passing
	    ichip = 3;
	    buf_send[0] = (imod << 11) + (ichip << 8) + mb_xmit_enable_1 + (0x1 << 16); // enable token 1 pass
	    i = 1;
	    k = 1;
	    i = pcie_send(hDev, i, k, px);

	    buf_send[0] = (imod << 11) + (ichip << 8) + mb_xmit_enable_2 + (0x0 << 16); // disable token 2 pass
	    i = 1;
	    k = 1;
	    i = pcie_send(hDev, i, k, px);

	    imod = imod_xmit; //       reset XMIT LINK IN DPA
	    ichip = 3;
	    buf_send[0] = (imod << 11) + (ichip << 8) + mb_xmit_link_pll_reset + (0x1 << 16); //  reset XMIT LINK IN DPA
	    i = 1;
	    k = 1;
	    i = pcie_send(hDev, i, k, px);
	    usleep(1000);

	    imod = imod_xmit; //     reset XMIT LINK IN DPA
	    ichip = 3;
	    buf_send[0] = (imod << 11) + (ichip << 8) + mb_xmit_link_reset + (0x1 << 16); //  reset XMIT LINK IN DPA
	    i = 1;
	    k = 1;
	    i = pcie_send(hDev, i, k, px);

	    usleep(10000); //     wait for 10ms just in case

	    imod = imod_xmit; //     reset XMIT FIFO reset
	    ichip = 3;
	    buf_send[0] = (imod << 11) + (ichip << 8) + mb_xmit_dpa_fifo_reset + (0x1 << 16); //  reset XMIT LINK IN DPA
	    i = 1;
	    k = 1;
	    i = pcie_send(hDev, i, k, px);

	    imod = imod_xmit; //      test re-align circuit
	    ichip = 3;
	    buf_send[0] = (imod << 11) + (ichip << 8) + mb_xmit_dpa_word_align + (0x1 << 16); //  send alignment pulse
	    i = 1;
	    k = 1;
	    i = pcie_send(hDev, i, k, px);
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
	    printf("\n...XMIT setup complete\n");

	    if (print_debug == true)
	    {
		printf("\n setup_xmit debug : \n");
		printf("dmabufsize %x\n", dmabufsize);
		printf("imod_xmit_in %x\n", imod_xmit_in);
		printf("mb_xmit_modcount %x\n", mb_xmit_modcount);
		printf("mb_opt_dig_reset %x\n", mb_opt_dig_reset);
		printf("mb_xmit_enable_1 %x\n", mb_xmit_enable_1);
		printf("mb_xmit_link_pll_reset %x\n", mb_xmit_link_pll_reset);
		printf("mb_xmit_link_reset %x\n", mb_xmit_link_reset);
		printf("mb_xmit_dpa_fifo_reset %x\n", mb_xmit_dpa_fifo_reset);
		printf("mb_xmit_dpa_word_align %x\n", mb_xmit_dpa_word_align);
		printf("mb_xmit_rdstatus %x\n", mb_xmit_rdstatus);
		printf("mb_xmit_rdstatus %x\n", mb_xmit_rdstatus);
	    }
	}

	static int boot_fems(WDC_DEVICE_HANDLE hDev, int imod_xmit_in, int imod_st, bool fem_type, bool print_debug)
	{

	    printf("\n\nBooting FEBs...\n");
	    imod_xmit = imod_xmit_in;
	    for (imod_fem = (imod_xmit + 1); imod_fem < (imod_st + 1); imod_fem++)
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
		FILE *inpf_tpc = fopen("/home/ub/WinDriver/wizard/GRAMS_project_am/uBooNE_Nominal_Firmware/fem/module1x_140820_deb_fixbase_nf_8_31_2018.rbf", "r");
		FILE *inpf_pmt = fopen("/home/ub/WinDriver/wizard/GRAMS_project/uBooNE_Nominal_Firmware/pmt_fem/module1x_pmt_64MHz_new_head_07162013.rbf", "r");
		inpf = fem_type ? inpf_tpc : inpf_pmt;
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

	    if (print_debug == true)
	    {
		printf("\n fem boot debug : \n\n");
		printf("hDev %x\n", &hDev);
		printf("imod_fem %x\n", imod_fem);
		printf("imod_xmit_in %x\n", imod_xmit_in);
		printf("mb_feb_power_add %x\n", mb_feb_power_add);
		printf("mb_feb_conf_add %x\n", mb_feb_conf_add);
	    }
	    printf("\n...FEB booting done \n");
	}

	static int setup_fems(WDC_DEVICE_HANDLE hDev, int imod_xmit, int imod_st, int fem_slot, bool fem_type, FILE *outfile, int dmabufsize)
	{
	    bool fem_set_done = fem_type ? setup_tpc_fem(hDev, imod_xmit, imod_st, fem_slot, fem_type) : setup_pmt_fem(hDev, fem_slot, fem_type, 1599, outfile, dmabufsize);
	    printf("FEM in slot %i setup complete", fem_slot);
	    return 0;
	}

	static int setup_tpc_fem(WDC_DEVICE_HANDLE hDev, int imod_xmit, int imod_st, int fem_slot, bool fem_type)
	{
	    int adcdata = 1;
	    int fakeadcdata = 0;
	    int femfakedata = 0;
	    printf("\nSetting up TPC FEB's...\n");
	    for (imod_fem = (imod_xmit + 1); imod_fem < (imod_st + 1); imod_fem++)
	    {
		imod = imod_fem;
		ichip = 3;
		buf_send[0] = (imod << 11) + (ichip << 8) + mb_feb_dram_reset + (0x1 << 16); // reset FEB DRAM (step 1) // turm the DRAM reset on
		i = 1;
		k = 1;
		i = pcie_send(hDev, i, k, px);

		ichip = 3;
		buf_send[0] = (imod << 11) + (ichip << 8) + mb_feb_dram_reset + (0x0 << 16); // reset FEB DRAM (step 2) // turm the DRAM reset off
		i = 1;
		k = 1;
		i = pcie_send(hDev, i, k, px);
		usleep(5000); // wait for 5 ms for DRAM to be initialized

		ichip = 3;                                                                    // set module number
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

		    for (is = 0; is < 8; is++) // Set ADC address 2
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
			if (fakeadcdata == 1)
			    buf_send[1] = (((0x0) << 13) + (0xd)) + ((0xc) << 24) + ((0x0) << 16);
			else
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

		// set test mode
		ichip = mb_feb_pass_add;
		if (femfakedata == 1)
		    buf_send[0] = (imod << 11) + (ichip << 8) + mb_feb_test_source + (0x2 << 16); // set test source to 2
		else
		    buf_send[0] = (imod << 11) + (ichip << 8) + mb_feb_test_source + (0x0 << 16);
		i = 1;
		k = 1;
		i = pcie_send(hDev, i, k, px);

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
	    return 0;
	}

	static int setup_pmt_fem(WDC_DEVICE_HANDLE hDev, int imod_fem, int iframe_length, int mode, FILE *outinfo, int dmabufsize)
	{
	    UINT32 read_array[dmabufsize];

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


	    printf("mb_feb_pmt_precount %x\n", mb_feb_pmt_precount);
	    printf("mb_feb_pmt_delay1 %x\n", mb_feb_pmt_delay1);
	    printf("mb_feb_pmt_delay0 %x\n", mb_feb_pmt_delay0);
	    printf("mb_feb_pmt_ch_set %x\n", mb_feb_pmt_ch_set);

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
	    return 0;
	}

	static int setup_trig(WDC_DEVICE_HANDLE hDev, int trig_slot)
	{


	    imod = 0; // set offline test
	    ichip = 1;
	    buf_send[0] = (imod << 11) + (ichip << 8) + (mb_cntrl_test_on) + (0x0 << 16); // enable offline run on
	    i = 1;
	    k = 1;
	    i = pcie_send(hDev, i, k, px);

	    imod = imod_trig;
	    buf_send[0] = (imod << 11) + (mb_trig_run) + ((0x0) << 16); // set up trigger module run off
	    i = 1;
	    k = 1;
	    i = pcie_send(hDev, i, k, px);

	    imod = imod_trig;
	    buf_send[0] = (imod << 11) + (mb_trig_deadtime_size) + ((250 & 0xff) << 16); // set trigger module deadtime size
	    i = 1;
	    k = 1;
	    i = pcie_send(hDev, i, k, px);

	    imod = 0; // set offline test
	    ichip = 1;
	    buf_send[0] = (imod << 11) + (ichip << 8) + (mb_cntrl_test_off) + (0x0 << 16); // set controller test off
	    i = 1;
	    k = 1;
	    i = pcie_send(hDev, i, k, px);

	    imod = imod_trig; // Set number of ADC samples to (iframe+1)/8;
	    iframe = iframe_length;
	    buf_send[0] = (imod << 11) + (mb_trig_frame_size) + ((iframe & 0xffff) << 16);
	    i = 1;
	    k = 1;
	    i = pcie_send(hDev, i, k, px);

	    imod = 0; // load trig 1 position relative to the frame..
	    ichip = 1;
	    buf_send[0] = (imod << 11) + (ichip << 8) + (mb_cntrl_load_trig_pos) + ((itrig_delay & 0xffff) << 16); // enable test mode
	    i = 1;
	    k = 1;
	    i = pcie_send(hDev, i, k, px);

	    imod = imod_trig;
	    buf_send[0] = (imod << 11) + (mb_trig_mask8) + (0x2 << 16); // set mask1[3] on.
	    i = 1;
	    k = 1;
	    i = pcie_send(hDev, i, k, px);

	    imod = imod_trig;
	    buf_send[0] = (imod << 11) + (mb_trig_prescale8) + (0x0 << 16); // set prescale1 0
	    i = 1;
	    k = 1;
	    i = pcie_send(hDev, i, k, px);
	    return 0;
	}

	static int link_fems(WDC_DEVICE_HANDLE hDev, int imod_xmit, int imod_st, int dmabufsize)
	{

	    UINT32 read_array[dmabufsize];

	    for (imod_fem = (imod_st - 1); imod_fem > imod_xmit; imod_fem--) //     now reset all the link port receiver PLL
	    {
		printf("%i \n", imod_fem);
		imod = imod_fem;
		printf("Resetting link PLL for module %x \n", imod);
		ichip = 4;
		buf_send[0] = (imod << 11) + (ichip << 8) + mb_feb_pll_reset + (0x0 << 16); // reset LINKIN PLL
		i = 1;
		k = 1;
		i = pcie_send(hDev, i, k, px);
		usleep(1000); // give PLL time to reset
	    }
	    for (imod_fem = (imod_xmit + 1); imod_fem < (imod_st + 1); imod_fem++) // read back status
	    {
		printf("%i \n", imod_fem);

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
	    for (imod_fem = (imod_st - 1); imod_fem > imod_xmit; imod_fem--)
	    {
		printf("%i \n", imod_fem);
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
	    return 0;
	}
	*/
	/* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ FUNCTION DEFINATIONS 2^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
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

	enum
	{
	    MENU_MAIN_SCAN_PCI_BUS = 1,
	    MENU_MAIN_FIND_AND_OPEN,
	    MENU_MAIN_RW_ADDR,
	    MENU_MAIN_RW_CFG_SPACE,
	    MENU_MAIN_RW_REGS,
	    MENU_MAIN_ENABLE_DISABLE_INT,
	    MENU_MAIN_EVENTS,
	    MENU_MAIN_MB_TEST,     /* add new route for testing */
	    MENU_MAIN_JSEBII_TEST, /* add new route for testing */
	    MENU_MAIN_EXIT = DIAG_EXIT_MENU,
	};
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

	enum
	{
	    MENU_INT_ENABLE_DISABLE = 1,
	    MENU_INT_EXIT = DIAG_EXIT_MENU,
	};
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

	enum
	{
	    MENU_RW_REGS_READ_ALL = 1,
	    MENU_RW_REGS_READ_REG,
	    MENU_RW_REGS_WRITE_REG,
	    MENU_RW_REGS_EXIT = DIAG_EXIT_MENU,
	};
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

	enum
	{
	    MENU_EVENTS_REGISTER_UNREGISTER = 1,
	    MENU_EVENTS_EXIT = DIAG_EXIT_MENU,
	};
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

	enum
	{
	    MENU_RW_CFG_SPACE_READ_OFFSET = 1,
	    MENU_RW_CFG_SPACE_WRITE_OFFSET,
	    MENU_RW_CFG_SPACE_READ_ALL_REGS,
	    MENU_RW_CFG_SPACE_READ_REG,
	    MENU_RW_CFG_SPACE_WRITE_REG,
	    MENU_RW_CFG_SPACE_EXIT = DIAG_EXIT_MENU,
	};
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

	/* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ MBTEST  ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
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
	    static int imod_fem, imod_st, imod_last, itrig_type, last_dma_loop_size, imod_st1, imod_st2;
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
	    static UINT32 read_array[dma_buffer_size];
	    // static int idelay0, idelay1, threshold0, threshold1hg, threshold1lg, pmt_words, pmt_wordshg, pmt_wordslg;
	    static int threshold3hg, threshold3lg, threshold1hg, threshold1lg;
	    static int threshold3;
	    // static int cos_mult, cos_thres, beam_mult, beam_thres, en_top, en_upper, en_lower,
	    static int lg_ch, hg_ch, trig_ch, bg_ch, beam_mult, beam_thres, pmt_wordslg, pmt_wordshg, pmt_precount;

	    // static int irise, ifall, istart_time, use_pmt, pmt_testpulse, pmt_precount;
	    //  static int   ich_head, ich_sample, ich_frm,idebug,ntot_rec,nred;
	    //  static int   ineu,ibusy_send,ibusy_test,ihold_word,ndma_loop;
	    //  static int   irawprint,ifem_fst,ifem_lst,ifem_loop;
	    static int pmt_deadtimehg, pmt_deadtimelg, pmt_mich_window, bg, bge, tre, beam_size, pmt_width;
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
	    ifr = 0;
	    iwrite = 1;
	    iprint = 0;
	    icheck = 0;
	    istop = 0;
	    iwrited = 0;
	    dwDMABufSize = 20000;
	    bool print_debug = true;

	    /* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ MISC...  ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */
	    newcmd = 1;
	    mask1 = 0x0;
	    mask8 = 0x2;
	    struct timeval;
	    time_t rawtime;
	    char timestr[500];
	    time(&rawtime);
	    strftime(timestr, sizeof(timestr), "%Y_%m_%d", localtime(&rawtime));
	    printf("\n ######## PMT/TPC XMIT Readout (keyboard interrupt)\n\n Enter SUBRUN NUMBER or NAME:\t");
	    char subrun[50];
	    scanf("%s", &subrun);
	    printf("\nEnter desired DMA size (<%d)\t", dwDMABufSize);
	    scanf("%d", &ibytec);
	    dwDMABufSize = ibytec;
	    // UINT32 read_array[dwDMABufSize];
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
	    imod_pmt = 8;
	    imod_st1 = 8;
	    imod_st2 = 7;
	    // imod_st = 7;
	    imod_tpc = 7;
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
	    itrig_c = 0;
	    itrig_ext = 1;

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
	    inpf = fopen("/home/ub/WinDriver/wizard/GRAMS_project_am/uBooNE_Nominal_Firmware/xmit/readcontrol_110601_v3_play_header_hist_l1block_9_21_2018.rbf", "r"); // tpc readout akshay
	    // inpf = fopen("/home/ub/WinDriver/wizard/GRAMS_project_am/uBooNE_Nominal_Firmware/xmit/readcontrol_11601_v3_play_header_6_21_2013.rbf", "r");//tpc readout akshay

	    printf("\nBooting XMIT module...\n\n");
	    imod = imod_xmit;
	    ichip = mb_xmit_conf_add;
	    buf_send[0] = (imod << 11) + (ichip << 8) + 0x0 + (0x0 << 16); // turn conf to be on
	    i = 1;
	    k = 1;
	    i = pcie_send(hDev, i, k, px);

	    usleep(1000); // wait for a while
	    count = 0;
	    counta = 0;
	    nsend = 500;
	    ichip_c = 7; // set ichip_c to stay away from any other command in the
	    nword = 1;
	    imod = imod_xmit;
	    dummy1 = 0;

	    if (print_debug == true)
	    {
		printf("\n boot_xmit debug : \n");
		// printf("dmabufsize %x\n", dmabufsize);
		// printf("imod_xmit_in %x\n", imod_xmit_in);
		printf("mb_xmit_conf_add %x\n", mb_xmit_conf_add);
	    }

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
			printf("\tcounta = %d, first word = %x, %x, %x %x %x \n", counta, buf_send[0], carray[0], carray[1], carray[2], carray[3]);

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

	    nword = 1;
	    i = pcie_rec(hDev, 0, 1, nword, iprint, py); // init the receiver
	    nword = 1;
	    i = pcie_rec(hDev, 0, 1, nword, iprint, py); // init the receiver
	    imod = imod_xmit;
	    ichip = 3;
	    buf_send[0] = (imod << 11) + (ichip << 8) + mb_xmit_rdstatus + (0x0 << 16); // read out status

	    i = 1; // This is status read
	    k = 1;
	    i = pcie_send(hDev, i, k, px);
	    py = &read_array[0];
	    i = pcie_rec(hDev, 0, 2, nword, iprint, py); // read out 2 32 bits words
	    printf("XMIT status word = %x, %x \n", read_array[0], read_array[1]);
	    // printf("\nDo you want to read out TPC or PMT? Enter 0 for TPC, 1 for PMT, 2 for both\n");
	    // scanf("%i", &readtype);

	    /* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ XMIT SETTUP  ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */

	    // UINT32 read_array[dmabufsize];
	    printf("Setting up XMIT module\n");
	    imod = imod_xmit; // XMIT setup
	    ichip = 3;
	    buf_send[0] = (imod << 11) + (ichip << 8) + mb_xmit_modcount + ((imod_st1 - imod_xmit - 1) << 16); //                  -- number of FEM module -1, counting start at 0
	    i = 1;
	    k = 1;
	    i = pcie_send(hDev, i, k, px);

	    imod = imod_xmit; //     reset optical
	    ichip = 3;
	    buf_send[0] = (imod << 11) + (ichip << 8) + mb_opt_dig_reset + (0x1 << 16); // set optical reset on
	    i = 1;
	    k = 1;
	    i = pcie_send(hDev, i, k, px);

	    imod = imod_xmit; //     enable Neutrino Token Passing
	    ichip = 3;
	    buf_send[0] = (imod << 11) + (ichip << 8) + mb_xmit_enable_1 + (0x1 << 16); // enable token 1 pass
	    i = 1;
	    k = 1;
	    i = pcie_send(hDev, i, k, px);

	    buf_send[0] = (imod << 11) + (ichip << 8) + mb_xmit_enable_2 + (0x0 << 16); // disable token 2 pass
	    i = 1;
	    k = 1;
	    i = pcie_send(hDev, i, k, px);

	    imod = imod_xmit; //       reset XMIT LINK IN DPA
	    ichip = 3;
	    buf_send[0] = (imod << 11) + (ichip << 8) + mb_xmit_link_pll_reset + (0x1 << 16); //  reset XMIT LINK IN DPA
	    i = 1;
	    k = 1;
	    i = pcie_send(hDev, i, k, px);
	    usleep(1000);

	    imod = imod_xmit; //     reset XMIT LINK IN DPA
	    ichip = 3;
	    buf_send[0] = (imod << 11) + (ichip << 8) + mb_xmit_link_reset + (0x1 << 16); //  reset XMIT LINK IN DPA
	    i = 1;
	    k = 1;
	    i = pcie_send(hDev, i, k, px);

	    usleep(10000); //     wait for 10ms just in case

	    imod = imod_xmit; //     reset XMIT FIFO reset
	    ichip = 3;
	    buf_send[0] = (imod << 11) + (ichip << 8) + mb_xmit_dpa_fifo_reset + (0x1 << 16); //  reset XMIT LINK IN DPA
	    i = 1;
	    k = 1;
	    i = pcie_send(hDev, i, k, px);

	    imod = imod_xmit; //      test re-align circuit
	    ichip = 3;
	    buf_send[0] = (imod << 11) + (ichip << 8) + mb_xmit_dpa_word_align + (0x1 << 16); //  send alignment pulse
	    i = 1;
	    k = 1;
	    i = pcie_send(hDev, i, k, px);
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
	    printf("\n...XMIT setup complete\n");

	    if (print_debug == true)
	    {
		printf("\n setup_xmit debug : \n");
		// printf("dmabufsize %x\n", dmabufsize);
		// printf("imod_xmit_in %x\n", imod_xmit_in);
		printf("mb_xmit_modcount %x\n", mb_xmit_modcount);
		printf("mb_opt_dig_reset %x\n", mb_opt_dig_reset);
		printf("mb_xmit_enable_1 %x\n", mb_xmit_enable_1);
		printf("mb_xmit_link_pll_reset %x\n", mb_xmit_link_pll_reset);
		printf("mb_xmit_link_reset %x\n", mb_xmit_link_reset);
		printf("mb_xmit_dpa_fifo_reset %x\n", mb_xmit_dpa_fifo_reset);
		printf("mb_xmit_dpa_word_align %x\n", mb_xmit_dpa_word_align);
		printf("mb_xmit_rdstatus %x\n", mb_xmit_rdstatus);
		printf("mb_xmit_rdstatus %x\n", mb_xmit_rdstatus);
	    }
	    /* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ FEM BOOT  ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */

	    // pmt_adc_setup(hDev, imod_pmt, iframe_length, 1, outinfo);

        static int mode = 0;
        iframe = iframe_length;
        imod_fem = imod_pmt;

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
    //inpf = fopen("/home/uboonedaq/pmt_fpga_code/module1x_pmt_64MHz_new_head_07162013.rbf","r");
    inpf = fopen("/home/ub/WinDriver/wizard/GRAMS_project/uBooNE_Nominal_Firmware/pmt_fem/module1x_pmt_64MHz_new_head_07162013.rbf", "r");
    //inpf = fopen("/home/ub/module1x_pmt_64MHz_link_4192013.rbf", "r");
    //inpf = fopen("/home/ub/module1x_pmt_64MHz_new_head_12192013_allch.rbf","r");
    //inpf = fopen("/home/ub/module1x_pmt_64MHz_fermi_08032012_2.rbf","r");
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



	    // //imod_xmit = imod_xmit + 1;
	    for (imod_fem = (imod_tpc); imod_fem < (imod_st2 + 1); imod_fem++)
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
		FILE *inpf = fopen("/home/ub/WinDriver/wizard/GRAMS_project_am/uBooNE_Nominal_Firmware/fem/module1x_140820_deb_fixbase_nf_8_31_2018.rbf", "r");
		// FILE *inpf = fopen("/home/ub/WinDriver/wizard/GRAMS_project/uBooNE_Nominal_Firmware/pmt_fem/module1x_pmt_64MHz_new_head_07162013.rbf", "r");
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

	    // if (print_debug == true)
	    // {
	    //     printf("\n fem boot debug : \n\n");
	    //     printf("hDev %x\n", &hDev);
	    //     printf("imod_fem %x\n", imod_fem);
	    //     //printf("imod_xmit_in %x\n", imod_xmit_in);
	    //     printf("mb_feb_power_add %x\n", mb_feb_power_add);
	    //     printf("mb_feb_conf_add %x\n", mb_feb_conf_add);
	    // }

	    // imod_xmit = imod_xmit - 1;

	    printf("\n...FEB booting done \n");

	    /* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ FEM SETTUP  ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */

	    // // settup TPC FEM
	    int adcdata = 1;
	    int fakeadcdata = 0;
	    int femfakedata = 0;
	    // imod_xmit = imod_xmit+1;
	    printf("\nSetting up TPC FEB's...\n");
	    for (imod_fem = (imod_tpc); imod_fem < (imod_st2 + 1); imod_fem++)
	    {
		imod = imod_fem;
		ichip = 3;
		buf_send[0] = (imod << 11) + (ichip << 8) + mb_feb_dram_reset + (0x1 << 16); // reset FEB DRAM (step 1) // turm the DRAM reset on
		i = 1;
		k = 1;
		i = pcie_send(hDev, i, k, px);

		ichip = 3;
		buf_send[0] = (imod << 11) + (ichip << 8) + mb_feb_dram_reset + (0x0 << 16); // reset FEB DRAM (step 2) // turm the DRAM reset off
		i = 1;
		k = 1;
		i = pcie_send(hDev, i, k, px);
		usleep(5000); // wait for 5 ms for DRAM to be initialized

		ichip = 3;                                                                    // set module number
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

		    for (is = 0; is < 8; is++) // Set ADC address 2
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
			if (fakeadcdata == 1)
			    buf_send[1] = (((0x0) << 13) + (0xd)) + ((0xc) << 24) + ((0x0) << 16);
			else
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

		// set test mode
		ichip = mb_feb_pass_add;
		if (femfakedata == 1)
		    buf_send[0] = (imod << 11) + (ichip << 8) + mb_feb_test_source + (0x2 << 16); // set test source to 2
		else
		    buf_send[0] = (imod << 11) + (ichip << 8) + mb_feb_test_source + (0x0 << 16);
		i = 1;
		k = 1;
		i = pcie_send(hDev, i, k, px);

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
		if (imod == imod_st1)
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
	    // imod_xmit = imod_xmit-1;

	    /* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ TRIGGER_SETUP  ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */

	    /* FROM TPC PART*/
	    imod = 0; // set offline test
	    ichip = 1;
	    buf_send[0] = (imod << 11) + (ichip << 8) + (mb_cntrl_test_on) + (0x0 << 16); // enable offline run on
	    i = 1;
	    k = 1;
	    i = pcie_send(hDev, i, k, px);

	    imod = imod_trig;
	    buf_send[0] = (imod << 11) + (mb_trig_run) + ((0x0) << 16); // set up trigger module run off
	    i = 1;
	    k = 1;
	    i = pcie_send(hDev, i, k, px);

	    imod = imod_trig;
	    buf_send[0] = (imod << 11) + (mb_trig_deadtime_size) + ((250 & 0xff) << 16); // set trigger module deadtime size
	    i = 1;
	    k = 1;
	    i = pcie_send(hDev, i, k, px);

	    imod = 0; // set offline test
	    ichip = 1;
	    buf_send[0] = (imod << 11) + (ichip << 8) + (mb_cntrl_test_off) + (0x0 << 16); // set controller test off
	    i = 1;
	    k = 1;
	    i = pcie_send(hDev, i, k, px);

	    imod = imod_trig; // Set number of ADC samples to (iframe+1)/8;
	    iframe = iframe_length;
	    buf_send[0] = (imod << 11) + (mb_trig_frame_size) + ((iframe & 0xffff) << 16);
	    i = 1;
	    k = 1;
	    i = pcie_send(hDev, i, k, px);

	    imod = 0; // load trig 1 position relative to the frame..
	    ichip = 1;
	    buf_send[0] = (imod << 11) + (ichip << 8) + (mb_cntrl_load_trig_pos) + ((itrig_delay & 0xffff) << 16); // enable test mode
	    i = 1;
	    k = 1;
	    i = pcie_send(hDev, i, k, px);

	    imod = imod_trig;
	    buf_send[0] = (imod << 11) + (mb_trig_mask8) + (0x2 << 16); // set mask1[3] on.
	    i = 1;
	    k = 1;
	    i = pcie_send(hDev, i, k, px);

	    imod = imod_trig;
	    buf_send[0] = (imod << 11) + (mb_trig_prescale8) + (0x0 << 16); // set prescale1 0
	    i = 1;
	    k = 1;
	    i = pcie_send(hDev, i, k, px);
	    /* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ TRIGGER_SETUP  ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */

	    /*   here is common block from tpc */
	    for (imod_fem = (imod_st1); imod_fem > imod_xmit; imod_fem--) //     now reset all the link port receiver PLL
	    {
		printf("%i \n", imod_fem);
		imod = imod_fem;
		printf("Resetting link PLL for module %x \n", imod);
		ichip = 4;
		buf_send[0] = (imod << 11) + (ichip << 8) + mb_feb_pll_reset + (0x0 << 16); // reset LINKIN PLL
		i = 1;
		k = 1;
		i = pcie_send(hDev, i, k, px);
		usleep(1000); // give PLL time to reset
	    }

	    for (imod_fem = (imod_xmit + 1); imod_fem < (imod_st1 + 1); imod_fem++) // read back status
	    {
		printf("%i \n", imod_fem);

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

	    for (imod_fem = (imod_st1); imod_fem > imod_xmit; imod_fem--)
	    {
		printf("%i \n", imod_fem);
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

    for (imod_fem = (imod_xmit + 1); imod_fem < (imod_st1 + 1); imod_fem++)
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

    /* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ DMA & EB  ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */

    printf(" Enter number of events to read out and save:\n ");
    scanf("%d", &nevent);
    printf("\nEnter 1 to set the RUN on \n");
    scanf("%d", &ik);
    ndma_loop = 1;
    // // nword = (((64 * timesize * 3) / 2 + 5) * (imod_st - imod_xmit)) + 2; // total number of 32 bits word
    // // last_dma_loop_size = (nword * 4) % dwDMABufSize;
    // // ndma_loop = (nword * 4) / dwDMABufSize;
    ifr = 0;
    printf(" DMA will run %d loop(s) per event\n", (ndma_loop + 1));
    printf("\t%d loops with %d words and %d loop with %d words\n\n", ndma_loop, dwDMABufSize / 4, 1, dwDMABufSize / 4);
    // struct timeval starttest3312, endtest3312; // get a fresh timestamp
    // gettimeofday(&starttest3312, NULL);
    // long mytime3312, seconds3312, useconds3312;
    // seconds3312 = starttest3312.tv_sec;
    // useconds3312 = starttest3312.tv_usec;
    // printf("\n\nStart time of test: %ld sec %ld usec\n", seconds3312, useconds3312);
    // gettimeofday(&starttest3312, NULL);
    idebug = 1;

    // nc = 0;
    // iset = 0;
    // ncount = 0;
    // nwords = 0;
    // ibin = 1;
    // ic = 0;
    // ih = 0;
    // set = 0;

    /** begin timer **/
    /*struct timeval starttest3312, endtest3312;
    gettimeofday(&starttest3312, NULL);
    long mytime3312, seconds3312, useconds3312;
    seconds3312 = starttest3312.tv_sec;
    useconds3312 = starttest3312.tv_usec;
    printf("\n\nStart time of test: %ld sec %ld usec\n", seconds3312, useconds3312);
    // get a fresh timestamp
    gettimeofday(&starttest3312, NULL);*/

/*TPC DMA*/

    for (ijk = 0; ijk < nevent; ijk++)
    {
        if (ijk % 10 == 0)
            printf("\n===================> EVENT No. %i\n\n", ijk);
        ntot_rec = 0; // keeps track of number of words received in event
        for (iv = 0; iv < (ndma_loop + 1); iv++)
        {
            if (ifr == 0)
            { // first dma
                printf("\n\n\n First DMA\n\n\n");

                if (idebug == 1)
                    printf(" buffer allocation 1\n");
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
                        printf(" buffer allocation lower address = %x\n", u32Data);
                    u32Data = (pDma_rec1->Page->pPhysicalAddr >> 32) & 0xffffffff;
                    if (idebug == 1)
                        printf(" buffer allocation higher address = %x\n", u32Data);
                }

                if (idebug == 1)
                    printf(" buffer allocation 2\n");
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
                        printf(" buffer allocation 2 lower address = %x\n", u32Data);
                    u32Data = (pDma_rec2->Page->pPhysicalAddr >> 32) & 0xffffffff;
                    if (idebug == 1)
                        printf(" buffer allocation 2 higher address = %x\n", u32Data);
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
                    printf(" initial abort finished \n");
            } // end if first dma
            printf("\n\n\n Right before Sync\n\n\n");

            if ((iv % 2) == 0)
                buffp_rec32 = (UINT32 *)pbuf_rec1;
            else
                buffp_rec32 = (UINT32 *)pbuf_rec2;

            /* synch cache */
            // not needed in SL6?
            if ((iv % 2) == 0)
                WDC_DMASyncCpu(pDma_rec1);
            else
                WDC_DMASyncCpu(pDma_rec2);
                    if(idebug ==1) printf(" synch CPU \n");

            nwrite_byte = dwDMABufSize;
            // if (iv != ndma_loop) nwrite_byte = dwDMABufSize;
            // else nwrite_byte = last_dma_loop_size;

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
                        printf(" initialize the input fifo\n");
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
            printf("\n\n\n ifr = 1\n\n\n");

            /** set up DMA for both transceiver together **/
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

            /* byte count */
            dwAddrSpace = cs_bar;
            dwOffset = cs_dma_by_cnt;
            u32Data = nwrite_byte;
            WDC_WriteAddr32(hDev2, dwAddrSpace, dwOffset, u32Data);

            /* write this will start DMA */
            dwAddrSpace = 2;
            dwOffset = cs_dma_cntrl;
            if ((iv % 2) == 0)
                is = (pDma_rec1->Page->pPhysicalAddr >> 32) & 0xffffffff;
            else
                is = (pDma_rec2->Page->pPhysicalAddr >> 32) & 0xffffffff;
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

            // send trigger
            if (iv == 0)
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
                    imod = imod_trig;
                    buf_send[0] = (imod << 11) + (mb_trig_run) + ((0x1) << 16); // set up run
                    i = 1;
                    k = 1;
                    i = pcie_send(hDev, i, k, px);
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

            printf("\n\n\n Now we wait for DMA to fill \n\n\n");

            // extra wait just to be certain --- WK
            // usleep(20000);
            /***    check to see if DMA is done or not **/
            idone = 0;
            for (is = 0; is < 6000000; is++)
            {
                dwAddrSpace = cs_bar;
                u64Data = 0;
                dwOffset = cs_dma_cntrl;
                WDC_ReadAddr32(hDev2, dwAddrSpace, dwOffset, &u32Data);
                // if (idebug == 1)
                //   printf(" receive DMA status word %d %X \n", is, u32Data);
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

    /*for (imod_fem = (imod_xmit + 1); imod_fem < (imod_st2 + 1); imod_fem++)
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
    }*/



                printf("\n loop %d, DMA is not finished \n", iv);
                dwAddrSpace = cs_bar;
                dwOffset = cs_dma_by_cnt;
                WDC_ReadAddr64(hDev2, dwAddrSpace, dwOffset, &u64Data);
                printf(" DMA word count = %x, %x \n", (u64Data >> 32), (u64Data & 0xffff));
                if ((iv % 2) == 0)
                    WDC_DMASyncIo(pDma_rec1);
                else
                    WDC_DMASyncIo(pDma_rec2);
                nred = (nwrite_byte - (u64Data & 0xffff)) / 4;
                printf(" number of 32-bit words received: %d (this DMA) + %d (total before this DMA)\n", nred, ntot_rec);

                //   for (is=0; is < (nwrite_byte/4); is++) {

                //     if((is%8) ==0) printf("%4d", is);
                //     printf(" %8X", read_array[is]);
                //     if((is+1)%8 == 0) printf("\n");
                //   }

                if (iwrite == 1 || irawprint == 1)
                {
                    for (is = 0; is < nred; is++)
                    {
                        read_array[is] = *buffp_rec32++;
                        printf(" %8X", read_array[is]);

                        if (irawprint == 1)
                            read_array_compare[ntot_rec + is] = read_array[is];
                        if (irawprint == 1 && ijk == 0)
                            read_array_1st[ntot_rec + is] = read_array[is];
                        /*if (iwrited == 1)
                        {
                          fprintf(outf, " %8x", read_array[is]);
                          if ((((ntot_rec + is + 1) % 8) == 0))
                            fprintf(outf, "\n");
                        }*/
                    }
                    //	      scanf("%d",&ik);
                }
                ntot_rec = ntot_rec + nred;
                if (iwritem == 1)
                {
                    printf("\n\n\n Why are we here? \n\n\n");

                    sprintf(name, "./data/xmit_trig_bin_grams_dma_%i_%i_%i.dat", tnum, ijk, iv);
                    fd = creat(name, 0755);
                    n_write = write(fd, read_array, nred * 4);
                    close(fd);
                }
                else if (iwrite == 1 && dmasizewrite == 1)
                {

                    printf("\n\n\n Writing to file \n\n\n");

                    n_write = write(fd, read_array, nred * 4);

                    printf("\n\n\n writing done\n\n\n");
                }

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

            printf("\n\n\n Right before Sync\n\n\n");

            /* synch DMA i/O cache **/
            // SL6
            if ((iv % 2) == 0)
                WDC_DMASyncIo(pDma_rec1);
            else
                WDC_DMASyncIo(pDma_rec2);

            printf("\n\n\n Right after Sync\n\n\n");

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

            printf("\n\n\n writing 123 \n\n\n");

            nwrite = nwrite_byte / 4;
            if (iwrite == 1 || irawprint == 1)
            {
                for (is = 0; is < nwrite; is++)
                {
                    read_array[is] = *buffp_rec32++;
                    // if (irawprint == 1)
                    //   read_array_compare[ntot_rec + is] = read_array[is];
                    // if (irawprint == 1 && ijk == 0)
                    //   read_array_1st[ntot_rec + is] = read_array[is];
                    /*if (iwrited == 1)
                    {
                      fprintf(outf, " %8x", read_array[is]);
                      if ((((ntot_rec + is + 1) % 8) == 0))
                        fprintf(outf, "\n");
                    }*/
                }
            }
            if (iwritem == 1)
            {
                sprintf(name, "./data/xmit_trig_bin_grams_dma_%i_%i_%i.dat", tnum, ijk, iv);
                fd = creat(name, 0755);
                n_write = write(fd, read_array, nwrite * 4);
                close(fd);
            }
            else if (iwrite == 1 && dmasizewrite == 1)
            {

                /*for (is=0; is < (nwrite_byte/4); is++) {

                  if((is%8) ==0) printf("%4d", is);
                  printf(" %8X", read_array[is]);
                  if((is+1)%8 == 0) printf("\n");
                }*/

                n_write = write(fd, read_array, nwrite * 4);
            }
            ntot_rec = ntot_rec + nwrite;
        } // end dma loop

        // write to file: full event
        // if (iwrite == 1 && dmasizewrite == 0)
        //{
        //  n_write = write(fd, read_array_compare, ntot_rec * 4);
        //}

        printf("\n\n\n writing 1234 \n\n\n");

    } // end loop over events

    /*gettimeofday(&endtest3312, NULL);
    seconds3312 = endtest3312.tv_sec;
    useconds3312 = endtest3312.tv_usec;
    printf("\nEnd time of test:   %ld sec %ld usec\n", seconds3312, useconds3312);
    seconds3312 = endtest3312.tv_sec - starttest3312.tv_sec;
    useconds331 = endtest331.tv_usec - starttest3312.tv_usec;
    mytime3312 = seconds3312 * 1000000. + useconds3312 * 1.; // elapsed time in usec
    printf("\nElapsed time:\t\t\t %ld usec\n\n", mytime3312);
    */
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

    if (iwrite == 1 && iwritem == 0)
        close(fd);
    if (iwrited == 1)
        fclose(outf);
}

/*TPC+PMT DMA*/

//   for (ijk = 0; ijk < nevent; ijk++)
//   {
//       if (ijk % 10 == 0)
//           printf("\n===================> EVENT No. %i\n\n", ijk);
//       ntot_rec = 0; // keeps track of number of words received in event
//       for (iv = 0; iv < (ndma_loop + 1); iv++)
//       {
//           if (ifr == 0)
//           { // first dma

//               if (ntrans == 1)
//               {
//                   if (idebug == 1)
//                       printf(" First DMA: buffer allocation 1\n");
//                   dwStatus = WDC_DMAContigBufLock(hDev2, &pbuf_rec, dwOptions_rec, dwDMABufSize, &pDma_rec);
//                   if (WD_STATUS_SUCCESS != dwStatus)
//                   {
//                       printf("Failed locking a rec 1 Contiguous DMA buffer. Error 0x%lx - %s\n", dwStatus, Stat2Str(dwStatus));
//                       printf("enter 1 to continue \n");
//                       scanf("%d", &is);
//                   }
//                   else
//                   {
//                       u32Data = pDma_rec->Page->pPhysicalAddr & 0xffffffff;
//                       if (idebug == 1)
//                           printf(" First DMA: buffer allocation lower address = %x\n", u32Data);
//                       u32Data = (pDma_rec->Page->pPhysicalAddr >> 32) & 0xffffffff;
//                       if (idebug == 1)
//                           printf(" First DMA: buffer allocation higher address = %x\n", u32Data);
//                   }
//               }
//               else if (ntrans == 2)
//               {
//                   if (idebug == 1)
//                       printf(" First DMA: buffer allocation 1\n");
//                   dwStatus = WDC_DMAContigBufLock(hDev2, &pbuf_rec1, dwOptions_rec, dwDMABufSize, &pDma_rec1);
//                   if (WD_STATUS_SUCCESS != dwStatus)
//                   {
//                       printf("Failed locking a rec 1 Contiguous DMA buffer. Error 0x%lx - %s\n", dwStatus, Stat2Str(dwStatus));
//                       printf("enter 1 to continue \n");
//                       scanf("%d", &is);
//                   }
//                   else
//                   {
//                       u32Data = pDma_rec1->Page->pPhysicalAddr & 0xffffffff;
//                       if (idebug == 1)
//                           printf(" First DMA: buffer allocation lower address = %x\n", u32Data);
//                       u32Data = (pDma_rec1->Page->pPhysicalAddr >> 32) & 0xffffffff;
//                       if (idebug == 1)
//                           printf(" First DMA: buffer allocation higher address = %x\n", u32Data);
//                   }

//                   if (idebug == 1)
//                       printf(" First DMA: buffer allocation 2\n");
//                   dwStatus = WDC_DMAContigBufLock(hDev2, &pbuf_rec2, dwOptions_rec, dwDMABufSize, &pDma_rec2);
//                   if (WD_STATUS_SUCCESS != dwStatus)
//                   {
//                       printf("Failed locking a rec 2 Contiguous DMA buffer. Error 0x%lx - %s\n", dwStatus, Stat2Str(dwStatus));
//                       printf("enter 1 to continue \n");
//                       scanf("%d", &is);
//                   }
//                   else
//                   {
//                       u32Data = pDma_rec2->Page->pPhysicalAddr & 0xffffffff;
//                       if (idebug == 1)
//                           printf(" First DMA: buffer allocation 2 lower address = %x\n", u32Data);
//                       u32Data = (pDma_rec2->Page->pPhysicalAddr >> 32) & 0xffffffff;
//                       if (idebug == 1)
//                           printf(" First DMA: buffer allocation 2 higher address = %x\n", u32Data);
//                   }
//               }
//               // set tx mode register
//               u32Data = 0x00002000;
//               dwOffset = tx_md_reg;
//               dwAddrSpace = cs_bar;
//               WDC_WriteAddr32(hDev2, dwAddrSpace, dwOffset, u32Data);

//               // write this will abort previous DMA
//               dwAddrSpace = 2;
//               dwOffset = cs_dma_msi_abort;
//               u32Data = dma_abort;
//               WDC_WriteAddr32(hDev2, dwAddrSpace, dwOffset, u32Data);

//               // clear DMA register after the abort
//               dwAddrSpace = 2;
//               dwOffset = cs_dma_msi_abort;
//               u32Data = 0;
//               WDC_WriteAddr32(hDev2, dwAddrSpace, dwOffset, u32Data);
//               if (idebug == 1)
//                   printf(" First DMA: initial abort finished \n");
//           } // end if first dma

//           if (ntrans == 1)
//           {
//               buffp_rec32 = (UINT32 *)pbuf_rec;
//               WDC_DMASyncCpu(pDma_rec); // IS_709
//           }
//           else if (ntrans == 2)
//           {

//               if ((iv % 2) == 0)
//                   buffp_rec32 = (UINT32 *)pbuf_rec1;
//               else
//                   buffp_rec32 = (UINT32 *)pbuf_rec2;

//               if ((iv % 2) == 0)
//                   WDC_DMASyncCpu(pDma_rec1);
//               else
//                   WDC_DMASyncCpu(pDma_rec2);
//           }
//           // synch cache
//           // not needed in SL6?
//           if (idebug == 1)
//               printf(" synch CPU \n");

//           if (iv != ndma_loop)
//               nwrite_byte = dwDMABufSize;
//           else
//               nwrite_byte = last_dma_loop_size;

//           if (idebug == 1)
//               printf(" nwrite_byte is set to : %d\n", nwrite_byte);

//           // if(ntrans==1){
//           // for (is = 0; is < nwrite_byte/4; is++){  //IS_709
//           //     buffp_rec32[is]=0;  //zero out buffer
//           // }
//           // }
//           for (is = 1; is < 3; is++)
//           {
//               tr_bar = t1_tr_bar;
//               r_cs_reg = r1_cs_reg;
//               dma_tr = dma_tr1;
//               if (is == 2)
//               {
//                   tr_bar = t2_tr_bar;
//                   r_cs_reg = r2_cs_reg;
//                   dma_tr = dma_tr2;
//               }
//               if (idebug == 1)
//                   printf(" is = %d\n", is);
//               // initialize the receiver
//               u32Data = cs_init;
//               dwOffset = r_cs_reg;
//               dwAddrSpace = cs_bar;

//               // receiver only gets initialize for the 1st time
//               if (ifr == 0)
//               {
//                   if (idebug == 1)
//                       printf(" First DMA: initialize the input fifo\n");
//                   WDC_WriteAddr32(hDev2, dwAddrSpace, dwOffset, u32Data);
//               }

//               // start the receiver
//               dwAddrSpace = cs_bar;
//               u32Data = cs_start + nwrite_byte; // 32 bits mode == 4 bytes per word *2 fibers
//               if (idebug == 1)
//                   printf(" DMA loop %d with DMA data length %d \n", iv, nwrite_byte);
//               dwOffset = r_cs_reg;
//               WDC_WriteAddr32(hDev2, dwAddrSpace, dwOffset, u32Data);
//           }

//           ifr = 1;

//           // set up DMA for both transceiver together
//           if (ntrans == 1)
//           {
//               dwAddrSpace = cs_bar;
//               dwOffset = cs_dma_add_low_reg;
//               u32Data = pDma_rec->Page->pPhysicalAddr & 0xffffffff;
//               WDC_WriteAddr32(hDev2, dwAddrSpace, dwOffset, u32Data);

//               dwAddrSpace = cs_bar;
//               dwOffset = cs_dma_add_high_reg;
//               u32Data = (pDma_rec->Page->pPhysicalAddr >> 32) & 0xffffffff;
//               WDC_WriteAddr32(hDev2, dwAddrSpace, dwOffset, u32Data);
//           }
//           else if (ntrans == 2)
//           {
//               dwAddrSpace = cs_bar;
//               dwOffset = cs_dma_add_low_reg;
//               if ((iv % 2) == 0)
//                   u32Data = pDma_rec1->Page->pPhysicalAddr & 0xffffffff;
//               else
//                   u32Data = pDma_rec2->Page->pPhysicalAddr & 0xffffffff;
//               WDC_WriteAddr32(hDev2, dwAddrSpace, dwOffset, u32Data);

//               dwAddrSpace = cs_bar;
//               dwOffset = cs_dma_add_high_reg;
//               if ((iv % 2) == 0)
//                   u32Data = (pDma_rec1->Page->pPhysicalAddr >> 32) & 0xffffffff;
//               else
//                   u32Data = (pDma_rec2->Page->pPhysicalAddr >> 32) & 0xffffffff;
//               WDC_WriteAddr32(hDev2, dwAddrSpace, dwOffset, u32Data);
//           }

//             // set up DMA for both transceiver together
//             dwAddrSpace = cs_bar;
//             dwOffset = cs_dma_add_low_reg;
//             //if ((iv % 2) == 0)u32Data = pDma_rec1->Page->pPhysicalAddr & 0xffffffff;
//             //else u32Data = pDma_rec2->Page->pPhysicalAddr & 0xffffffff;
//                 u32Data = pDma_rec->Page->pPhysicalAddr & 0xffffffff;
//             WDC_WriteAddr32(hDev2, dwAddrSpace, dwOffset, u32Data);

//             dwAddrSpace = cs_bar;
//             dwOffset = cs_dma_add_high_reg;
//             //if ((iv % 2) == 0)u32Data = (pDma_rec1->Page->pPhysicalAddr >> 32) & 0xffffffff;
//             //else u32Data = (pDma_rec2->Page->pPhysicalAddr >> 32) & 0xffffffff;
//                 u32Data = (pDma_rec->Page->pPhysicalAddr >> 32) & 0xffffffff;
//             WDC_WriteAddr32(hDev2, dwAddrSpace, dwOffset, u32Data);
      
//             // byte count
//            dwAddrSpace = cs_bar;
//            dwOffset = cs_dma_by_cnt;
//            u32Data = nwrite_byte;
//            WDC_WriteAddr32(hDev2, dwAddrSpace, dwOffset, u32Data);

//            if (ntrans == 1)
//            {
//                // write this will start DMA
//                dwAddrSpace = 2;
//                dwOffset = cs_dma_cntrl;
//                is = (pDma_rec->Page->pPhysicalAddr >> 32) & 0xffffffff;
//            }
//            else if (ntrans == 2)
//            {
//                dwAddrSpace = 2;
//                dwOffset = cs_dma_cntrl;
//                if ((iv % 2) == 0)
//                    is = (pDma_rec1->Page->pPhysicalAddr >> 32) & 0xffffffff;
//                else
//                    is = (pDma_rec2->Page->pPhysicalAddr >> 32) & 0xffffffff;
//            }
//            if (is == 0)
//            {
//                if (idebug == 1)
//                    printf(" use 3dw \n");
//                u32Data = dma_tr12 + dma_3dw_rec;
//            }
//            else
//            {
//                u32Data = dma_tr12 + dma_4dw_rec;
//                if (idebug == 1)
//                    printf(" use 4dw \n");
//            }
//            WDC_WriteAddr32(hDev2, dwAddrSpace, dwOffset, u32Data);
//            if (idebug == 1)
//                printf(" DMA set up done, byte count = %d\n", nwrite_byte);

//            itrig_c = 0;
//            itrig_ext = 1;

//            // send trigger
//            if (iv == 0 && ijk == 0)
//            {
//                if (itrig_c == 1)
//                {
//                    imod = imod_trig;
//                    buf_send[0] = (imod << 11) + mb_trig_pctrig + ((0x0) << 16);
//                    i = 1;
//                    k = 1;
//                    i = pcie_send(hDev, i, k, px);
//                    usleep(5000); // 3x frame size
//                }
//                else if (itrig_ext == 1)
//                {
//                    //
//                    //     only need to restart the run if the we use the test data or 1st run
//                    //
//                    printf("Run turning on....\n");
//                    imod = imod_trig;
//                    buf_send[0] = (imod << 11) + (mb_trig_run) + ((0x1) << 16); // set up run
//                    i = 1;
//                    k = 1;
//                    i = pcie_send(hDev, i, k, px);
//                    usleep(5000);
//                }
//                else
//                {
//                    imod = 0;
//                    ichip = 1;
//                    buf_send[0] = (imod << 11) + (ichip << 8) + mb_cntrl_set_trig1 + (0x0 << 16); // send trigger
//                    i = 1;
//                    k = 1;
//                    i = pcie_send(hDev, i, k, px);
//                    if (idebug == 1)
//                        printf(" trigger send  \n");
//                    usleep(5000);
//                }
//            }

//            // extra wait just to be certain --- WK
//            // usleep(20000);
//            //    check to see if DMA is done or not
//            idone = 0;
//            // iwritem=0;
//            for (is = 0; is < 6000000; is++)
//            {
//                dwAddrSpace = cs_bar;
//                u64Data = 0;
//                dwOffset = cs_dma_cntrl;
//                WDC_ReadAddr32(hDev2, dwAddrSpace, dwOffset, &u32Data);
//                // if (idebug == 1)printf(" receive DMA status word %d %X \n", is, u32Data);
//                if ((u32Data & dma_in_progress) == 0)
//                {
//                    idone = 1;
//                    if (idebug == 1)
//                        printf(" receive DMA complete %d \n", is);
//                }
//                if ((u32Data & dma_in_progress) == 0)
//                    break;
//            }
//            if (idone == 0)
//            {
//                printf("\n loop %d, DMA is not finished \n", iv);
//                dwAddrSpace = cs_bar;
//                dwOffset = cs_dma_by_cnt;
//                WDC_ReadAddr64(hDev2, dwAddrSpace, dwOffset, &u64Data);
//                printf(" DMA word count = %x, %x \n", (u64Data >> 32), (u64Data & 0xffff));
//                if (ntrans == 1)
//                    WDC_DMASyncIo(pDma_rec);
//                else if (ntrans == 2)
//                {
//                    if ((iv % 2) == 0)
//                        WDC_DMASyncIo(pDma_rec1);
//                    else
//                        WDC_DMASyncIo(pDma_rec2);
//                }
//                nred = (nwrite_byte - (u64Data & 0xffff)) / 4;
//                printf(" number of 32-bit words received: %d (this DMA) + %d (total before this DMA)\n", nred, ntot_rec);
//                iwrite = 1;
//                irawprint = 0;
//                if (iwrite == 1 || irawprint == 1)
//                {
//                    for (is = 0; is < nred; is++)
//                    {
//                        read_array[is] = *buffp_rec32++;
//                        printf(" %8x", read_array[is]);
//                        if (irawprint == 1)
//                            read_array_compare[ntot_rec + is] = read_array[is];
//                        if (irawprint == 1 && ijk == 0)
//                            read_array_1st[ntot_rec + is] = read_array[is];
//                        if (iwrited == 1)
//                        {
//                            fprintf(outf, " %8x", read_array[is]);
//                            if ((((ntot_rec + is + 1) % 8) == 0))
//                                fprintf(outf, "\n");
//                        }
//                    }
//                }
//                ntot_rec = ntot_rec + nred;

//                // iwritem=1;
//                iwrite = 1;       // 1
//                dmasizewrite = 1; // 1

//                dwAddrSpace = cs_bar;
//                u64Data = 0;
//                dwOffset = t1_cs_reg;
//                WDC_ReadAddr64(hDev2, dwAddrSpace, dwOffset, &u64Data);
//                printf(" Status word for channel 1 after read = %x, %x \n", (u64Data >> 32), (u64Data & 0xffff));
//                dwAddrSpace = cs_bar;
//                u64Data = 0;
//                dwOffset = t2_cs_reg;
//                WDC_ReadAddr64(hDev2, dwAddrSpace, dwOffset, &u64Data);
//                printf(" status word for channel 2 after read = %x, %x \n", (u64Data >> 32), (u64Data & 0xffff));
//                // write this will abort previous DMA
//                dwAddrSpace = 2;
//                dwOffset = cs_dma_msi_abort;
//                u32Data = dma_abort;
//                WDC_WriteAddr32(hDev2, dwAddrSpace, dwOffset, u32Data);

//                // clear DMA register after the abort
//                dwAddrSpace = 2;
//                dwOffset = cs_dma_msi_abort;
//                u32Data = 0;
//                WDC_WriteAddr32(hDev2, dwAddrSpace, dwOffset, u32Data);
//            }

//            if (idone == 0)
//                break;

//            if (ntrans == 1) // synch DMA i/O cache
//                WDC_DMASyncIo(pDma_rec);
//            else if (ntrans == 2)
//            {
//                if ((iv % 2) == 0)
//                    WDC_DMASyncIo(pDma_rec1);
//                else
//                    WDC_DMASyncIo(pDma_rec2);
//            }
//            if (idebug == 1)
//            {
//                dwAddrSpace = cs_bar;
//                u64Data = 0;
//                dwOffset = t1_cs_reg;
//                WDC_ReadAddr64(hDev2, dwAddrSpace, dwOffset, &u64Data);
//                printf(" status word for Channel 1 after read = %x, %x \n", (u64Data >> 32), (u64Data & 0xffff));
//                dwAddrSpace = cs_bar;
//                u64Data = 0;
//                dwOffset = t2_cs_reg;
//                WDC_ReadAddr64(hDev2, dwAddrSpace, dwOffset, &u64Data);
//                printf(" status word for channel 2 after read = %x, %x \n", (u64Data >> 32), (u64Data & 0xffff));
//            }

//            nwrite = nwrite_byte / 4;
//            if (iwrite == 1 || irawprint == 1)
//            {
//                for (is = 0; is < nwrite; is++)
//                {
//                    read_array[is] = *buffp_rec32++;
//                    if (irawprint == 1)
//                        read_array_compare[ntot_rec + is] = read_array[is];
//                    if (irawprint == 1 && ijk == 0)
//                        read_array_1st[ntot_rec + is] = read_array[is];
//                    if (iwrited == 1)
//                    {

//                        // n_write = write(outf,read_array,nwrite_byte);
//                        fprintf(outf, " %8x", read_array[is]);
//                        if ((((ntot_rec + is + 1) % 8) == 0))
//                            fprintf(outf, "\n");
//                    }
//                }
//            }

//            if (iwrite == 1 && dmasizewrite == 1)
//            {
//                for (is = 0; is < (nwrite_byte / 4); is++)
//                {

//                    if ((is % 8) == 0)
//                        printf("%4d", is);
//                    printf(" %8X", read_array[is]);
//                    if ((is + 1) % 8 == 0)
//                        printf("\n");
//                }
//                n_write = write(fd, read_array, nwrite * 4);
//            }

//            if (iwritem == 1)
//            {
//                sprintf(name, "./xmit_trig_bin_grams_dma_%s_%i_%i.dat", subrun, ijk, iv);
//                fd2 = creat(name, 0755);
//                n_write = write(fd2, read_array, nwrite * 4);
//                close(fd2);
//            }

//            ntot_rec = ntot_rec + nwrite;

//        } // end dma loop

//     }

// }

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
    //inpf = fopen("/home/uboonedaq/pmt_fpga_code/module1x_pmt_64MHz_new_head_07162013.rbf","r");
    inpf = fopen("/home/ub/WinDriver/wizard/GRAMS_project/uBooNE_Nominal_Firmware/pmt_fem/module1x_pmt_64MHz_new_head_07162013.rbf", "r");
    //inpf = fopen("/home/ub/module1x_pmt_64MHz_link_4192013.rbf", "r");
    //inpf = fopen("/home/ub/module1x_pmt_64MHz_new_head_12192013_allch.rbf","r");
    //inpf = fopen("/home/ub/module1x_pmt_64MHz_fermi_08032012_2.rbf","r");
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



/*

   // gettimeofday(&endtest3312, NULL);
   // seconds3312 = endtest3312.tv_sec;
   // useconds3312 = endtest3312.tv_usec;
   // printf("\nEnd time of test:   %ld sec %ld usec\n", seconds3312, useconds3312);
   // seconds3312 = endtest3312.tv_sec - starttest3312.tv_sec;
   // useconds3312 = endtest3312.tv_usec - starttest3312.tv_usec;
   // mytime3312 = seconds3312 * 1000000. + useconds3312 * 1.; // elapsed time in usec
   // printf("\nElapsed time:\t\t\t %ld usec\n\n", mytime3312);

   imod = imod_trig;
   buf_send[0] = (imod << 11) + (mb_trig_run) + ((0x0) << 16); // set up run off
   i = 1;
   k = 1;
   i = pcie_send(hDev, i, k, px);
}*/
