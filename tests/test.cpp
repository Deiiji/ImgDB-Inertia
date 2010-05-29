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
#include <sstream>
#include "Qt/qimage.h"
#include "imgdb.h"

//Lenna image IDs
#define LENNA_ORIG_ID 6
#define LENNA_ORIG_GRAINY_ID 7
#define LENNA_NEW_VISIT_ID 8

//Test baked torso texture IDs
#define UPPER_TEST 9
#define UPPER_TEST_FAKE 10
#define UPPER_TEST_GRAIN 11

//Solid color test IDs
#define WHITE_TEST_ID 12
#define BLACK_TEST_ID 13

std::string realOrFake(float difference, float threshold);
void quickCompare(ImgDB* ourDB, const int dbId, const int firstImage, const int secondImage, float threshold, const std::string message);

template <class T>
bool from_string(T& t, 
                 const std::string& s, 
                 std::ios_base& (*f)(std::ios_base&))
{
  std::istringstream iss(s);
  return !(iss >> f >> t).fail();
}


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
	//0.016 is our "magic number" to determine if the image is the same or not. Can possibly be changed at runtime
	//targa-based tests are commented out until I get targa files working with QImage
	
	/*
	std::cout << "Baked Texture Comparisons" << std::endl << "============" << std::endl;
	
	testDB->addImage(1, UPPER_TEST, "./testimages/bakedlayers/upper-test.tga");
	testDB->addImage(1, UPPER_TEST_FAKE, "./testimages/bakedlayers/upper-test-fake.tga");
	testDB->addImage(1, UPPER_TEST_GRAIN, "./testimages/bakedlayers/upper-test-grain.tga");
	
	std::cout << "Real -> Grainy Real: " << realOrFake(testDB->calcAvglDiff(1, UPPER_TEST, UPPER_TEST_GRAIN), 0.016) << std::endl;
	std::cout << "Real -> Grainy Fake: " << realOrFake(testDB->calcAvglDiff(1, UPPER_TEST, UPPER_TEST_FAKE), 0.016) << std::endl;
	std::cout << std::endl;
	*/
	
	std::cout << "Lenna Comparisons" << std::endl << "============" << std::endl;
	
	testDB->addImage(1, LENNA_ORIG_GRAINY_ID, "./testimages/lena/lenna-orig-grainy.jpg");
	testDB->addImage(1, LENNA_NEW_VISIT_ID, "./testimages/lena/lena-visit1.jpg");
	
	quickCompare(testDB, 1, LENNA_ORIG_ID, LENNA_ORIG_GRAINY_ID, 0.016, "Original -> Grainy Original");
	quickCompare(testDB, 1, LENNA_ORIG_ID, LENNA_NEW_VISIT_ID, 0.016, "Original -> New Image");
	std::cout << std::endl << std::endl;
	
	//according to this test there's a margin of error of at least 0.40%...
	std::cout << "Solid Color Comparisons" << std::endl << "============" << std::endl;
	
	testDB->addImage(1, WHITE_TEST_ID, "./testimages/solid/white.png");
	testDB->addImage(1, BLACK_TEST_ID, "./testimages/solid/black.png");
	
	quickCompare(testDB, 1, WHITE_TEST_ID, BLACK_TEST_ID, 0.016, "Solid White -> Solid Black");
	std::cout << std::endl << std::endl;
	
	//everything seems to be in order... let's test the destructor.
	delete testDB;
	
	std::cout << "We haven't crashed so far, all tests must have completed successfully!" << std::endl;
	return 0;
}

std::string realOrFake(float difference, float threshold)
{
	if(difference < threshold)
		return "possibly the same image";
	else
		return "possibly different images";
}

void quickCompare(ImgDB* ourDB, const int dbId, const int firstImage, const int secondImage, float threshold, const std::string message)
{
	float imageDifference = ourDB->calcAvglDiff(dbId, firstImage, secondImage);
	
	char prettyDifference[7]; //the percentage will be 6 chars max, plus null
	sprintf(prettyDifference, "%0.2f", imageDifference * 100);
	
	std::cout << message << ": " << realOrFake(imageDifference, threshold) << " (" << prettyDifference << "% different)" << std::endl;
}
