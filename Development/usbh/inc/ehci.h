/**
 * @brief   MCXN947V USBH(EHCI) Low Layer
 * @author  masa
 * @version 1.00 
 */

#ifndef __EHCI_H__
#define __EHCI_H__

#include "usb20.h"
#include "usbphy.h"

#define EHCI  USBHS1__USBC

#ifdef __INTELLISENSE__
#ifdef __ALIGNED
#undef __ALIGNED
#endif
#define __ALIGNED(x)
#endif

#define MAX_EP_DCI      31
#define MAX_DEVICE_NUM  3

typedef struct{
  void (*ehciUsbIntAsyncCb)(void);
	void (*ehciUsbIntPeridicCb)(void);
	void (*ehciUsbErrIntCb)(void);
	void (*ehciFrameListRlovCb)(void);
	void (*ehciHostSysErrCb)(void);
	void (*ehciIntrOnAsyncAdvCb)(void);
	void (*ehciPortConnectStatChgCb)(void);
	void (*ehciPortEnDisCb)(void);
	void (*ehciPortOvCurrCb)(void);
  void (*ehciGpTimerCb)(void);
}ehci_int_cb_t;

typedef enum{
	USB_INT_ASYNC = 0,
  USB_INT_PERIODIC,
	USB_ERROR_INT,
	FRAME_LIST_ROLLOVER,
	SYS_ERROR,
	ASYNC_ADVANCE,
	PORT_CSC,
	PORT_PED,
	PORT_OCC,
  GPTIMER0,
}ehci_int_type_t;

typedef struct{
	uint32_t DWORD0_QHHLP;
	uint32_t DWORD1_EC0;
	uint32_t DWORD2_EC1;
	uint32_t DWORD3_CQLP;
	uint32_t DWORD4_NQLP;
	uint32_t DWORD5_ANQLP;
	uint32_t DWORD6_QTO;
	uint32_t DWORD7_BP0;
	uint32_t DWORD8_BP1;
	uint32_t DWORD9_BP2;
	uint32_t DWORD10_BP3;
	uint32_t DWORD11_BP4;
} ehci_QH_t;

typedef struct{
	ehci_QH_t QH;
	uint32_t reserved[3];
  void (*completeCallback)(uint8_t, uint8_t, uint16_t);
} ehci_QH_array_t;

typedef struct{
	uint32_t DWORD0_NQP;
	uint32_t DWORD1_ANQP;
	uint32_t DWORD2_QTO;
	uint32_t DWORD3_BP0;
	uint32_t DWORD4_BP1;
	uint32_t DWORD5_BP2;
	uint32_t DWORD6_BP3;
	uint32_t DWORD7_BP4;
} ehci_qTD_t;

typedef struct{
	uint32_t DWORD0_NLP;
	uint32_t DWORD1_TSC0;
	uint32_t DWORD2_TSC1;
	uint32_t DWORD3_TSC2;
	uint32_t DWORD4_TSC3;
	uint32_t DWORD5_TSC4;
	uint32_t DWORD6_TSC5;
	uint32_t DWORD7_TSC6;
	uint32_t DWORD8_TSC7;
	uint32_t DWORD9_BP0;
	uint32_t DWORD10_BP1;
	uint32_t DWORD11_BP2;
	uint32_t DWORD12_BP3;
	uint32_t DWORD13_BP4;
	uint32_t DWORD14_BP5;
	uint32_t DWORD15_BP6;	
} ehci_iTD_t;

typedef struct{
	uint32_t DWORD0_NLP;
	uint32_t DWORD1_ETTC;
	uint32_t DWORD2_MSC;
	uint32_t DWORD3_TSC;
	uint32_t DWORD4_BP0;
	uint32_t DWORD5_BP1;
	uint32_t DWORD6_BLP;
} ehci_siTD_t;

#define EHCI_PFL_NUM 1024

#define EHCI_qTD_NQP_T 0x1UL
#define EHCI_qTD_NQP_NTEP(x) (uint32_t)((uint32_t)(x) & 0xFFFFFFE0)
#define EHCI_qTD_ANQP_T 0x1UL
#define EHCI_qTD_ANQP_ANTEP(x) (uint32_t)((uint32_t)(x) & 0xFFFFFFE0)
#define EHCI_qTD_QTO_dt_0 0
#define EHCI_qTD_QTO_dt_1 0x80000000
#define EHCI_qTD_QTO_TBT_Msk  0x7FFF0000
#define EHCI_qTD_QTO_TBT(x) (uint32_t)((uint32_t)(x) << 16)
#define EHCI_qTD_QTO_IOC (1UL << 15)
#define EHCI_qTD_QTO_CP_0 0UL
#define EHCI_qTD_QTO_CP_1 (1UL << 12)
#define EHCI_qTD_QTO_CP_2 (2UL << 12)
#define EHCI_qTD_QTO_CP_3 (3UL << 12)
#define EHCI_qTD_QTO_CP_4 (4UL << 12)
#define EHCI_qTD_QTO_CERR (3UL << 10)
#define EHCI_qTD_QTO_PID_OUT 0
#define EHCI_qTD_QTO_PID_IN (1UL << 8)
#define EHCI_qTD_QTO_PID_SETUP (2UL << 8)
#define EHCI_qTD_QTO_Status_Active (0x00000080UL)
#define EHCI_qTD_QTO_Status_Halted (0x00000040UL)
#define EHCI_qTD_QTO_Status_Msk (0x000000FFUL)
#define EHCI_qTD_BPx_BPL(x) (uint32_t)((uint32_t)(x) & 0xFFFFF000)
#define EHCI_qTD_BP0_CO(x) (uint32_t)((uint32_t)(x) & 0x00000FFF)

