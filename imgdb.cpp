/******************************************************************************
imgSeek ::  C++ database implementation
---------------------------------------
begin                : Fri Jan 17 2003
email                : nieder|at|mail.ru

Copyright (C) 2003-2009 Ricardo Niederberger Cabral

Clean-up and speed-ups by Geert Janssen <geert at ieee.org>, Jan 2006:
- removed lots of dynamic memory usage
- SigStruct now holds only static data
- db save and load much faster
- made Qt image reading faster using scanLine()
- simpler imgBin initialization
- corrected pqResults calculation; did not get best scores

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 ******************************************************************************/

/* C Includes */
#include <ctime> 
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* STL Includes */
#include <fstream>
#include <iostream>

using namespace std;

/* imgSeek includes */

/* Database */
#include "bloom_filter.h"
#include "imgdb.h"

#undef min
#undef max

//this will probably break horribly if it ever touches windows.h or stl crap, this is a reminder for me to actually remove this.
#define max(a, b)  (((a) > (b)) ? (a) : (b))
#define min(a, b)  (((a) > (b)) ? (b) : (a))

/* Fixed weight mask for pixel positions (i,j).
Each entry x = i*NUM_PIXELS + j, gets value max(i,j) saturated at 5.
To be treated as a constant.
 */

// Macros
#define validate_dbid(dbId) (dbSpace.count(dbId)) 
#define validate_imgid(dbId, imgId) (dbSpace.count(dbId) && (dbSpace[dbId]->sigs.count(imgId)))


void ImgDB::initImgBin()
{
	imgBinInited = 1;
	srand((unsigned)time(0)); 

	/* setup initial fixed weights that each coefficient represents */
	int i, j;

	/*
	0 1 2 3 4 5 6 i
	0 0 1 2 3 4 5 5 
	1 1 1 2 3 4 5 5
	2 2 2 2 3 4 5 5
	3 3 3 3 3 4 5 5
	4 4 4 4 4 4 5 5
	5 5 5 5 5 5 5 5
	5 5 5 5 5 5 5 5
	j
	 */

	/* Every position has value 5, */
	memset(imgBin, 5, NUM_PIXELS_SQUARED);

	/* Except for the 5 by 5 upper-left quadrant: */
	for (i = 0; i < 5; i++)
		for (j = 0; j < 5; j++)
			imgBin[i * 128 + j] = max(i, j);
	// Note: imgBin[0] == 0

}

void ImgDB::initDbase(const int dbId) {    
	/* should be called before adding images */
	if (!imgBinInited) initImgBin();
	
	if (dbSpace.count(dbId))  { // db id already used?
		cerr << "ERROR: dbId already in use" << endl;
		return;
	}
	dbSpace[dbId] = new dbSpaceStruct();
}

void ImgDB::closeDbase() {
	/* should be called before exiting app */
#ifdef DebugLib
	std::cout << "clearing database" << std::endl;
#endif
	for (dbSpaceIterator it = dbSpace.begin(); it != dbSpace.end(); it++) {
#ifdef DebugLib
		std::cout << "resetting db " << it->first << std::endl;
#endif
		removedb(it->first);
#ifdef DebugLib
		//std::cout << "resetting db " << (*it).first << "'s imgbin" << std::endl;
#endif
		//let's not touch this for now. NEEDS to be fixed later.
		//delete it->second;
		it->second = NULL;
	}
}

int ImgDB::getImageWidth(const int dbId, long int id) {
	if (!validate_imgid(dbId, id)) { cerr << "ERROR: image id (" << id << ") not found on given dbid (" << dbId << ") or dbid not existant" << endl ; return 0;};
	return dbSpace[dbId]->sigs[id]->width;
}

bool ImgDB::isImageOnDB(const int dbId, long int id) {
	return dbSpace[dbId]->sigs.count(id) > 0;
}

int ImgDB::getImageHeight(const int dbId, long int id) {
	if (!validate_imgid(dbId, id)) { cerr << "ERROR: image id (" << id << ") not found on given dbid (" << dbId << ") or dbid not existant" << endl ; return 0;};
	return dbSpace[dbId]->sigs[id]->height;
}

double_vector ImgDB::getImageAvgl(const int dbId, long int id) {
	double_vector res;
	if (!dbSpace[dbId]->sigs.count(id))
		return res;
	for(int i=0;i<3; i++) {
		res.push_back(dbSpace[dbId]->sigs[id]->avgl[i]);
	}
	return res;
}

