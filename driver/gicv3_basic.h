// ----------------------------------------------------------
// GICv3 Helper Functions for AArch64
// Header
//
// Copyright (C) Arm Limited, 2019 All rights reserved.
//
// The example code is provided to you as an aid to learning when working
// with Arm-based technology, including but not limited to programming tutorials.
// Arm hereby grants to you, subject to the terms and conditions of this Licence,
// a non-exclusive, non-transferable, non-sub-licensable, free-of-charge licence,
// to use and copy the Software solely for the purpose of demonstration and
// evaluation.
// ------------------------------------------------------------


#ifndef __gicv3_h
#define __gicv3_h

#define DEBUG_GIC       1

#include <log.h>
#include <gicv3_registers.h>
#include <stdint.h>

/* GICD_TYPER field descriptions */
#define ESPI_range_SHIFT 27
#define ESPI_range_MASK (0x1fUL << ESPI_range_SHIFT)
#define RSS_SHIFT 26
#define RSS_MASK (0x1 << RSS_SHIFT)
#define No1N_SHIFT 25
#define No1N_MASK (0x1 << No1N_SHIFT)
#define A3V_SHIFT 24
#define A3V_MASK (0x1 << A3V_SHIFT)
#define IDbits_SHIFT 19
#define IDbits_MASK (0x1f << IDbits_SHIFT)
#define DVIS_SHIFT 19
#define DVIS_MASK (0x1 << DVIS_SHIFT)
#define MBIS_SHIFT 16
#define MBIS_MASK (0x1 << MBIS_SHIFT)
#define num_LPIs_SHIFT 11
#define num_LPIs_MASK (0x1f << num_LPIs_SHIFT)
#define SecurityExtn_SHIFT 10
#define SecurityExtn_MASK (0x1 << SecurityExtn_SHIFT)
#define ESPI_SHIFT 8
#define ESPI_MASK (0x1 << ESPI_SHIFT)
#define CPUNumber_SHIFT 5
#define CPUNumber_MASK (0x7 << CPUNumber_SHIFT)
#define ITLinesNumber_SHIFT 0
#define ITLinesNumber_MASK (0x1f << ITLinesNumber_SHIFT)

/* GICR_TYPER field descriptions */
#define PPInum_SHIFT 27
#define PPInum_MASK (0x1fUL << PPInum_SHIFT)
#define VSGI_SHIFT 26
#define VSGI_MASK (0x1 << VSGI_SHIFT)
#define CommonLPIAff_SHIFT 24
#define CommonLPIAff_MASK (0x3 << CommonLPIAff_SHIFT)
#define Processor_Number_SHIFT 8
#define Processor_Number_MASK (0xffff << Processor_Number_SHIFT)
#define RVPEID_SHIFT 7
#define RVPEID_MASK (0x1 << RVPEID_SHIFT)
#define MPAM_SHIFT 6
#define MPAM_MASK (0x1 << MPAM_SHIFT)
#define DPGS_SHIFT 5
#define DPGS_MASK (0x1 << DPGS_SHIFT)
#define Last_SHIFT 4
#define Last_MASK (0x1 << Last_SHIFT)
#define DirectLPI_SHIFT 3
#define DirectLPI_MASK (0x1 << DirectLPI_SHIFT)
#define Dirty_SHIFT 2
#define Dirty_MASK (0x1 << Dirty_SHIFT)
#define VLPIS_SHIFT 1
#define VLPIS_MASK (0x1 << VLPIS_SHIFT)
#define PLPIS_SHIFT 0
#define PLPIS_MASK (0x1 << PLPIS_SHIFT)

#define GET_FIELD(reg,mask,shift)   ((reg & mask ) >> shift)

// ------------------------------------------------------------
// Address Functions
// ------------------------------------------------------------

//
// THESE FUNCTIONS MUST BE CALLED TO SET THE REGISTER FILE
// LOCATIONS BEFORE USING THE OTHER FUNCTIONS!
//

// Sets the address of the Distributor and Redistributors
// dist   = virtual address of the Distributor
// rdist  = virtual address of the first RD_base register page
void setGICAddr(void* dist, void* rdist);


// ------------------------------------------------------------
// Discovery function for Distributor and Redistributors
// ------------------------------------------------------------

// Returns the number of PPIs in the GICv3.1 extended range
uint32_t getExtPPI(uint32_t rd);

// Returns the number of SPIs in the GICv3.0 range
uint32_t getSPI(void);

// Returns the number of SPIs in the GICv3.1 extended SPI range
uint32_t getExtSPI(void);

// ------------------------------------------------------------
// Distributor Functions
// ------------------------------------------------------------

