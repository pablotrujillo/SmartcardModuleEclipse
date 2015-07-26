/*
 * SmartcardAU9520.cpp
 *
 *  Created on: Jul 24, 2015
 *      Author: Pablo Trujillo	(Smartmatic Corp) <pablo.trujillo@smartmatic.com>
 *      Original Code: Marcel Mancini (Smartmatic Corp) <>
 *      TODO: add license and doc-comments
 */

#include "CardReaderAU9520.h"
using namespace std;

CardReaderAU9520::CardReaderAU9520(){
	init(0,0);
}

CardReaderAU9520::CardReaderAU9520(uint16_t iVendor,uint16_t iProduct) {
	init(iVendor,iProduct);
}

CardReaderAU9520::~CardReaderAU9520() {

	if(interface_locked)
		libusb_release_interface(handle, interface);

	if(libusbIsOk)
		libusb_exit(0);
}

bool CardReaderAU9520::ReaderAlcor_Open(){
#ifdef DEBUG
	cout << "fcnt_OPEN_in "<< endl;
#endif

	libusb_device **devs = NULL, *dev = NULL;
	int i = 0 , result = false;
	ssize_t cnt;
	uError = LIBUSB_SUCCESS;
	uError = libusb_init(NULL);

	if (uError < 0){
#ifdef DEBUG
		cout << "failed to init libusb: " << uError << endl;
#endif
			        return false;
	}

	libusbIsOk = true;

	cnt = libusb_get_device_list(NULL, &devs);

	if (cnt < 0)
		{
#ifdef DEBUG
			cout << "libusb_get_device_list() failed" << cnt << endl;
#endif
			uError = (int)cnt;
	        return uError;
		}

		while ((dev = devs[i++]) != NULL)
		{
			struct libusb_device_descriptor desc;

			uError = libusb_get_device_descriptor(dev, &desc);
			if (uError < 0)
			{
				result = false;
				break;
			}
#ifdef DEBUG
			cout <<"Parsing USB bus/device: ";
			ios::fmtflags f(cout.flags());
			cout << hex << uppercase << desc.idVendor << ":" << desc.idProduct;
			cout.flags(f);
			cout << " (bus " << (int)libusb_get_bus_number(dev);
			cout << ", device " << (int)libusb_get_device_address(dev) << ")" << endl;
#endif

			uError = libusb_open(dev, &handle);

			if (uError < 0)
			{
#ifdef DEBUG
				cout << "Error: " << uError;
				if(uError == LIBUSB_ERROR_ACCESS) cout << "(Access Denied!)"  << endl;
#endif
				continue;
			}

			struct libusb_config_descriptor *config_desc;
			const struct libusb_interface *usb_interface = NULL;
			int num = 0;

			if(desc.idVendor == VID  && desc.idProduct == PID)
			{
#ifdef DEBUG
				cout << "Device found";
#endif
				uError = libusb_get_active_config_descriptor(dev, &config_desc);
				if (uError < 0)
				{
#ifdef DEBUG
					cout << "Can't get config descriptor: " << uError << endl;
#endif
					result = false; break;
				}
#ifdef DEBUG
				cout << "\n Got config desc!!!" << endl;
#endif
				usb_interface = get_ccid_usb_interfaceAlt(config_desc, &num);

				if ( usb_interface == NULL )
				{
					if (0 == num)
					{
#ifdef DEBUG
						cout << "  NOT a CCID/ICCD device";
#endif
						uError = READER_NOCCID_DEVICE;
					}
					else{
#ifdef DEBUG
						cout << "Error checking CCID/ICCD interface";
#endif
						uError = READER_ERROR_CHECK_CCID;
					}

					result = false;
					break;
				}
#ifdef DEBUG
				cout << "Asking for interface!!!"<<  endl;
#endif

				interface = usb_interface->altsetting->bInterfaceNumber;

				if (libusb_kernel_driver_active(handle, interface)) // driver active
				{
#ifdef DEBUG
					cout << "kernel driver is active" << endl;
#endif
					uError = libusb_detach_kernel_driver(handle, interface);
					if (uError != 0){
#ifdef DEBUG
						cout << "Error detaching kernel driver" << endl;
#endif
						result = false;
						break;
					}
				}

				uError = libusb_claim_interface(handle, interface);
				if (uError != 0)
				{
					cout << "Error claiming Interface: (Error: " << uError <<")"<< endl;
					result = false;
					break;
				}
#ifdef DEBUG
				cout << "Got interface!!!" << endl;
#endif
				bulk_out = 0;
				bulk_in = 0;
				interrupt = 0;

				for (i=0; i<usb_interface->altsetting->bNumEndpoints; i++)
				{
					if (usb_interface->altsetting->endpoint[i].bmAttributes
							== LIBUSB_TRANSFER_TYPE_INTERRUPT)
					{
						interrupt =	usb_interface->altsetting->endpoint[i].bEndpointAddress;
#ifdef DEBUG
						cout<<"interrupt point is " << interrupt << " ";
#endif

					}
					if (usb_interface->altsetting->endpoint[i].bmAttributes
									!= LIBUSB_TRANSFER_TYPE_BULK)
									continue;

					int	bEndpointAddress = usb_interface->altsetting->endpoint[i].bEndpointAddress;

					if ((bEndpointAddress & LIBUSB_ENDPOINT_DIR_MASK)
							== LIBUSB_ENDPOINT_IN)
					{
						bulk_in = bEndpointAddress;
#ifdef DEBUG
						cout<<"bulk in is: "<< bulk_in << " ";
#endif

					}
					if ((bEndpointAddress & LIBUSB_ENDPOINT_DIR_MASK)
							== LIBUSB_ENDPOINT_OUT)
					{
						bulk_out = bEndpointAddress;
#ifdef DEBUG
						cout<<"bulk out is: " << bulk_out << " ";
#endif

					}
				}

#ifdef DEBUG
				cout << endl << "..Connected.." << endl;
#endif
				result = true;
				break;
			}
			else
			{
				if(handle != NULL)
					libusb_close(handle);
				continue;
			}
		}// END while ((dev = devs[i++]) != NULL)

	if (devs != NULL)
		libusb_free_device_list(devs, 1);
#ifdef DEBUG
	cout << "fcnt_OPEN_out "<< endl;
#endif
	return result;
}


