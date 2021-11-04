#ifndef PARAMETER_DEFINITION_HPP
#define PARAMETER_DEFINITION_HPP

/* If You want to add new box, add new field to BoxId with unique ID.
 * If You want to add new parameters that will be sent over CAN, add new fields to ParameterId with unique ID.
 */
enum class BoxId : uint32_t
{
    BOX1 = 0x001, //KLS
    BOX2 = 0x002, //MPPT
    BOX3 = 0x004, //Battery
    BOX4 = 0x008, //Raspberry Pi, RFD, 4G
    BOX5 = 0x010, //QNA
    BOX6 = 0x020, //Pitot, forward draft, valve pump,
    BOX7 = 0x040, //rear draft, pump with valve
    BOX8 = 0x080, //Electric hub - fuses
    AHRS = 0x100,
};

enum class ParameterId : uint16_t
{
    //-----------------------------------------------------------------------------------------------------------
    // NAME                ID            Description                     Type       Unit         Range
    //-----------------------------------------------------------------------------------------------------------
    
    /*** BOX 1 ***/
    // KLS
    kls_speed             = 0x00,   //Actual speed                      int32_t     rpm         0-600
    kls_current,                    //Actual motor current              int32_t     0,1A        0,0 - 400,0
    kls_bateryVoltage,              //Actual batery voltage             int32_t     0,1V        0,0 - 180,0
    kls_throtleSignal,              //Throtle signal                    int32_t      V          0~255 maps 0~5V
    kls_controllerTemperature,      //Controller temperature            int32_t     *C          (offset 40)
    kls_motorTemperature,           //Controller temperature            int32_t     *C          (offset 30)

    /*** BOX 2 ***/
    //MPPT 1
     mppt1_panelPower       = 0x10, //  0xEDBC Panel power              float       W
     mppt1_panelVoltage,            //  0xEDBB Panel voltage            float       V
     mppt1_panelCurrent,            //  0xEDBD Panel current (1)        float       A
     mppt1_chargerCurrent,          //  0xEDD7 Charger current (3)      float       A

    //MPPT 2
     mppt2_panelPower       = 0x20, //  0xEDBC Panel power              float       W
     mppt2_panelVoltage,            //  0xEDBB Panel voltage            float       V
     mppt2_panelCurrent,            //  0xEDBD Panel current (1)        float       A
     mppt2_chargerCurrent,          //  0xEDD7 Charger current (3)      float       A

    //MPPT 3
    mppt3_panelPower        = 0x30, //  0xEDBC Panel power              float       W
    mppt3_panelVoltage,             //  0xEDBB Panel voltage            float       V
    mppt3_panelCurrent,             //  0xEDBD Panel current (1)        float       A
    mppt3_chargerCurrent,           //  0xEDD7 Charger current (3)      float       A

    //MPPT 4
    mppt4_panelPower        = 0x40, //  0xEDBC Panel power              float       W
    mppt4_panelVoltage,             //  0xEDBB Panel voltage            float       V
    mppt4_panelCurrent,             //  0xEDBD Panel current (1)        float       A
    mppt4_chargerCurrent,           //  0xEDD7 Charger current (3)      float       A


    //Temp&higro
    //...... = 0x90


    /*** BOX 3 ***/
    //Bat (LIFePO4) 1
    bat1_chargingCurrent    = 0x60, //Charging Current                  int32_t
    bat1_dischargingCurrent,        //Discharging Current               int32_t
    bat1_state,                     //State register                    int32_t
    bat1_chargeLevelPercentage,     //Charge Level in %                 int32_t
    bat1_chargeLevelAh,             //Charge Level in Ah                int32_t
    bat1_capacity,                  //Capacity                          int32_t
    bat1_voltage,
    bat1_power,
    bat1_cellVoltage1,              //Cell Voltage  1                   int32_t
    bat1_cellVoltage2,              //Cell Voltage  2                   int32_t
    bat1_cellVoltage3,              //Cell Voltage  3                   int32_t
    bat1_cellVoltage4,              //Cell Voltage  4                   int32_t
    bat1_cellVoltage5,              //Cell Voltage  5                   int32_t
    bat1_cellVoltage6,              //Cell Voltage  6                   int32_t
    bat1_cellVoltage7,              //Cell Voltage  7                   int32_t
    bat1_cellVoltage8,              //Cell Voltage  8                   int32_t
    bat1_cellVoltage9,              //Cell Voltage  9                   int32_t
    bat1_cellVoltage10,             //Cell Voltage  10                  int32_t
    bat1_cellVoltage11,             //Cell Voltage  11                  int32_t
    bat1_cellVoltage12,             //Cell Voltage  12                  int32_t
    bat1_cellVoltage13,             //Cell Voltage  13                  int32_t
    bat1_cellVoltage14,             //Cell Voltage  14                  int32_t

