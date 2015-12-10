/*
 * ACFDetector.cpp
 *
 *  Created on: 2015年11月9日
 *      Author: edison
 */


#include "ACFDetector.h"


using namespace acf;
using namespace std;
using cv::Size;
using cv::Mat;



ACFDetector::ACFDetector(ACFDetector::Builder* builder) {
	this->_builder = builder;
	OUT("ACFDetector(Builder*)");
	this->_cascThr = -1;
	this->_gtDir = builder->_gtDir;
	this->_modelDs = builder->_modelDs;
	this->_modelDsPad = builder->_modelDsPad;
	this->_name = builder->_name;
	this->_negImgDir = builder->_negImgDir;
	this->_negWinDir = builder->_negWinDir;
	this->_posImgDir = builder->_posImgDir;
	this->_posWinDir = builder->_posWinDir;
	this->_stride = builder->_stride;
	this->_clf = builder->_clf;
}

ACFDetector::	~ACFDetector(){
	delete[] this->_clf->child;
	delete[] this->_clf->fids;
	delete[] this->_clf->hs;
	delete[] this->_clf->thrs;
	delete this->_clf;
	delete this->_builder;
	OUT("~ACFDetector()");
}

//ACFDetector::Builder::Builder(const string configFile){ cout << "Builder(config) from " << configFile << endl;}
ACFDetector::Builder::Builder(const string name,const string posDir,const string gtDir){
	OUT("Builder(string,string,string)");
	this->_name = name;
	this->_posImgDir = posDir;
	this->_gtDir = gtDir;
}

ACFDetector::Builder::~Builder(){
	OUT("~Builder() ");
}

//ostream& acf::operator<< (ostream& os,const acf::ACFDetector::Builder& b){
//	cout<< "Builder<<";
//	return os;
//}

std::ostream& acf::operator<<(std::ostream&os ,const ACFDetector& d){
	cout<< "ACFDetector{";
	cout << "name:"<<d._name<<',';
	cout << "posDir:"<<d._posImgDir<<',';
	cout << "gtDir:"<<d._gtDir<<',';
	cout << "negDir:"<<d._negImgDir<<',';
	cout << "posWinDir:"<<d._posWinDir<<',';
	cout << "negWinDir:"<<d._negWinDir<<',';
	cout << "stride:"<<d._stride<<',';
	cout << "modelDsPad:"<<d._modelDsPad<<',';
	cout << "modelDs:"<<d._modelDs<<"...}";
	return os;
}

ACFDetector::Builder* ACFDetector::Builder::negImgDir(std::string negDir){
	this->_negImgDir = negDir;
	return this;
}

ACFDetector::Builder* ACFDetector::Builder::posWinDir(std::string posWinDir){
	this->_posWinDir = posWinDir;
	return this;
}

ACFDetector::Builder* ACFDetector::Builder::negWinDir(std::string negWinDir){
	this->_negWinDir = negWinDir;
	return this;
}

ACFDetector::Builder* ACFDetector::Builder::modelDs(Size size){
	this->_modelDs = size;
	return this;
}

ACFDetector::Builder* ACFDetector::Builder::modelDsPad(cv::Size size){
	this->_modelDsPad = size;
	return this;
}

ACFDetector::Builder* ACFDetector::Builder::stride(int s){
	this->_stride = s;
	return this;
}

ACFDetector::Builder* ACFDetector::Builder::Classifier(Clf* c){
	this->_clf = c;
	return this;
}
/**
 * Builder constructor to get a object
 */
ACFDetector ACFDetector:: Builder::build(){
	OUT("Build ACF Detector From Builder");
	return ACFDetector(this);
}

/**
 *
 */