// Sets the group enable bits in the Distributor, with GICv3 mode selected
uint32_t  enableGIC(void);


// ------------------------------------------------------------
// Redistributor Functions
//
// These functions make a number of assumptions about the
// memory used for the command queue:
// * The memory is flat mapped (VA==PA)
// * The memory is either coherent or non-cacheble.
//
// ------------------------------------------------------------

// Get Redistributer number for a given affinity
// affinity
uint32_t getRedistID(uint32_t affinity);


// Wakes the currently select redistributor
// rd    = Redistributor number
uint32_t wakeUpRedist(uint32_t rd);

// ----------------------------------------------------------
// SGI, PPIs and SPI configuration functions
// ----------------------------------------------------------

// Enables the INTID
// INTID = INTID of interrupt
// rd    = Redistributor number (ignored if INTID is a SPI)
uint32_t enableInt(uint32_t INTID, uint32_t rd);


// Disables the INTID
// INTID = INTID of interrupt
// rd    = Redistributor number (ignored if INTID is a SPI)
uint32_t disableInt(uint32_t INTID, uint32_t rd);


// Sets the priority of the specified INITD
// INTID    = INTID of interrupt
// rd    = Redistributor number (ignored if INTID is a SPI)
// priority = priority (8-bit unsigned value)
uint32_t setIntPriority(uint32_t INTID, uint32_t rd, uint8_t priority);


// Sets the target CPUs of the specified INTID
// INTID    = INTID of interrupt (must be in the range 32 to 1019)
// mode     = Routing mode
// aff<n>   = Affinity co-ordinate of target
#define GICV3_ROUTE_MODE_ANY             (0x80000000)
#define GICV3_ROUTE_MODE_COORDINATE      (0)
uint32_t setIntRoute(uint32_t INTID, uint32_t mode, uint32_t affinity);


// Configures the INTID as edge or level sensitive
// INTID = INTID of interrupt
// rd    = Redistributor number (ignored if INTID is a SPI)
// conf  = Whether the INTID should edge or level (use define above)
#define GICV3_CONFIG_LEVEL               (0)
#define GICV3_CONFIG_EDGE                (2)
uint32_t setIntType(uint32_t INTID, uint32_t rd, uint32_t conf);


// Set security/group of the specified INTID
// INTID = INTID of interrupt (ID must be less than 32)
// rd    = Redistributor number (ignored if INTID is a SPI)
// group = Security/group setting
#define GICV3_GROUP0                     (0)
#define GICV3_GROUP1_NON_SECURE          (1)
#define GICV3_GROUP1_SECURE              (2)


uint32_t setIntGroup(uint32_t INTID, uint32_t rd, uint32_t group);


// Sets the pending bit of the specified INIT
// INTID   = INTID of interrupt (ID must be less than 1020)
uint32_t setIntPending(uint32_t INTID, uint32_t rd);


// Clears the pending bit of the specified INTID
// INTID   = INTID of interrupt (ID must be less than 1020)
uint32_t clearIntPending(uint32_t INTID, uint32_t rd);

// ------------------------------------------------------------
// CPU Interface functions
// ------------------------------------------------------------

// Enables group 0 interrupts
// The lowest EL that access the ICC_IGRPEN0_EL1 is determined
// by the routine of the FIQ exception.
void enableGroup0Ints(void);


// Disables group 0 interrupts
// The lowest EL that access the ICC_IGRPEN0_EL1 is determined
// by the routine of the FIQ exception.
void disableGroup0Ints(void);


// Enables group 1 interrupts for current security state
// The lowest EL that access the ICC_IGRPEN1_EL1 is determined
// by the routine of the IRQ exception.
void enableGroup1Ints(void);


// Disables group 1 interrupts for current security state
// The lowest EL that access the ICC_IGRPEN1_EL1 is determined
// by the routine of the IRQ exception.
void disableGroup1Ints(void);


// Enables group 1 interrupts for non-secure state
//
// Used by EL3 to enable non-secure group 1 interrupts
void enableNSGroup1Ints(void);


// Disables group 1 interrupts for non-secure state
//
// Used by EL3 to disable non-secure group 1 interrupts
void disableNSGroup1Ints(void);


// Returns the value of the ICC_IAR0_EL1 (Group 0 Interrupt Acknowledge)
uint32_t readIARGrp0(void);


// Writes INTID to the End Of Interrupt register
void writeEOIGrp0(uint32_t INTID);


// Writes INTID to the De-active Interrupt register
void writeDIR(uint32_t INTID);


// Returns the value of the ICC_IAR1_EL1 (Group 1 Interrupt Acknowledge)
uint32_t readIARGrp1(void);


// Writes INTID to the Aliased End Of Interrupt register
void writeEOIGrp1(uint32_t INTID);