int ImgDB::addImageFromImage(const int dbId, const long int id, Image * image ) {

	/* id is a unique image identifier
	filename is the image location
	thname is the thumbnail location for this image
	doThumb should be set to 1 if you want to save the thumbnail on thname
	Images with a dimension smaller than ignDim are ignored
	 */

	if (!image) {
		cerr << "ERROR: unable to add null image" << endl;
		return 0;
	}

	// Made static for speed; only used locally
	static Unit cdata1[16384];
	static Unit cdata2[16384];
	static Unit cdata3[16384];
	int i;
	int width, height;

	ExceptionInfo exception;

	Image *resize_image;

	/*
	Initialize the image info structure and read an image.
	 */
	GetExceptionInfo(&exception);
	
	width = image->columns;
	height = image->rows;

	resize_image = SampleImage(image, 128, 128, &exception);

	DestroyImage(image);

	DestroyExceptionInfo(&exception);
	
	if (!resize_image) {
		cerr << "ERROR: unable to resize image" << endl;
		return 0;
	}

	// store color value for basic channels
	unsigned char rchan[16384];
	unsigned char gchan[16384];
	unsigned char bchan[16384];

	GetExceptionInfo(&exception);

	const PixelPacket *pixel_cache = AcquireImagePixels(resize_image, 0, 0, 128, 128, &exception);

	for (int idx = 0; idx < 16384; idx++) {
		rchan[idx] = pixel_cache->red;
		gchan[idx] = pixel_cache->green;
		bchan[idx] = pixel_cache->blue;
		pixel_cache++;
	}

	DestroyImage(resize_image);
	
	transformChar(rchan, gchan, bchan, cdata1, cdata2, cdata3);

	DestroyExceptionInfo(&exception);

	SigStruct *nsig = new SigStruct();
	nsig->id = id;
	nsig->width = width;
	nsig->height = height;

	if (dbSpace[dbId]->sigs.count(id)) {		   
		delete dbSpace[dbId]->sigs[id];
		dbSpace[dbId]->sigs.erase(id);

		cerr << "ERROR: dbId already in use" << endl;
		return 0;		

	}
	// insert into sigmap
	dbSpace[dbId]->sigs[id] = nsig;
	// insert into ids bloom filter
	dbSpace[dbId]->imgIdsFilter->insert(id);

	calcHaar(cdata1, cdata2, cdata3,
			nsig->sig1, nsig->sig2, nsig->sig3, nsig->avgl);

	for (i = 0; i < NUM_COEFS; i++) {	// populate buckets


#ifdef FAST_POW_GEERT
		int x, t;
		// sig[i] never 0
		int x, t;

		x = nsig->sig1[i];
		t = (x < 0);		/* t = 1 if x neg else 0 */
		/* x - 0 ^ 0 = x; i - 1 ^ 0b111..1111 = 2-compl(x) = -x */
		x = (x - t) ^ -t;
		dbSpace[dbId]->imgbuckets[0][t][x].push_back(id);

		x = nsig->sig2[i];
		t = (x < 0);
		x = (x - t) ^ -t;
		dbSpace[dbId]->imgbuckets[1][t][x].push_back(id);

		x = nsig->sig3[i];
		t = (x < 0);
		x = (x - t) ^ -t;
		dbSpace[dbId]->imgbuckets[2][t][x].push_back(id);

		should not fail

#else //FAST_POW_GEERT
		//long_array3 imgbuckets = dbSpace[dbId]->imgbuckets;
		if (nsig->sig1[i]>0) dbSpace[dbId]->imgbuckets[0][0][nsig->sig1[i]].push_back(id);
		if (nsig->sig1[i]<0) dbSpace[dbId]->imgbuckets[0][1][-nsig->sig1[i]].push_back(id);

		if (nsig->sig2[i]>0) dbSpace[dbId]->imgbuckets[1][0][nsig->sig2[i]].push_back(id);
		if (nsig->sig2[i]<0) dbSpace[dbId]->imgbuckets[1][1][-nsig->sig2[i]].push_back(id);

		if (nsig->sig3[i]>0) dbSpace[dbId]->imgbuckets[2][0][nsig->sig3[i]].push_back(id);
		if (nsig->sig3[i]<0) dbSpace[dbId]->imgbuckets[2][1][-nsig->sig3[i]].push_back(id);   

#endif //FAST_POW_GEERT

	}

	// success after all
	return 1;

}

int ImgDB::addImageBlob(const int dbId, const long int id, const void *blob, const long length) {
	ExceptionInfo exception;
	ImageInfo *image_info;

	image_info = CloneImageInfo((ImageInfo *) NULL);

	Image *image = BlobToImage(image_info, blob, length, &exception);
	if (exception.severity != UndefinedException) CatchException(&exception);
		
	DestroyImageInfo(image_info);
	return addImageFromImage(dbId, id, image);
}