void CardReaderAU9520::init(uint16_t iVendor,uint16_t iProduct)
{
		VID=iVendor;
		libusbIsOk = false;
		PID=iProduct;
		handle = NULL;
		interrupt = 0;
		bulk_in = 0;
		bulk_out = 0;
		interface= 0;
		interface_locked = false;
		uError = LIBUSB_SUCCESS;
}

void CardReaderAU9520::ReaderAlcor_SetLastError(int32_t SetError){
	uError = SetError;
}

int32_t CardReaderAU9520::ReaderAlcor_GetLastError()
{
	return uError;
}

CardStatus CardReaderAU9520::ReaderAlcor_getCardPresent()
{
	//// INTERRUPT
	int length = 100;
	int actual_length = 0;
	CardStatus Cstat = nochange;
	unsigned char buffer[200];
	std::vector<unsigned char> result;
	uError = LIBUSB_SUCCESS;

	uError = libusb_bulk_transfer(handle,
	interrupt, buffer, length,
	&actual_length,100);

	if(uError == LIBUSB_SUCCESS){
#ifdef DEBUG
		std:: cout << "\nInterrupt response:";
		for(int i=0;i<actual_length;i++)
			cout << "{" << (int)buffer[i]<< "}-";
		cout << endl << "Card Is Present Value: " << (buffer[1] & 0x0F) << endl;
#endif
		if((buffer[1] & 0x0F) == 0x02) Cstat  = removed;
		else
			if((buffer[1] & 0x0F) == 0x03) Cstat = inserted;
	}
	else
		if(uError == LIBUSB_ERROR_TIMEOUT) Cstat = nochange;


	return Cstat;
}

int CardReaderAU9520::ReaderAlcor_Send(vector<unsigned char> data, int *actual_length){

	uError = LIBUSB_SUCCESS;
	uError = libusb_bulk_transfer(handle,
			bulk_out, &data[0], data.size(),
			actual_length, TIMEOUT);
	return uError;
}

vector<unsigned char> CardReaderAU9520::ReaderAlcor_Receive(){

	vector<unsigned char> data;
	int length = 100;
	unsigned char buffer[300];
	int actual_length = 0;
	uError = LIBUSB_SUCCESS;
	data.clear();
		uError = libusb_bulk_transfer(handle,
				bulk_in, buffer, length,
			&actual_length, TIMEOUT);
	if(uError){
#ifdef DEBUG2
		cout << "(fnct_ReaderAlcor_Receive) Response: " << endl;
#endif
		for(int i=0;i<actual_length;i++)
		{
			data.push_back(buffer[i]);
#ifdef DEBUG2
			cout << "{" << (int)buffer[i] << "}-";
#endif
		}


#ifdef DEBUG2
	cout << endl;
#endif
	}

	return data;
}

const struct libusb_interface* CardReaderAU9520::get_ccid_usb_interfaceAlt(struct libusb_config_descriptor *desc, int *num)
{
	const struct libusb_interface *usb_interface = NULL;
	int i;
	#ifdef O2MICRO_OZ776_PATCH
	int readerID;
	#endif

	// if multiple interfaces use the first one with CCID class type
	for (i = *num; i < desc->bNumInterfaces; i++)
	{
			usb_interface = &desc->interface[i];
			// store the interface number for further reference
			*num = i;
			break;

	}

	#ifdef O2MICRO_OZ776_PATCH
	readerID = (dev->descriptor.idVendor << 16) + dev->descriptor.idProduct;
	if (usb_interface != NULL && (0 == usb_interface->altsetting->extra_length)) // this is the bug
	{
		int j;
		for (j=0; j<usb_interface->altsetting->bNumEndpoints; j++)
		{
			// find the extra[] array
			if (54 == usb_interface->altsetting->endpoint[j].extra_length)
			{
				// get the extra[] from the endpoint
				usb_interface->altsetting->extra_length = 54;
				usb_interface->altsetting->extra =
					usb_interface->altsetting->endpoint[j].extra;
				// avoid double free on close
				usb_interface->altsetting->endpoint[j].extra = NULL;
				usb_interface->altsetting->endpoint[j].extra_length = 0;
				break;
			}
		}
	}
	#endif

	return usb_interface;
} // get_ccid_usb_interface
