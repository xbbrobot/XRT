/*
 * This file is part of the Xilinx DMA IP Core driver for Linux
 *
 * Copyright (c) 2017-present,  Xilinx, Inc.
 * All rights reserved.
 *
 * This source code is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * The full GNU General Public License is included in this distribution in
 * the file called "COPYING".
 */

#ifndef __XDEV_H__
#define __XDEV_H__
/**
 * @file
 * @brief This file contains the declarations for QDMA PCIe device
 *
 */
#include <linux/types.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <linux/pci.h>

#include "libqdma_export.h"
#include "qdma_qconf_mgr.h"
#ifdef DEBUGFS
#include "qdma_debugfs.h"
#endif

/**
 * QDMA bars
 */
#define QDMA_BAR_NUM			6
/**
 * QDMA config bar size - 64MB
 */
#define QDMA_MAX_BAR_LEN_MAPPED		0x4000000

/*
 *module_param_array:
 *config_bar=<bus_num(16bits)><pf0_config_bar(4bits)><pf1_config_bar(4bits)>
 *		<pf2_config_bar(4bits)><pf3_config_bar(4bits)>
 *config_bar=<bus_num(16bits)><vf_pf0_config_bar(4bits)>
 *		<vf_pf1_config_bar(4bits)><vf_pf2_config_bar(4bits)>
 *		<vf_pf3_config_bar(4bits)>
 *
 */
#define BUS_NUM_MASK			0xFFFF0000
#define BUS_NUM_SHIFT			16

#define PF_DEV_0_MASK			0x0000F000
#define PF_DEV_0_SHIFT			12
#define PF_DEV_1_MASK			0x00000F00
#define PF_DEV_1_SHIFT			8
#define PF_DEV_2_MASK			0x000000F0
#define PF_DEV_2_SHIFT			4
#define PF_DEV_3_MASK			0x0000000F
#define PF_DEV_3_SHIFT			0

#define VF_PF_IDENTIFIER_MASK	0xF
#define VF_PF_IDENTIFIER_SHIFT  8

enum qdma_pf_devices {
	PF_DEVICE_0 = 0,
	PF_DEVICE_1,
	PF_DEVICE_2,
	PF_DEVICE_3
};
/**
 * number of bits to describe the DMA transfer descriptor
 */
#define QDMA_DESC_BLEN_BITS	28
/**
 * maximum size of a single DMA transfer descriptor
 */
#define QDMA_DESC_BLEN_MAX	((1 << (QDMA_DESC_BLEN_BITS)) - 1)

/**
 * obtain the 32 most significant (high) bits of a 32-bit or 64-bit address
 */
#define PCI_DMA_H(addr) ((addr >> 16) >> 16)
/**
 * obtain the 32 least significant (low) bits of a 32-bit or 64-bit address
 */
#define PCI_DMA_L(addr) (addr & 0xffffffffUL)

/**
 * Xiling DMA device forward declaration
 */
struct xlnx_dma_dev;

/* XDMA PCIe device specific book-keeping */
/**
 * Flag for device offline
 */
#define XDEV_FLAG_OFFLINE	0x1
/**
 * Flag for IRQ
 */
#define XDEV_FLAG_IRQ		0x2
/**
 * Maximum number of interrupts supported per device
 */
#define XDEV_NUM_IRQ_MAX	8

/**
 * interrupt call back function handlers
 */
typedef irqreturn_t (*f_intr_handler)(int irq_index, int irq, void *dev_id);

/**
 * @struct - intr_coal_conf
 * @brief	interrut coalescing configuration
 */
struct intr_coal_conf {
	/**< interrupt vector index */
	u16 vec_id;
	/**< number of entries in interrupt ring per vector */
	u16 intr_rng_num_entries;
	/**< interrupt ring base address */
	dma_addr_t intr_ring_bus;
	struct qdma_intr_ring *intr_ring_base;
	/**< color value indicates the valid entry in the interrupt ring */
	u8 color;
	/**< interrupt ring consumer index */
	unsigned int cidx;
};

/**
 * Macros for Hardware Version info
 */