int ImgDB::addImage(const int dbId, const long int id, char *filename) {

	if (dbSpace[dbId]->sigs.count(id)) { // image already in db
		return 0;		
	}

	ExceptionInfo exception;
	GetExceptionInfo(&exception);

	ImageInfo *image_info;
	image_info = CloneImageInfo((ImageInfo *) NULL);
	(void) strcpy(image_info->filename, filename);
	Image *image = ReadImage(image_info, &exception);
	if (exception.severity != UndefinedException) CatchException(&exception);
	DestroyImageInfo(image_info);
	DestroyExceptionInfo(&exception);

	if (!image) {
		cerr << "ERROR: unable to read image" << endl;
		return 0;
	}
	
	return addImageFromImage(dbId, id, image);
}

int ImgDB::loaddbfromstream(const int dbId, std::ifstream& f, srzMetaDataStruct& md) {

	if (!dbSpace.count(dbId))  { // haven't been inited yet
		initDbase(dbId);
	} else { // already exists, so reset first
		resetdb(dbId);
	}

	long int id;
	int sz;	
	// read buckets
	for (int c = 0; c < 3; c++)
		for (int pn = 0; pn < 2; pn++)
			for (int i = 0; i < 16384; i++) {
				f.read((char *) &(sz), sizeof(int));
				for (int k = 0; k < sz; k++) {
					f.read((char *) &(id), sizeof(long int));
					dbSpace[dbId]->imgbuckets[c][pn][i].push_back(id);
				}
			}

	// read sigs
	sigMap::size_type szt;
	f.read((char *) &(szt), sizeof(sigMap::size_type));

	if (md.iskVersion < SRZ_V0_6_0) {
		cout << "INFO migrating database from a version prior to 0.6" << endl;
		// read sigs
		for (int k = 0; k < szt; k++) {
			sigStructV06* nsig06 = new sigStructV06();
			f.read((char *) nsig06, sizeof(sigStructV06));
			SigStruct* nsig = new SigStruct(nsig06);
			dbSpace[dbId]->sigs[nsig->id]=nsig;
		}
		return 1;

	} else { // current version

		DiskSigStruct* ndsig = new DiskSigStruct();
		for (int k = 0; k < szt; k++) {

			f.read((char *) ndsig, sizeof(DiskSigStruct));
			SigStruct* nsig = new SigStruct(ndsig);
			// insert new sig
			dbSpace[dbId]->sigs[nsig->id]=nsig;
			// insert into ids bloom filter
			dbSpace[dbId]->imgIdsFilter->insert(nsig->id);
		}
		delete ndsig;
		return 1;
	}
}

srzMetaDataStruct ImgDB::loadGlobalSerializationMetadata(std::ifstream& f) {

	srzMetaDataStruct md;

	// isk version
	f.read((char *) &(md.iskVersion), sizeof(int));

	// binding language
	f.read((char *) &(md.bindingLang), sizeof(int));

	// trial or full
	if (md.iskVersion < SRZ_V0_7_0) {
		f.read((char *) &(md.isTrial), sizeof(int));
	}
	
	// platform
	f.read((char *) &(md.compilePlat), sizeof(int));

	// ok, I have some valid metadata
	md.isValidMetadata = 1;

	return md;
}

int ImgDB::loaddb(const int dbId, char *filename) {
	std::ifstream f(filename, ios::binary);
	if (!f.is_open()) {
		cerr << "ERROR: unable to open file for read ops:" << filename << endl;
		return 0;		


	}

	int isMetadata = f.peek();

	srzMetaDataStruct md;
	md.isValidMetadata = 0;

	if (isMetadata == SRZ_VERSIONED) { // has metadata
		f.read((char *) &(isMetadata), sizeof(int));
		md = loadGlobalSerializationMetadata(f);		
	}

	int res = loaddbfromstream(dbId, f, md);

	f.close();
	return res;
}

int ImgDB::loadalldbs(char* filename) {
	std::ifstream f(filename, ios::binary);	
	
	if (!f.is_open()) { // file not found, perhaps its the first start
		return 0;
	}

	int isMetadata = f.peek();

	srzMetaDataStruct md;
	md.isValidMetadata = 0;

	if (isMetadata == SRZ_VERSIONED) {// has metadata
		f.read((char *) &(isMetadata), sizeof(int));
		if (isMetadata != SRZ_VERSIONED) {
			cerr << "ERROR: peek diff read" << endl;
			return 0;			
		}
		md = loadGlobalSerializationMetadata(f);
	}

	int dbId = 1;
	int res = 0;
	int sz = 0;

	f.read((char *) &(sz), sizeof(int)); // number of dbs

	for (int k = 0; k < sz; k++) { // for each db
		f.read((char *) &(dbId), sizeof(int)); // db id
		res += loaddbfromstream(dbId, f, md);
	}

	f.close();
	return res;
}


