/*
 * MemoryCard.cpp
 *
 *  Created on: Jul 25, 2015
 *      Author: Pablo Trujillo	(Smartmatic Corp) <pablo.trujillo@smartmatic.com>
 *      Original Code: Marcel Mancini (Smartmatic Corp) <>
 *      TODO: add license and doc-comments
 */

#include "MemoryCard.h"

using namespace std;
//TODO: use more generic VID PID concept. prepare to use 9540.
//TODO: once all working, change to use libusb multi-session.
#define Reader_VID 0x058f
#define Reader_PID 0x9520

MemoryCard::MemoryCard() {

	MemoryCardReader = new CardReaderAU9520(Reader_VID,Reader_PID);
}

MemoryCard::~MemoryCard() {
	// TODO Auto-generated destructor stub
	delete(MemoryCardReader);
}

CardStatus MemoryCard::mCard_CardPresent(){
	return MemoryCardReader->ReaderAlcor_getCardPresent();
}

void MemoryCard::mCard_SetLastError(int32_t ErrorSet)
{
	MemoryCardReader->ReaderAlcor_SetLastError(ErrorSet);
}

bool MemoryCard::mCard_OpenDevice(){
	return MemoryCardReader->ReaderAlcor_Open();
}

bool MemoryCard::mCard_IsMemoryCard(){

	bool MemoryType =  false;
	std::vector<unsigned char> result;

	/*mCard_switchToMode(Asynchronous);

	if(mCard_powerOnNormal()) cout << "Power On Normal Success";
	else cout << "Power on Normal Failed";
	cout << "Error " << mCard_GetLastError();
	cout << endl;
*/
	mCard_switchToMode(Asynchronous);
	mCard_powerOnNormal();
	mCard_switchToMode(Asynchronous);
	mCard_powerOnNormal();
	mCard_switchToMode(I2C);
	mCard_powerOnI2C();
	mCard_switchToMode(Asynchronous);

	result = mCard_readCommandI2C(0x00,0x09,0x02);

	if(result.size()>0){
		MemoryType = true;
		}

	return MemoryType;
}

int32_t MemoryCard::mCard_GetLastError(){
	return MemoryCardReader->ReaderAlcor_GetLastError();
}

std::vector<unsigned char> MemoryCard::mCard_Read(int address, int count)
{
	unsigned char lowerByte;
	unsigned char upperByte;
	lowerByte = address & 0x000000ff;
	upperByte = (address & 0x0000ff00) >> 8;
	std::vector<unsigned char> finalResult;

	try
	{

		for(int j=0;j<count/64;j++)
		{
			std::vector<unsigned char> tempResult;

			tempResult= mCard_readCommandI2C(upperByte,lowerByte, 0x40);

			finalResult.insert(finalResult.end(),tempResult.begin(),tempResult.end());
			if(lowerByte<0xC0)
			{
				lowerByte+=0x40;
			}
			else
			{
				upperByte+=0x01;
				lowerByte=(0x40)-(0xFF-lowerByte);
				//lowerByte=0x00;
			}
		}
		int rest= count%64;
		unsigned char restByte;
		restByte = rest & 0x000000ff;
		{
			std::vector<unsigned char> restResult;
			restResult= mCard_readCommandI2C(upperByte,lowerByte,restByte);
			finalResult.insert(finalResult.end(),restResult.begin(),restResult.end());
		}

	}
	catch(std::exception &ex)
	{
#ifdef DEBUG
		cout <<"exception eaten: " << ex.what()<< endl;
#endif
	}

	return finalResult;
}

void MemoryCard::mCard_Write(int address, std::vector<unsigned char> buffer, int offset, int count)
{
	try
	{
		unsigned char lowerByte;
		unsigned char upperByte;
		lowerByte = address & 0x000000ff;
		upperByte = (address & 0x0000ff00) >> 8;
		std::vector<unsigned char>::iterator it;
		it=buffer.begin();
		for(unsigned int j=0;j<buffer.size()/64;j++)
		{
			std::vector<unsigned char> temp;
			temp.assign(it,it+64);

				mCard_writeCommandI2C(temp,upperByte,lowerByte);

			it+=64;
			if(lowerByte<0xC0)
			{
				lowerByte+=0x40;
			}
			else
			{
				upperByte+=0x01;
				lowerByte=(0x40)-(0xFF-lowerByte);
		//			lowerByte=0x00;
			}
		}
		int rest= buffer.size()%64;
		if(rest!=0)
		{
			std::vector<unsigned char> restResult;
			restResult.assign(it,it+rest);
			mCard_writeCommandI2C(restResult,upperByte,lowerByte);
		}
	}
	catch(std::exception &ex)
	{
#ifdef DEBUG
		cout << "exception eaten: " << ex.what() << endl;
#endif
	}
}


