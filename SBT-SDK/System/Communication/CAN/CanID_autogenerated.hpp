#ifndef PARAMETER_DEFINITION_HPP
#define PARAMETER_DEFINITION_HPP

#include <cstdint>

/**
 * !!!WARNING!!!
 * This file is autogenerated, any changes will not be retained after next
 * generation.
 *
 * @brief In this file are Source, Param, Group, Message ID definitions.
 */

namespace SBT::System::Comm::CAN_ID {

enum class Source : uint8_t {
    DEFAULT = 0x00,
    MPPT_CONTROLLER_1 = 0x01,
    MPPT_CONTROLLER_2 = 0x02,
    LIFEPO4_CONTROLLER_1 = 0x03,
    LIFEPO4_CONTROLLER_2 = 0x04,
    MPPT_1 = 0x05,
    MPPT_2 = 0x06,
    MPPT_3 = 0x07,
    MPPT_4 = 0x08,
    LIFEPO4_1 = 0x09,
    LIFEPO4_2 = 0x0A,
    LIFEPO4_3 = 0x0B,
    LIFEPO4_4 = 0x0C,
    UNKNOWN
};

enum class Param : uint16_t {
    DEFAULT = 0x00,
    HEARTBEAT = 0x01,
    LIFEPO4_GENERAL = 0x02,
    LIFEPO4_CELLS_1 = 0x03,
    LIFEPO4_CELLS_2 = 0x04,
    LIFEPO4_CELLS_3 = 0x05,
    MPPT_GENERAL = 0x06,
    UNKNOWN
};

enum class Group : uint8_t {
    DEFAULT = 0x00,
    LIFEPO4_DATA = 0x01,
    UNKNOWN
};

struct Message_t {
    uint8_t priority;
    Param paramID;
    Group group;
};

namespace Message {

constexpr Message_t HEARTBEAT = {0, Param::HEARTBEAT, Group::DEFAULT};

constexpr Message_t LIFEPO4_GENERAL = {7, Param::LIFEPO4_GENERAL,
                                       Group::LIFEPO4_DATA};

constexpr Message_t LIFEPO4_CELLS_1 = {7, Param::LIFEPO4_CELLS_1,
                                       Group::LIFEPO4_DATA};

constexpr Message_t LIFEPO4_CELLS_2 = {7, Param::LIFEPO4_CELLS_2,
                                       Group::LIFEPO4_DATA};

constexpr Message_t LIFEPO4_CELLS_3 = {7, Param::LIFEPO4_CELLS_3,
                                       Group::LIFEPO4_DATA};

constexpr Message_t MPPT_GENERAL = {7, Param::MPPT_GENERAL, Group::DEFAULT};

} // namespace Message

} // namespace SBT::System::Comm::CAN_ID

#endif