int ImgDB::savedbtostream(const int dbId, std::ofstream& f) {
	/*
	Serialization order:
	for each color {0,1,2}:
	for {positive,negative}:
	for each 128x128 coefficient {0-16384}:
	[int] bucket size (size of list of ids)
	for each id:
	[long int] image id
	[int] number of images (signatures)
	for each image:
	[long id] image id
	for each sig coef {0-39}:  (the NUM_COEFS greatest coefs)
	for each color {0,1,2}:
	[int] coef index (signed)
	for each color {0,1,2}:
	[double] average luminance
	[int] image width
	[int] image height

	 */
	int sz;
	long int id;

	if (!validate_dbid(dbId)) { cerr << "ERROR: database space not found (" << dbId << ")" << endl; return 0;}

	// save buckets
	for (int c = 0; c < 3; c++) {
		for (int pn = 0; pn < 2; pn++) {
			for (int i = 0; i < 16384; i++) {
				sz = dbSpace[dbId]->imgbuckets[c][pn][i].size();

				f.write((char *) &(sz), sizeof(int));
				long_listIterator end = dbSpace[dbId]->imgbuckets[c][pn][i].end();
				for (long_listIterator it = dbSpace[dbId]->imgbuckets[c][pn][i].begin(); it != end; it++) {
					f.write((char *) &((*it)), sizeof(long int));
				}
			}
		}
	}

	// save sigs
	sigMap::size_type szt = dbSpace[dbId]->sigs.size();

	f.write((char *) &(szt), sizeof(sigMap::size_type));

	for (sigIterator it = dbSpace[dbId]->sigs.begin(); it != dbSpace[dbId]->sigs.end(); it++) {
		id = (*it).first;
		SigStruct* sig = (SigStruct*) (it->second);
		DiskSigStruct dsig(*sig);
		f.write((char *) (&dsig), sizeof(DiskSigStruct));	
	}

	return 1;
}

void ImgDB::saveGlobalSerializationMetadata(std::ofstream& f) {

	int wval;

	// is versioned
	wval = SRZ_VERSIONED;
	f.write((char*)&(wval), sizeof(int));

	// isk version
	wval = SRZ_CUR_VERSION;
	f.write((char*)&(wval), sizeof(int));

	// binding language
	wval = SRZ_LANG_CPP;
	f.write((char*)&(wval), sizeof(int));

	// platform	
#ifdef _WINDOWS
	wval = SRZ_PLAT_WINDOWS;	
	f.write((char*)&(wval), sizeof(int));	
#else
	wval = SRZ_PLAT_LINUX;
	f.write((char*)&(wval), sizeof(int));	
#endif
}

int ImgDB::savedb(const int dbId, char *filename) {
	std::ofstream f(filename, ios::binary);
	if (!f.is_open()) {
		cerr << "ERROR: error opening file for write ops" << endl;
		return 0;			


	}

	saveGlobalSerializationMetadata(f);

	int res = savedbtostream( dbId, f);
	f.close();
	return res;
}

int ImgDB::savealldbs(char* filename) {
	std::ofstream f(filename, ios::binary);
	if (!f.is_open()) {
		cerr << "ERROR: error opening file for write ops" << endl;
		return 0;			
	}

	saveGlobalSerializationMetadata(f);

	int res = 0;
	int sz = dbSpace.size();
	f.write((char *) &(sz), sizeof(int)); // num dbs
	int dbId;

	for (dbSpaceIterator it = dbSpace.begin(); it != dbSpace.end(); it++) {
		dbId = (*it).first;
		f.write((char *) &(dbId), sizeof(int)); // db id
		res += savedbtostream( dbId, f);
	}

	f.close();
	return res;
}

