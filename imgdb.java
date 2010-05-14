/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 1.3.31
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package net.imgseek.imgdb.core;

public class imgdb {
  public static DoubleVector queryImgID(int dbId, int id, int numres) {
    return new DoubleVector(imgdbJNI.queryImgID(dbId, id, numres), true);
  }

  public static DoubleVector queryImgIDFast(int dbId, int id, int numres) {
    return new DoubleVector(imgdbJNI.queryImgIDFast(dbId, id, numres), true);
  }

  public static DoubleVector queryImgData(int dbId, SWIGTYPE_p_int sig1, SWIGTYPE_p_int sig2, SWIGTYPE_p_int sig3, SWIGTYPE_p_double avgl, int numres, int sketch) {
    return new DoubleVector(imgdbJNI.queryImgData(dbId, SWIGTYPE_p_int.getCPtr(sig1), SWIGTYPE_p_int.getCPtr(sig2), SWIGTYPE_p_int.getCPtr(sig3), SWIGTYPE_p_double.getCPtr(avgl), numres, sketch), true);
  }

  public static int addImage(int dbId, int id, String filename) {
    return imgdbJNI.addImage(dbId, id, filename);
  }

  public static int savedb(int dbId, String filename) {
    return imgdbJNI.savedb(dbId, filename);
  }

  public static int loaddb(int dbId, String filename) {
    return imgdbJNI.loaddb(dbId, filename);
  }

  public static int savealldbs(String filename) {
    return imgdbJNI.savealldbs(filename);
  }

  public static int loadalldbs(String filename) {
    return imgdbJNI.loadalldbs(filename);
  }

  public static int removeID(int dbId, int id) {
    return imgdbJNI.removeID(dbId, id);
  }

  public static int resetdb(int dbId) {
    return imgdbJNI.resetdb(dbId);
  }

  public static void initDbase(int dbId) {
    imgdbJNI.initDbase(dbId);
  }

  public static void closeDbase() {
    imgdbJNI.closeDbase();
  }

  public static int getImgCount(int dbId) {
    return imgdbJNI.getImgCount(dbId);
  }

  public static boolean isImageOnDB(int dbId, int id) {
    return imgdbJNI.isImageOnDB(dbId, id);
  }

  public static int getImageHeight(int dbId, int id) {
    return imgdbJNI.getImageHeight(dbId, id);
  }

  public static int getImageWidth(int dbId, int id) {
    return imgdbJNI.getImageWidth(dbId, id);
  }

  public static double calcAvglDiff(int dbId, int id1, int id2) {
    return imgdbJNI.calcAvglDiff(dbId, id1, id2);
  }

  public static double calcDiff(int dbId, int id1, int id2) {
    return imgdbJNI.calcDiff(dbId, id1, id2);
  }

  public static DoubleVector getImageAvgl(int dbId, int id1) {
    return new DoubleVector(imgdbJNI.getImageAvgl(dbId, id1), true);
  }

  public static int addImageBlob(int dbId, int id, SWIGTYPE_p_void blob, int length) {
    return imgdbJNI.addImageBlob(dbId, id, SWIGTYPE_p_void.getCPtr(blob), length);
  }

  public static IntVector getDBList() {
    return new IntVector(imgdbJNI.getDBList(), true);
  }

  public static LongIntVector getImgIdList(int dbId) {
    return new LongIntVector(imgdbJNI.getImgIdList(dbId), true);
  }

  public static boolean isValidDB(int dbId) {
    return imgdbJNI.isValidDB(dbId);
  }

  public static int destroydb(int dbId) {
    return imgdbJNI.destroydb(dbId);
  }

  public static boolean deactivateTrial(int key) {
    return imgdbJNI.deactivateTrial(key);
  }

  public static boolean removedb(int dbId) {
    return imgdbJNI.removedb(dbId);
  }

  public static boolean addKeywordImg(int dbId, int id, int hash) {
    return imgdbJNI.addKeywordImg(dbId, id, hash);
  }

  public static boolean addKeywordsImg(int dbId, int id, IntVector hashes) {
    return imgdbJNI.addKeywordsImg(dbId, id, IntVector.getCPtr(hashes), hashes);
  }

  public static boolean removeKeywordImg(int dbId, int id, int hash) {
    return imgdbJNI.removeKeywordImg(dbId, id, hash);
  }

  public static boolean removeAllKeywordImg(int dbId, int id) {
    return imgdbJNI.removeAllKeywordImg(dbId, id);
  }

  public static IntVector getKeywordsImg(int dbId, int id) {
    return new IntVector(imgdbJNI.getKeywordsImg(dbId, id), true);
  }

  public static DoubleVector queryImgIDKeywords(int dbId, int id, int numres, int kwJoinType, IntVector keywords) {
    return new DoubleVector(imgdbJNI.queryImgIDKeywords(dbId, id, numres, kwJoinType, IntVector.getCPtr(keywords), keywords), true);
  }

  public static DoubleVector queryImgIDFastKeywords(int dbId, int id, int numres, int kwJoinType, IntVector keywords) {
    return new DoubleVector(imgdbJNI.queryImgIDFastKeywords(dbId, id, numres, kwJoinType, IntVector.getCPtr(keywords), keywords), true);
  }

  public static LongIntVector getAllImgsByKeywords(int dbId, int numres, int kwJoinType, IntVector keywords) {
    return new LongIntVector(imgdbJNI.getAllImgsByKeywords(dbId, numres, kwJoinType, IntVector.getCPtr(keywords), keywords), true);
  }

  public static double getKeywordsVisualDistance(int dbId, int distanceType, IntVector keywords) {
    return imgdbJNI.getKeywordsVisualDistance(dbId, distanceType, IntVector.getCPtr(keywords), keywords);
  }

  public static IntVector getKeywordsPopular(int dbId, int numres) {
    return new IntVector(imgdbJNI.getKeywordsPopular(dbId, numres), true);
  }

  public static ClusterVector getClusterDb(int dbId, int numClusters) {
    return new ClusterVector(imgdbJNI.getClusterDb(dbId, numClusters), true);
  }

  public static ClusterVector getClusterKeywords(int dbId, int numClusters, IntVector keywords) {
    return new ClusterVector(imgdbJNI.getClusterKeywords(dbId, numClusters, IntVector.getCPtr(keywords), keywords), true);
  }

  public static bloom_filter getIdsBloomFilter(int dbId) {
    long cPtr = imgdbJNI.getIdsBloomFilter(dbId);
    return (cPtr == 0) ? null : new bloom_filter(cPtr, false);
  }

}
