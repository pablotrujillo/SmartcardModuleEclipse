/*
 * SmartcardAU9520.h
 *
 *  Created on: Jul 24, 2015
 *      Author: Pablo Trujillo	(Smartmatic Corp) <pablo.trujillo@smartmatic.com>
 *      Original Code: Marcel Mancini (Smartmatic Corp) <>
 *      TODO: add license and doc-comments
 */

#ifndef CARDREADERAU9520_H_
#define CARDREADERAU9520_H_

#include <iostream>
#include <vector>
#include <libusb-1.0/libusb.h>

#define DEBUG

enum ReaderError
{
	// NOT a CCID/ICCD device
	READER_NOCCID_DEVICE = -200,
	READER_ERROR_CHECK_CCID = -201,
};

enum CardStatus{
	removed,
	inserted,
	nochange,
};

class CardReaderAU9520 {
	uint16_t VID;
	uint16_t PID;
	int32_t uError;
	bool libusbIsOk;
	int interface;
	bool interface_locked;
	int bulk_out ;
	int bulk_in;
	int interrupt;
	struct libusb_device_handle *handle;
	static const int TIMEOUT = 300;

private:
	void init(uint16_t iVendor,uint16_t iProduct);
	const struct libusb_interface*
	get_ccid_usb_interfaceAlt(struct libusb_config_descriptor *desc, int *num);


public:
	CardReaderAU9520();
	CardReaderAU9520(uint16_t iVendor,uint16_t iProduct);
	virtual ~CardReaderAU9520();
	CardStatus ReaderAlcor_getCardPresent();
	bool ReaderAlcor_Open();
	bool ReaderAlcor_Close();
	int ReaderAlcor_Send(std::vector<unsigned char> data, int *actual_length);
	//int SendEx(std::vector<unsigned char> data);
	std::vector<unsigned char> ReaderAlcor_Receive();
	int32_t ReaderAlcor_GetLastError();
	void ReaderAlcor_SetLastError(int32_t SetError);
};

#endif /* CARDREADERAU9520_H_ */
