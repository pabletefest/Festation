#include "cdrom.hpp"

#include <utility>

festation::CdromDrive::CdromDrive()
{
    m_regs.HINTSTS.reserved = 0x7;
}

festation::CdromDrive::~CdromDrive()
{

}

auto festation::CdromDrive::read8(uint32_t address) -> uint8_t
{
    switch (address)
    {
    case 0x1F801800:
        return m_regs.HSTS.raw;
    case 0x1F801801:
        return m_regs.RESULT;
    case 0x1F801802:
        /** @todo */
        return m_regs.RDDATA & 0xFF;
    case 0x1F801803:
        switch (m_regs.HSTS.RA)
        {
        case 0:
        case 2:
            m_regs.HINTMSK.reserved = 0x7;
            return m_regs.HINTMSK.raw;
        case 1:
        case 3:
            return m_regs.HINTSTS.raw;
        default:
            std::unreachable();
        }
        break;
    default:
        std::unreachable();
    }

    return 0;
}

auto festation::CdromDrive::write8(uint32_t address, uint8_t value) -> void
{
    switch (address)
    {
    case 0x1F801800:
        m_regs.HSTS.RA = value & 3;
        break;
    case 0x1F801801:
        break;
    case 0x1F801802:
        switch (m_regs.HSTS.RA)
        {
        case 0:
            break;
        case 1:
            m_regs.HINTMSK.raw = value;
            m_regs.HINTMSK.reserved = 0;
            break;
        case 2:
            break;
        case 3:
            break;
        default:
            std::unreachable();
        }
        break;
    case 0x1F801803:
        switch (m_regs.HSTS.RA)
        {
        case 0:
            break;
        case 1:
            m_regs.HCLRCTL.raw = value;
            break;
        case 2:
            break;
        case 3:
            break;
        default:
            std::unreachable();
        }
        break;
    default:
        std::unreachable();
    }
}
