/**
 * @file TC110DriveUnit.h
 * @brief Parameter definitions and device class for the Pfeiffer Vacuum TC 110.
 * 
 * @author Ryan Massie (rmassie)
 * @date 3/4/26
 */

#pragma once

#include <cstdint>
#include "PfiefferDevice.h"

namespace Pfieffer {

/**
 * @brief Enum representing the TC110 parameter numbers for easy, type-safe reference.
 */
enum class TC110Cmd : uint16_t {
    // Control Commands
    Heating = 1, Standby = 2, RUTimeCtrl = 4, ErrorAckn = 9,
    PumpgStatn = 10, EnableVent = 12, CfgSpdSwPt = 17, CfgDO2 = 19,
    MotorPump = 23, CfgDO1 = 24, OpModeBKP = 25, SpdSetMode = 26,
    GasMode = 27, VentMode = 30, CfgAccA1 = 35, CfgAccB1 = 36,
    CfgAccA2 = 37, CfgAccB2 = 38, SealingGas = 50, CfgAO1 = 55,
    CtrlViaInt = 60, IntSelLckd = 61, CfgDI1 = 62, CfgDI2 = 63,

    // Status Requests
    RemotePrio = 300, SpdSwPtAtt = 302, ErrorCode = 303, OvTempElec = 304,
    OvTempPump = 305, SetSpdAtt = 306, PumpAccel = 307, SetRotSpd_Hz = 308,
    ActualSpd_Hz = 309, DrvCurrent = 310, OpHrsPump = 311, FwVersion = 312,
    DrvVoltage = 313, OpHrsElec = 314, NominalSpd_Hz = 315, DrvPower = 316,
    PumpCycles = 319, TempElec = 326, TempPmpBot = 330, AccelDecel = 336,
    Pressure = 340, TempBearng = 342, TempMotor = 346, ElecName = 349,
    CtrName = 350, CtrSoftware = 351, HwVersion = 354, ErrHist1 = 360,
    ErrHist2 = 361, ErrHist3 = 362, ErrHist4 = 363, ErrHist5 = 364,
    ErrHist6 = 365, ErrHist7 = 366, ErrHist8 = 367, ErrHist9 = 368,
    ErrHist10 = 369, SetRotSpd_rpm = 397, ActualSpd_rpm = 398, NominalSpd_rpm = 399,

    // Set value settings
    RUTimeSVal = 700, SpdSwPt1 = 701, SpdSVal = 707, PwrSVal = 708,
    SwoffBKP = 710, SwOnBKP = 711, StdbySVal = 717, SpdSwPt2 = 719,
    VentSpd = 720, VentTime = 721, Gaugetype = 738, NomSpdConf = 777,
    ParamSet = 794, ServiceLin = 795, RS485Adr = 797
};

/**
 * @brief TC110 Device implementation inheriting from the PfiefferDevice CRTP template.
 */
class TC110DriveUnit : public PfiefferDevice<TC110DriveUnit> {
public:
    explicit TC110DriveUnit(uint8_t address = 1) : PfiefferDevice<TC110DriveUnit>(address) {}

