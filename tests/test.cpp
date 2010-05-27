//      test.cpp
//      
//      Copyright 2010 Hazim Gazov <Hazim.Gazov@gmail.com>
//      
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 2 of the License, or
//      (at your option) any later version.
//      
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//      
//      You should have received a copy of the GNU General Public License
//      along with this program; if not, write to the Free Software
//      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
//      MA 02110-1301, USA.


#include <iostream>
#include "imgdb.h"

//Lenna image IDs
#define LENNA_ORIG_ID 6

//Test baked torso texture IDs
#define UPPER_TEST 7
#define UPPER_TEST_FAKE 8
#define UPPER_TEST_GRAIN 9

std::string realOrFake(float difference, float threshold);

int main(int argc, char **argv)
{
	//alright, let's test a few things
	ImgDB* testDB = new ImgDB();
	
	testDB->initDbase(1);
	testDB->initDbase(99);
	testDB->addImage(1, LENNA_ORIG_ID, "./testimages/lena/lenna-orig.jpg");
	
	//make sure all images and databases that should exist do.
	std::cout << "Database List:" << std::endl << std::endl;
	
	testDB->lazyPrintDBList();
	
	std::cout << std::endl;
	
	std::cout << "Image List for database 1:" << std::endl << std::endl;
	
	testDB->lazyPrintImgIdList(1);
	
	std::cout << std::endl;
	
	//let's just save the first database for now
	testDB->savedb(1, "./test.db1");
	
	//ok, let's try saving all the databases.
	testDB->savealldbs("./test.db");
	
	//junk everything so we can test reloading
	testDB->closeDbase();
	testDB->loadalldbs("./test.db");
	
	std::cout << "Database List:" << std::endl << std::endl;
	
	testDB->lazyPrintDBList();
	
	std::cout << std::endl;
	
	std::cout << "Image List for database 1:" << std::endl << std::endl;
	
	testDB->lazyPrintImgIdList(1);
	
	std::cout << std::endl << std::endl;
	
	//let's check image similarity now, add more test images
	testDB->addImage(1, UPPER_TEST, "./testimages/bakedlayers/upper-test.tga");
	testDB->addImage(1, UPPER_TEST_FAKE, "./testimages/bakedlayers/upper-test-fake.tga");
	testDB->addImage(1, UPPER_TEST_GRAIN, "./testimages/bakedlayers/upper-test-grain.tga");
	
	//0.016 is our "magic number" to determine if the image is the same or not. Can possibly be changed at runtime
	std::cout << "Real -> Grainy Real: " << realOrFake(testDB->calcAvglDiff(1, UPPER_TEST, UPPER_TEST_GRAIN), 0.016) << std::endl;
	std::cout << "Real -> Grainy Fake: " << realOrFake(testDB->calcAvglDiff(1, UPPER_TEST, UPPER_TEST_FAKE), 0.016) << std::endl;
	std::cout << std::endl;
	
	//everything seems to be in order... let's test the destructor.
	delete testDB;
	
	std::cout << "We haven't crashed so far, all tests must have completed successfully!" << std::endl;
	return 0;
}

std::string realOrFake(float difference, float threshold)
{
	if(difference < threshold)
		return "possibly real";
	else
		return "possibly fake";
}
