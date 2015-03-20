
//
#ifndef SignalModel_H
#define SignalModel_H





#include <vtkImageData.h>
#include <vtkFloatArray.h>
//header files for math function(pow,exp,cos)
#include <math.h>
#include <vector>
//header files for standart In/Output
#include <iostream>
//header files for file write
#include <fstream>


//file out
void fileout(vtkFloatArray*,char* name);

class  SignalModeling
{
public:
	SignalModeling();
	~SignalModeling();
//parameters
//functions

	//model convolove with the FRF func
	vtkFloatArray* ConvolveWithHRF();

	//fMRIModelViewGaussianDownsampleList
	//# takes a list in, subsamples it to a new length
	//# and returns the new list.
	vtkFloatArray* GaussianDownsampleList(vtkFloatArray* inputList);

	//function DCBasis
	vtkFloatArray* BuildDiscreteCosineBasis();

	//compute baseline
	vtkFloatArray* Baseline();

	//number of time points
	void SetLen(int len) {this->numTimePoints=len;}
	void SetTR(float TR) {this->TR=TR;}
	void SetOnset(vtkFloatArray* onset) {this->onset=onset;}
	void SetDuration(vtkFloatArray* duration) {this->duration=duration;}
	void SetOnset(std::vector< std::vector < int > > onset);
	void SetDuration(std::vector< std::vector < int >  > duration);
	//number of conditions
	void SetnumComponent(int num_component) {this->num_component = num_component;}

private:
	
//parameters:
	float TR;
	int numTimePoints;
	float TimeIncrement;
	vtkFloatArray* flipHRF;
	vtkFloatArray* onset;
	vtkFloatArray* duration;
	int num_component;
	int NumberOfCosineBasis;

// functions:
	//stimuli signal generator
	//wave-form: BoxCar
	vtkFloatArray* ComputeBoxCar(vtkFloatArray* onset,vtkFloatArray* duration);

	//HRF point data
	vtkFloatArray* ComputeHRF();
	
	//setup a gaussian kernal for convolution
	//invoked by GaussianDownsampleList()
	vtkFloatArray* ComputeGaussianFilter();
	
	//discrete cosine function
	//func as name
	//invoked by FindNumberOfCosineBasis
	float ComputeDefaultHighpassTemporalCutoff();
	int FindNumberOfCosineBasis();

	//rescale to range [-1.0,1.0]
	vtkFloatArray* Rescale(vtkFloatArray* Inputlist);
};

//workflow
vtkFloatArray* GenerateDesignMatrix(SignalModeling* siglm);






#endif