std::vector<double> ImgDB::queryImgDataFiltered(const int dbId, Idx * sig1, Idx * sig2, Idx * sig3, double *avgl, int numres, int sketch, bloom_filter* bfilter) {
	int idx, c;
	int pn;
	Idx *sig[3] = { sig1, sig2, sig3 };

	if (bfilter) { // make sure images not on filter are penalized
		for (sigIterator sit = dbSpace[dbId]->sigs.begin(); sit != dbSpace[dbId]->sigs.end(); sit++) {

			if (!bfilter->contains((*sit).first)) { // image doesnt have keyword, just give it a terrible score
				(*sit).second->score = 99999999;
			} else { // ok, image content should be taken into account
				(*sit).second->score = 0;
				for (c = 0; c < 3; c++) {
					(*sit).second->score += weights[sketch][0][c] * fabs((*sit).second->avgl[c] - avgl[c]);
				}
			}
		}
		delete bfilter;

	} else { // search all images 
		for (sigIterator sit = dbSpace[dbId]->sigs.begin(); sit != dbSpace[dbId]->sigs.end(); sit++) {
			(*sit).second->score = 0;
			for (c = 0; c < 3; c++) {
				(*sit).second->score += weights[sketch][0][c] * fabs((*sit).second->avgl[c] - avgl[c]);
			}
		}
	}

	for (int b = 0; b < NUM_COEFS; b++) {	// for every coef on a sig
		for (c = 0; c < 3; c++) {
			//TODO see if FAST_POW_GEERT gives the same results			
#ifdef FAST_POW_GEERT    	
			pn  = sig[c][b] < 0;
			idx = (sig[c][b] - pn) ^ -pn;
#else
			pn = 0;
			if (sig[c][b]>0) {
				pn = 0;
				idx = sig[c][b];
			} else {
				pn = 1;
				idx = -sig[c][b];
			}
#endif
			// update the score of every image which has this coef
			long_listIterator end = dbSpace[dbId]->imgbuckets[c][pn][idx].end();
			for (long_listIterator uit = dbSpace[dbId]->imgbuckets[c][pn][idx].begin();
			uit != end; uit++) {
				dbSpace[dbId]->sigs[(*uit)]->score -= weights[sketch][imgBin[idx]][c];
			}
		}
	}

	sigPriorityQueue pqResults;		/* results priority queue; largest at top */

	sigIterator sit = dbSpace[dbId]->sigs.begin();

	vector<double> V;

	// Fill up the numres-bounded priority queue (largest at top):
	for (int cnt = 0; cnt < numres; cnt++) {
		if (sit == dbSpace[dbId]->sigs.end()) {
			// No more images; cannot get requested numres, so just return these initial ones.
			return V;
		}
		pqResults.push(*(*sit).second);
		sit++;
	}
	

	for (; sit != dbSpace[dbId]->sigs.end(); sit++) {
		// only consider if not ignored due to keywords and if is a better match than the current worst match
		if (((*sit).second->score < 99999) && ((*sit).second->score < pqResults.top().score)) {
			// Make room by dropping largest entry:
			pqResults.pop();
			// Insert new entry:
			pqResults.push(*(*sit).second);
		}
	}

	SigStruct curResTmp;            /* current result waiting to be returned */
	while (pqResults.size()) {
		curResTmp = pqResults.top();
		pqResults.pop();
		if (curResTmp.score < 99999) {
			V.insert(V.end(), curResTmp.id);
			V.insert(V.end(), curResTmp.score);
		}
	}

	return V;

}


/* sig1,2,3 are int arrays of length NUM_COEFS 
avgl is the average luminance
numres is the max number of results
sketch (0 or 1) tells which set of weights to use
 */
std::vector<double> ImgDB::queryImgData(const int dbId, Idx * sig1, Idx * sig2, Idx * sig3, double *avgl, int numres, int sketch) {
	return queryImgDataFiltered(dbId, sig1, sig2, sig3, avgl, numres, sketch, 0);

}

/* Will only look at avg lum
 * sig1,2,3 are int arrays of length NUM_COEFS 
avgl is the average luminance
numres is the max number of results
sketch (0 or 1) tells which set of weights to use
 */
std::vector<double> ImgDB::queryImgDataFast(const int dbId, Idx * sig1, Idx * sig2, Idx * sig3, double *avgl, int numres, int sketch) {
	int c;

	vector<double> V;

	if (!validate_dbid(dbId)) { cerr << "ERROR: database space not found (" << dbId << ")" << endl; return V;}

	for (sigIterator sit = dbSpace[dbId]->sigs.begin(); sit != dbSpace[dbId]->sigs.end(); sit++) {
		(*sit).second->score = 0;
		for (c = 0; c < 3; c++) {
			(*sit).second->score += weights[sketch][0][c]
													   * fabs((*sit).second->avgl[c] - avgl[c]);
		}
	}

	sigPriorityQueue pqResults;		/* results priority queue; largest at top */
	sigIterator sit = dbSpace[dbId]->sigs.begin();


	// Fill up the numres-bounded priority queue (largest at top):
	for (int cnt = 0; cnt < numres; cnt++) {
		if (sit == dbSpace[dbId]->sigs.end())
			// No more images; cannot get requested numres, alas.
			return V;
		pqResults.push(*(*sit).second);
		sit++;
	}

	for (; sit != dbSpace[dbId]->sigs.end(); sit++) {
		if ((*sit).second->score < pqResults.top().score) {
			// Make room by dropping largest entry:
			pqResults.pop();
			// Insert new entry:
			pqResults.push(*(*sit).second);
		}
	}

	SigStruct curResTmp;            /* current result waiting to be returned */
	while (pqResults.size()) {
		curResTmp = pqResults.top();
		pqResults.pop();
		V.insert(V.end(), curResTmp.id);
		V.insert(V.end(), curResTmp.score);
	}

	return V;

}

