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

    /*** BOX 1 ***/
    // KLS
    actualSpeed = 0x00,    //Actual speed           type: int;  range: 0-600;    unit: rpm;
    actualMotorCurren,     //Actual motor current   type: int;  range: 0,0 - 400,0; unit: 0,1A;
    actualBateryVoltage,   //Actual batery voltage  type: int;  range: 0,0 - 180,0; unit: 0,1V;
    throtleSignal,         //Throtle signal         type: int;  range: "0~255 maps 0~5V";
    controllerTemperature, //Controller temperature type: int;  unit: *C (offset 40)
    motorTemperature,      //Controller temperature type: int;  unit: *C (offset 30)

    /*** BOX 2 ***/
    //MPPT 1                        //  ID     Description                    Scale   Type    Unit
     mppt1_panelPower=0x10,         //  0xEDBC Panel power                    0.01     un32    W
     mppt1_panelVoltage,            //  0xEDBB Panel voltage                  0.01     un16    V
     mppt1_panelCurrent,            //  0xEDBD Panel current (*1)             0.1      un16    A
     mppt1_panelMaximumVoltage,     //  0xEDB8 Panel maximum voltage (*2)     0.01     un16    V .....
     mppt1_chargerMaximumCurrent,   //  0xEDDF Charger maximum current (*1)   0.01     un16    A
     mppt1_batteryMaximumCurrent,   //  0xEDF0 Battery maximum current        0.1      un16    A .......
     mppt1_chargerCurrent,          //  0xEDD7 Charger current (3)            0.1      un16    A
     mppt1_chargerVoltage,          //  0xEDD5 Charger voltage (3)            0.01     un16    V
     mppt1_batteryVoltage,          //  0xEDEF Battery voltage (4)            1        un8     V

    //MPPT 2
     mppt2_panelPower = 0x20,       //  0xEDBC Panel power                    0.01    un32   W
     mppt2_panelVoltage,            //  0xEDBB Panel voltage                  0.01    un16   V
     mppt2_panelCurrent,            //  0xEDBD Panel current (1)              0.1     un16   A
     mppt2_panelMaximumVoltage,     //  0xEDB8 Panel maximum voltage (*2)     0.01    un16   V
     mppt2_chargerMaximumCurrent,   //  0xEDDF Charger maximum current (*1)   0.01    un16   A
     mppt2_batteryMaximumCurrent,   //  0xEDF0 Battery maximum current        0.1     un16   A .......
     mppt2_chargerCurrent,          //  0xEDD7 Charger current (3)            0.1     un16   A
     mppt2_chargerVoltage,          //  0xEDD5 Charger voltage (3)            0.01    un16   V
     mppt2_batteryVoltage,          //  0xEDEF Battery voltage (4)            1       un8    V

    //MPPT 3
    mppt3_panelPower = 0x30,       //  0xEDBC Panel power                     0.01    un32   W
    mppt3_panelVoltage,            //  0xEDBB Panel voltage                   0.01    un16   V
    mppt3_panelCurrent,            //  0xEDBD Panel current (1)               0.1     un16   A
    mppt3_panelMaximumVoltage,     //  0xEDB8 Panel maximum voltage (*2)      0.01    un16   V
    mppt3_chargerMaximumCurrent,   //  0xEDDF Charger maximum current (*1)    0.01    un16   A
    mppt3_batteryMaximumCurrent,   //  0xEDF0 Battery maximum current         0.1     un16   A .......
    mppt3_chargerCurrent,          //  0xEDD7 Charger current (3)             0.1     un16   A
    mppt3_chargerVoltage,          //  0xEDD5 Charger voltage (3)             0.01    un16   V
    mppt3_batteryVoltage,          //  0xEDEF Battery voltage (4)             1       un8    V

    //MPPT 4
    mppt4_panelPower = 0x40,       //  0xEDBC Panel power                     0.01    un32   W
    mppt4_panelVoltage,            //  0xEDBB Panel voltage                   0.01    un16   V
    mppt4_panelCurrent,            //  0xEDBD Panel current (1)               0.1     un16   A
    mppt4_panelMaximumVoltage,     //  0xEDB8 Panel maximum voltage (*2)      0.01    un16   V
    mppt4_chargerMaximumCurrent,   //  0xEDDF Charger maximum current (*1)    0.01    un16   A
    mppt4_batteryMaximumCurrent,   //  0xEDF0 Battery maximum current         0.1     un16   A .......
    mppt4_chargerCurrent,          //  0xEDD7 Charger current (3)             0.1     un16   A
    mppt4_chargerVoltage,          //  0xEDD5 Charger voltage (3)             0.01    un16   V
    mppt4_batteryVoltage,          //  0xEDEF Battery voltage (4)             1       un8    V


    //Temp&higro
    //...... = 0x90


    /*** BOX 3 ***/
    //Bat (LIFePO4) 1
    bat1_chargingCurrent = 0x60, //Charging Current      type: int;
    bat1_dischargingCurrent,     //Discharging Current   type: int;
    bat1_state,                  ///State register       type: int;
    bat1_chargeLevelPercentage,  //Charge Level in %     type: int;
    bat1_chargeLevelAh,          //Charge Level in Ah    type: int;
    bat1_capacity,               //Capacity              type: int;
    bat1_cellVoltage1,           //Cell Voltage  1       type: int; //TODO(rradmoski): We have 14 cels - how to fit it concept is needed
    bat1_cellVoltage2,           //Cell Voltage  2       type: int;
    bat1_cellVoltage3,           //Cell Voltage  3       type: int;

    //Bat (LIFePO4) 2
    bat2_chargingCurrent = 0x70, //Charging Current      type: int;
    bat2_dischargingCurrent,     //Discharging Current   type: int;
    bat2_state,                  ///State register       type: int;
    bat2_chargeLevelPercentage,  //Charge Level in %     type: int;
    bat2_chargeLevelAh,          //Charge Level in Ah    type: int;
    bat2_capacity,               //Capacity              type: int;
    bat2_cellVoltage1,           //Cell Voltage  1       type: int; //TODO(rradmoski): We have 14 cels - how to fit it concept is needed
    bat2_cellVoltage2,           //Cell Voltage  2       type: int;
    bat2_cellVoltage3,           //Cell Voltage  3       type: int;

    //Bat (LIFePO4) 3
    bat3_chargingCurrent = 0x80, //Charging Current      type: int;
    bat3_dischargingCurrent,     //Discharging Current   type: int;
    bat3_state,                  ///State register       type: int;
    bat3_chargeLevelPercentage,  //Charge Level in %     type: int;
    bat3_chargeLevelAh,          //Charge Level in Ah    type: int;
    bat3_capacity,               //Capacity              type: int;
    bat3_cellVoltage1,           //Cell Voltage  1       type: int; //TODO(rradmoski): We have 14 cels - how to fit it concept is needed
    bat3_cellVoltage2,           //Cell Voltage  2       type: int;
    bat3_cellVoltage3,           //Cell Voltage  3       type: int;

    //Bat (LIFePO4) 4
    bat4_chargingCurrent = 0x90, //Charging Current      type: int;
    bat4_dischargingCurrent,     //Discharging Current   type: int;
    bat4_state,                  ///State register       type: int;
    bat4_chargeLevelPercentage,  //Charge Level in %     type: int;
    bat4_chargeLevelAh,          //Charge Level in Ah    type: int;
    bat4_capacity,               //Capacity              type: int;
    bat4_cellVoltage1,           //Cell Voltage  1       type: int; //TODO(rradmoski): We have 14 cels - how to fit it concept is needed
    bat4_cellVoltage2,           //Cell Voltage  2       type: int;
    bat4_cellVoltage3,           //Cell Voltage  3       type: int;


    /*** BOX OTHER ***/
    // AHRS
    ACCEL_X = 0xA0, // float
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