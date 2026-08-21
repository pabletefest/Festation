#include "cdrom.hpp"
#include "utils/logger.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <utility>

/** @brief Average seek time of 1/60th of a second in CPU cycles (should be dynamic to emulate it properly) */
static constexpr uint64_t FIXED_SEEK_TIME = 33868800 / 60;

festation::CdromDrive::CdromDrive(InterruptsHandler& intrHndRef, Scheduler& scheduler)
    : m_interruptsHandler(intrHndRef), m_scheduler(scheduler)
{
    m_regs.HINTSTS.reserved = 0x7;
    m_regs.HSTS.PRMEMPT = 1;
    m_regs.HSTS.PRMWRDY = 1;

    // TEMP
    // m_internalStatusCode.shellOpen = 1;
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
        return readSectorByte();
    case 0x1F801803:
        switch (m_regs.HSTS.RA)
        {
        case 0:
        case 2:
            return m_regs.HINTMSK.raw | 0xE0;
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

auto festation::CdromDrive::read16(uint32_t address) -> uint16_t
{
    uint16_t result{};

    switch (address)
    {
    case 0x1F801802:
        result = read8(address);
        result |= read8(address) << 8;
        break;
    default:
        std::unreachable();
    }

    return result;
}

auto festation::CdromDrive::read32(uint32_t address) -> uint32_t
{
    uint32_t result{};

    switch (address)
    {
    case 0x1F801802:
        result = read16(address);
        result |= read16(address) << 16;
        break;
    default:
        std::unreachable();
    }

    return result;
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
    case 0x02:
        processSetlocCmd();
        break;
    case 0x06:
        processReadNCmd();
        break;
    case 0x0E:
        processSetmodeCmd();
        break;
    case 0x15:
        processSeekLCmd();
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
            assert(false);
            break;
        }
    }
        break;
    case 0x1A:
        processGetIdCmd();
        break;
    default:
        LOG_DEBUG("CDROM: unimplemented command {:02X}h", cmd);
        assert(false);
        break;
    }
}

auto festation::CdromDrive::processNopCmd() -> void
{
    LOG_DEBUG("CDROM: Nop");
    constexpr uint64_t int3Delay = 0xC4E1;
    /**
     * @brief 
     * In reality, error bits are cleared after lid is opened and the CD reinserted. 
     * SW later on call NOP to check the state so we simulate that here.
     */
    m_internalStatusCode.error = 0;
    m_internalStatusCode.idError = 0;
    m_internalStatusCode.seekError = 0;

    m_regs.RESULT.append(m_internalStatusCode.raw);
    m_internalStatusCode.shellOpen = 0;
    
    m_scheduler.scheduleEvent({ EventType::CdromInt3, int3Delay, [this]() {
        LOG_DEBUG("CDROM: INT3 response");
        m_regs.HINTSTS.INTSTS = CDROM_INT3_ACKNOWLEDGE;

        if (isInterrupt()) {
            m_interruptsHandler.setInterruptSource(InterruptSource::CdromSrc);
        }
    }});
}

