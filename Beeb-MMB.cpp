// ******************************************************************************
// Beeb-MMB Splitter/Merger
// @created 20.02.2021
//
// This source file is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This source file is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// ******************************************************************************

#define APP_VERSION 20210317

#include <fstream>
#include <iostream>

using namespace std;

bool file_exist(const char* fileName)
{
	ifstream infile(fileName);
	return infile.good();
}

void hitKey()
{
	cout << endl;
	cout << "Hit any key to exit.";
	cin.get();
	return;
}

int main(int argc, char* argv[])
{
	cout << "Beeb-MMB Splitter/Merger v" << APP_VERSION << endl;
	cout << "(c) 2021 Thorsten M. Wahl" << endl << endl;

	//argc = 2;
	//argv[1] = (char*)"C:\\Users\\twahl\\Desktop\\BEEB\\BEEB.MMB";

	//argc = 3;
	//argv[1] = (char*)"C:\\Users\\twahl\\Desktop\\BEEB\\BAD APPLE.dsk";
	//argv[2] = (char*)"C:\\Users\\twahl\\Desktop\\BEEB\\EMPTY.dsk";

	if (argc > 1)
	{
		string filePath = argv[1];
		filePath.erase(filePath.find_last_of("\\") + 1);

		string fileName = argv[1];
		fileName.erase(0, filePath.find_last_of("\\") + 1);

		//-----------------------------------------------------------------------------------------------------------------------
		// Splitt MMB to disks
		//-----------------------------------------------------------------------------------------------------------------------

		if ((fileName.substr(fileName.find_last_of(".")) == ".MMB") || (fileName.substr(fileName.find_last_of(".")) == ".mmb"))
		{
			uint8_t header[16] = {};
			uint8_t directory[511][16] = {};
			string diskName[511] = {};

			cout << "Loading disks from '" << argv[1] << "'" << endl << endl;
			ifstream ifs(argv[1], ios::binary);

			// Read header from MMB (16 Bytes)
			for (uint8_t i = 0x00; i <= 0x0F; i++)
			{
				header[i] = ifs.get();
				//cout << hex(header[i], 2) << " ";
			}

			// Check for MMB
			uint8_t magicNumber[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
			for (uint8_t i = 0x08; i <= 0x0F; i++)
			{
				if (header[i] != magicNumber[i - 8])
				{
					cout << "Sorry but this dosn't seem to be an MMB file." << endl;
					hitKey();
					return -1;
				}
			}

			// Read directory of disks (511 * 16 Bytes)
			for (uint16_t i = 0; i <= 510; i++)
			{
				// Read full directory entry
				for (uint8_t j = 0x00; j <= 0x0F; j++)
				{
					directory[i][j] = ifs.get();
				}
				// Convert first 12 bytes to disknames if disk is valid
				if (directory[i][15] == 0x00 || directory[i][15] == 0x0F)
				{
					for (uint8_t j = 0; j <= 11; j++)
					{
						if (directory[i][j] != 0) diskName[i] += directory[i][j];
					}
				}
			}

			// Read all disks, write only valid disks (511 * 200 kBytes)
			uint8_t *diskImage = new uint8_t[0x032000];
			uint16_t counter = 0;

			for (uint16_t i = 0; i <= 510; i++)
			{
				for (uint32_t j = 0x000000; j <= 0x031FFF; j++)
				{
					diskImage[j] = ifs.get();
				}
				if (directory[i][15] == 0x00 || directory[i][15] == 0x0F)
				{
					while (file_exist((filePath + diskName[i] + ".dsk").c_str()))
					{
						diskName[i] += "I";
					}
					diskName[i] += ".dsk";
					cout << "Writing disk '" << diskName[i] << "'" << endl;
					ofstream ofs((filePath + diskName[i]).c_str(), ios::binary);
					for (int j = 0x000000; j <= 0x031FFF; j++)
					{
						ofs.put(diskImage[j]);
					}
					ofs.close();
					counter++;
				}
			}

			// Close input BEEB.MMB
			delete diskImage;
			ifs.close();

			cout << endl;
			cout << (int)counter << " disk(s) written." << endl;
			hitKey();
			return 0;
		}

		//-----------------------------------------------------------------------------------------------------------------------
		// Merge disks into MMB
		//-----------------------------------------------------------------------------------------------------------------------

		if ((fileName.substr(fileName.find_last_of(".")) == ".DSK") || (fileName.substr(fileName.find_last_of(".")) == ".dsk"))
		{
			string diskName[511] = {};
			uint8_t *diskImage[511] = {};

			// Read independent disks into memory
			for (uint16_t i = 0; i <= argc - 2; i++)
			{
				diskName[i] = argv[i + 1];
				diskName[i].erase(0, filePath.find_last_of("\\") + 1);

				diskImage[i] = new uint8_t[0x032000];

				ifstream ifs(argv[i + 1], ios::binary);
				cout << "Reading disk '" << diskName[i] << "'" << endl;

				for (uint32_t j = 0x000000; j <= 0x031FFF; j++)
				{
					diskImage[i][j] = ifs.get();
				}

				ifs.close();
			}
			cout << endl;

			// Open new BEEB.MMB
			cout << "Building a new BEEB.MMB:" << endl;
			ofstream ofs((filePath + "BEEB.MMB").c_str(), ios::binary);

			// Write header of MMB
			uint8_t fileHeader[] = { 0x00, 0x01, 0x02, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
			for (uint8_t i = 0x00; i <= 0x0F; i++)
			{
				ofs.put(fileHeader[i]);
			}

			// Write valid directory entries
			cout << "Writing valid directory entries." << endl;
			for (uint16_t i = 0; i <= argc - 2; i++)
			{
				for (uint8_t j = 0; j <= 11; j++)
				{
					if (diskName[i][j] == 0x2E)
					{
						for (uint8_t x = j; x <= 11; x++)
						{
							ofs.put(0x00);
						};
						j = 11;
					}
					else
					{
						ofs.put(diskName[i][j]);
					}
				}
				ofs.put(0x00);
				ofs.put(0x00);
				ofs.put(0x00);
				ofs.put(0x0F);
			}

			// Fill rest with empty directory entries
			cout << "Writing empty directory entries." << endl;
			for (uint16_t i = argc; i <= 511; i++)
			{
				for (uint8_t j = 0x00; j <= 0x0E; j++)
				{
					ofs.put(0x00);
				}
				ofs.put(0xF0);
			}

			// Write valid disk images
			for (uint16_t i = 0; i <= argc - 2; i++)
			{
				cout << "Writing disk '" << diskName[i] << "'" << endl;
				for (uint32_t j = 0x000000; j <= 0x031FFF; j++)
				{
					ofs.put(diskImage[i][j]);
				}
				delete diskImage[i];
			}

			// Fill rest with unformated disk images
			cout << "Writing unformated disks";
			for (uint16_t i = argc; i <= 511; i++)
			{
				for (uint32_t j = 0x000000; j <= 0x031FFF; j++)
				{
					ofs.put(0x00);
				}
				if (i % 100 == 0) cout << ".";
			}

			// Close new BEEB.MMB
			ofs.close();

			cout << endl << endl;
			cout << argc - 1 << " valid disk(s) written." << endl;
			hitKey();
			return 0;
		}

		//-----------------------------------------------------------------------------------------------------------------------

		cout << "Sorry but this dosn't seem to be an .MMB or .dsk file." << endl;
		hitKey();
		return -1;
	}
	else
	{
		// Show "help" message
		cout << "Drop a BEEB.MMB compatible file onto Beep-MMB to split it into disks." << endl;
		cout << "Drop one or more disks onto Beep-MMB to build a BEEB.MMB file." << endl;
		hitKey();
	}
	return 0;
}
