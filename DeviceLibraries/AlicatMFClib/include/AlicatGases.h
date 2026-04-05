#ifndef ALICAT_GASES_H
#define ALICAT_GASES_H
/**
 * @file AlicatGases.h
 * @brief Gas definition table for Alicat Mass Flow Controllers.
 *
 * Contains all standard Alicat gas IDs, short names, and long names
 * as defined in the Alicat gas selection table.
 *
 * @author Ryan Massie (rmassie)
 * @date 3/31/26
 */

#include <cstddef>
#include <cstdint>

/**
 * @brief Holds the ID, short name, and long name for a single Alicat gas entry.
 */
struct AlicatGasDef
{
    uint8_t     id;
    const char *shortName;
    const char *longName;
};

/**
 * @brief Full Alicat gas table (130 entries).
 *
 * IDs are non-contiguous, use lookupAlicatGasById() rather than indexing directly.
 */
inline constexpr AlicatGasDef ALICAT_GAS_TABLE[] = {
    {0, "Air", "Air (Clean Dry)"},
    {1, "Ar", "Argon"},
    {2, "CH4", "Methane"},
    {3, "CO", "Carbon Monoxide"},
    {4, "CO2", "Carbon Dioxide"},
    {5, "C2H6", "Ethane"},
    {6, "H2", "Hydrogen"},
    {7, "He", "Helium"},
    {8, "N2", "Nitrogen"},
    {9, "N2O", "Nitrous Oxide"},
    {10, "Ne", "Neon"},
    {11, "O2", "Oxygen"},
    {12, "C3H8", "Propane"},
    {13, "nC4H10", "Normal Butane"},
    {14, "C2H2", "Acetylene"},
    {15, "C2H4", "Ethylene (Ethene)"},
    {16, "iC4H10", "Isobutane"},
    {17, "Kr", "Krypton"},
    {18, "Xe", "Xenon"},
    {19, "SF6", "Sulfur Hexafluoride"},
    {20, "C-25", "25% CO2, 75% Ar"},
    {21, "C-10", "10% CO2, 90% Ar"},
    {22, "C-8", "8% CO2, 92% Ar"},
    {23, "C-2", "2% CO2, 98% Ar"},
    {24, "C-75", "75% CO2, 25% Ar"},
    {25, "He-25", "25% He, 75% Ar"},
    {26, "He-75", "75% He, 25% Ar"},
    {27, "A1025", "90% He, 7.5% Ar, 2.5% CO2"},
    {28, "Star29", "Stargon CS (90% Ar, 8% CO2, 2% O2)"},
    {29, "P-5", "5% CH4, 95% Ar"},
    {30, "NO", "Nitric Oxide"},
    {31, "NF3", "Nitrogen Trifluoride"},
    {32, "NH3", "Ammonia"},
    {33, "Cl2", "Chlorine"},
    {34, "H2S", "Hydrogen Sulfide"},
    {35, "SO2", "Sulfur Dioxide"},
    {36, "C3H6", "Propylene"},
    {80, "1Buten", "1-Butylene"},
    {81, "cButen", "Cis-Butene (cis-2-Butene)"},
    {82, "iButen", "Isobutene"},
    {83, "tButen", "Trans-2-Butene"},
    {84, "COS", "Carbonyl Sulfide"},
    {85, "DME", "Dimethylether (C2H6O)"},
    {86, "SiH4", "Silane"},
    {100, "R-11", "Trichlorofluoromethane (CCl3F)"},
    {101, "R-115", "Chloropentafluoroethane (C2ClF5)"},
    {102, "R-116", "Hexafluoroethane (C2F6)"},
    {103, "R-124", "Chlorotetrafluoroethane (C2HClF4)"},
    {104, "R-125", "Pentafluoroethane (CF3CHF2)"},
    {105, "R-134A", "Tetrafluoroethane (CH2FCF3)"},
    {106, "R-14", "Tetrafluoromethane (CF4)"},
    {107, "R-142b", "Chlorodifluoroethane (CH3CClF2)"},
    {108, "R-143a", "Trifluoroethane (C2H3F3)"},
    {109, "R-152a", "Difluoroethane (C2H4F2)"},
    {110, "R-22", "Difluoromonochloromethane (CHClF2)"},
    {111, "R-23", "Trifluoromethane (CHF3)"},
    {112, "R-32", "Difluoromethane (CH2F2)"},
    {113, "R-318", "Octafluorocyclobutane (C4F8)"},
    {114, "R-404A", "44% R-125, 4% R-134A, 52% R-143A"},
    {115, "R-407C", "23% R-32, 25% R-125, 52% R-143A"},
    {116, "R-410A", "50% R-32, 50% R-125"},
    {117, "R-507A", "50% R-125, 50% R-143A"},
    {140, "C-15", "15% CO2, 85% Ar"},
    {141, "C-20", "20% CO2, 80% Ar"},
    {142, "C-50", "50% CO2, 50% Ar"},
    {143, "He-50", "50% He, 50% Ar"},
    {144, "He-90", "90% He, 10% Ar"},
    {145, "Bio5M", "5% CH4, 95% CO2"},
    {146, "Bio10M", "10% CH4, 90% CO2"},
    {147, "Bio15M", "15% CH4, 85% CO2"},
    {148, "Bio20M", "20% CH4, 80% CO2"},
    {149, "Bio25M", "25% CH4, 75% CO2"},
    {150, "Bio30M", "30% CH4, 70% CO2"},
    {151, "Bio35M", "35% CH4, 65% CO2"},
    {152, "Bio40M", "40% CH4, 60% CO2"},
    {153, "Bio45M", "45% CH4, 55% CO2"},
    {154, "Bio50M", "50% CH4, 50% CO2"},
    {155, "Bio55M", "55% CH4, 45% CO2"},
    {156, "Bio60M", "60% CH4, 40% CO2"},
    {157, "Bio65M", "65% CH4, 35% CO2"},
    {158, "Bio70M", "70% CH4, 30% CO2"},
    {159, "Bio75M", "75% CH4, 25% CO2"},
    {160, "Bio80M", "80% CH4, 20% CO2"},
    {161, "Bio85M", "85% CH4, 15% CO2"},
    {162, "Bio90M", "90% CH4, 10% CO2"},
    {163, "Bio95M", "95% CH4, 5% CO2"},
    {164, "EAN-32", "32% O2, 68% N2"},
    {165, "EAN-36", "36% O2, 64% N2"},
    {166, "EAN-40", "40% O2, 60% N2"},
    {167, "HeOx20", "20% O2, 80% He"},
    {168, "HeOx21", "21% O2, 79% He"},
    {169, "HeOx30", "30% O2, 70% He"},
    {170, "HeOx40", "40% O2, 60% He"},
    {171, "HeOx50", "50% O2, 50% He"},
    {172, "HeOx60", "60% O2, 40% He"},
    {173, "HeOx80", "80% O2, 20% He"},
    {174, "HeOx99", "99% O2, 1% He"},
    {175, "EA-40", "Enriched Air-40% O2"},
    {176, "EA-60", "Enriched Air-60% O2"},
    {177, "EA-80", "Enriched Air-80% O2"},
    {178, "Metab", "Metabolic Exhalant (16% O2, 78.04% N2, 5% CO2, 0.96% Ar)"},
    {179, "LG-4.5", "4.5% CO2, 13.5% N2, 82% He"},
    {180, "LG-6", "6% CO2, 14% N2, 80% He"},
    {181, "LG-7", "7% CO2, 14% N2, 79% He"},
    {182, "LG-9", "9% CO2, 15% N2, 76% He"},
    {183, "HeNe-9", "9% Ne, 91% He"},
    {184, "LG-9.4", "9.4% CO2, 19.25% N2, 71.35% He"},
    {185, "SynG-1", "40% H2, 29% CO, 20% CO2, 11% CH4"},
    {186, "SynG-2", "64% H2, 28% CO, 1% CO2, 7% CH4"},
    {187, "SynG-3", "70% H2, 4% CO, 25% CO2, 1% CH4"},
    {188, "SynG-4", "83% H2, 14% CO, 3% CH4"},
    {189, "NatG-1", "93% CH4, 3% C2H6, 1% C3H8, 2% N2, 1% CO2"},
    {190, "NatG-2", "95% CH4, 3% C2H6, 1% N2, 1% CO2"},
    {191, "NatG-3", "95.2% CH4, 2.5% C2H6, 0.2% C3H8, 0.1% C4H10, 1.3% N2, 0.7% CO2"},
    {192, "CoalG", "50% H2, 35% CH4, 10% CO, 5% C2H4"},
    {193, "Endo", "75% H2, 25% N2"},
    {194, "HHO", "66.67% H2, 33.33% O2"},
    {195, "HD-5", "LPG: 96.1% C3H8, 1.5% C2H6, 0.4% C3H6, 1.9% n-C4H10"},
    {196, "HD-10", "LPG: 85% C3H8, 10% C3H6, 5% n-C4H10"},
    {197, "OCG-89", "89% O2, 7% N2, 4% Ar"},
    {198, "OCG-93", "93% O2, 3% N2, 4% Ar"},
    {199, "OCG-95", "95% O2, 1% N2, 4% Ar"},
    {200, "FG-1", "2.5% O2, 10.8% CO2, 85.7% N2, 1% Ar"},
    {201, "FG-2", "2.9% O2, 14% CO2, 82.1% N2, 1% Ar"},
    {202, "FG-3", "3.7% O2, 15% CO2, 80.3% N2, 1% Ar"},
    {203, "FG-4", "7% O2, 12% CO2, 80% N2, 1% Ar"},
    {204, "FG-5", "10% O2, 9.5% CO2, 79.5% N2, 1% Ar"},
    {205, "FG-6", "13% O2, 7% CO2, 79% N2, 1% Ar"},
    {206, "P-10", "10% CH4, 90% Ar"},
    {210, "D-2", "Deuterium"},
};