void detectOneScale(ACFDetector* detector,std::vector<BoundingBox>& bbs,vector<Mat*> chns){
	//  // get inputs
//	  float *chns;// = (float*) mxGetData(prhs[0]);
//	//  mxArray *trees = (mxArray*) prhs[1];
//	//  const int shrink = (int) mxGetScalar(prhs[2]);
//
//	const int modelHt = detector->_modelDsPad.height;
//	const int modelWd = detector->_modelDsPad.width;
//	const int stride = detector->_stride;
//	const float cascThr = detector->_cascThr;
//
//	//  // extract relevant fields from trees
//	float *thrs =  detector->_clf->thrs;
//	float *hs =  detector->_clf->hs;
//	uint32 *fids = detector->_clf->fids;
//	uint32 *child = detector->_clf->child;
//	const int treeDepth = detector->_clf->treeDepth;
//	const int nTreeNodes = (int) detector->_clf->nTreeNodes;
//	const int nTrees = (int) detector->_clf->nTrees;
//
//	const int height = chns[0]->rows;
//	const int width = chns[0]->cols;
//
//	// get dimensions and constants
//	const mwSize *chnsSize = mxGetDimensions(prhs[0]);
//
//	const int nChns = mxGetNumberOfDimensions(prhs[0])<=2 ? 1 : (int) chnsSize[2];
//	const mwSize *fidsSize = mxGetDimensions(mxGetField(trees,0,"fids"));
//
//	const int height1 = (int) ceil(float(height*shrink-modelHt+1)/stride);
//	const int width1 = (int) ceil(float(width*shrink-modelWd+1)/stride);
//
//	// construct cids array
//	int nFtrs = modelHt/shrink*modelWd/shrink*nChns;
//	uint32 *cids = new uint32[nFtrs]; int m=0;
//	for( int z=0; z<nChns; z++ )
//	for( int c=0; c<modelWd/shrink; c++ )
//	  for( int r=0; r<modelHt/shrink; r++ )
//		cids[m++] = z*width*height + c*height + r;
//
//	// apply classifier to each patch
//	vector<int> rs, cs; vector<float> hs1;
//	for( int c=0; c<width1; c++ ) for( int r=0; r<height1; r++ ) {
//	float h=0, *chns1=chns+(r*stride/shrink) + (c*stride/shrink)*height;
//	if( treeDepth==1 ) {
//	  // specialized case for treeDepth==1
//	  for( int t = 0; t < nTrees; t++ ) {
//		uint32 offset=t*nTreeNodes, k=offset, k0=0;
//		getChild(chns1,cids,fids,thrs,offset,k0,k);
//		h += hs[k]; if( h<=cascThr ) break;
//	  }
//	} else if( treeDepth==2 ) {
//	  // specialized case for treeDepth==2
//	  for( int t = 0; t < nTrees; t++ ) {
//		uint32 offset=t*nTreeNodes, k=offset, k0=0;
//		getChild(chns1,cids,fids,thrs,offset,k0,k);
//		getChild(chns1,cids,fids,thrs,offset,k0,k);
//		h += hs[k]; if( h<=cascThr ) break;
//	  }
//	} else if( treeDepth>2) {
//	  // specialized case for treeDepth>2
//	  for( int t = 0; t < nTrees; t++ ) {
//		uint32 offset=t*nTreeNodes, k=offset, k0=0;
//		for( int i=0; i<treeDepth; i++ )
//		  getChild(chns1,cids,fids,thrs,offset,k0,k);
//		h += hs[k]; if( h<=cascThr ) break;
//	  }
//	} else {
//	  // general case (variable tree depth)
//	  for( int t = 0; t < nTrees; t++ ) {
//		uint32 offset=t*nTreeNodes, k=offset, k0=k;
//		while( child[k] ) {
//		  float ftr = chns1[cids[fids[k]]];
//		  k = (ftr<thrs[k]) ? 1 : 0;
//		  k0 = k = child[k0]-k+offset;
//		}
//		h += hs[k]; if( h<=cascThr ) break;
//	  }
//	}
//	if(h>cascThr) { cs.push_back(c); rs.push_back(r); hs1.push_back(h); }
//	}
//	delete [] cids; m=cs.size();
//
//	// convert to bbs
//	plhs[0] = mxCreateNumericMatrix(m,5,mxDOUBLE_CLASS,mxREAL);
//	double *bbs = (double*) mxGetData(plhs[0]);
//	for( int i=0; i<m; i++ ) {
//	bbs[i+0*m]=cs[i]*stride; bbs[i+2*m]=modelWd;
//	bbs[i+1*m]=rs[i]*stride; bbs[i+3*m]=modelHt;
//	bbs[i+4*m]=hs1[i];
//	}
}

void ACFDetector::detectImg(std::vector<BoundingBox>& bbs,Mat image){
	OUT("detect image");

}



void ACFDetector::train(){
	OUT("train a detector here ");
}

void ACFDetector::test(){
	OUT("test module here");
}

inline void getChild( float *chns1, uint32 *cids, uint32 *fids,
  float *thrs, uint32 offset, uint32 &k0, uint32 &k ){
  float ftr = chns1[cids[fids[k]]];
  k = (ftr<thrs[k]) ? 1 : 2;
  k0=k+=k0*2; k+=offset;
}



