
/*! ***************************************************************************
 *
 * \brief     I2C functions
 * \file      i2c.cpp
 * \author    Hugo Arends
 * \date      July 2025
 *
 * \copyright 2025 HAN University of Applied Sciences. All Rights Reserved.
 *            \n\n
 *            Permission is hereby granted, free of charge, to any person
 *            obtaining a copy of this software and associated documentation
 *            files (the "Software"), to deal in the Software without
 *            restriction, including without limitation the rights to use,
 *            copy, modify, merge, publish, distribute, sublicense, and/or sell
 *            copies of the Software, and to permit persons to whom the
 *            Software is furnished to do so, subject to the following
 *            conditions:
 *            \n\n
 *            The above copyright notice and this permission notice shall be
 *            included in all copies or substantial portions of the Software.
 *            \n\n
 *            THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *            EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 *            OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *            NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 *            HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 *            WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *            FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 *            OTHER DEALINGS IN THE SOFTWARE.
 *
 *****************************************************************************/
 #include "i2c.h"
 #include "fsl_common.h"
 #include "fsl_lpi2c.h"
 #include "fsl_lpflexcomm.h"
 #include "fsl_port.h"
 #include "board.h"
 #include <cstdint>

 namespace {
 constexpr uint32_t kLpi2cInstance = 2U;
 constexpr uint32_t kLpi2cClockHz = 12000000U;
 constexpr uint32_t kI2cBaudRateHz = 100000U;
 LPI2C_Type* const kI2cBase = LPI2C2;
 bool i2c_initialized = false;
 }

// -----------------------------------------------------------------------------
// Local type definitions
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Local function prototypes
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Local variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Local function implementation
// -----------------------------------------------------------------------------

/**
 * @brief Initializes the I2C master interface.
 *
 * Configures the SDA and SCL pins as outputs, enables pull-up resistors,
 * and sets the SCL bit rate to 100kHz based on the CPU frequency.
 */
void i2c_master_init(void)
{
    if (i2c_initialized) {
        return;
    }

    // The FRDM-MCXN947 Arduino I2C header uses FC2_P0/P1 on P4_0/P4_1.
    CLOCK_EnableClock(kCLOCK_Port4);
    const port_pin_config_t i2c_pin_config = {
        kPORT_PullUp,
        kPORT_LowPullResistor,
        kPORT_FastSlewRate,
        kPORT_PassiveFilterDisable,
        kPORT_OpenDrainDisable,
        kPORT_LowDriveStrength,
        kPORT_MuxAlt2,
        kPORT_InputBufferEnable,
        kPORT_InputNormal,
        kPORT_UnlockRegister};
    PORT_SetPinConfig(PORT4, 0U, &i2c_pin_config); // FC2_P0 / SDA
    PORT_SetPinConfig(PORT4, 1U, &i2c_pin_config); // FC2_P1 / SCL

    CLOCK_SetClkDiv(kCLOCK_DivFlexcom2Clk, 1U);
    CLOCK_AttachClk(kFRO12M_to_FLEXCOMM2);
    RESET_ClearPeripheralReset(kFC2_RST_SHIFT_RSTn);
    LP_FLEXCOMM_Init(kLpi2cInstance, LP_FLEXCOMM_PERIPH_LPI2C);

    lpi2c_master_config_t config;
    LPI2C_MasterGetDefaultConfig(&config);
    config.baudRate_Hz = kI2cBaudRateHz;
    LPI2C_MasterInit(kI2cBase, &config, kLpi2cClockHz);
    i2c_initialized = true;
}

/**
 * @brief Writes data to an I2C slave device.
 *
 * Generates a start condition, sends the slave address with write bit,
 * writes the specified number of bytes from the provided buffer, and
 * generates a stop condition.
 *
 * @param dev_address 7-bit I2C slave device address.
 * @param p Pointer to the data buffer to be written.
 * @param len Number of bytes to write.
 */
bool i2c_write(const uint8_t dev_address, uint8_t *p, const uint8_t len)
{
    if (!i2c_initialized || p == nullptr || len == 0U) {
        return false;
    }

    lpi2c_master_transfer_t transfer = {};
    transfer.flags = kLPI2C_TransferDefaultFlag;
    transfer.slaveAddress = dev_address;
    transfer.direction = kLPI2C_Write;
    transfer.data = p;
    transfer.dataSize = len;
    return LPI2C_MasterTransferBlocking(kI2cBase, &transfer) == kStatus_Success;
}

/**
 * @brief Reads data from an I2C slave device.
 *
 * Generates a start condition, sends the slave address with read bit,
 * reads the specified number of bytes into the provided buffer, and
 * generates a stop condition.
 *
 * @param dev_address 7-bit I2C slave device address.
 * @param p Pointer to the buffer where read data will be stored.
 * @param len Number of bytes to read.
 */
bool i2c_read(const uint8_t dev_address, uint8_t *p, const uint8_t len)
{
    if (!i2c_initialized || p == nullptr || len == 0U) {
        return false;
    }

    lpi2c_master_transfer_t transfer = {};
    transfer.flags = kLPI2C_TransferDefaultFlag;
    transfer.slaveAddress = dev_address;
    transfer.direction = kLPI2C_Read;
    transfer.data = p;
    transfer.dataSize = len;
    return LPI2C_MasterTransferBlocking(kI2cBase, &transfer) == kStatus_Success;
}

bool i2c_read_register(const uint8_t dev_address,
                       const uint8_t reg,
                       uint8_t *p,
                       const uint8_t len)
{
    if (!i2c_initialized || p == nullptr || len == 0U) {
        return false;
    }

    // The SDK emits: START + address(W) + register + repeated START +
    // address(R) + data + STOP. This keeps the register pointer and the
    // read operation in one atomic transaction, including 24-byte reads.
    lpi2c_master_transfer_t transfer = {};
    transfer.flags = kLPI2C_TransferDefaultFlag;
    transfer.slaveAddress = dev_address;
    transfer.direction = kLPI2C_Read;
    transfer.subaddress = reg;
    transfer.subaddressSize = 1U;
    transfer.data = p;
    transfer.dataSize = len;
    return LPI2C_MasterTransferBlocking(kI2cBase, &transfer) == kStatus_Success;
}