#define EHCI_QH_QHHLP_T 0x1UL
#define EHCI_QH_QHHLP_TYP_iTD 0
#define EHCI_QH_QHHLP_TYP_QH (1UL << 1)
#define EHCI_QH_QHHLP_TYP_siTD (2UL << 1)
#define EHCI_QH_QHHLP_TYP_FSTN (3UL << 1)
#define EHCI_QH_QHHLP_QHHLP(x) (uint32_t)((uint32_t)(x) & 0xFFFFFFE0)
#define EHCI_QH_EC0_RL_Pos 28
#define EHCI_QH_EC0_C (1UL << 27)
#define EHCI_QH_EC0_MPL(x) (uint32_t)((uint32_t)(x) << 16)
#define EHCI_QH_EC0_MPL_Msk	0x07FF0000
#define EHCI_QH_EC0_H (1UL << 15)
#define EHCI_QH_EC0_DTC_QH 0
#define EHCI_QH_EC0_DTC_qTD (1UL << 14)
#define EHCI_QH_EC0_EPS_FS 0
#define EHCI_QH_EC0_EPS_LS (1UL << 12)
#define EHCI_QH_EC0_EPS_HS (2UL << 12)
#define EHCI_QH_EC0_Endpt(x) (uint32_t)(((uint32_t)(x) & 0x0000000F) << 8)
#define EHCI_QH_EC0_I (1UL << 7)
#define EHCI_QH_EC0_DA(x) (uint32_t)((uint32_t)(x) & 0x0000007F)
#define EHCI_QH_EC1_Mult_1 (1UL << 30)
#define EHCI_QH_EC1_Mult_2 (2UL << 30)
#define EHCI_QH_EC1_Mult_3 (3UL << 30)
#define EHCI_QH_EC1_PN(x) (uint32_t)((uint32_t)(x) << 23)
#define EHCI_QH_EC1_HA(x) (uint32_t)((uint32_t)(x) << 16)
#define EHCI_QH_EC1_SCM(x) (uint32_t)((uint32_t)(x) << 8)
#define EHCI_QH_EC1_ISM(x) ((uint32_t)(x))
#define EHCI_QH_NQLP(x) (uint32_t)((uint32_t)(x) & 0xFFFFFFE0)
#define EHCI_QH_NQLP_T 1UL

#define EHCI_iTD_NLP_T 0x1UL
#define EHCI_iTD_NLP_TYP_iTD 0
#define EHCI_iTD_NLP_TYP_QH (1UL << 1)
#define EHCI_iTD_NLP_TYP_siTD (2UL << 1)
#define EHCI_iTD_NLP_TYP_FSTN (3UL << 1)
#define EHCI_iTD_NLP_LP(x) (uint32_t)((uint32_t)(x) & 0xFFFFFFE0)
#define EHCI_iTD_TSCx_Status_Active 0x80000000UL
#define EHCI_iTD_TSCx_Status_Msk 0xF0000000UL
#define EHCI_iTD_TSCx_TL(x) (uint32_t)((uint32_t)(x) << 16)
#define EHCI_iTD_TSCx_IOC (1UL << 15)
#define EHCI_iTD_TSCx_PG_0 0
#define EHCI_iTD_TSCx_PG_1 (1UL << 12)
#define EHCI_iTD_TSCx_PG_2 (2UL << 12)
#define EHCI_iTD_TSCx_PG_3 (3UL << 12)
#define EHCI_iTD_TSCx_PG_4 (4UL << 12)
#define EHCI_iTD_TSCx_PG_5 (5UL << 12)
#define EHCI_iTD_TSCx_PG_6 (6UL << 12)
#define EHCI_iTD_TSCx_OFFSET(x) (uint32_t)((uint32_t)(x) & 0x00000FFF)
#define EHCI_iTD_BPx_BP(x) (uint32_t)((uint32_t)(x) & 0xFFFFF000)
#define EHCI_iTD_BP0_EndPt(x) (uint32_t)(((uint32_t)(x) & 0x0000000F) << 8)
#define EHCI_iTD_BP0_DA(x) (uint32_t)((uint32_t)(x) & 0x00000007)
#define EHCI_iTD_BP1_D_OUT 0
#define EHCI_iTD_BP1_D_IN (1UL << 11)
#define EHCI_iTD_BP1_MPS(x) (uint32_t)((uint32_t)(x) & 0x000007FF)
#define EHCI_iTD_BP2_Mult_1 (1UL)
#define EHCI_iTD_BP2_Mult_2 (2UL)
#define EHCI_iTD_BP2_Mult_3 (3UL)

