/*
 * Copyright 2019 Solace Corporation. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *  https://www.apache.org/licenses/LICENSE-2.0
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */

#include <cstdlib>
#include <iostream>
#include <iterator>
#include <tuple>
#include <memory>

#include "scc-toolkit.h"
#include "Modem_Connector.h"

namespace {

const char* DEVICE_CERT = "device-cert";

}

namespace {

const char* DIGICERT_CA = "digicert-ca";

}

namespace {

const char* DEVICE_PRIVATE_KEY = "device-private-key";

}

////////////////////////////////////////////////////////////////////////////////
// Read the certificate DER file
////////////////////////////////////////////////////////////////////////////////
scc::OctetString readCert(std::string derCertFileName)
{
    std::ifstream certFile;
    certFile.exceptions ( std::ifstream::failbit | std::ifstream::badbit );
    try {
        certFile.open(derCertFileName, std::ios_base::binary);
    } catch (const std::ifstream::failure &e) {
        std::cout << "Couldn't open certificate file ["
            << derCertFileName << "]" << std::endl;
        throw;
    }

    std::istreambuf_iterator<char> start(certFile), end;

    return std::vector<uint8_t>(start, end);
}

////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv) {
	if (argc!=4){
		std::cout << "Usage:"<< argv[0] << " <private-key-path> <device-certificate-path> <ca-certificate-path> " << std::endl;
		return 0;
	}
    std::cout << "Import device private key and certificate" << std::endl;
	std::cout << "Private key: " << argv[1] << std::endl;
	std::cout << "Device certificate: " << argv[2] << std::endl;
	std::cout << "CA certificate: " << argv[3] << std::endl;



    std::unique_ptr<scc::UICC_Connector> pIfd;

    //try connecting to the modem first
    std::vector<std::string> portList { "/dev/ttyUSB2", "/dev/ttyUSB1" };
    //try each Port until we find one with the Modem and SCC applet
    for (std::string tstPortName: portList)
        try {
            pIfd = std::make_unique<scc::Modem_Connector>(tstPortName);
            scc::SCC_Application sccApp(*pIfd);
            std::cout << "Opened Modem [" << pIfd->getName() << "]" << std::endl;
            //we found a suitable reader
            break;
        } catch (const scc::UICC_Connector_Exception &e) {
        	std::cout << "Could not open [" << tstPortName << "]" << std::endl;
        	std::cout << "Error [" << e.getError() << "]" << std::endl;
            pIfd = nullptr;
        }

    if (pIfd == nullptr) {
        std::cout << "Couldn't open any UICC connections" << std::endl;
        return EXIT_FAILURE;
    }

    try {
        scc::SCC_Application sccApp(*pIfd);

        int numBlocks = 4;
        std::vector<uint8_t> tmp(50);
        std::fill (tmp.begin(), tmp.end(), 0x55);
        auto blk { scc::OctetString(tmp) };


        //TEMP until cipher allows varying lengths ... TODO
        std::vector<uint8_t> tmp1(96);
        std::fill (tmp1.begin(), tmp1.end(), 0xAA);
        auto blk1 { scc::OctetString(tmp1) };

        int messageLen = numBlocks*blk.length();


        std::cout << "***************************" << std::endl;
        std::cout << "*** Delete All Entries ****" << std::endl;
        std::cout << "***************************" << std::endl;
        sccApp.deleteAllEntries();
        std::cout << "Erased all Entries" << std::endl;

        std::cout << "************************" << std::endl;
        std::cout << "*** Generate Random  ***" << std::endl;
        std::cout << "************************" << std::endl;
        auto rand = sccApp.getRandom(512);
        std::cout << "Random: " << rand << std::endl;


        std::cout << "***********************" << std::endl;
        std::cout << "*** RSA Private Key ***" << std::endl;
        std::cout << "************************" << std::endl;
        auto rsaPrivateKeyData = readCert(argv[1]);
		sccApp.createEntry(
		                    0x2001,
		                    DEVICE_PRIVATE_KEY,
							rsaPrivateKeyData
		                    );
        //read out the data we just wrote ...
        std::cout << "Read using Entry ID" << std::endl;
        std::cout << "  Entry size: " << sccApp.getEntrySize(0x2001) << std::endl;
        std::cout << "  Entry : GUID " << sccApp.getEntry(0x2001) << std::endl;
        std::cout << std::endl;

        std::cout << "Read using label" << std::endl;
		std::cout << "  Entry size: " << sccApp.getEntrySize(DEVICE_PRIVATE_KEY)
				<< std::endl;
		std::cout << "  Entry : GUID " << sccApp.getEntry(DEVICE_PRIVATE_KEY)
				<< std::endl;

        std::cout << "***********************" << std::endl;
        std::cout << "*** RSA Certificate ***" << std::endl;
        std::cout << "***********************" << std::endl;

        ////////////////////////////////////////////////////////////////////////
         // Read device certificate and store on card
         ////////////////////////////////////////////////////////////////////////
         auto rsaCertData = readCert(argv[2]);

         sccApp.createCertificate(
                     0x5001,
                     DEVICE_CERT,
                     rsaCertData
                     );

         //read out the certificate -- using the ID of entry
         auto rsaCertValue { sccApp.getCert(0x5001) };
         std::cout << "Certificate: " << rsaCertValue << std::endl;

		scc::SCC_RSACertificate rsaCert { sccApp, DEVICE_CERT };

         std::cout << "RSA Certificate ID: [" << std::hex << rsaCert.getId()
                   << "]" << std::endl;

         ////////////////////////////////////////////////////////////////////////
          // Read root CA certificate and store on card
          ////////////////////////////////////////////////////////////////////////
          auto rsaCACertData = readCert(argv[3]);

          sccApp.createCertificate(
                      0x5022,
                      DIGICERT_CA,
					  rsaCACertData
                      );

          auto rsaCACertValue { sccApp.getCert(DIGICERT_CA) };
          std::cout << "Certificate: " << rsaCACertValue << std::endl;

		scc::SCC_RSACertificate rsaCACert { sccApp, DIGICERT_CA };

          std::cout << "RSA Certificate ID: [" << std::hex << rsaCACert.getId()
                    << "]" << std::endl;



    } catch (const std::ifstream::failure &e) {
        std::cout << "Caught: file handling error" << std::endl;
    } catch (scc::UICC_Exception &e) {
    	//auto [sw12, ins];
		std::tuple<int, int> error = e.getError();
        std::cout << "Caught UICC_Exception: INS[" << std::hex << std::get<0>(error) << "] SW12[" << std::get<1>(error) << "]" << std::endl;
    } catch (const scc::UICC_Connector_Exception &e) {
        std::cout << "Caught UICC_Connector_Exception: " << e.getError() << std::endl;
    } catch (...) {
        std::cout << "Caught default excecption" << std::endl;
    }

    return EXIT_SUCCESS;
}