/**
 * @brief Number of entries in the Alicat gas definition table.
 */
inline constexpr std::size_t ALICAT_GAS_TABLE_SIZE = sizeof(ALICAT_GAS_TABLE) / sizeof(ALICAT_GAS_TABLE[0]);

/**
 * @brief Look up a gas definition by its numeric ID.
 * @param id The Alicat gas number (e.g. 1 for Argon).
 * @return Pointer to the matching AlicatGasDef, or nullptr if not found.
 */
inline constexpr const AlicatGasDef *lookupAlicatGasById(uint8_t id)
{
    for (std::size_t i = 0; i < ALICAT_GAS_TABLE_SIZE; ++i)
    {
        if (ALICAT_GAS_TABLE[i].id == id)
            return &ALICAT_GAS_TABLE[i];
    }
    return nullptr;
}

// --- Gas ID Defines ---
// Use these with AlicatMFC::setGas() for readable, validated gas selection.
inline constexpr uint8_t ALICAT_GAS_AIR          = 0;
inline constexpr uint8_t ALICAT_GAS_AR           = 1;
inline constexpr uint8_t ALICAT_GAS_CH4          = 2;
inline constexpr uint8_t ALICAT_GAS_CO           = 3;
inline constexpr uint8_t ALICAT_GAS_CO2          = 4;
inline constexpr uint8_t ALICAT_GAS_C2H6         = 5;
inline constexpr uint8_t ALICAT_GAS_H2           = 6;
inline constexpr uint8_t ALICAT_GAS_HE           = 7;
inline constexpr uint8_t ALICAT_GAS_N2           = 8;
inline constexpr uint8_t ALICAT_GAS_N2O          = 9;
inline constexpr uint8_t ALICAT_GAS_NE           = 10;
inline constexpr uint8_t ALICAT_GAS_O2           = 11;
inline constexpr uint8_t ALICAT_GAS_C3H8         = 12;
inline constexpr uint8_t ALICAT_GAS_NC4H10       = 13;
inline constexpr uint8_t ALICAT_GAS_C2H2         = 14;
inline constexpr uint8_t ALICAT_GAS_C2H4         = 15;
inline constexpr uint8_t ALICAT_GAS_IC4H10       = 16;
inline constexpr uint8_t ALICAT_GAS_KR           = 17;
inline constexpr uint8_t ALICAT_GAS_XE           = 18;
inline constexpr uint8_t ALICAT_GAS_SF6          = 19;
inline constexpr uint8_t ALICAT_GAS_C_25         = 20;
inline constexpr uint8_t ALICAT_GAS_C_10         = 21;
inline constexpr uint8_t ALICAT_GAS_C_8          = 22;
inline constexpr uint8_t ALICAT_GAS_C_2          = 23;
inline constexpr uint8_t ALICAT_GAS_C_75         = 24;
inline constexpr uint8_t ALICAT_GAS_HE_25        = 25;
inline constexpr uint8_t ALICAT_GAS_HE_75        = 26;
inline constexpr uint8_t ALICAT_GAS_A1025        = 27;
inline constexpr uint8_t ALICAT_GAS_STAR29       = 28;
inline constexpr uint8_t ALICAT_GAS_P_5          = 29;
inline constexpr uint8_t ALICAT_GAS_NO           = 30;
inline constexpr uint8_t ALICAT_GAS_NF3          = 31;
inline constexpr uint8_t ALICAT_GAS_NH3          = 32;
inline constexpr uint8_t ALICAT_GAS_CL2          = 33;
inline constexpr uint8_t ALICAT_GAS_H2S          = 34;
inline constexpr uint8_t ALICAT_GAS_SO2          = 35;
inline constexpr uint8_t ALICAT_GAS_C3H6         = 36;
inline constexpr uint8_t ALICAT_GAS_BUTENE_1     = 80;
inline constexpr uint8_t ALICAT_GAS_CIS_BUTENE   = 81;
inline constexpr uint8_t ALICAT_GAS_ISO_BUTENE   = 82;
inline constexpr uint8_t ALICAT_GAS_TRANS_BUTENE = 83;
inline constexpr uint8_t ALICAT_GAS_COS          = 84;
inline constexpr uint8_t ALICAT_GAS_DME          = 85;
inline constexpr uint8_t ALICAT_GAS_SIH4         = 86;
inline constexpr uint8_t ALICAT_GAS_R11          = 100;
inline constexpr uint8_t ALICAT_GAS_R115         = 101;
inline constexpr uint8_t ALICAT_GAS_R116         = 102;
inline constexpr uint8_t ALICAT_GAS_R124         = 103;
inline constexpr uint8_t ALICAT_GAS_R125         = 104;
inline constexpr uint8_t ALICAT_GAS_R134A        = 105;
inline constexpr uint8_t ALICAT_GAS_R14          = 106;
inline constexpr uint8_t ALICAT_GAS_R142B        = 107;
inline constexpr uint8_t ALICAT_GAS_R143A        = 108;
inline constexpr uint8_t ALICAT_GAS_R152A        = 109;
inline constexpr uint8_t ALICAT_GAS_R22          = 110;
inline constexpr uint8_t ALICAT_GAS_R23          = 111;
inline constexpr uint8_t ALICAT_GAS_R32          = 112;
inline constexpr uint8_t ALICAT_GAS_R318         = 113;
inline constexpr uint8_t ALICAT_GAS_R404A        = 114;
inline constexpr uint8_t ALICAT_GAS_R407C        = 115;
inline constexpr uint8_t ALICAT_GAS_R410A        = 116;
inline constexpr uint8_t ALICAT_GAS_R507A        = 117;
inline constexpr uint8_t ALICAT_GAS_C_15         = 140;
inline constexpr uint8_t ALICAT_GAS_C_20         = 141;
inline constexpr uint8_t ALICAT_GAS_C_50         = 142;
inline constexpr uint8_t ALICAT_GAS_HE_50        = 143;
inline constexpr uint8_t ALICAT_GAS_HE_90        = 144;
inline constexpr uint8_t ALICAT_GAS_BIO5M        = 145;
inline constexpr uint8_t ALICAT_GAS_BIO10M       = 146;
inline constexpr uint8_t ALICAT_GAS_BIO15M       = 147;
inline constexpr uint8_t ALICAT_GAS_BIO20M       = 148;
inline constexpr uint8_t ALICAT_GAS_BIO25M       = 149;
inline constexpr uint8_t ALICAT_GAS_BIO30M       = 150;
inline constexpr uint8_t ALICAT_GAS_BIO35M       = 151;
inline constexpr uint8_t ALICAT_GAS_BIO40M       = 152;
inline constexpr uint8_t ALICAT_GAS_BIO45M       = 153;
inline constexpr uint8_t ALICAT_GAS_BIO50M       = 154;
inline constexpr uint8_t ALICAT_GAS_BIO55M       = 155;
inline constexpr uint8_t ALICAT_GAS_BIO60M       = 156;
inline constexpr uint8_t ALICAT_GAS_BIO65M       = 157;
inline constexpr uint8_t ALICAT_GAS_BIO70M       = 158;
inline constexpr uint8_t ALICAT_GAS_BIO75M       = 159;
inline constexpr uint8_t ALICAT_GAS_BIO80M       = 160;
inline constexpr uint8_t ALICAT_GAS_BIO85M       = 161;
inline constexpr uint8_t ALICAT_GAS_BIO90M       = 162;
inline constexpr uint8_t ALICAT_GAS_BIO95M       = 163;
inline constexpr uint8_t ALICAT_GAS_EAN_32       = 164;
inline constexpr uint8_t ALICAT_GAS_EAN_36       = 165;
inline constexpr uint8_t ALICAT_GAS_EAN_40       = 166;
inline constexpr uint8_t ALICAT_GAS_HEOX20       = 167;
inline constexpr uint8_t ALICAT_GAS_HEOX21       = 168;
inline constexpr uint8_t ALICAT_GAS_HEOX30       = 169;
inline constexpr uint8_t ALICAT_GAS_HEOX40       = 170;
inline constexpr uint8_t ALICAT_GAS_HEOX50       = 171;
inline constexpr uint8_t ALICAT_GAS_HEOX60       = 172;
inline constexpr uint8_t ALICAT_GAS_HEOX80       = 173;
inline constexpr uint8_t ALICAT_GAS_HEOX99       = 174;
inline constexpr uint8_t ALICAT_GAS_EA_40        = 175;
inline constexpr uint8_t ALICAT_GAS_EA_60        = 176;
inline constexpr uint8_t ALICAT_GAS_EA_80        = 177;
inline constexpr uint8_t ALICAT_GAS_METAB        = 178;
inline constexpr uint8_t ALICAT_GAS_LG_4_5       = 179;
inline constexpr uint8_t ALICAT_GAS_LG_6         = 180;
inline constexpr uint8_t ALICAT_GAS_LG_7         = 181;
inline constexpr uint8_t ALICAT_GAS_LG_9         = 182;
inline constexpr uint8_t ALICAT_GAS_HENE_9       = 183;
inline constexpr uint8_t ALICAT_GAS_LG_9_4       = 184;
inline constexpr uint8_t ALICAT_GAS_SYNG_1       = 185;
inline constexpr uint8_t ALICAT_GAS_SYNG_2       = 186;
inline constexpr uint8_t ALICAT_GAS_SYNG_3       = 187;
inline constexpr uint8_t ALICAT_GAS_SYNG_4       = 188;
inline constexpr uint8_t ALICAT_GAS_NATG_1       = 189;
inline constexpr uint8_t ALICAT_GAS_NATG_2       = 190;
inline constexpr uint8_t ALICAT_GAS_NATG_3       = 191;
inline constexpr uint8_t ALICAT_GAS_COALG        = 192;
inline constexpr uint8_t ALICAT_GAS_ENDO         = 193;
inline constexpr uint8_t ALICAT_GAS_HHO          = 194;
inline constexpr uint8_t ALICAT_GAS_HD_5         = 195;
inline constexpr uint8_t ALICAT_GAS_HD_10        = 196;
inline constexpr uint8_t ALICAT_GAS_OCG_89       = 197;
inline constexpr uint8_t ALICAT_GAS_OCG_93       = 198;
inline constexpr uint8_t ALICAT_GAS_OCG_95       = 199;
inline constexpr uint8_t ALICAT_GAS_FG_1         = 200;
inline constexpr uint8_t ALICAT_GAS_FG_2         = 201;
inline constexpr uint8_t ALICAT_GAS_FG_3         = 202;
inline constexpr uint8_t ALICAT_GAS_FG_4         = 203;
inline constexpr uint8_t ALICAT_GAS_FG_5         = 204;
inline constexpr uint8_t ALICAT_GAS_FG_6         = 205;
inline constexpr uint8_t ALICAT_GAS_P_10         = 206;
inline constexpr uint8_t ALICAT_GAS_D_2          = 210;

#endif // ALICAT_GASES_H