/* sig1,2,3 are int arrays of lenght NUM_COEFS 
avgl is the average luminance
thresd is the limit similarity threshold. Only images with score > thresd will be a result
`sketch' tells which set of weights to use
sigs is the source to query on (map of signatures)
every search result is removed from sigs. (right now this functn is only used by clusterSim)
 */
long_list ImgDB::queryImgDataForThres(const int dbId, sigMap * tsigs,
		Idx * sig1, Idx * sig2, Idx * sig3,
		double *avgl, float thresd, int sketch) {
	int idx, c;
	int pn;
	long_list res;
	Idx *sig[3] = { sig1, sig2, sig3 };

	if (!validate_dbid(dbId)) { cerr << "ERROR: database space not found (" << dbId << ")" << endl; return res;}

	for (sigIterator sit = (*tsigs).begin(); sit != (*tsigs).end(); sit++) {
		(*sit).second->score = 0;
		for (c = 0; c < 3; c++)
			(*sit).second->score += weights[sketch][0][c]
													   * fabs((*sit).second->avgl[c] - avgl[c]);
	}
	for (int b = 0; b < NUM_COEFS; b++) {	// for every coef on a sig
		for (c = 0; c < 3; c++) {
#ifdef FAST_POW_GEERT    	
			pn  = sig[c][b] < 0;
			idx = (sig[c][b] - pn) ^ -pn;
#else
			pn = 0;
			if (sig[c][b]>0) {
				pn = 0;
				idx = sig[c][b];
			} else {
				pn = 1;
				idx = -sig[c][b];
			}
#endif
			// update the score of every image which has this coef
			long_listIterator end = dbSpace[dbId]->imgbuckets[c][pn][idx].end();
			for (long_listIterator uit = dbSpace[dbId]->imgbuckets[c][pn][idx].begin();
			uit != end; uit++) {
				if ((*tsigs).count((*uit)))
					// this is an ugly line 
					(*tsigs)[(*uit)]->score -=
						weights[sketch][imgBin[idx]][c];
			}
		}
	}
	for (sigIterator sit = (*tsigs).begin(); sit != (*tsigs).end(); sit++) {
		if ((*sit).second->score < thresd) {
			res.push_back((*sit).second->id);
			(*tsigs).erase((*sit).second->id);
		}
	}
	return res;
}

long_list ImgDB::queryImgDataForThresFast(sigMap * tsigs, double *avgl, float thresd, int sketch) {
	
	// will only look for average luminance
	long_list res;

	for (sigIterator sit = (*tsigs).begin(); sit != (*tsigs).end(); sit++) {
		(*sit).second->score = 0;
		for (int c = 0; c < 3; c++)
			(*sit).second->score += weights[sketch][0][c]
													   * fabs((*sit).second->avgl[c] - avgl[c]);
		if ((*sit).second->score < thresd) {
			res.push_back((*sit).second->id);
			(*tsigs).erase((*sit).second->id);
		}
	}
	return res;
}

std::vector<double> ImgDB::queryImgID(const int dbId, long int id, int numres) {
	/*query for images similar to the one that has this id
	numres is the maximum number of results
	 */

	if (id == -1) { // query random images
		vector<double> Vres;
		if (!validate_dbid(dbId)) { cerr << "ERROR: database space not found (" << dbId << ")" << endl; return Vres;}
		long int sz = dbSpace[dbId]->sigs.size();
		int_hashset includedIds;
		sigIterator it = dbSpace[dbId]->sigs.begin();
		for (int var = 0; var < min(sz, numres); ) { // var goes from 0 to numres
			long int rint = rand()%(sz);
			for(int pqp =0; pqp < rint; pqp++) {
				it ++;			
				if (it == dbSpace[dbId]->sigs.end()) {
					it = dbSpace[dbId]->sigs.begin();
					continue;
				}
			}

			if ( includedIds.count((*it).first) == 0 ) { // havent added this random result yet
				Vres.insert(Vres.end(), (*it).first );
				Vres.insert(Vres.end(), 0 );				
				includedIds.insert((*it).first);
				++var;
			}
		}
		return Vres;
	}

	if (!validate_imgid(dbId, id)) { cerr << "ERROR: image id (" << id << ") not found on given dbid (" << dbId << ") or dbid not existant" << endl ; return std::vector<double>();};

	return queryImgData(dbId, dbSpace[dbId]->sigs[id]->sig1, dbSpace[dbId]->sigs[id]->sig2, dbSpace[dbId]->sigs[id]->sig3,
			dbSpace[dbId]->sigs[id]->avgl, numres, 0);
}