bool MemoryCard::mCard_powerOnNormal()
{
		bool success = false;

		std::vector<unsigned char> result;
		std::vector<unsigned char> powerOn;

	    powerOn.push_back(0x62);
    	//data lenght
		powerOn.push_back(0x00);
		powerOn.push_back(0x00);
		powerOn.push_back(0x00);
		powerOn.push_back(0x00);
    	//slot
		powerOn.push_back(0x00);
    	//sequence
		powerOn.push_back(0x01);
    	//RFU
		powerOn.push_back(0x00);
		powerOn.push_back(0x00);
		powerOn.push_back(0x00);
		//// POWER ON THE CARD
		int        actual_length = 0;

		//Send

		if(MemoryCardReader->ReaderAlcor_Send(powerOn,&actual_length)){
			//ANSWER
			result = MemoryCardReader->ReaderAlcor_Receive();
			if(result.size() > 0)
			{
#ifdef DEBUG
				cout << "Power ON  Normal Command response:" << endl;
				for(int i=0;i<actual_length;i++)
				cout << "{" << (int)result[i] <<"}-";
				cout << endl << "Success Values: [7]= " << result[7]<< " [8]= " << result[8] << endl;

#endif
				if((result[7]==0) || result[8]==0)
				success = true;
			}
		}
		usleep(500000); // TODO: check if this is necessary.
		return success;
}
bool MemoryCard::mCard_powerOnI2C()
{
	int rlt = false;

	std::vector<unsigned char> powerOn;
	//ABDATA
	powerOn.push_back(0x40);
	powerOn.push_back(0x51);
	powerOn.push_back(0x00);
	//5 zeros
	powerOn.push_back(0x00);
	powerOn.push_back(0x00);
	powerOn.push_back(0x00);
	powerOn.push_back(0x00);
	powerOn.push_back(0x00);

	////POWER ON THE CARD
	std::vector<unsigned char> result = mCard_sendPCToReader(powerOn);

#ifdef DEBUG
	std:: cout << endl << "Power ON  I2C Command response:";
	for(int i=0;i<(int)result.size();i++)
	{
		cout << "{" << (int)result[i]<<"}-";
	}
	cout << endl;
#endif

	if(result.size() >= 10)
	{
		if(result[7] != 0 || result[8] != 0)
		{
			rlt = false;
		}
		else
		{
			rlt = true;
		}
	}
	// change on code --> case: size < 10 is covered, what says the logic? P.T.
	return rlt;
}
bool MemoryCard::mCard_switchToMode(cardMode mode)
{
	int rlt = false;

	std::vector<unsigned char> switchToI2C;
    //ABDATA
	switchToI2C.push_back(0x40);
	switchToI2C.push_back(0x50);
	//type of mode
	if(mode == I2C)
	{
		switchToI2C.push_back(0x02);
	}
	else if(mode == Asynchronous)
	{
		switchToI2C.push_back(0x01);
	}
	//5 zeros
	switchToI2C.push_back(0x00);
	switchToI2C.push_back(0x00);
	switchToI2C.push_back(0x00);
	switchToI2C.push_back(0x00);
	switchToI2C.push_back(0x00);
	//// LETS SWITCH
	std::vector<unsigned char> result = mCard_sendPCToReader(switchToI2C);

	if (result.size() > 0)
		{
#ifdef DEBUG
		std::cout << "Switch Command response:";
		for(int i=0;i<(int)result.size();i++)
		{
			cout << "{" << (int)result[i]<<"}-";
		}
		cout << endl;
#endif

		if(result.size() >= 10)
		{
			if(result[7] != 0 || result[8] != 0)
			{
				rlt = false;
			}
			else
			{
				rlt = true;
			}
		}
	}
	// change on code --> case: size < 10 is covered, what says the logic? P.T.
	return rlt;
}
bool MemoryCard::mCard_setI2CAddress(unsigned char addressHigh,unsigned char addressLow,int pageSize)
{
		bool rlt = false;

		std::vector<unsigned char> I2CAddW;
    	//ABDATA
		I2CAddW.push_back(0x40);
		I2CAddW.push_back(0x60);
		//Address of device
		I2CAddW.push_back(0x50);
		I2CAddW.push_back(addressLow);
		//page size (8 for writin, 0 for reading)
		I2CAddW.push_back(pageSize);
		/////
		I2CAddW.push_back(0x00);
		I2CAddW.push_back(0x00);
		I2CAddW.push_back(0x00);
		//// LETS  I2C	ADD
		std::vector<unsigned char> result = mCard_sendPCToReader(I2CAddW);
		//// ANSWER
#ifdef DEBUG
        std::cout << "I2C ADD    Command response:";
		for(int i=0;i<(int)result.size();i++)
		{
            cout << "{" << (unsigned char)result[i] <<"}-";
		}
		cout << endl;
#endif
		if(result.size() >= 10)
		{
			if(result[7] != 0 || result[8] != 0)
			{
				rlt =  false;
			}
			else
			{
				rlt = true;
			}
		}
		return rlt;
}
vector<unsigned char> MemoryCard::mCard_sendPCToReader(vector<unsigned char> command)
{
	int actual_length = 0;

	std::vector<unsigned char> pcToReader;
	std::vector<unsigned char> result;

	//PCrdrEscapeCommand
	pcToReader.push_back(0x6B);
    //data length
	pcToReader.push_back(command.size());
	pcToReader.push_back(0x00);
	pcToReader.push_back(0x00);
	pcToReader.push_back(0x00);
	//slot
	pcToReader.push_back(0x00);
    //sequence
	pcToReader.push_back(0xFF);
	//RFU
	pcToReader.push_back(0x00);
	pcToReader.push_back(0x00);
	pcToReader.push_back(0x00);
	//ABDATA
	pcToReader.insert(pcToReader.end(),command.begin(), command.end());

	result.empty();

	//Send

	if(MemoryCardReader->ReaderAlcor_Send(pcToReader,&actual_length)){
		//ANSWER
		result = MemoryCardReader->ReaderAlcor_Receive();
	}

	return result;
}
std::vector<unsigned char> MemoryCard::mCard_readCommandI2C(unsigned char addressHigh,unsigned char addressLow, ulong lenghtToRead)
{
		mCard_setI2CAddress(addressHigh,addressLow,0);
        std::vector<unsigned char> readCmd;
        std::vector<unsigned char> result;
	    //ABDATA
		readCmd.push_back(0x40);
		readCmd.push_back(0x62);
    	//length low byte first and high byte second
		readCmd.push_back(lenghtToRead);
		readCmd.push_back(0x00);
		//Four zeros???
		readCmd.push_back(0x00);
		readCmd.push_back(0x00);
		readCmd.push_back(0x00);
		readCmd.push_back(0x00);

		//// LETS TRY TO READ
		result.empty();

		result = mCard_sendPCToReader(readCmd);

		////ANSWER
#ifdef DEBUG
        std::cout << "Read Command response:";
#endif

        std::vector<unsigned char> finalResult;

        for(int i=0;i<(int)result.size();i++){
#ifdef DEBUG
			cout << "{" << result[i] << "}-";
#endif
			if(i >= 10)
			{
				finalResult.push_back(result[i]);
			}
		}
#ifdef DEBUG
		cout << endl;
#endif
		return finalResult;
}
//TODO: this command got no status, no idea if write was good, check this
void MemoryCard::mCard_writeCommandI2C(std::vector<unsigned char> rawData, unsigned char addressHigh,unsigned char addressLow){
		std::vector<unsigned char> result;

		mCard_setI2CAddress(addressHigh,addressLow,8);
		////WRITE COMMAND
        std::vector<unsigned char> writeCmd;
	    //ABDATA
		writeCmd.push_back(0x40);
		writeCmd.push_back(0x61);
		//
		writeCmd.push_back(rawData.size());
		writeCmd.push_back(0x00);
		//4 zeros
		writeCmd.push_back(0x00);
		writeCmd.push_back(0x00);
		writeCmd.push_back(0x00);
		writeCmd.push_back(0x00);
		//data
		writeCmd.insert(writeCmd.end(), rawData.begin(),rawData.end());
		//// LETS TRY TO WRITE
  		 result = mCard_sendPCToReader(writeCmd);
		//// ANSWER todo: check this values to know if the write was done
#ifdef DEBUG
	    std::cout << "Write Command response:";
		for(int i=0;i<(int)result.size();i++)
			cout << "{" << (int)result[i] << "}-";
		cout << endl;
#endif
		}

