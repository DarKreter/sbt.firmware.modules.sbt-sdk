#pragma once

#include <stdint.h>

/**
 * !!!WARNING!!!
 * This file is autogenerated, any changes will not be retained after next
 * generation.
 *
 * @brief In this file are struct definitions, Pack and Unpack function
 * declarations.
 */

namespace SBT::System::Comm {

// DBC file version
#define VER_CANPARSER_MAJ (0U)
#define VER_CANPARSER_MIN (0U)

#ifdef CANPARSER_USE_DIAG_MONITORS
// This file must define:
// base monitor struct
// function signature for HASH calculation: (@GetFrameHash)
// function signature for getting system tick value: (@GetSystemTick)
#include "canmonitorutil.h"

#endif // CANPARSER_USE_DIAG_MONITORS

// def @LIFEPO4_CELLS_1 CAN Message (1    0x1)
#define LIFEPO4_CELLS_1_IDE              (0U)
#define LIFEPO4_CELLS_1_DLC              (8U)
#define LIFEPO4_CELLS_1_CANID            (0x1)
// signal: @cellVoltage1
#define CANPARSER_cellVoltage1_CovFactor (0.001000)
#define CANPARSER_cellVoltage1_toS(x)                                          \
    ((uint16_t)(((x) - (0.000000)) / (0.001000)))
#define CANPARSER_cellVoltage1_fromS(x)  ((((x) * (0.001000)) + (0.000000)))
// signal: @cellVoltage2
#define CANPARSER_cellVoltage2_CovFactor (0.001000)
#define CANPARSER_cellVoltage2_toS(x)                                          \
    ((uint16_t)(((x) - (0.000000)) / (0.001000)))
#define CANPARSER_cellVoltage2_fromS(x)  ((((x) * (0.001000)) + (0.000000)))
// signal: @cellVoltage3
#define CANPARSER_cellVoltage3_CovFactor (0.001000)
#define CANPARSER_cellVoltage3_toS(x)                                          \
    ((uint16_t)(((x) - (0.000000)) / (0.001000)))
#define CANPARSER_cellVoltage3_fromS(x)  ((((x) * (0.001000)) + (0.000000)))
// signal: @cellVoltage4
#define CANPARSER_cellVoltage4_CovFactor (0.001000)
#define CANPARSER_cellVoltage4_toS(x)                                          \
    ((uint16_t)(((x) - (0.000000)) / (0.001000)))
#define CANPARSER_cellVoltage4_fromS(x)  ((((x) * (0.001000)) + (0.000000)))
// signal: @cellVoltage5
#define CANPARSER_cellVoltage5_CovFactor (0.001000)
#define CANPARSER_cellVoltage5_toS(x)                                          \
    ((uint16_t)(((x) - (0.000000)) / (0.001000)))
#define CANPARSER_cellVoltage5_fromS(x) ((((x) * (0.001000)) + (0.000000)))

typedef struct {
#ifdef CANPARSER_USE_BITS_SIGNAL

    uint16_t cellVoltage1; //      Bits=12 Factor= 0.001000        Unit:'V'

#ifdef CANPARSER_USE_SIGFLOAT
    sigfloat_t cellVoltage1_phys;
#endif // CANPARSER_USE_SIGFLOAT

    uint16_t cellVoltage2; //      Bits=12 Factor= 0.001000        Unit:'V'

#ifdef CANPARSER_USE_SIGFLOAT
    sigfloat_t cellVoltage2_phys;
#endif // CANPARSER_USE_SIGFLOAT

    uint16_t cellVoltage3; //      Bits=12 Factor= 0.001000        Unit:'V'

#ifdef CANPARSER_USE_SIGFLOAT
    sigfloat_t cellVoltage3_phys;
#endif // CANPARSER_USE_SIGFLOAT

    uint16_t cellVoltage4; //      Bits=12 Factor= 0.001000        Unit:'V'

#ifdef CANPARSER_USE_SIGFLOAT
    sigfloat_t cellVoltage4_phys;
#endif // CANPARSER_USE_SIGFLOAT

    uint16_t cellVoltage5; //      Bits=12 Factor= 0.001000        Unit:'V'

#ifdef CANPARSER_USE_SIGFLOAT
    sigfloat_t cellVoltage5_phys;
#endif // CANPARSER_USE_SIGFLOAT

#else

    uint16_t cellVoltage1; //      Bits=12 Factor= 0.001000        Unit:'V'

#ifdef CANPARSER_USE_SIGFLOAT
    sigfloat_t cellVoltage1_phys;
#endif // CANPARSER_USE_SIGFLOAT

    uint16_t cellVoltage2; //      Bits=12 Factor= 0.001000        Unit:'V'

#ifdef CANPARSER_USE_SIGFLOAT
    sigfloat_t cellVoltage2_phys;
#endif // CANPARSER_USE_SIGFLOAT

    uint16_t cellVoltage3; //      Bits=12 Factor= 0.001000        Unit:'V'

#ifdef CANPARSER_USE_SIGFLOAT
    sigfloat_t cellVoltage3_phys;
#endif // CANPARSER_USE_SIGFLOAT

    uint16_t cellVoltage4; //      Bits=12 Factor= 0.001000        Unit:'V'

#ifdef CANPARSER_USE_SIGFLOAT
    sigfloat_t cellVoltage4_phys;
#endif // CANPARSER_USE_SIGFLOAT

    uint16_t cellVoltage5; //      Bits=12 Factor= 0.001000        Unit:'V'

#ifdef CANPARSER_USE_SIGFLOAT
    sigfloat_t cellVoltage5_phys;
#endif // CANPARSER_USE_SIGFLOAT

#endif // CANPARSER_USE_BITS_SIGNAL

#ifdef CANPARSER_USE_DIAG_MONITORS

    FrameMonitor_t mon1;

#endif // CANPARSER_USE_DIAG_MONITORS

} LIFEPO4_CELLS_1_t;

// def @LIFEPO4_CELLS_2 CAN Message (2    0x2)
#define LIFEPO4_CELLS_2_IDE              (0U)
#define LIFEPO4_CELLS_2_DLC              (8U)
#define LIFEPO4_CELLS_2_CANID            (0x2)
// signal: @cellVoltage6
#define CANPARSER_cellVoltage6_CovFactor (0.001000)
#define CANPARSER_cellVoltage6_toS(x)                                          \
    ((uint16_t)(((x) - (0.000000)) / (0.001000)))
#define CANPARSER_cellVoltage6_fromS(x)  ((((x) * (0.001000)) + (0.000000)))
// signal: @cellVoltage7
#define CANPARSER_cellVoltage7_CovFactor (0.001000)
#define CANPARSER_cellVoltage7_toS(x)                                          \
    ((uint16_t)(((x) - (0.000000)) / (0.001000)))
#define CANPARSER_cellVoltage7_fromS(x)  ((((x) * (0.001000)) + (0.000000)))
// signal: @cellVoltage8
#define CANPARSER_cellVoltage8_CovFactor (0.001000)
#define CANPARSER_cellVoltage8_toS(x)                                          \
    ((uint16_t)(((x) - (0.000000)) / (0.001000)))
#define CANPARSER_cellVoltage8_fromS(x)  ((((x) * (0.001000)) + (0.000000)))
// signal: @cellVoltage9
#define CANPARSER_cellVoltage9_CovFactor (0.001000)
#define CANPARSER_cellVoltage9_toS(x)                                          \
    ((uint16_t)(((x) - (0.000000)) / (0.001000)))
#define CANPARSER_cellVoltage9_fromS(x)  ((((x) * (0.001000)) + (0.000000)))
// signal: @cellVoltageA
#define CANPARSER_cellVoltageA_CovFactor (0.001000)
#define CANPARSER_cellVoltageA_toS(x)                                          \
    ((uint16_t)(((x) - (0.000000)) / (0.001000)))
#define CANPARSER_cellVoltageA_fromS(x) ((((x) * (0.001000)) + (0.000000)))

typedef struct {
#ifdef CANPARSER_USE_BITS_SIGNAL

    uint16_t cellVoltage6; //      Bits=12 Factor= 0.001000        Unit:'V'

#ifdef CANPARSER_USE_SIGFLOAT
    sigfloat_t cellVoltage6_phys;
#endif // CANPARSER_USE_SIGFLOAT

    uint16_t cellVoltage7; //      Bits=12 Factor= 0.001000        Unit:'V'

#ifdef CANPARSER_USE_SIGFLOAT
    sigfloat_t cellVoltage7_phys;
#endif // CANPARSER_USE_SIGFLOAT

    uint16_t cellVoltage8; //      Bits=12 Factor= 0.001000        Unit:'V'

#ifdef CANPARSER_USE_SIGFLOAT
    sigfloat_t cellVoltage8_phys;
#endif // CANPARSER_USE_SIGFLOAT

    uint16_t cellVoltage9; //      Bits=12 Factor= 0.001000        Unit:'V'

#ifdef CANPARSER_USE_SIGFLOAT
    sigfloat_t cellVoltage9_phys;
#endif // CANPARSER_USE_SIGFLOAT

    uint16_t cellVoltageA; //      Bits=12 Factor= 0.001000        Unit:'V'

#ifdef CANPARSER_USE_SIGFLOAT
    sigfloat_t cellVoltageA_phys;
#endif // CANPARSER_USE_SIGFLOAT

#else

    uint16_t cellVoltage6; //      Bits=12 Factor= 0.001000        Unit:'V'

#ifdef CANPARSER_USE_SIGFLOAT
    sigfloat_t cellVoltage6_phys;
#endif // CANPARSER_USE_SIGFLOAT

    uint16_t cellVoltage7; //      Bits=12 Factor= 0.001000        Unit:'V'

#ifdef CANPARSER_USE_SIGFLOAT
    sigfloat_t cellVoltage7_phys;
#endif // CANPARSER_USE_SIGFLOAT

    uint16_t cellVoltage8; //      Bits=12 Factor= 0.001000        Unit:'V'

#ifdef CANPARSER_USE_SIGFLOAT
    sigfloat_t cellVoltage8_phys;
#endif // CANPARSER_USE_SIGFLOAT

    uint16_t cellVoltage9; //      Bits=12 Factor= 0.001000        Unit:'V'

#ifdef CANPARSER_USE_SIGFLOAT
    sigfloat_t cellVoltage9_phys;
#endif // CANPARSER_USE_SIGFLOAT

    uint16_t cellVoltageA; //      Bits=12 Factor= 0.001000        Unit:'V'

#ifdef CANPARSER_USE_SIGFLOAT
    sigfloat_t cellVoltageA_phys;
#endif // CANPARSER_USE_SIGFLOAT

#endif // CANPARSER_USE_BITS_SIGNAL

#ifdef CANPARSER_USE_DIAG_MONITORS

    FrameMonitor_t mon1;

#endif // CANPARSER_USE_DIAG_MONITORS

} LIFEPO4_CELLS_2_t;

// def @LIFEPO4_CELLS_3 CAN Message (3    0x3)
#define LIFEPO4_CELLS_3_IDE              (0U)
#define LIFEPO4_CELLS_3_DLC              (8U)
#define LIFEPO4_CELLS_3_CANID            (0x3)
// signal: @cellVoltageB
#define CANPARSER_cellVoltageB_CovFactor (0.001000)
#define CANPARSER_cellVoltageB_toS(x)                                          \
    ((uint16_t)(((x) - (0.000000)) / (0.001000)))
#define CANPARSER_cellVoltageB_fromS(x)  ((((x) * (0.001000)) + (0.000000)))
// signal: @cellVoltageC
#define CANPARSER_cellVoltageC_CovFactor (0.001000)
#define CANPARSER_cellVoltageC_toS(x)                                          \
    ((uint16_t)(((x) - (0.000000)) / (0.001000)))
#define CANPARSER_cellVoltageC_fromS(x)  ((((x) * (0.001000)) + (0.000000)))
// signal: @cellVoltageD
#define CANPARSER_cellVoltageD_CovFactor (0.001000)
#define CANPARSER_cellVoltageD_toS(x)                                          \
    ((uint16_t)(((x) - (0.000000)) / (0.001000)))
#define CANPARSER_cellVoltageD_fromS(x)  ((((x) * (0.001000)) + (0.000000)))
// signal: @cellVoltageE
#define CANPARSER_cellVoltageE_CovFactor (0.001000)
#define CANPARSER_cellVoltageE_toS(x)                                          \
    ((uint16_t)(((x) - (0.000000)) / (0.001000)))
#define CANPARSER_cellVoltageE_fromS(x) ((((x) * (0.001000)) + (0.000000)))

typedef struct {
#ifdef CANPARSER_USE_BITS_SIGNAL

    uint16_t cellVoltageB; //      Bits=12 Factor= 0.001000        Unit:'V'

#ifdef CANPARSER_USE_SIGFLOAT
    sigfloat_t cellVoltageB_phys;
#endif // CANPARSER_USE_SIGFLOAT

    uint16_t cellVoltageC; //      Bits=12 Factor= 0.001000        Unit:'V'

#ifdef CANPARSER_USE_SIGFLOAT
    sigfloat_t cellVoltageC_phys;
#endif // CANPARSER_USE_SIGFLOAT

    uint16_t cellVoltageD; //      Bits=12 Factor= 0.001000        Unit:'V'

#ifdef CANPARSER_USE_SIGFLOAT
    sigfloat_t cellVoltageD_phys;
#endif // CANPARSER_USE_SIGFLOAT

    uint16_t cellVoltageE; //      Bits=12 Factor= 0.001000        Unit:'V'

#ifdef CANPARSER_USE_SIGFLOAT
    sigfloat_t cellVoltageE_phys;
#endif // CANPARSER_USE_SIGFLOAT

#else

    uint16_t cellVoltageB; //      Bits=12 Factor= 0.001000        Unit:'V'

#ifdef CANPARSER_USE_SIGFLOAT
    sigfloat_t cellVoltageB_phys;
#endif // CANPARSER_USE_SIGFLOAT

    uint16_t cellVoltageC; //      Bits=12 Factor= 0.001000        Unit:'V'

#ifdef CANPARSER_USE_SIGFLOAT
    sigfloat_t cellVoltageC_phys;
#endif // CANPARSER_USE_SIGFLOAT

    uint16_t cellVoltageD; //      Bits=12 Factor= 0.001000        Unit:'V'

#ifdef CANPARSER_USE_SIGFLOAT
    sigfloat_t cellVoltageD_phys;
#endif // CANPARSER_USE_SIGFLOAT

    uint16_t cellVoltageE; //      Bits=12 Factor= 0.001000        Unit:'V'

#ifdef CANPARSER_USE_SIGFLOAT
    sigfloat_t cellVoltageE_phys;
#endif // CANPARSER_USE_SIGFLOAT

#endif // CANPARSER_USE_BITS_SIGNAL

#ifdef CANPARSER_USE_DIAG_MONITORS

    FrameMonitor_t mon1;

#endif // CANPARSER_USE_DIAG_MONITORS

} LIFEPO4_CELLS_3_t;

// def @LIFEPO4_GENERAL CAN Message (4    0x4)
#define LIFEPO4_GENERAL_IDE               (0U)
#define LIFEPO4_GENERAL_DLC               (8U)
#define LIFEPO4_GENERAL_CANID             (0x4)
// signal: @voltage
#define CANPARSER_voltage_CovFactor       (0.001000)
#define CANPARSER_voltage_toS(x)          ((uint16_t)(((x) - (0.000000)) / (0.001000)))
#define CANPARSER_voltage_fromS(x)        ((((x) * (0.001000)) + (0.000000)))
// signal: @currentCharge
#define CANPARSER_currentCharge_CovFactor (0.100000)
#define CANPARSER_currentCharge_toS(x)                                         \
    ((uint16_t)(((x) - (0.000000)) / (0.100000)))
#define CANPARSER_currentCharge_fromS(x)     ((((x) * (0.100000)) + (0.000000)))
// signal: @currentDischarge
#define CANPARSER_currentDischarge_CovFactor (0.100000)
#define CANPARSER_currentDischarge_toS(x)                                      \
    ((uint16_t)(((x) - (0.000000)) / (0.100000)))
#define CANPARSER_currentDischarge_fromS(x) ((((x) * (0.100000)) + (0.000000)))

typedef struct {
#ifdef CANPARSER_USE_BITS_SIGNAL

    uint16_t voltage; //      Bits=16 Factor= 0.001000        Unit:'V'

#ifdef CANPARSER_USE_SIGFLOAT
    sigfloat_t voltage_phys;
#endif // CANPARSER_USE_SIGFLOAT

    uint16_t currentCharge; //      Bits=12 Factor= 0.100000        Unit:'A'

#ifdef CANPARSER_USE_SIGFLOAT
    sigfloat_t currentCharge_phys;
#endif // CANPARSER_USE_SIGFLOAT

    uint16_t currentDischarge; //      Bits=12 Factor= 0.100000        Unit:'A'

#ifdef CANPARSER_USE_SIGFLOAT
    sigfloat_t currentDischarge_phys;
#endif // CANPARSER_USE_SIGFLOAT

    uint8_t temperature; //      Bits= 8 Unit:'Celcius'

    uint8_t capacity; //      Bits= 8 Unit:'Percent'

#else

    uint16_t voltage; //      Bits=16 Factor= 0.001000        Unit:'V'

#ifdef CANPARSER_USE_SIGFLOAT
    sigfloat_t voltage_phys;
#endif // CANPARSER_USE_SIGFLOAT

    uint16_t currentCharge; //      Bits=12 Factor= 0.100000        Unit:'A'

#ifdef CANPARSER_USE_SIGFLOAT
    sigfloat_t currentCharge_phys;
#endif // CANPARSER_USE_SIGFLOAT

    uint16_t currentDischarge; //      Bits=12 Factor= 0.100000        Unit:'A'

#ifdef CANPARSER_USE_SIGFLOAT
    sigfloat_t currentDischarge_phys;
#endif // CANPARSER_USE_SIGFLOAT

    uint8_t temperature; //      Bits= 8 Unit:'Celcius'

    uint8_t capacity; //      Bits= 8 Unit:'Percent'

#endif // CANPARSER_USE_BITS_SIGNAL

#ifdef CANPARSER_USE_DIAG_MONITORS

    FrameMonitor_t mon1;

#endif // CANPARSER_USE_DIAG_MONITORS

} LIFEPO4_GENERAL_t;

// def @MPPT_GENERAL CAN Message (4    0x4)
#define MPPT_GENERAL_IDE                 (0U)
#define MPPT_GENERAL_DLC                 (8U)
#define MPPT_GENERAL_CANID               (0x4)
// signal: @panelVoltage
#define CANPARSER_panelVoltage_CovFactor (0.001000)
#define CANPARSER_panelVoltage_toS(x)                                          \
    ((uint32_t)(((x) - (0.000000)) / (0.001000)))
#define CANPARSER_panelVoltage_fromS(x)  ((((x) * (0.001000)) + (0.000000)))
// signal: @panelCurrent
#define CANPARSER_panelCurrent_CovFactor (0.100000)
#define CANPARSER_panelCurrent_toS(x)                                          \
    ((uint16_t)(((x) - (0.000000)) / (0.100000)))
#define CANPARSER_panelCurrent_fromS(x)    ((((x) * (0.100000)) + (0.000000)))
// signal: @chargerCurrent
#define CANPARSER_chargerCurrent_CovFactor (0.100000)
#define CANPARSER_chargerCurrent_toS(x)                                        \
    ((uint16_t)(((x) - (0.000000)) / (0.100000)))
#define CANPARSER_chargerCurrent_fromS(x) ((((x) * (0.100000)) + (0.000000)))
// signal: @panelPower
#define CANPARSER_panelPower_CovFactor    (0.100000)
#define CANPARSER_panelPower_toS(x)                                            \
    ((uint16_t)(((x) - (0.000000)) / (0.100000)))
#define CANPARSER_panelPower_fromS(x) ((((x) * (0.100000)) + (0.000000)))

typedef struct {
#ifdef CANPARSER_USE_BITS_SIGNAL

    uint32_t panelVoltage; //      Bits=18 Factor= 0.001000        Unit:'V'

#ifdef CANPARSER_USE_SIGFLOAT
    sigfloat_t panelVoltage_phys;
#endif // CANPARSER_USE_SIGFLOAT

    uint16_t panelCurrent; //      Bits=12 Factor= 0.100000        Unit:'A'

#ifdef CANPARSER_USE_SIGFLOAT
    sigfloat_t panelCurrent_phys;
#endif // CANPARSER_USE_SIGFLOAT

    uint16_t chargerCurrent; //      Bits=12 Factor= 0.100000        Unit:'A'

#ifdef CANPARSER_USE_SIGFLOAT
    sigfloat_t chargerCurrent_phys;
#endif // CANPARSER_USE_SIGFLOAT

    uint16_t panelPower; //      Bits=16 Factor= 0.100000        Unit:'W'

#ifdef CANPARSER_USE_SIGFLOAT
    sigfloat_t panelPower_phys;
#endif // CANPARSER_USE_SIGFLOAT

#else

    uint32_t panelVoltage; //      Bits=18 Factor= 0.001000        Unit:'V'

#ifdef CANPARSER_USE_SIGFLOAT
    sigfloat_t panelVoltage_phys;
#endif // CANPARSER_USE_SIGFLOAT

    uint16_t panelCurrent; //      Bits=12 Factor= 0.100000        Unit:'A'

#ifdef CANPARSER_USE_SIGFLOAT
    sigfloat_t panelCurrent_phys;
#endif // CANPARSER_USE_SIGFLOAT

    uint16_t chargerCurrent; //      Bits=12 Factor= 0.100000        Unit:'A'

#ifdef CANPARSER_USE_SIGFLOAT
    sigfloat_t chargerCurrent_phys;
#endif // CANPARSER_USE_SIGFLOAT

    uint16_t panelPower; //      Bits=16 Factor= 0.100000        Unit:'W'

#ifdef CANPARSER_USE_SIGFLOAT
    sigfloat_t panelPower_phys;
#endif // CANPARSER_USE_SIGFLOAT

#endif // CANPARSER_USE_BITS_SIGNAL

#ifdef CANPARSER_USE_DIAG_MONITORS

    FrameMonitor_t mon1;

#endif // CANPARSER_USE_DIAG_MONITORS

} MPPT_GENERAL_t;

// def @HEARTBEAT CAN Message (4    0x4)
#define HEARTBEAT_IDE              (0U)
#define HEARTBEAT_DLC              (8U)
#define HEARTBEAT_CANID            (0x4)
// signal: @upTime
#define CANPARSER_upTime_CovFactor (0.001000)
#define CANPARSER_upTime_toS(x)    ((uint32_t)(((x) - (0.000000)) / (0.001000)))
#define CANPARSER_upTime_fromS(x)  ((((x) * (0.001000)) + (0.000000)))

typedef struct {
#ifdef CANPARSER_USE_BITS_SIGNAL

    uint32_t upTime; //      Bits=32 Factor= 0.001000        Unit:'s'

#ifdef CANPARSER_USE_SIGFLOAT
    sigfloat_t upTime_phys;
#endif // CANPARSER_USE_SIGFLOAT

    uint8_t canTxMessFailCount; //      Bits= 8

    uint8_t canRxMessFailCount; //      Bits= 8

#else

    uint32_t upTime; //      Bits=32 Factor= 0.001000        Unit:'s'

#ifdef CANPARSER_USE_SIGFLOAT
    sigfloat_t upTime_phys;
#endif // CANPARSER_USE_SIGFLOAT

    uint8_t canTxMessFailCount; //      Bits= 8

    uint8_t canRxMessFailCount; //      Bits= 8

#endif // CANPARSER_USE_BITS_SIGNAL

#ifdef CANPARSER_USE_DIAG_MONITORS

    FrameMonitor_t mon1;

#endif // CANPARSER_USE_DIAG_MONITORS

} HEARTBEAT_t;

// Function signatures

/**
 * @brief Unpacks raw CAN frame payload into LIFEPO4_CELLS_1_t struct
 * @param _d pointer to payload to unpack
 * @return LIFEPO4_CELLS_1_t unpacked object
 */
[[nodiscard]] LIFEPO4_CELLS_1_t Unpack_LIFEPO4_CELLS_1(const uint8_t* _d);
#ifdef CANPARSER_USE_CANSTRUCT
void Pack_LIFEPO4_CELLS_1(LIFEPO4_CELLS_1_t* _m,
                          __CoderDbcCanFrame_t__* cframe);
#else
/**
 * @brief Packs LIFEPO4_CELLS_1_t object into raw 8-byte long payload
 * @param _m pointer to LIFEPO4_CELLS_1_t object to pack
 * @param _d pointer to payload, where LIFEPO4_CELLS_1_t object will be packed
 */
void Pack_LIFEPO4_CELLS_1(LIFEPO4_CELLS_1_t* _m, uint8_t* _d);
#endif // CANPARSER_USE_CANSTRUCT

/**
 * @brief Unpacks raw CAN frame payload into LIFEPO4_CELLS_2_t struct
 * @param _d pointer to payload to unpack
 * @return LIFEPO4_CELLS_2_t unpacked object
 */
[[nodiscard]] LIFEPO4_CELLS_2_t Unpack_LIFEPO4_CELLS_2(const uint8_t* _d);
#ifdef CANPARSER_USE_CANSTRUCT
void Pack_LIFEPO4_CELLS_2(LIFEPO4_CELLS_2_t* _m,
                          __CoderDbcCanFrame_t__* cframe);
#else
/**
 * @brief Packs LIFEPO4_CELLS_2_t object into raw 8-byte long payload
 * @param _m pointer to LIFEPO4_CELLS_2_t object to pack
 * @param _d pointer to payload, where LIFEPO4_CELLS_2_t object will be packed
 */
void Pack_LIFEPO4_CELLS_2(LIFEPO4_CELLS_2_t* _m, uint8_t* _d);
#endif // CANPARSER_USE_CANSTRUCT

/**
 * @brief Unpacks raw CAN frame payload into LIFEPO4_CELLS_3_t struct
 * @param _d pointer to payload to unpack
 * @return LIFEPO4_CELLS_3_t unpacked object
 */
[[nodiscard]] LIFEPO4_CELLS_3_t Unpack_LIFEPO4_CELLS_3(const uint8_t* _d);
#ifdef CANPARSER_USE_CANSTRUCT
void Pack_LIFEPO4_CELLS_3(LIFEPO4_CELLS_3_t* _m,
                          __CoderDbcCanFrame_t__* cframe);
#else
/**
 * @brief Packs LIFEPO4_CELLS_3_t object into raw 8-byte long payload
 * @param _m pointer to LIFEPO4_CELLS_3_t object to pack
 * @param _d pointer to payload, where LIFEPO4_CELLS_3_t object will be packed
 */
void Pack_LIFEPO4_CELLS_3(LIFEPO4_CELLS_3_t* _m, uint8_t* _d);
#endif // CANPARSER_USE_CANSTRUCT

/**
 * @brief Unpacks raw CAN frame payload into LIFEPO4_GENERAL_t struct
 * @param _d pointer to payload to unpack
 * @return LIFEPO4_GENERAL_t unpacked object
 */
[[nodiscard]] LIFEPO4_GENERAL_t Unpack_LIFEPO4_GENERAL(const uint8_t* _d);
#ifdef CANPARSER_USE_CANSTRUCT
void Pack_LIFEPO4_GENERAL(LIFEPO4_GENERAL_t* _m,
                          __CoderDbcCanFrame_t__* cframe);
#else
/**
 * @brief Packs LIFEPO4_GENERAL_t object into raw 8-byte long payload
 * @param _m pointer to LIFEPO4_GENERAL_t object to pack
 * @param _d pointer to payload, where LIFEPO4_GENERAL_t object will be packed
 */
void Pack_LIFEPO4_GENERAL(LIFEPO4_GENERAL_t* _m, uint8_t* _d);
#endif // CANPARSER_USE_CANSTRUCT

/**
 * @brief Unpacks raw CAN frame payload into MPPT_GENERAL_t struct
 * @param _d pointer to payload to unpack
 * @return MPPT_GENERAL_t unpacked object
 */
[[nodiscard]] MPPT_GENERAL_t Unpack_MPPT_GENERAL(const uint8_t* _d);
#ifdef CANPARSER_USE_CANSTRUCT
void Pack_MPPT_GENERAL(MPPT_GENERAL_t* _m, __CoderDbcCanFrame_t__* cframe);
#else
/**
 * @brief Packs MPPT_GENERAL_t object into raw 8-byte long payload
 * @param _m pointer to MPPT_GENERAL_t object to pack
 * @param _d pointer to payload, where MPPT_GENERAL_t object will be packed
 */
void Pack_MPPT_GENERAL(MPPT_GENERAL_t* _m, uint8_t* _d);
#endif // CANPARSER_USE_CANSTRUCT

/**
 * @brief Unpacks raw CAN frame payload into HEARTBEAT_t struct
 * @param _d pointer to payload to unpack
 * @return HEARTBEAT_t unpacked object
 */
[[nodiscard]] HEARTBEAT_t Unpack_HEARTBEAT(const uint8_t* _d);
#ifdef CANPARSER_USE_CANSTRUCT
void Pack_HEARTBEAT(HEARTBEAT_t* _m, __CoderDbcCanFrame_t__* cframe);
#else
/**
 * @brief Packs HEARTBEAT_t object into raw 8-byte long payload
 * @param _m pointer to HEARTBEAT_t object to pack
 * @param _d pointer to payload, where HEARTBEAT_t object will be packed
 */
void Pack_HEARTBEAT(HEARTBEAT_t* _m, uint8_t* _d);
#endif // CANPARSER_USE_CANSTRUCT

} // namespace SBT::System::Comm