std::vector<double> ImgDB::queryImgIDFiltered(const int dbId, long int id, int numres, bloom_filter* bf) {
	/*query for images similar to the one that has this id
	numres is the maximum number of results
	 */

	if (!validate_imgid(dbId, id)) { cerr << "ERROR: image id (" << id << ") not found on given dbid (" << dbId << ") or dbid not existant" << endl ; return std::vector<double>();};
	return queryImgDataFiltered(dbId, dbSpace[dbId]->sigs[id]->sig1, dbSpace[dbId]->sigs[id]->sig2, dbSpace[dbId]->sigs[id]->sig3,
			dbSpace[dbId]->sigs[id]->avgl, numres, 0, bf);
}

std::vector<double> ImgDB::queryImgIDFast(const int dbId, long int id, int numres) {
	/*query for images similar to the one that has this id
	numres is the maximum number of results
	 */
	if (!validate_imgid(dbId, id)) { cerr << "ERROR: image id (" << id << ") not found on given dbid (" << dbId << ") or dbid not existant" << endl ; return std::vector<double>();};

	return queryImgDataFast(dbId, dbSpace[dbId]->sigs[id]->sig1, dbSpace[dbId]->sigs[id]->sig2, dbSpace[dbId]->sigs[id]->sig3,
			dbSpace[dbId]->sigs[id]->avgl, numres, 0);
}

int ImgDB::removeID(const int dbId, long int id) {

	if (!validate_imgid(dbId, id)) { cerr << "ERROR: image id (" << id << ") not found on given dbid (" << dbId << ") or dbid not existant" << endl ; return 0;};

	delete dbSpace[dbId]->sigs[id];
	dbSpace[dbId]->sigs.erase(id);
	// remove id from each bucket it could be in
	for (int c = 0; c < 3; c++)
		for (int pn = 0; pn < 2; pn++)
			for (int i = 0; i < 16384; i++)
				dbSpace[dbId]->imgbuckets[c][pn][i].remove(id);
	return 1;
}

double ImgDB::calcAvglDiff(const int dbId, long int id1, long int id2) {

	sigMap sigs = dbSpace[dbId]->sigs;

	/* return the average luminance difference */

	// are images on db ?
	if (!validate_imgid(dbId, id1)) { cerr << "ERROR: image id (" << id1 << ") not found on given dbid (" << dbId << ") or dbid not existant" << endl ; return 0;};
	if (!validate_imgid(dbId, id2)) { cerr << "ERROR: image id (" << id2 << ") not found on given dbid (" << dbId << ") or dbid not existant" << endl ; return 0;};	

	return fabs(sigs[id1]->avgl[0] - sigs[id2]->avgl[0])
	+ fabs(sigs[id1]->avgl[1] - sigs[id2]->avgl[1])
	+ fabs(sigs[id1]->avgl[2] - sigs[id2]->avgl[2]);
}	

double ImgDB::calcDiff(const int dbId, long int id1, long int id2)
{
	/* use it to tell the content-based difference between two images
	 */

	if (!validate_dbid(dbId)) { cerr << "ERROR: database space not found (" << dbId << ")" << endl; return 0;}

	if (!isImageOnDB(dbId,id1) ||
			!isImageOnDB(dbId,id2)) {
		cerr << "ERROR: image ids not found" << endl;
		return 0;			
	}

	sigMap sigs = dbSpace[dbId]->sigs;

	double diff = calcAvglDiff(dbId, id1, id2);
	Idx *sig1[3] = { sigs[id1]->sig1, sigs[id1]->sig2, sigs[id1]->sig3 };
	Idx *sig2[3] = { sigs[id2]->sig1, sigs[id2]->sig2, sigs[id2]->sig3 };

	for (int b = 0; b < NUM_COEFS; b++)
		for (int c = 0; c < 3; c++)
			for (int b2 = 0; b2 < NUM_COEFS; b2++)
				if (sig2[c][b2] == sig1[c][b])
					diff -= weights[0][imgBin[abs(sig1[c][b])]][c];

	return diff;
}

