#ifndef ALICAT_ERRORS_H
#define ALICAT_ERRORS_H
/**
 * @file AlicatErrors.h
 * @brief Alicat MFC status and error code definitions.
 *
 * Contains the full table of Alicat status codes that may appear in the
 * trailing columns of a data frame, along with lookup helpers and defines.
 *
 * Status codes appear as 3-character tokens appended after the gas type field
 * when a special condition is present. Multiple codes may appear simultaneously.
 *
 * Reference: Alicat Serial Primer, Status/Error Codes section.
 *
 * @author Ryan Massie (rmassie)
 * @date 3/31/26
 */

#include <cstddef>
#include <cstring>

/**
 * @brief Holds the 3-character code and human-readable description
 *        for a single Alicat status/error condition.
 */
struct AlicatStatusDef
{
    const char *code;        // 3-character status code (e.g. "ADC")
    const char *description; // Human-readable explanation
};

/**
 * @brief Full Alicat status code table (11 entries).
 *
 * Use lookupAlicatStatusByCode() rather than indexing directly.
 */
inline constexpr AlicatStatusDef ALICAT_STATUS_TABLE[] = {
    {"ADC", "ADC internal communication error"},
    {"EXH", "Manual exhaust valve override is enabled (max downstream opening)"},
    {"HLD", "Valve drive hold enabled (bypass active loop control)"},
    {"LCK", "Display buttons are disabled"},
    {"MOV", "Mass flow rate overage (outside measurable range)"},
    {"OPL", "Overpressure limit is enabled"},
    {"OVR", "Totalizer has rolled over or frozen at max value"},
    {"POV", "Pressure reading overage (outside measurable range)"},
    {"TMF", "Totalizer missed flow data (due to MOV or VOV error)"},
    {"TOV", "Temperature reading overage (outside measurable range)"},
    {"VOV", "Volumetric flow rate overage (outside measurable range)"},
};

/**
 * @brief Number of entries in the Alicat status code table.
 */
inline constexpr std::size_t ALICAT_STATUS_TABLE_SIZE = sizeof(ALICAT_STATUS_TABLE) / sizeof(ALICAT_STATUS_TABLE[0]);

/**
 * @brief Look up a status definition by its 3-character code.
 * @param code Null-terminated 3-character status code string.
 * @return Pointer to the matching AlicatStatusDef, or nullptr if not found.
 */
inline const AlicatStatusDef *lookupAlicatStatusByCode(const char *code)
{
    if (code == nullptr)
        return nullptr;
    for (std::size_t i = 0; i < ALICAT_STATUS_TABLE_SIZE; ++i)
    {
        if (std::strncmp(ALICAT_STATUS_TABLE[i].code, code, 3) == 0)
            return &ALICAT_STATUS_TABLE[i];
    }
    return nullptr;
}

// --- Status Code String Defines ---
// Use these for comparisons against AlicatDataFrame::statusCodes entries.
inline constexpr const char *ALICAT_STATUS_ADC = "ADC";
inline constexpr const char *ALICAT_STATUS_EXH = "EXH";
inline constexpr const char *ALICAT_STATUS_HLD = "HLD";
inline constexpr const char *ALICAT_STATUS_LCK = "LCK";
inline constexpr const char *ALICAT_STATUS_MOV = "MOV";
inline constexpr const char *ALICAT_STATUS_OPL = "OPL";
inline constexpr const char *ALICAT_STATUS_OVR = "OVR";
inline constexpr const char *ALICAT_STATUS_POV = "POV";
inline constexpr const char *ALICAT_STATUS_TMF = "TMF";
inline constexpr const char *ALICAT_STATUS_TOV = "TOV";
inline constexpr const char *ALICAT_STATUS_VOV = "VOV";

#endif // ALICAT_ERRORS_H