    //Bat (LIFePO4) 2
    bat2_chargingCurrent    = 0x80, //Charging Current                  int32_t
    bat2_dischargingCurrent,        //Discharging Current               int32_t
    bat2_state,                     //State register                    int32_t
    bat2_chargeLevelPercentage,     //Charge Level in %                 int32_t
    bat2_chargeLevelAh,             //Charge Level in Ah                int32_t
    bat2_capacity,                  //Capacity                          int32_t
    bat2_voltage,
    bat2_power,
    bat2_cellVoltage1,              //Cell Voltage  1                   int32_t
    bat2_cellVoltage2,              //Cell Voltage  2                   int32_t
    bat2_cellVoltage3,              //Cell Voltage  3                   int32_t
    bat2_cellVoltage4,              //Cell Voltage  4                   int32_t
    bat2_cellVoltage5,              //Cell Voltage  5                   int32_t
    bat2_cellVoltage6,              //Cell Voltage  6                   int32_t
    bat2_cellVoltage7,              //Cell Voltage  7                   int32_t
    bat2_cellVoltage8,              //Cell Voltage  8                   int32_t
    bat2_cellVoltage9,              //Cell Voltage  9                   int32_t
    bat2_cellVoltage10,             //Cell Voltage  10                  int32_t
    bat2_cellVoltage11,             //Cell Voltage  11                  int32_t
    bat2_cellVoltage12,             //Cell Voltage  12                  int32_t
    bat2_cellVoltage13,             //Cell Voltage  13                  int32_t
    bat2_cellVoltage14,             //Cell Voltage  14                  int32_t

    //Bat (LIFePO4) 3
    bat3_chargingCurrent    = 0xA0, //Charging Current                  int32_t
    bat3_dischargingCurrent,        //Discharging Current               int32_t
    bat3_state,                     //State register                    int32_t
    bat3_chargeLevelPercentage,     //Charge Level in %                 int32_t
    bat3_chargeLevelAh,             //Charge Level in Ah                int32_t
    bat3_capacity,                  //Capacity                          int32_t
    bat3_voltage,
    bat3_power,
    bat3_cellVoltage1,              //Cell Voltage  1                   int32_t
    bat3_cellVoltage2,              //Cell Voltage  2                   int32_t
    bat3_cellVoltage3,              //Cell Voltage  3                   int32_t
    bat3_cellVoltage4,              //Cell Voltage  4                   int32_t
    bat3_cellVoltage5,              //Cell Voltage  5                   int32_t
    bat3_cellVoltage6,              //Cell Voltage  6                   int32_t
    bat3_cellVoltage7,              //Cell Voltage  7                   int32_t
    bat3_cellVoltage8,              //Cell Voltage  8                   int32_t
    bat3_cellVoltage9,              //Cell Voltage  9                   int32_t
    bat3_cellVoltage10,             //Cell Voltage  10                  int32_t
    bat3_cellVoltage11,             //Cell Voltage  11                  int32_t
    bat3_cellVoltage12,             //Cell Voltage  12                  int32_t
    bat3_cellVoltage13,             //Cell Voltage  13                  int32_t
    bat3_cellVoltage14,             //Cell Voltage  14                  int32_t

    //Bat (LIFePO4) 4
    bat4_chargingCurrent    = 0xC0, //Charging Current                  int32_t
    bat4_dischargingCurrent,        //Discharging Current               int32_t
    bat4_state,                     //State register                    int32_t
    bat4_chargeLevelPercentage,     //Charge Level in %                 int32_t
    bat4_chargeLevelAh,             //Charge Level in Ah                int32_t
    bat4_capacity,                  //Capacity                          int32_t
    bat4_voltage,
    bat4_power,
    bat4_cellVoltage1,              //Cell Voltage  1                   int32_t
    bat4_cellVoltage2,              //Cell Voltage  2                   int32_t
    bat4_cellVoltage3,              //Cell Voltage  3                   int32_t
    bat4_cellVoltage4,              //Cell Voltage  4                   int32_t
    bat4_cellVoltage5,              //Cell Voltage  5                   int32_t
    bat4_cellVoltage6,              //Cell Voltage  6                   int32_t
    bat4_cellVoltage7,              //Cell Voltage  7                   int32_t
    bat4_cellVoltage8,              //Cell Voltage  8                   int32_t
    bat4_cellVoltage9,              //Cell Voltage  9                   int32_t
    bat4_cellVoltage10,             //Cell Voltage  10                  int32_t
    bat4_cellVoltage11,             //Cell Voltage  11                  int32_t
    bat4_cellVoltage12,             //Cell Voltage  12                  int32_t
    bat4_cellVoltage13,             //Cell Voltage  13                  int32_t
    bat4_cellVoltage14,             //Cell Voltage  14                  int32_t


    /*** BOX OTHER ***/
    // AHRS
    ACCEL_X                 = 0xE0, // float
    ACCEL_Y,
    ACCEL_Z,
    GYRO_X,
    GYRO_Y,
    GYRO_Z,
    MAG_X,
    MAG_Y,
    MAG_Z,
    LATITUDE,
    LONGITUDE,
    SPEED,
    AHRS_STATUS // int32
};
#endif

/*
 * Link to table with all parameters:
 * https://docs.google.com/spreadsheets/d/1KpmXx6eCro7qfBD9NuBzhJrCtfSilcJNvQqe6ezVOdA/edit#gid=0
 * 
 */