auto festation::CdromDrive::processSetlocCmd() -> void
{
    constexpr uint64_t firstIntDelay = 0xC4E1;
    
    uint8_t amm = m_regs.PARAMETERS.next();
    uint8_t ass = m_regs.PARAMETERS.next();
    uint8_t asect = m_regs.PARAMETERS.next();
    
    assert(convertMSFtoLDA(convertBCDtoBinary(amm), 
        convertBCDtoBinary(ass), 
            convertBCDtoBinary(asect)) >= 0);
    
    LOG_DEBUG("CDROM: Setloc ({:02X}:{:02X}:{:02X})", amm, ass, asect);

    bool areParamsValidBCD = isValidBCD(amm) && isValidBCD(ass) 
        && isValidBCD(asect);

    bool areParamsValidInput = (amm < 0x75) && (ass < 0x60) && (asect < 0x75);

    if (areParamsValidBCD && areParamsValidInput) {
        m_seekTargetBCD.minutes = amm;
        m_seekTargetBCD.seconds = ass;
        m_seekTargetBCD.sector = asect;

        m_regs.RESULT.append(m_internalStatusCode.raw);

        m_scheduler.scheduleEvent({ EventType::CdromInt3, firstIntDelay, [this]() {
            LOG_DEBUG("CDROM: INT3 response");
            m_regs.HINTSTS.INTSTS = CDROM_INT3_ACKNOWLEDGE;

            if (isInterrupt()) {
                m_interruptsHandler.setInterruptSource(InterruptSource::CdromSrc);
            }
        }});
    }
    else {
        m_internalStatusCode.error = 1;
        m_regs.RESULT.append(m_internalStatusCode.raw);
        m_regs.RESULT.append(0x10);

        m_scheduler.scheduleEvent({ EventType::CdromInt5, firstIntDelay, [this]() {
            LOG_DEBUG("CDROM: INT5 response");
            m_regs.HINTSTS.INTSTS = CDROM_INT5_DISK_ERROR;

            if (isInterrupt()) {
                m_interruptsHandler.setInterruptSource(InterruptSource::CdromSrc);
            }
        }});
    }
}

auto festation::CdromDrive::processReadNCmd() -> void
{
    LOG_DEBUG("CDROM: ReadN");
    constexpr uint64_t int3Delay = 0xC4E1;

    m_internalStatusCode.raw &= 0x1F;
    m_regs.RESULT.append(m_internalStatusCode.raw);
    m_internalStatusCode.read = 1;
    
    m_scheduler.scheduleEvent({ EventType::CdromInt3, int3Delay, [this]() {
        LOG_DEBUG("CDROM: INT3 response");
        m_regs.HINTSTS.INTSTS = CDROM_INT3_ACKNOWLEDGE;

        if (isInterrupt()) {
            m_interruptsHandler.setInterruptSource(InterruptSource::CdromSrc);
        }

        checkAndScheduleReadINT1();
    }});
}

auto festation::CdromDrive::processSetmodeCmd() -> void
{
    constexpr uint64_t int3Delay = 0xC4E1;
    
    uint8_t sectorSize = m_mode.sectorSize;
    m_mode.raw = m_regs.PARAMETERS.next();

    LOG_DEBUG("CDROM: Setmode ({:02X}h)", m_mode.raw);

    if (m_mode.ignoreBit) {
        m_mode.sectorSize = sectorSize;
    }

    m_sectorBlock.data.resize(CD_SECTOR_SIZES[m_mode.sectorSize]);

    m_regs.RESULT.append(m_internalStatusCode.raw);
    
    m_scheduler.scheduleEvent({ EventType::CdromInt3, int3Delay, [this]() {
        LOG_DEBUG("CDROM: INT3 response");
        m_regs.HINTSTS.INTSTS = CDROM_INT3_ACKNOWLEDGE;

        if (isInterrupt()) {
            m_interruptsHandler.setInterruptSource(InterruptSource::CdromSrc);
        }
    }});
}

