/*
 * MemoryCard.h
 *
 *  Created on: Jul 25, 2015
 *      Author: Pablo Trujillo	(Smartmatic Corp) <pablo.trujillo@smartmatic.com>
 *      Original Code: Marcel Mancini (Smartmatic Corp) <>
 *      TODO: add license and doc-comments
 */

#ifndef MEMORYCARD_H_
#define MEMORYCARD_H_

#include <iostream>
#include <vector>
#include "CardReaderAU9520.h"


enum cardMode
{
	I2C,
	Asynchronous
};


class MemoryCard {

	CardReaderAU9520 *MemoryCardReader;

public:
	MemoryCard();
	bool mCard_OpenDevice();
	virtual ~MemoryCard();
	std::vector<unsigned char> mCard_Read(int address, int count);
	void mCard_Write(int address, std::vector<unsigned char> buffer, int offset, int count);
	bool mCard_IsMemoryCard();
	int32_t mCard_GetLastError();
	CardStatus mCard_CardPresent();
	void mCard_SetLastError(int32_t ErrorSet);
private:

	bool mCard_switchToMode(cardMode mode);
	bool mCard_powerOnI2C();
	bool mCard_powerOnNormal();
	bool mCard_setI2CAddress(unsigned char addressHigh,unsigned char addressLow,int pageSize);
	std::vector<unsigned char> mCard_sendPCToReader(std::vector<unsigned char> command);
	std::vector<unsigned char> mCard_readCommandI2C(unsigned char addressHigh,unsigned char addressLow,ulong lenghtToRead);
	void mCard_writeCommandI2C(std::vector<unsigned char> rawData, unsigned char addressHigh,unsigned char addressLow);
};

#endif /* MEMORYCARD_H_ */
