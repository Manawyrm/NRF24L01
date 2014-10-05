#include "NRF24L01.h"

#include <stdlib.h>
#include <delay.h>

//HIGH functions
extern void NRF24L01_init(void)
{
	NRF24L01_LOW_init_IO();
	nrf24l01_config_t* config = malloc(sizeof(nrf24l01_config_t));
	config->value = 0;
	#if NRF24L01_PRESET_RX == TRUE
		config->prim_rx = 1;
	#endif
	config->pwr_up = 1;
	config->crco = 1;
	#if WIRELESS_EN_RT_IRQ != TRUE
		config->mask_max_rt = 1;
	#endif
	#if WIRELESS_EN_TX_IRQ != TRUE
		config->mask_tx_ds = 1;
	#endif
	#if WIRELESS_EN_RX_IRQ != TRUE
		config->mask_rx_dr = 1;
	#endif
	NRF24L01_LOW_set_register(NRF24L01_REG_CONFIG, config->value);
	free(config);
	nrf24l01_rf_ch_t* rf_ch = malloc(sizeof(nrf24l01_rf_ch_t));
	rf_ch->value = 0;
	rf_ch->rf_ch = WIRELESS_CHANNEL;
	NRF24L01_LOW_set_register(NRF24L01_REG_RF_CH, rf_ch->value);
	free(rf_ch);
	nrf24l01_rf_setup_t* rf_setup = malloc(sizeof(nrf24l01_rf_setup_t));
	rf_setup->value = 0;
	rf_setup->rf_pwr = NRF24L01_PRESET_TXPWR;
	rf_setup->rf_dr = NRF24L01_PRESET_BAUDRATE;
	NRF24L01_LOW_set_register(NRF24L01_REG_RF_SETUP, rf_setup->value);
	free(rf_setup);
	NRF24L01_CE_HIGH;
}

extern void NRF24L01_send_data(uint8_t* data, uint8_t len)
{
	NRF24L01_CE_LOW;
	nrf24l01_config_t* config = malloc(sizeof(nrf24l01_config_t));
	config->value = NRF24L01_LOW_get_register(NRF24L01_REG_CONFIG);
	config->prim_rx = 0;
	config->pwr_up = 1;
	NRF24L01_LOW_set_register(NRF24L01_REG_CONFIG, config->value);
	free(config);
	NRF24L01_CSN_LOW;
	spi_fast_shift(NRF24L01_CMD_FLUSH_TX);
	NRF24L01_CSN_HIGH;
	NRF24L01_CSN_LOW;
	spi_fast_shift(NRF24L01_CMD_W_TX_PAYLOAD);
	spi_transmit_sync(data, len);
	NRF24L01_CSN_HIGH;
	NRF24L01_CE_HIGH;
	_delay_us(10);
	NRF24L01_CE_LOW;
}

extern void NRF24L01_get_received_data(uint8_t* data, uint8_t len)
{
	NRF24L01_CSN_LOW;
	spi_fast_shift(NRF24L01_CMD_R_RX_PAYLOAD);
	spi_transfer_sync(data, data, len);
	NRF24L01_CSN_HIGH;
	NRF24L01_LOW_set_register(NRF24L01_REG_STATUS, NRF24L01_MASK_STATUS_RX_DR);
}

extern uint8_t NRF24L01_get_status(void)
{
	return NRF24L01_LOW_read_byte(NRF24L01_CMD_NOP);
}

extern uint8_t NRF24L01_get_payload_len(uint8_t pipe)
{
	return NRF24L01_LOW_get_register(NRF24L01_REG_RX_PW_P0 + pipe);
}

//LOW functions
void NRF24L01_LOW_init_IO(void)
{
	MODULE_CE_DDR	|= (1<<MODULE_CE_PIN);
	MODULE_CSN_DDR	|= (1<<MODULE_CSN_PIN);
	NRF24L01_CE_LOW;
	NRF24L01_CSN_HIGH;
}

void NRF24L01_LOW_set_register(uint8_t regaddr, uint8_t val)
{
	NRF24L01_CSN_LOW;
	spi_fast_shift(NRF24L01_CMD_W_REGISTER | regaddr);
	spi_fast_shift(val);
	NRF24L01_CSN_HIGH;
}

uint8_t NRF24L01_LOW_get_register(uint8_t regaddr)
{
	uint8_t byte;
	NRF24L01_LOW_read_register(regaddr, &byte, 1);
	return byte;
}

void NRF24L01_LOW_write_register(uint8_t regaddr, uint8_t* data, uint8_t len)
{
	NRF24L01_CSN_LOW;
	spi_fast_shift(NRF24L01_CMD_W_REGISTER | regaddr);
	spi_transmit_sync(data, len);
	NRF24L01_CSN_HIGH;
}

void NRF24L01_LOW_read_register(uint8_t regaddr, uint8_t* data, uint8_t len)
{
	NRF24L01_CSN_LOW;
	spi_fast_shift(NRF24L01_CMD_R_REGISTER | regaddr);
	spi_transfer_sync(data, data, len);
	NRF24L01_CSN_HIGH;
}

void NRF24L01_LOW_read_register(uint8_t regaddr, uint8_t* data, uint8_t len)
{
	NRF24L01_CSN_LOW;
	spi_fast_shift(NRF24L01_CMD_R_REGISTER | regaddr);
	spi_transfer_sync(data, data, len);
	NRF24L01_CSN_HIGH;
}

uint8_t NRF24L01_LOW_read_byte(uint8_t cmd)
{
	NRF24L01_CSN_LOW;
	uint8_t data = spi_fast_shift(cmd);
	NRF24L01_CSN_HIGH;
	return data;
}