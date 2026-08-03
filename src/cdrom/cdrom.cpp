#include "cdrom.hpp"
#include "utils/logger.hpp"

#include <utility>

festation::CdromDrive::CdromDrive(InterruptsHandler& intrHndRef, Scheduler& scheduler)
    : m_interruptsHandler(intrHndRef), m_scheduler(scheduler)
{
    m_regs.HINTSTS.reserved = 0x7;
    m_regs.HSTS.PRMEMPT = 1;
    m_regs.HSTS.PRMWRDY = 1;
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
        return m_regs.RESULT.next();
    case 0x1F801802:
        /** @todo */
        break;
    case 0x1F801803:
        switch (m_regs.HSTS.RA)
        {
        case 0:
        case 2:
            m_regs.HINTMSK.reserved = 0x7;
            return m_regs.HINTMSK.raw;
        case 1:
        case 3:
            return m_regs.HINTSTS.raw | 0xE0;
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
        m_regs.COMMAND = value;
        decodeCommand();
        m_regs.PARAMETERS.drain();
        break;
    case 0x1F801802:
        switch (m_regs.HSTS.RA)
        {
        case 0:
            m_regs.PARAMETERS.append(value);
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

auto festation::CdromDrive::decodeCommand() -> void
{
    uint8_t cmd = m_regs.COMMAND;
    switch (cmd)
    {
    case 0x01:
        processNopCmd();
        break;
    case 0x19:
    {
        uint8_t param = m_regs.PARAMETERS.next();
        switch (param)
        {
            case 0x20:
            processBiosVersionCmd();
            break;
            default:
            LOG_DEBUG("CDROM: unimplemented test command {:02X}h", param);
            break;
        }
        break;
    }
    case 0x1A:
        processGetIdCmd();
        break;
    default:
        LOG_DEBUG("CDROM: unimplemented command {:02X}h", cmd);
        break;
    }
}

auto festation::CdromDrive::processNopCmd() -> void
{
    LOG_DEBUG("CDROM: Nop");
    constexpr uint64_t int3Delay = 0xC4E1;

    m_regs.RESULT.append(m_internalStatusCode.raw);

    m_internalStatusCode.shellOpen = 0;
    m_regs.HINTSTS.INTSTS = CDROM_INT3_ACKNOWLEDGE;

    if (m_regs.HINTSTS.INTSTS & m_regs.HINTMSK.ENINT) {
        m_scheduler.scheduleEvent({ EventType::CdromInt3, int3Delay, [this]() {
            LOG_DEBUG("CDROM: INT3 response");
            m_interruptsHandler.setInterruptSource(InterruptSource::CdromSrc);
        }});
    }
}

auto festation::CdromDrive::processBiosVersionCmd() -> void
{
    LOG_DEBUG("CDROM: GetBiosVersion");
    constexpr uint64_t int3Delay = 0xC4E1;

    /** @brief For now hardcoded to: PSX (PU-7)               19 Sep 1994, version vC0 (a) */
    m_regs.RESULT.append(0x94);
    m_regs.RESULT.append(0x09);
    m_regs.RESULT.append(0x19);
    m_regs.RESULT.append(0xC0);

    m_regs.HINTSTS.INTSTS = CDROM_INT3_ACKNOWLEDGE;

    if (m_regs.HINTSTS.INTSTS & m_regs.HINTMSK.ENINT) {
        m_scheduler.scheduleEvent({ EventType::CdromInt3, int3Delay, [this]() {
            LOG_DEBUG("CDROM: INT3 response");
            m_interruptsHandler.setInterruptSource(InterruptSource::CdromSrc);
        }});
    }
}

auto festation::CdromDrive::processGetIdCmd() -> void
{
    LOG_DEBUG("CDROM: GetId");
    constexpr uint64_t int3Delay = 0xC4E1;

    m_regs.RESULT.append(m_internalStatusCode.raw);

    m_regs.HINTSTS.INTSTS = CDROM_INT3_ACKNOWLEDGE;

    if (m_regs.HINTSTS.INTSTS & m_regs.HINTMSK.ENINT) {
        m_scheduler.scheduleEvent({ EventType::CdromInt3, int3Delay, [this]() {
            LOG_DEBUG("CDROM: INT3 response");
            m_interruptsHandler.setInterruptSource(InterruptSource::CdromSrc);

            constexpr uint64_t int3Delay = 0x4A00;

            m_regs.RESULT.append(m_internalStatusCode.raw);
            m_regs.RESULT.append(0);
            m_regs.RESULT.append(0);
            m_regs.RESULT.append(0);
            m_regs.RESULT.append(0x53);
            m_regs.RESULT.append(0x43);
            m_regs.RESULT.append(0x45);
            m_regs.RESULT.append(0x41);
            
            m_regs.HINTSTS.INTSTS = CDROM_INT2_COMPLETE;

            if (m_regs.HINTSTS.INTSTS & m_regs.HINTMSK.ENINT) {
                m_scheduler.scheduleEvent({ EventType::CdromInt3, int3Delay, [this]() {
                    LOG_DEBUG("CDROM: INT2 response");
                    m_interruptsHandler.setInterruptSource(InterruptSource::CdromSrc);
                }});
            }

        }});
    }
}