#define EHCI_siTD_NLP_T 0x1UL
#define EHCI_siTD_NLP_TYP_iTD 0
#define EHCI_siTD_NLP_TYP_QH (1UL << 1)
#define EHCI_siTD_NLP_TYP_siTD (2UL << 1)
#define EHCI_siTD_NLP_TYP_FSTN (3UL << 1)
#define EHCI_siTD_NLP_LP(x) (uint32_t)((uint32_t)(x) & 0xFFFFFFE0)
#define EHCI_siTD_ETTC_DIR_OUT 0UL
#define EHCI_siTD_ETTC_DIR_IN 0x80000000UL
#define EHCI_siTD_ETTC_PN(x) (uint32_t)(((uint32_t)(x) & 0x0000007F) << 24)
#define EHCI_siTD_ETTC_HA(x) (uint32_t)(((uint32_t)(x) & 0x0000007F) << 16)
#define EHCI_siTD_ETTC_EndPt(x) (uint32_t)(((uint32_t)(x) & 0x0000000F) << 8)
#define EHCI_siTD_ETTC_DA(x) (uint32_t)((uint32_t)(x) & 0x00000007)
#define EHCI_siTD_MSC_SCM(x) (uint32_t)(((uint32_t)(x) & 0x000000FF) << 8)
#define EHCI_siTD_MSC_SSM(x) (uint32_t)((uint32_t)(x) & 0x000000FF)
#define EHCI_siTD_TSC_IOC 0x80000000UL
#define EHCI_siTD_TSC_P_0 0UL
#define EHCI_siTD_TSC_P_1 0x40000000UL
#define EHCI_siTD_TSC_TBT(x) (uint32_t)(((uint32_t)(x) & 0x000003FF) << 16)
#define EHCI_siTD_TSC_uFCPM(x) (uint32_t)(((uint32_t)(x) & 0x000000FF) << 8)
#define EHCI_siTD_TSC_Status_Active (0x00000080UL)
#define EHCI_siTD_TSC_Status_Msk (0x000000FFUL)
#define EHCI_siTD_BPx_BPL(x) (uint32_t)((uint32_t)(x) & 0xFFFFF000)
#define EHCI_siTD_BP0_CO(x) (uint32_t)((uint32_t)(x) & 0x00000FFF)
#define EHCI_siTD_BP1_TP(x) (uint32_t)(((uint32_t)(x) & 0x00000003) << 3)
#define EHCI_siTD_BP1_TC(x) (uint32_t)((uint32_t)(x) & 0x00000007UL)
#define EHCI_siTD_BLP_sBP(x) (uint32_t)((uint32_t)(x) & 0xFFFFFFE0)
#define EHCI_siTD_BLP_T 0x00000001UL
 
#define EHCI_PFL_T 0x1UL
#define EHCI_PFL_TYP_iTD 0
#define EHCI_PFL_TYP_QH (1UL << 1)
#define EHCI_PFL_TYP_siTD (2UL << 1)
#define EHCI_PFL_TYP_FSTN (3UL << 1)
#define EHCI_PFL_NLP(x) (uint32_t)((uint32_t)(x) & 0xFFFFFFE0)

static inline void EHCI_DisInt(void)
{
  NVIC->ICER[2] = 0x4;
}

static inline void EHCI_EnaInt(void)
{
  NVIC->ISER[2] = 0x4;
}


void EHCI_SysInit(void);
void EHCI_SetCallback(ehci_int_type_t type, void func(void));

typedef enum{
	HCD_OK = 0,
	HCD_FULL = -1,
	HCD_INVALID_EP = -2,
	HCD_SLOT0_BUSY = -3,
	HCD_INVALID_DESC = -4,
	HCD_INVALID_STRUCT = -5,
	HCD_UNSUPPORTED_CLASS = -6,
	HCD_INVALID_DESC_CONTINUITY = -7,
	HCD_UNSUPPORTED_SAMFREQ = -8,
	HCD_INVALID_PARAM = -9,
	HCD_NULL = -10,
  HCD_INVALID_STATE = -11,
  HCD_UNSUPPORTED_CLASS_AUDIO = -12,
} hcd_Status_t;

enum{
  HCD_UNUSED,
  HCD_USED
};

typedef struct{
  uint8_t state;
  uint8_t devAddr;
  usb_psiv_t speed;
  uint8_t hubAddr;
  uint8_t hubPort;
  usbDesc_Device_t* deviceDesc;
} hcd_DeviceInfo_t;

#endif /*__EHCI_H__*/