    /**
     * @brief Constexpr array mapping all known TC110 Parameters to definitions.
     */
    static constexpr PfiefferParamDef DICT[] = {
        // {ParamID, "Name", DataType, AccessType, MinVal, MaxVal, DefaultVal, NonVolatile}
        // Basic Control Commands
        {1, "Heating", 0, AccessType::READ_WRITE, 0, 1, 0, true},
        {2, "Standby", 0, AccessType::READ_WRITE, 0, 1, 0, true},
        {4, "RUTimeCtrl", 0, AccessType::READ_WRITE, 0, 1, 1, true},
        {9, "ErrorAckn", 0, AccessType::WRITE_ONLY, 1, 1, 1, false},
        {10, "PumpgStatn", 0, AccessType::READ_WRITE, 0, 1, 0, true},
        {12, "EnableVent", 0, AccessType::READ_WRITE, 0, 1, 0, true},
        {17, "CfgSpdSwPt", 7, AccessType::READ_WRITE, 0, 1, 0, true},
        {19, "CfgDO2", 7, AccessType::READ_WRITE, 0, 15, 1, true},
        {23, "MotorPump", 0, AccessType::READ_WRITE, 0, 1, 1, true},
        {24, "CfgDO1", 7, AccessType::READ_WRITE, 0, 15, 0, true},
        {25, "OpModeBKP", 7, AccessType::READ_WRITE, 0, 2, 0, true},
        {26, "SpdSetMode", 7, AccessType::READ_WRITE, 0, 1, 0, true},
        {27, "GasMode", 7, AccessType::READ_WRITE, 0, 2, 0, true},
        {30, "VentMode", 7, AccessType::READ_WRITE, 0, 2, 0, true},
        {35, "CfgAccA1", 7, AccessType::READ_WRITE, 0, 8, 0, true},
        {36, "CfgAccB1", 7, AccessType::READ_WRITE, 0, 8, 1, true},
        {37, "CfgAccA2", 7, AccessType::READ_WRITE, 0, 8, 3, true},
        {38, "CfgAccB2", 7, AccessType::READ_WRITE, 0, 8, 2, true},
        {50, "SealingGas", 0, AccessType::READ_WRITE, 0, 1, 0, true},
        {55, "CfgAO1", 7, AccessType::READ_WRITE, 0, 4, 0, true},
        {60, "CtrlViaInt", 7, AccessType::READ_WRITE, 1, 255, 1, true},
        {61, "IntSelLckd", 0, AccessType::READ_WRITE, 0, 1, 0, true},
        {62, "CfgDI1", 7, AccessType::READ_WRITE, 0, 6, 1, true},
        {63, "CfgDI2", 7, AccessType::READ_WRITE, 0, 6, 2, true},
        
        // Status Requests
        {300, "RemotePrio", 0, AccessType::READ_ONLY, 0, 1, 0, false},
        {302, "SpdSwPtAtt", 0, AccessType::READ_ONLY, 0, 1, 0, false},
        {303, "ErrorCode", 4, AccessType::READ_ONLY, 0, 0, 0, false},
        {304, "OvTempElec", 0, AccessType::READ_ONLY, 0, 1, 0, false},
        {305, "OvTempPump", 0, AccessType::READ_ONLY, 0, 1, 0, false},
        {306, "SetSpdAtt", 0, AccessType::READ_ONLY, 0, 1, 0, false},
        {307, "PumpAccel", 0, AccessType::READ_ONLY, 0, 1, 0, false},
        {308, "SetRotSpd", 1, AccessType::READ_ONLY, 0, 999999, 0, false},
        {309, "ActualSpd", 1, AccessType::READ_ONLY, 0, 999999, 0, false},
        {310, "DrvCurrent", 2, AccessType::READ_ONLY, 0, 9999.99, 0, false},
        {311, "OpHrsPump", 1, AccessType::READ_ONLY, 0, 65535, 0, true},
        {312, "FwVersion", 4, AccessType::READ_ONLY, 0, 0, 0, false},
        {313, "DrvVoltage", 2, AccessType::READ_ONLY, 0, 9999.99, 0, false},
        {314, "OpHrsElec", 1, AccessType::READ_ONLY, 0, 65535, 0, true},
        {315, "NominalSpd", 1, AccessType::READ_ONLY, 0, 999999, 0, false},
        {316, "DrvPower", 1, AccessType::READ_ONLY, 0, 999999, 0, false},
        {319, "PumpCycles", 1, AccessType::READ_ONLY, 0, 65535, 0, true},
        {326, "TempElec", 1, AccessType::READ_ONLY, 0, 999999, 0, false},
        {330, "TempPmpBot", 1, AccessType::READ_ONLY, 0, 999999, 0, false},
        {336, "AccelDecel", 1, AccessType::READ_ONLY, 0, 999999, 0, false},
        {340, "Pressure", 7, AccessType::READ_ONLY, 1e-10, 1000, 0, false},
        {342, "TempBearng", 1, AccessType::READ_ONLY, 0, 999999, 0, false},
        {346, "TempMotor", 1, AccessType::READ_ONLY, 0, 999999, 0, false},
        {349, "ElecName", 4, AccessType::READ_ONLY, 0, 0, 0, false},
        {350, "CtrName", 4, AccessType::READ_ONLY, 0, 0, 0, false},
        {351, "CtrSoftware", 4, AccessType::READ_ONLY, 0, 0, 0, false},
        {354, "HWVersion", 4, AccessType::READ_ONLY, 0, 0, 0, false},
        {360, "ErrHist1", 4, AccessType::READ_ONLY, 0, 0, 0, true},
        {361, "ErrHist2", 4, AccessType::READ_ONLY, 0, 0, 0, true},
        {362, "ErrHist3", 4, AccessType::READ_ONLY, 0, 0, 0, true},
        {363, "ErrHist4", 4, AccessType::READ_ONLY, 0, 0, 0, true},
        {364, "ErrHist5", 4, AccessType::READ_ONLY, 0, 0, 0, true},
        {365, "ErrHist6", 4, AccessType::READ_ONLY, 0, 0, 0, true},
        {366, "ErrHist7", 4, AccessType::READ_ONLY, 0, 0, 0, true},
        {367, "ErrHist8", 4, AccessType::READ_ONLY, 0, 0, 0, true},
        {368, "ErrHist9", 4, AccessType::READ_ONLY, 0, 0, 0, true},
        {369, "ErrHist10", 4, AccessType::READ_ONLY, 0, 0, 0, true},
        {397, "SetRotSpd", 1, AccessType::READ_ONLY, 0, 999999, 0, false},
        {398, "ActualSpd", 1, AccessType::READ_ONLY, 0, 999999, 0, false},
        {399, "NominalSpd", 1, AccessType::READ_ONLY, 0, 999999, 0, false},

        // Set value settings
        {700, "RUTimeSVal", 1, AccessType::READ_WRITE, 1, 120, 8, true},
        {701, "SpdSwPt1", 1, AccessType::READ_WRITE, 50, 97, 80, true},
        {707, "SpdSVal", 2, AccessType::READ_WRITE, 20, 100, 50, true},
        {708, "PwrSVal", 7, AccessType::READ_WRITE, 10, 100, 100, true},
        {710, "SwoffBKP", 1, AccessType::READ_WRITE, 0, 1000, 0, true},
        {711, "SwOnBKP", 1, AccessType::READ_WRITE, 0, 1000, 0, true},
        {717, "StdbySVal", 2, AccessType::READ_WRITE, 20, 100, 66.7, true},
        {719, "SpdSwPt2", 1, AccessType::READ_WRITE, 5, 97, 20, true},
        {720, "VentSpd", 7, AccessType::READ_WRITE, 40, 98, 50, true},
        {721, "VentTime", 1, AccessType::READ_WRITE, 6, 3600, 3600, true},
        {738, "Gaugetype", 4, AccessType::READ_WRITE, 0, 0, 0, false},
        {777, "NomSpdConf", 1, AccessType::READ_WRITE, 0, 1500, 0, true},
        {794, "ParamSet", 7, AccessType::READ_WRITE, 0, 1, 0, false},
        {795, "ServiceLin", 7, AccessType::READ_WRITE, 795, 795, 795, false},
        {797, "RS485Adr", 1, AccessType::READ_WRITE, 1, 255, 1, true}
    };

    /**
     * @brief Inherited CRTP static callback to fetch a parameter bound array entry.
     */
    static const PfiefferParamDef* getParamDef(uint16_t parameterNumber) {
        for (const auto& def : DICT) {
            if (def.number == parameterNumber) {
                return &def;
            }
        }
        return nullptr;
    }
};

} // namespace Pfieffer