#define RTL1_VERSION                      0
#define RTL2_VERSION                      1
#define VIVADO_RELEASE_2018_3             0
#define VIVADO_RELEASE_2018_2             1
#define EVEREST_SOFT_IP                   0
#define EVEREST_HARD_IP                   1

/**
 * intr_type_list - interrupt types
 */
enum intr_type_list {
	INTR_TYPE_ERROR,	/**< error interrupt */
	INTR_TYPE_USER,		/**< user interrupt */
	INTR_TYPE_DATA,		/**< data interrupt */
	INTR_TYPE_MAX		/**< max interrupt */
};

#define QDMA_USER_INTR_NUM	7

/**
 * @struct - intr_vec_map_type
 * @brief	interrupt vector map details
 */
struct intr_vec_map_type {
	enum intr_type_list intr_type;	/**< interrupt type */
	int intr_vec_index;		/**< interrupt vector index */
	f_intr_handler intr_handler;	/**< interrupt handler */
};

/**< Interrupt info for MSI-X interrupt vectors per device */
struct intr_info_t {
	/**< msix_entry list for all vectors */
	char msix_name[QDMA_DEV_NAME_MAXLEN + 16];
	/**< queue list for each interrupt */
	struct list_head intr_list;
	/**< number of queues assigned for each interrupt */
	int intr_list_cnt;
	/**< interrupt vector map */
	struct intr_vec_map_type intr_vec_map;
};

/**
 * @struct - xlnx_dma_dev
 * @brief	Xilinx DMA device details
 */
struct xlnx_dma_dev {
	/**< Xilinx DMA device name */
	char mod_name[QDMA_DEV_NAME_MAXLEN];
	/**< DMA device configuration */
	struct qdma_dev_conf conf;
	/**< DMA device list */
	struct list_head list_head;
	/**< DMA device lock to protects concurrent access */
	spinlock_t lock;
	/**< DMA device hardware program lock */
	spinlock_t hw_prg_lock;
	/**< device flags */
	unsigned int flags;
	u8 flr_prsnt:1;
	/**< flag to indicate the FLR present status */
	u8 st_mode_en:1;
	/**< flag to indicate the streaming mode enabled status */
	u8 mm_mode_en:1;
	/**< flag to indicate the memory mapped mode enabled status */
	u8 stm_en:1;
	/**< flag to indicate the presence of STM */
	/**< sriov info */
	void *vf_info;
	/**< number of virtual functions */
	u8 vf_count;
	/**< function id */
	u16 func_id;
#ifdef __QDMA_VF__
	/**< parent function id, valid only for virtual function */
	u8 func_id_parent;
#else
	/**< number of physical functions */
	u8 pf_count;
#endif
	/**< max mm channels */
	u8 mm_channel_max;
	u8 stm_rev;
	/**< PCIe config. bar */
	void __iomem *regs;
	/** PCIe Bar for STM config */
	void __iomem *stm_regs;

	/**< number of MSI-X interrupt vectors per device */
	int num_vecs;
	/**< msix_entry list for all MSIx vectors associated for device */
	struct msix_entry *msix;
	/**< interrupt info list for all MSIx vectors associated for device */
	struct intr_info_t *dev_intr_info_list;
	/**< data vector start index */
	int dvec_start_idx;
	/**< DMA private device to hold the qdma que details */
	void *dev_priv;
	/**< dsa configured max pkt size that STM can support */
	u32 pipe_stm_max_pkt_size;
	/**< list of interrupt coalescing configuration for each vector */
	struct intr_coal_conf  *intr_coal_list;
	/**< legacy interrupt vector */
	int vector_legacy;

#ifdef DEBUGFS
	/** debugfs device root */
	struct dentry *dbgfs_dev_root;
	/** debugfs queue root */
	struct dentry *dbgfs_queues_root;
	/* lock for creating qidx directory */
	spinlock_t qidx_lock;
#endif

	/** number of packets processed in pf */
	unsigned long long total_mm_h2c_pkts;
	unsigned long long total_mm_c2h_pkts;
	unsigned long long total_st_h2c_pkts;
	unsigned long long total_st_c2h_pkts;
	/**< for upper layer calling function */
	unsigned int dev_ulf_extra[0];
};