int ImgDB::destroydb(const int dbId) {
	if (!validate_dbid(dbId)) { cerr << "ERROR: database space not found (" << dbId << ")" << endl; return 0;}
	throw string("not yet implemented");		
	return 1;
}

int ImgDB::resetdb(const int dbId) {	

	if (!validate_dbid(dbId)) { cerr << "ERROR: database space not found (" << dbId << ")" << endl; return 0;}
	//TODO delete kwdstructs from globalKwdsMap
	//TODO there is a memleak here:
	// ==19336==    at 0x4C25F6C: operator new(unsigned long) (vg_replace_malloc.c:230)
	// ==19336==    by 0x878CFE0: std::string::_Rep::_S_create(unsigned long, unsigned long, std::allocator<char> const&) (in /usr/lib/libstdc++.so.6.0.10)
	// ==19336==    by 0x878D88A: std::string::_Rep::_M_clone(std::allocator<char> const&, unsigned long) (in /usr/lib/libstdc++.so.6.0.10)
	// ==19336==    by 0x878E2AB: std::string::string(std::string const&) (in /usr/lib/libstdc++.so.6.0.10)
	// ==19336==    by 0x78B046F: std::vector<std::string, std::allocator<std::string> >::_M_insert_aux(__gnu_cxx::__normal_iterator<std::string*, std::vector<std::string, std::allocator<std::string> > >, std::string const&) (new_allocator.h:108)
	// ==19336==    by 0x78B4F40: bloom_filter::generate_unique_salt() (stl_vector.h:694)
	// ==19336==    by 0x78968D8: resetdb(int) (bloom_filter.h:58)
	// ==19336==    by 0x78982FC: _wrap_resetdb (imgdb_python_linux_wrap.h:11785)   
	
	// first deallocate db memory
	
	// deallocate all buckets
	
	for (int c = 0; c < 3; c++)
		for (int pn = 0; pn < 2; pn++)
			for (int i = 0; i < 16384; i++)
				dbSpace[dbId]->imgbuckets[c][pn][i].clear();
				
	//delete sigs
	
	for (sigIterator it = dbSpace[dbId]->sigs.begin(); it != dbSpace[dbId]->sigs.end(); it++) {
		delete (*it).second;
	}   
	 
	//TODO must also clear other stuff, like ids filter
	
	dbSpace[dbId]->sigs.clear(); // this is making windows choke
	// dbSpace[dbId]->sigs = sigMap();
		
	delete dbSpace[dbId];
	dbSpace.erase(dbId);

	// finally the reset itself
	dbSpace[dbId] = new dbSpaceStruct();

	return 1;

}

long int ImgDB::getImgCount(const int dbId) {
	if (!validate_dbid(dbId)) { cerr << "ERROR: database space not found (" << dbId << ")" << endl; return 0;}	
	return dbSpace[dbId]->sigs.size();
}

bloom_filter* ImgDB::getIdsBloomFilter(const int dbId) {
	if (!validate_dbid(dbId)) { cerr << "ERROR: database space not found (" << dbId << ")" << endl; return 0;}
	return dbSpace[dbId]->imgIdsFilter;
}

std::vector<int> ImgDB::getDBList() {
	vector<int> ids;
	for (dbSpaceIterator it = dbSpace.begin(); it != dbSpace.end(); it++) {
		ids.push_back((*it).first);
	}		
	return ids;
}

void ImgDB::lazyPrintDBList() {
	int i;
	std::vector<int> dbListNums = getDBList();
	int vectorSize = dbListNums.size();

	for(i=0; i < vectorSize; ++i)
	{
		std::cout << dbListNums[i] << std::endl;
	}
}

std::vector<long int> ImgDB::getImgIdList(const int dbId) {
	vector<long int> ids;

	// TODO is there a faster way for getting a maps key list and returning a vector from it ?
	for (sigIterator it = dbSpace[dbId]->sigs.begin(); it != dbSpace[dbId]->sigs.end(); it++) {
		ids.push_back((*it).first);
	}

	return ids;
}

void ImgDB::lazyPrintImgIdList(const int dbId) {
	int i;
	vector<long int> imgIdListNums = getImgIdList(dbId);
	int vectorSize = imgIdListNums.size();

	for(i=0; i < vectorSize; ++i)
	{
		std::cout << imgIdListNums[i] << std::endl;
	}
}

bool ImgDB::isValidDB(const int dbId) {
	return dbSpace.count(dbId); 
}

bool ImgDB::removedb(const int dbId) {
	if (!validate_dbid(dbId)) { cerr << "ERROR: database space not found (" << dbId << ")" << endl; return false;}

	if (resetdb(dbId)) {
		dbSpace.erase(dbId);
		return 1;	
	}	
	return 0;
}
