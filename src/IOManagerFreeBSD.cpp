/*
 * This file contains the FreeBSD-related methods of
 * Dynamo's main class (IOManager.cpp).
 */

/*
 * Copyright (c) 2017 Eric van Gyzen
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "IOManager.h"
#include "IOTargetDisk.h"

#include <cstring>

//
// Checking for all accessible drives.  Storing them, and returning the number
// of disks found.  Drives are reported in order so that Iometer does not
// need to sort them.
//
int Manager::Report_Disks(Target_Spec * /*disk_spec*/)
{
	int count = 0;
	
	// TODO

	return (count);
}

int Manager::Report_TCP(Target_Spec * tcp_spec)
{
	int count = 0;

	cout << "Reporting TCP network information..." << endl;

	for (int i = 0; i < MAX_NUM_INTERFACES; ++i) {
		// TODO 
		// get ip address for the interface
		strlcpy(tcp_spec[count].name, "0.0.0.0",
		    sizeof(tcp_spec[count].name));
		tcp_spec[count].type = TCPClientType;
		count++;

#ifdef _DEBUG
		cout << "   Found " << tcp_spec[count].name << "." << endl;
#endif
	}
	cout << "   done." << endl;
	return (count);
}

// vim: noet sts=8 sw=8