/*****************************************************************************/
/**
 * xlnx_dma_device_flag_check() - helper function to check the flag status
 *
 * @param[in]	xdev:	pointer to xilinx dma device
 * @param[in]	f:	flag value
 *
 *
 * @return	1 if the flag is on
 * @return	0 if the flag is off
 *****************************************************************************/
static inline int xlnx_dma_device_flag_check(struct xlnx_dma_dev *xdev,
					unsigned int f)
{
	unsigned long flags;

	spin_lock_irqsave(&xdev->lock, flags);
	if (xdev->flags & f) {
		spin_unlock_irqrestore(&xdev->lock, flags);
		return 1;
	}
	spin_unlock_irqrestore(&xdev->lock, flags);
	return 0;
}

/*****************************************************************************/
/**
 * xlnx_dma_device_flag_test_n_set() - helper function to test n set the flag
 *
 * @param[in]	xdev:	pointer to xilinx dma device
 * @param[in]	f:	flag value
 *
 *
 * @return	1 if the flag is already enabled
 * @return	0 if the flag is off
 *****************************************************************************/
static inline int xlnx_dma_device_flag_test_n_set(struct xlnx_dma_dev *xdev,
					 unsigned int f)
{
	unsigned long flags;
	int rv = 0;

	spin_lock_irqsave(&xdev->lock, flags);
	if (xdev->flags & f) {
		spin_unlock_irqrestore(&xdev->lock, flags);
		rv = 1;
	} else
		xdev->flags |= f;
	spin_unlock_irqrestore(&xdev->lock, flags);
	return rv;
}

/*****************************************************************************/
/**
 * xdev_flag_set() - helper function to set the device flag
 *
 * @param[in]	xdev:	pointer to xilinx dma device
 * @param[in]	f:	flag value
 *
 *
 * @return	none
 *****************************************************************************/
static inline void xdev_flag_set(struct xlnx_dma_dev *xdev, unsigned int f)
{
	unsigned long flags;

	spin_lock_irqsave(&xdev->lock, flags);
	xdev->flags |= f;
	spin_unlock_irqrestore(&xdev->lock, flags);
}

/*****************************************************************************/
/**
 * xlnx_dma_device_flag_test_n_set() - helper function to clear the device flag
 *
 * @param[in]	xdev:	pointer to xilinx dma device
 * @param[in]	f:	flag value
 *
 * @return	none
 *****************************************************************************/
static inline void xdev_flag_clear(struct xlnx_dma_dev *xdev, unsigned int f)
{
	unsigned long flags;

	spin_lock_irqsave(&xdev->lock, flags);
	xdev->flags &= ~f;
	spin_unlock_irqrestore(&xdev->lock, flags);
}

/*****************************************************************************/
/**
 * xdev_find_by_pdev() - find the xdev using struct pci_dev
 *
 * @param[in]	pdev:	pointer to struct pci_dev
 *
 * @return	pointer to xlnx_dma_dev on success
 * @return	NULL on failure
 *****************************************************************************/
struct xlnx_dma_dev *xdev_find_by_pdev(struct pci_dev *pdev);

/*****************************************************************************/
/**
 * xdev_find_by_idx() - find the xdev using the index value
 *
 * @param[in]	idx:	index value in the xdev list
 *
 * @return	pointer to xlnx_dma_dev on success
 * @return	NULL on failure
 *****************************************************************************/
struct xlnx_dma_dev *xdev_find_by_idx(int idx);

/*****************************************************************************/
/**
 * xdev_list_first() - handler to return the first xdev entry from the list
 *
 * @return	pointer to first xlnx_dma_dev on success
 * @return	NULL on failure
 *****************************************************************************/
struct xlnx_dma_dev *xdev_list_first(void);

/*****************************************************************************/
/**
 * xdev_list_next() - handler to return the next xdev entry from the list
 *
 * @param[in]	xdev:	pointer to current xdev
 *
 * @return	pointer to next xlnx_dma_dev on success
 * @return	NULL on failure
 *****************************************************************************/
struct xlnx_dma_dev *xdev_list_next(struct xlnx_dma_dev *xdev);

/*****************************************************************************/
/**
 * xdev_list_dump() - list the dma device details
 *
 * @param[in]	buflen:		length of the input buffer
 * @param[out]	buf:		message buffer
 *
 * @return	pointer to next xlnx_dma_dev on success
 * @return	NULL on failure
 *****************************************************************************/