auto festation::CdromDrive::processSeekLCmd() -> void
{
    LOG_DEBUG("CDROM: SeekL");
    constexpr uint64_t int3Delay = 0xC4E1;

    /** @brief This could also be done on the Setloc command implementation */
    uint8_t minutes = convertBCDtoBinary(m_seekTargetBCD.minutes); 
    uint8_t seconds = convertBCDtoBinary(m_seekTargetBCD.seconds); 
    uint8_t sector = convertBCDtoBinary(m_seekTargetBCD.sector);
    m_lda = convertMSFtoLDA(minutes, seconds, sector);

    m_internalStatusCode.raw &= 0x1F;
    m_regs.RESULT.append(m_internalStatusCode.raw);
    m_internalStatusCode.seek = 1;

    m_scheduler.scheduleEvent({ EventType::CdromInt3, int3Delay, [this]() {
        LOG_DEBUG("CDROM: INT3 response");
        m_regs.HINTSTS.INTSTS = CDROM_INT3_ACKNOWLEDGE;

        if (isInterrupt()) {
            m_interruptsHandler.setInterruptSource(InterruptSource::CdromSrc);
        }

        constexpr uint64_t int2Delay = FIXED_SEEK_TIME;

        m_internalStatusCode.raw &= 0x1F;
        m_regs.RESULT.append(m_internalStatusCode.raw);

        m_scheduler.scheduleEvent({ EventType::CdromInt2, int2Delay, [this]() {
            LOG_DEBUG("CDROM: INT2 response");
            m_regs.HINTSTS.INTSTS = CDROM_INT2_COMPLETE;

            if (isInterrupt()) {
                m_interruptsHandler.setInterruptSource(InterruptSource::CdromSrc);
            }
        }});
    }});
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

    m_scheduler.scheduleEvent({ EventType::CdromInt3, int3Delay, [this]() {
        LOG_DEBUG("CDROM: INT3 response");
        m_regs.HINTSTS.INTSTS = CDROM_INT3_ACKNOWLEDGE;

        if (isInterrupt()) {
            m_interruptsHandler.setInterruptSource(InterruptSource::CdromSrc);
        }
    }});
}

auto festation::CdromDrive::processGetIdCmd() -> void
{
    LOG_DEBUG("CDROM: GetId");
    constexpr uint64_t int3Delay = 0xC4E1;

    m_regs.RESULT.append(m_internalStatusCode.raw);

    m_scheduler.scheduleEvent({ EventType::CdromInt3, int3Delay, [this]() {
        LOG_DEBUG("CDROM: INT3 response");
        m_regs.HINTSTS.INTSTS = CDROM_INT3_ACKNOWLEDGE;

        if (isInterrupt()) {
            m_interruptsHandler.setInterruptSource(InterruptSource::CdromSrc);
        }

        constexpr uint64_t int2Delay = 0x4A00;

        m_regs.RESULT.append(m_internalStatusCode.raw);
        m_regs.RESULT.append(0);
        m_regs.RESULT.append(0x20); // Assuming Mode 2 (should be checked paring CUE)
        m_regs.RESULT.append(0);
        m_regs.RESULT.append(0x53);
        m_regs.RESULT.append(0x43);
        m_regs.RESULT.append(0x45);
        m_regs.RESULT.append(0x41);

        m_scheduler.scheduleEvent({ EventType::CdromInt2, int2Delay, [this]() {
            LOG_DEBUG("CDROM: INT2 response");
            m_regs.HINTSTS.INTSTS = CDROM_INT2_COMPLETE;

            if (isInterrupt()) {
                m_interruptsHandler.setInterruptSource(InterruptSource::CdromSrc);
            }
        }});
    }});
}

auto festation::CdromDrive::isInterrupt() const -> bool
{
    return (m_regs.HINTSTS.raw & 0x1F) && (m_regs.HINTMSK.raw & 0x1F);
}

auto festation::CdromDrive::checkAndScheduleReadINT1() -> void
{ 
    constexpr uint64_t int1Delay = 0x4A00;

    m_regs.RESULT.append(m_internalStatusCode.raw);

    m_scheduler.scheduleEvent({ EventType::CdromInt1, int1Delay, [this]() {
        LOG_DEBUG("CDROM: INT1 response");
        m_regs.HINTSTS.INTSTS = CDROM_INT1_DATA_READY;

        if (isInterrupt()) {
            m_interruptsHandler.setInterruptSource(InterruptSource::CdromSrc);
        }

        if (m_internalStatusCode.read) {
            checkAndScheduleReadINT1();
        }
    }});
}

auto festation::CdromDrive::readSectorByte() -> uint8_t
{
    if (m_sectorBlock.nextByte == 0) {
        m_lda++;

        auto result = m_cdReader.readCdSector(m_lda, m_sectorBlock.data);

        if (!result) {
            /** @todo */
            LOG_ERROR("Error reading a CD sector");
            assert(false);
        }
    }

    return std::to_integer<uint8_t>(m_sectorBlock.data[m_sectorBlock.nextByte++]);
}
