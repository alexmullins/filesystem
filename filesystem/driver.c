#pragma once

#include "driver.h"

int EraseAllSectors() {
	if (!driverFile) {
		ensureOpen();
	}

	writeAllOnes();
}

void ensureOpen() {
	// check if file exists:

	// if it doesn't exist:
	//		open it 
	// if it does exist:
	//		open it and zero it.
}