int xdev_list_dump(char *buf, int buflen);

/*****************************************************************************/
/**
 * xdev_check_hndl() - helper function to validate the device handle
 *
 * @param[in]	f:		device name
 * @param[in]	pdev:	pointer to struct pci_dev
 * @param[in]	hndl:	device handle
 *
 * @return	0: success
 * @return	EINVAL: on failure
 *****************************************************************************/
int xdev_check_hndl(const char *f, struct pci_dev *pdev, unsigned long hndl);


#ifdef __QDMA_VF__
/*****************************************************************************/
/**
 * xdev_sriov_vf_offline() - API to set the virtual function to offline mode
 *
 * @param[in]	xdev:		pointer to xdev
 * @param[in]	func_id:	function identifier
 *
 * @return	0: success
 * @return	-1: on failure
 *****************************************************************************/
int xdev_sriov_vf_offline(struct xlnx_dma_dev *xdev, u8 func_id);

/*****************************************************************************/
/**
 * xdev_sriov_vf_online() - API to set the virtual function to online mode
 *
 * @param[in]	xdev:		pointer to xdev
 * @param[in]	func_id:	function identifier
 *
 * @return	0: success
 * @return	-1: on failure
 *****************************************************************************/
int xdev_sriov_vf_online(struct xlnx_dma_dev *xdev, u8 func_id);
#elif defined(CONFIG_PCI_IOV)
/* SR-IOV */
/*****************************************************************************/
/**
 * xdev_sriov_vf_online() - API to disable the virtual function
 *
 * @param[in]	xdev:		pointer to xdev
 *
 * @return	none
 *****************************************************************************/
void xdev_sriov_disable(struct xlnx_dma_dev *xdev);

/*****************************************************************************/
/**
 * xdev_sriov_vf_online() - API to enable the virtual function
 *
 * @param[in]	xdev:		pointer to xdev
 * @param[in]	func_id:	function identifier
 *
 * @return	number of vfs enabled on success
 * @return	-1: on failure
 *****************************************************************************/
int xdev_sriov_enable(struct xlnx_dma_dev *xdev, int num_vfs);

/*****************************************************************************/
/**
 * xdev_sriov_vf_offline() - API to set the virtual function to offline mode
 *
 * @param[in]	xdev:		pointer to xdev
 * @param[in]	func_id:	function identifier
 *
 * @return	none
 *****************************************************************************/
void xdev_sriov_vf_offline(struct xlnx_dma_dev *xdev, u8 func_id);

/*****************************************************************************/
/**
 * xdev_sriov_vf_offline() - API to set the virtual function to offline mode
 *
 * @param[in]	xdev:		pointer to xdev
 * @param[in]	func_id:	function identifier
 *
 * @return	0: success
 * @return	-1: on failure
 *****************************************************************************/
int xdev_sriov_vf_online(struct xlnx_dma_dev *xdev, u8 func_id);

/*****************************************************************************/
/**
 * xdev_sriov_vf_offline() - API to configure the fmap for virtual function
 *
 * @param[in]	xdev:		pointer to xdev
 * @param[in]	func_id:	function identifier
 * @param[in]	qbase:		queue start
 * @param[in]	qmax:		queue max
 *
 * @return	0: success
 * @return	-1: on failure
 *****************************************************************************/
int xdev_sriov_vf_fmap(struct xlnx_dma_dev *xdev, u8 func_id,
			unsigned short qbase, unsigned short qmax);
#else
/** dummy declaration for xdev_sriov_disable()
 *  When virtual function is not enabled
 */
#define xdev_sriov_disable(xdev)
/** dummy declaration for xdev_sriov_enable()
 *  When virtual function is not enabled
 */
#define xdev_sriov_enable(xdev, num_vfs)
/** dummy declaration for xdev_sriov_vf_offline()
 *  When virtual function is not enabled
 */
#define xdev_sriov_vf_offline(xdev, func_id)
/** dummy declaration for xdev_sriov_vf_online()
 *  When virtual function is not enabled
 */
#define xdev_sriov_vf_online(xdev, func_id)
#endif

#endif /* XDMA_LIB_H */
