//============================================================================
// Name        : testMemoryCard.cpp
// Author      : Pablo Trujillo
// Version     :
// Copyright   : Smartmatic Corp
// Description : Test Memody card functions
//============================================================================

#include <iostream>
#include "MemoryCard.h"

using namespace std;
// from test: if card is inserted on the beginning  this is reported, on contrary: no report is given.
int main() {
	MemoryCard TestCard;
	if(!TestCard.mCard_OpenDevice())
		cout << "[TEST- opendevice]: Could no open device" << endl;
		cout << "[TEST - CardPresent()]:";


	for(int t=0; t < 3; t++)
	{
		switch(TestCard.mCard_CardPresent())
		{
		case inserted:
			cout << ":Card Inserted" << endl;
			break;
		case removed:
			cout << ":Card Removed" << endl;
			break;
		case nochange:
		default:
			break;
		}

		usleep(100);
	}
	cout << "TEST ENDED" << endl;
// Is memory card:  init: inserted, test if is a memory card (we need a memory card).

	if(TestCard.mCard_IsMemoryCard())
		cout << "is memory card";
	else
		cout << "is not memory card";
	cout << endl;


// TEST WRITE==>> init: inserted, then write.




//TEST READ


return 0;
}