// Sets the Priority mask register for the core run on
// priority - mask value (8-bit unsigned value).
//
// NOTE: An interrupt must be high priority (lower numeric value) than the mask to be sent
void setPriorityMask(uint32_t priority);


// Sets ICC_BPR0_EL1 for the core run on
// priority - Binary piont value
void setBPR0(uint32_t priority);


// Sets ICC_BPR1_EL1 for the core run on
// priority - Binary piont value
void setBPR1(uint32_t priority);


// Returns the priority of the current active interrupt
uint32_t getRunningPriority(void);

uint32_t get_fixed_Priority_bits(void);

// ------------------------------------------------------------
// SGIs
// ------------------------------------------------------------

#define GICV3_SGI_AFF3_SHIFT          (48)
#define GICV3_SGI_AFF2_SHIFT          (32)
#define GICV3_SGI_AFF1_SHIFT          (16)

#define GICV3_SGI_ROUTING_ALL         ((uint64_t)1 << 40)
#define GICV3_SGI_ROUTING_LIST        (0)

#define GICV3_SGI_TARGET_CPU0         (0x0001)
#define GICV3_SGI_TARGET_CPU1         (0x0002)
#define GICV3_SGI_TARGET_CPU2         (0x0004)
#define GICV3_SGI_TARGET_CPU3         (0x0008)
#define GICV3_SGI_TARGET_CPU4         (0x0010)
#define GICV3_SGI_TARGET_CPU5         (0x0020)
#define GICV3_SGI_TARGET_CPU6         (0x0040)
#define GICV3_SGI_TARGET_CPU7         (0x0080)
#define GICV3_SGI_TARGET_CPU8         (0x0100)
#define GICV3_SGI_TARGET_CPU9         (0x0200)
#define GICV3_SGI_TARGET_CPU10        (0x0400)
#define GICV3_SGI_TARGET_CPU11        (0x0800)
#define GICV3_SGI_TARGET_CPU12        (0x1000)
#define GICV3_SGI_TARGET_CPU13        (0x2000)
#define GICV3_SGI_TARGET_CPU14        (0x4000)
#define GICV3_SGI_TARGET_CPU15        (0x8000)

#define GICV3_SGI_ID0                 (0x0 << 24)
#define GICV3_SGI_ID1                 (0x1 << 24)
#define GICV3_SGI_ID2                 (0x2 << 24)
#define GICV3_SGI_ID3                 (0x3 << 24)
#define GICV3_SGI_ID4                 (0x4 << 24)
#define GICV3_SGI_ID5                 (0x5 << 24)
#define GICV3_SGI_ID6                 (0x6 << 24)
#define GICV3_SGI_ID7                 (0x7 << 24)
#define GICV3_SGI_ID8                 (0x8 << 24)
#define GICV3_SGI_ID9                 (0x9 << 24)
#define GICV3_SGI_ID10                (0xA << 24)
#define GICV3_SGI_ID11                (0xB << 24)
#define GICV3_SGI_ID12                (0xC << 24)
#define GICV3_SGI_ID13                (0xD << 24)
#define GICV3_SGI_ID14                (0xE << 24)
#define GICV3_SGI_ID15                (0xF << 24)

// Send a Group 0 SGI
void sendGroup0SGI(uint32_t INTID, uint64_t mode, uint32_t target_list);


// Send a Group 1 SGI, current security state
void sendGroup1SGI(uint32_t INTID, uint64_t mode, uint32_t target_list);


// Send a Group 1 SGI, other security state
void sendOtherGroup1SGI(uint32_t INTID, uint64_t mode, uint32_t target_list);


// Sets non-secure state ability to generate secure group 0/1 SGIs
// INTID = INTID of interrupt (must be 0 to 15)
#define GICV3_SGI_NO_NS_ACCESS        (0)
#define GICV3_SGI_NS_ACCESS_GROUP0    (0x1)
#define GICV3_SGI_NS_ACCESS_GROUP1    (0x2)

void configNSAccessSGI(uint32_t INTID, unsigned access);

uint32_t getGICDTyper(void);
uint64_t getGICRTyper(void);


extern struct GICv3_dist_if*       gic_dist;
extern struct GICv3_rdist_if*      gic_rdist;

extern unsigned int getICC_CTLR_EL3(void);
extern void setICC_CTLR_EL3(unsigned int);
extern void setICC_CTLR_EL1(unsigned int value);

unsigned int getICC_SRE_EL3(void);
unsigned int getICC_SRE_EL2(void);
unsigned int getICC_SRE_EL1(void);
#endif

// ----------------------------------------------------------
// End of gicv3_basic.h
// ----------------------------------------------------------
