#ifndef VMM_H
#define VMM_H

#define PAGE_SIZE 4096 // 4KB 
#define PAGE_SHIFT 12 // 2^12 = Page Size
#define VA_BITS 48 // Virtual Address

/* Block Descriptor Bits */

#define NOT_GlOBAL                (1UL << 11) // TLB maintence across ASID
#define ACCESS                    (1UL << 10) // Recent memory access

#define SH_SHIFT                  (8) 
#define SH_MASK                   (0x3UL << SH_SHIFT) // Shareability access categories
#define SH_NON_SHAREABLE          (0x0UL << SH_SHIFT)   
#define SH_OUTER_SHAREABLE        (0x2UL << SH_SHIFT)
#define SH_INNER_SHAREABLE        (0x3UL << SH_SHIFT)  

#define AP_READ_ONLY              (1UL << 7) // Privileged read
#define AP_ALLOW_E0               (1UL << 6) // Allow user mode
#define NS                        (1UL << 5) // Secure vs Non Secure access

#define ATTRINDX_SHIFT            (2)
#define ATTRINDX_MASK             (0x7UL << ATTRINDX_SHIFT) // Memory behavior selector
#define ATTRINDX(i)               ((i) << ATTRINDX_SHIFT)

#define TABLE_DESCRIPTOR          (1UL << 1) 
#define BLOCK_DESCRIPTOR          (0UL << 1)
#define VALID                     (1UL << 0) // Entry valid

/* MAIR_EL1 */

#define DEVICE_MEMORY_nGnRnE      (0x00UL)
#define NORMAL_MEMORY_CACHEABLE   (0xFFUL)
#define MAIR_ATTR(n, val)         ((val) << ((n) * 8))
#define MAIR_VALUE                (MAIR_ATTR(0, DEVICE_MEMORY_nGnRnE) | MAIR_ATTR(1, NORMAL_MEMORY_CACHEABLE))

/* TCR_EL1 */

#define T0SZ                      (64 - VA_BITS)
#define T1SZ_SHIFT                (16)
#define T1SZ                      (64 - VA_BITS)
#define TCR_T1SZ                  (T1SZ << T1SZ_SHIFT)

#define TG0_SHIFT                 (14)
#define TG0                       (0x00UL) // 4KB
#define TCR_TG0_4KB               (TG0 << TG0_SHIFT) 
#define TG1_SHIFT                 (30)
#define TG1                       (0x2UL)
#define TCR_TG1_4KB               (TG1 << TG1_SHIFT) 

#define IPS_SHIFT                 (32)
#define IPS                       (0x1UL) // 36 bits, 64GB
#define TCR_IPS_64GB              (IPS << IPS_SHIFT)

#define IRGN0_SHIFT               (8)
#define IRGN0                     (0x1UL) // Normal memory, Inner write back
#define TCR_IRGN0_WB              (IRGN0 << IRGN0_SHIFT)
#define IRGN1_SHIFT               (24) 
#define IRGN1                     (0x1UL)
#define TCR_IRGN1_WB              (IRGN1 << IRGN1_SHIFT)

#define ORGN0_SHIFT               (10)
#define ORGN0                     (0x1UL) // Normal memory, Outer write back
#define TCR_ORGN0_WB              (ORGN0 << ORGN0_SHIFT)
#define ORGN1_SHIFT               (26)
#define ORGN1                     (0x1UL)
#define TCR_ORGN1_WB              (ORGN1 << ORGN1_SHIFT)

#define SH0_SHIFT                 (12)
#define SH0                       (0x3UL) // Inner Shareable
#define TCR_SH0_IS                (SH0 << SH0_SHIFT)
#define SH1_SHIFT                 (28)
#define SH1                       (0x3UL)
#define TCR_SH1_IS                (SH1 << SH1_SHIFT)

#define TCR_VALUE                (T0SZ            \
                                | TCR_T1SZ       \
                                | TCR_TG0_4KB    \
                                | TCR_TG1_4KB    \
                                | TCR_IPS_64GB   \
                                | TCR_IRGN0_WB   \
                                | TCR_IRGN1_WB   \
                                | TCR_ORGN0_WB   \
                                | TCR_ORGN1_WB   \
                                | TCR_SH0_IS     \
                                | TCR_SH1_IS     \
                                )

#endif 
