/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkGLMEstimator.cxx,v $
  Date:      $Date: 2006/01/31 17:47:59 $
  Version:   $Revision: 1.9 $

=========================================================================auto=*/

#include "GeneralLinearModel.h"
#include "vtkGLMEstimator.h"
#include "vtkObjectFactory.h"
#include "vtkImageData.h"
#include "vtkPointData.h"
#include "vtkCommand.h"
#include "vtkImageFFT.h"
#include "vtkImageRFFT.h"
#include "vtkImageIdealHighPass.h"
#include "vtkImageAppend.h"
#include "vtkImageAccumulate.h"
#include "vtkImageExtractComponents.h"
#include "vtkExtractVOI.h"
#include "vtkImageViewer.h"
#include "vtkGLMDetector.h"

//add by qinshuo 9-16
#include "vtkInformationVector.h"
#include "vtkInformation.h"
#include <iostream>


vtkStandardNewMacro(vtkGLMEstimator);

vtkGLMEstimator::vtkGLMEstimator()
{
    this->Cutoff = 0.0;
    this->LowerThreshold = 0.0;
    this->HighPassFiltering = 0;
    this->GrandMean = 0.0;
    this->GlobalEffect = 0;

    this->GlobalMeans = NULL;
    this->Detector = NULL; 
    this->TimeCourse = NULL; 
    this->RegionTimeCourse = NULL;
    this->RegionVoxels = NULL;

	 this->Information->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkImageData" );
    this->Information->Set(vtkAlgorithm::INPUT_IS_REPEATABLE(),0);

}


vtkGLMEstimator::~vtkGLMEstimator()
{
    if (this->TimeCourse != NULL)
    {
        this->TimeCourse->Delete();
    }
    if (this->RegionTimeCourse != NULL)
    {
        this->RegionTimeCourse->Delete();
    }
    if (this->RegionVoxels != NULL)
    {
        this->RegionVoxels->Delete();
    }
}


vtkFloatArray *vtkGLMEstimator::GetRegionTimeCourse(vtkInformationVector **inputVector)
{
    int numberOfInputs;
#if (VTK_MAJOR_VERSION >= 5)
    numberOfInputs = this->GetNumberOfInputConnections(0);
#else
    numberOfInputs = this->NumberOfInputs;
#endif
    // Checks the input list
    if (numberOfInputs == 0 || Getinputx(0,inputVector) == NULL)
    {
        vtkErrorMacro( <<"No input image data in this filter.");
        return NULL;
    }

    if (this->RegionVoxels == NULL)
    {
        vtkErrorMacro( <<"Indices of all voxels in the ROI is required.");
        return NULL;
    }

    if (this->RegionTimeCourse != NULL) 
    {
        this->RegionTimeCourse->Delete();
    }
    this->RegionTimeCourse = vtkFloatArray::New();
    this->RegionTimeCourse->SetNumberOfTuples(numberOfInputs);
    this->RegionTimeCourse->SetNumberOfComponents(1);

    //short *val; ---qinshuo 2014.12.2
	float *val;
    int size = this->RegionVoxels->GetNumberOfTuples();

    for (int ii = 0; ii < numberOfInputs; ii++)
    {
        float total = 0;
        for (int jj = 0; jj < size; jj++)
        {
            short x = (short)this->RegionVoxels->GetComponent(jj, 0);
            short y = (short)this->RegionVoxels->GetComponent(jj, 1);
            short z = (short)this->RegionVoxels->GetComponent(jj, 2);

            val = (float *)Getinputx(ii,inputVector)->GetScalarPointer(x, y, z); 
            total += *val;
        }

        this->RegionTimeCourse->SetComponent(ii, 0, (float)(total/size)); 
    }

    return this->RegionTimeCourse;
}


vtkFloatArray *vtkGLMEstimator::GetTimeCourse(int i, int j, int k,vtkInformationVector **inputVector)
{
    int numberOfInputs;
#if (VTK_MAJOR_VERSION >= 5)
    numberOfInputs = this->GetNumberOfInputConnections(0);
#else
    numberOfInputs = this->NumberOfInputs;
#endif
    // Checks the input list
    if (numberOfInputs == 0 || Getinputx(0,inputVector) == NULL)
    {
        vtkErrorMacro( <<"No input image data in this filter.");
        return NULL;
    }

    if (this->TimeCourse != NULL) 
    {
        this->TimeCourse->Delete();
    }
    this->TimeCourse = vtkFloatArray::New();
    this->TimeCourse->SetNumberOfTuples(numberOfInputs);
    this->TimeCourse->SetNumberOfComponents(1);

    //short----by qinshuo 2014.12.2
	float *val;
    for (int ii = 0; ii < numberOfInputs; ii++)
    {
		//short----by qinshuo 2014.12.2
        val = (float *)Getinputx(ii,inputVector)->GetScalarPointer(i, j, k); 
        this->TimeCourse->SetComponent(ii, 0, *val); 
    }

    // Execute high-pass filter on the timecourse
    if (this->HighPassFiltering) 
    {
        this->PerformHighPassFiltering();
    }

    return this->TimeCourse;

}


void vtkGLMEstimator::PerformHighPassFiltering()
{
    int numberOfInputs;
#if (VTK_MAJOR_VERSION >= 5)
    numberOfInputs = this->GetNumberOfInputConnections(0);
#else
    numberOfInputs = this->NumberOfInputs;
#endif
    // We are going to perform high pass filtering on the time course 
    // of a specific voxel. First, we convert the time course from
    // a vtkFloatArray to vtkImageData.
    vtkImageData *img = vtkImageData::New();
	img->GetPointData()->SetScalars(this->TimeCourse);
    img->SetDimensions(numberOfInputs, 1, 1);
    img->SetSpacing(1.0, 1.0, 1.0);
    img->SetOrigin(0.0, 0.0, 0.0);
#if VTK_MAJOR_VERSION <=5 
	img->SetScalarType(VTK_FLOAT);
#else
#endif

    // FFT on the vtkImageData
    vtkImageFFT *fft = vtkImageFFT::New();
#if VTK_MAJOR_VERSION <= 5
    fft->SetInput(img); 
#else
	fft->SetInputData(img);
#endif

    // Cut frequency on the vtkImageData on frequence domain
    vtkImageIdealHighPass *highPass = vtkImageIdealHighPass::New();
#if VTK_MAJOR_VERSION <= 5
    highPass->SetInput(fft->GetOutput());
#else
	highPass->SetInputData(fft->GetOutput());
#endif
    highPass->SetXCutOff(this->Cutoff);         
    highPass->SetYCutOff(this->Cutoff); 
    highPass->ReleaseDataFlagOff();

    // RFFT on the vtkImageData following frequency cutoff
    vtkImageRFFT *rfft = vtkImageRFFT::New();
#if VTK_MAJOR_VERSION <= 5
    rfft->SetInput(highPass->GetOutput());
#else
	rfft->SetInputData(highPass->GetOutput());
#endif 
    // The vtkImageData now holds two components: real and imaginary.
    // The real component is the image (time course) we want to plot
    vtkImageExtractComponents *real = vtkImageExtractComponents::New();
#if VTK_MAJOR_VERSION <= 5
	real->SetInput(rfft->GetOutput());
#else
	real->SetInputData(rfft->GetOutput());
#endif 
    real->SetComponents(0);
    real->Update();

    // Update the vtkFloatArray of the time course
    vtkDataArray *arr = real->GetOutput()->GetPointData()->GetScalars();
    for (int i = 0; i < numberOfInputs; i++) 
    {
        float x = (float) arr->GetComponent(i, 0);
        this->TimeCourse->SetComponent(i, 0, x);
    }

    // Clean up
    highPass->Delete();
    real->Delete();
    rfft->Delete();
    fft->Delete();
    img->Delete();
}


void vtkGLMEstimator::ComputeMeans(vtkInformationVector **inputVector)
{
    int numberOfInputs;
#if (VTK_MAJOR_VERSION >= 5)
    numberOfInputs = this->GetNumberOfInputConnections(0);
#else
    numberOfInputs = this->NumberOfInputs;
#endif
    if (this->GlobalMeans != NULL)
    {
        delete [] this->GlobalMeans;
    }
    this->GlobalMeans = new float [numberOfInputs];

    // this class is for single volume stats; here we use
    // it to get voxel intensity mean for the entire volume.
    vtkImageAccumulate *ia = vtkImageAccumulate::New();

    // all voxels in the same bin
    ia->SetComponentExtent(0, 0, 0, 0, 0, 0);

    ia->SetComponentOrigin(0.0, 0.0, 0.0);
    ia->SetComponentSpacing(1.0, 1.0, 1.0);

    int imgDim[3];  
    Getinputx(0,inputVector)->GetDimensions(imgDim);
    int dim = imgDim[0] * imgDim[1] * imgDim[2];
    float gt = 0.0;
    
    // for progress update (bar)
    unsigned long count = 0;
    unsigned long target = (unsigned long)(numberOfInputs * dim / 100.0);
    target++;

    for (int i = 0; i < numberOfInputs; i++)
    {
        // get original mean for each volume
#if (VTK_MAJOR_VERSION <= 5)
		ia->SetInput(Getinputx(i,inputVector));
#else
		ia->SetInputData(Getinputx(i, inputVector));
#endif
        ia->Update();
        double *means = ia->GetMean();
        // 4.0 is arbitray. I did experiment on 1, 2, 4, 6, 8 and 10.
        // 4 seems the best with my 45-volume test data set.
        this->GlobalMeans[i] = (float) (means[0] / 4.0);

        // get new mean for each volume
        double total = 0.0;
        //short---by qinshuo 2014.12.2
		float *ptr = (float *) Getinputx(i,inputVector)->GetScalarPointer();
//        short *ptr = (short *) this->GetInput(i)->GetScalarPointer();
		int count2 = 0;
        for (int ii = 0; ii < dim; ii++)
        {
            if (ptr[ii] >= this->GlobalMeans[i])
            {
                total += ptr[ii];
                count2++;
            }

            // status bar update
            if (!(count%target))
            {
                UpdateProgress(count / (100.0*target));
            }
            count++;
        }
        this->GlobalMeans[i] = (float) (total / count2);
        gt += this->GlobalMeans[i];
    }
    ia->Delete();

    // grand mean
    this->GrandMean = gt / numberOfInputs;
}
 

void vtkGLMEstimator::SimpleExecute(vtkImageData *inputs, vtkImageData* output)
{
	std::cout<<"testing SimpleExecute()"<<std::endl;
    std::cout<<"input connections"<<this->GetNumberOfInputConnections(0)<<std::endl;
	std::cout<<"input port"<<this->GetNumberOfInputPorts()<<std::endl;
}	


/*	int numberOfInputs;
#if (VTK_MAJOR_VERSION >= 5)
    numberOfInputs = this->GetNumberOfInputConnections(0);
#else
    numberOfInputs = this->NumberOfInputs;
#endif
    if (numberOfInputs == 0 || this->GetInput(0) == NULL)
    {
        vtkErrorMacro( << "No input image data in this filter.");
        return;
    }

    // compute global means and grand mean for the sequence
    if (this->GlobalEffect > 0)
    {
        ComputeMeans();
    }
	std::cout<<"vtkGLMEstimator testing simplexecute()"<<std::endl;
    // Sets up properties for output vtkImageData
    int noOfRegressors = ((vtkGLMDetector *)this->Detector)->GetDesignMatrix()->GetNumberOfComponents();
    int imgDim[3];  
    int vox;
        
    this->GetInput(0)->GetDimensions(imgDim);
    output->SetScalarType(VTK_FLOAT);
    output->SetOrigin(this->GetInput(0)->GetOrigin());
    output->SetSpacing(this->GetInput(0)->GetSpacing());
    // The scalar components hold the following:
    // for each regressor: beta value
    // plus chisq (the sum of squares of the residuals from the best-fit)
    // plus the correlation coefficient at lag 1 generated from error modeling.
    // plus one % signal change for each regressors
    output->SetNumberOfScalarComponents(noOfRegressors * 2 + 2);
    output->SetDimensions(imgDim[0], imgDim[1], imgDim[2]);
    output->AllocateScalars();
   
    // Array holding time course of a voxel
    vtkFloatArray *tc = vtkFloatArray::New();
    tc->SetNumberOfTuples(numberOfInputs);
    tc->SetNumberOfComponents(1);

    // for progress update (bar)
    unsigned long count = 0;
    unsigned long target = (unsigned long)(imgDim[0]*imgDim[1]*imgDim[2] / 100.0);
    target++;

    // Use memory allocation for MS Windows VC++ compiler.
    // beta[noOfRegressors] is not allowed by MS Windows VC++ compiler.
    float *beta = new float [noOfRegressors];

    // array of % signal change; one for each beta
    float *pSigChanges = new float [noOfRegressors];

    vox = 0;
    vtkDataArray *scalarsInOutput = output->GetPointData()->GetScalars();
    // Voxel iteration through the entire image volume
    for (int kk = 0; kk < imgDim[2]; kk++)
    {
        for (int jj = 0; jj < imgDim[1]; jj++)
        {
            for (int ii = 0; ii < imgDim[0]; ii++)
            {
                // Gets time course for this voxel
                float total = 0.0;
                float scaledTotal = 0.0;
                for (int i = 0; i < numberOfInputs; i++)
                {
                    short *value 
                        = (short *)this->GetInput(i)->GetScalarPointer(ii, jj, kk);

                    // time course is scaled by user option
                    float scale = 1.0;
                    if (this->GlobalEffect == 1)
                    {
                        scale = 100.0 / this->GrandMean;
                    }
                    else if (this->GlobalEffect == 2)
                    {
                        scale = 100.0 / this->GlobalMeans[i];
                    }
                    else if (this->GlobalEffect == 3)
                    {
                        scale = (100.0 / this->GlobalMeans[i]) * (100.0 / this->GrandMean);
                    }

                    float v = scale * (*value);
                    scaledTotal += v;
                    tc->SetComponent(i, 0, v);
                    total += *value;
                }   

                float chisq, p;
                if ((total/numberOfInputs) > this->LowerThreshold)
                {
                    // first pass parameter estimates without modeling autocorrelation structure.
                    ((vtkGLMDetector *)this->Detector)->DisableAR1Modeling ( );
                    ((vtkGLMDetector *)this->Detector)->FitModel( tc, beta, &chisq );
                    // for testing
                    p = 0.0;
                    if ( 0 ) {
                        ((vtkGLMDetector *)this->Detector)->ComputeResiduals ( tc, beta );
                        // second pass parameter estimates, with whitened temporal autocorrelation.
                        p = ((vtkGLMDetector *)this->Detector)->ComputeCorrelationCoefficient ( );
                        ((vtkGLMDetector *)this->Detector)->PreWhitenDataAndResiduals ( tc, p );
                        ((vtkGLMDetector *)this->Detector)->EnableAR1Modeling ( );
                        ((vtkGLMDetector *)this->Detector)->FitModel( tc, beta, &chisq );
                    }
                     // now have all we need to compute inferences

                    // compute % signal changes for all betas
                    float mean = scaledTotal / numberOfInputs;
                    for (int dd = 0; dd < noOfRegressors; dd++)
                    {
                        pSigChanges[dd] = 100 * beta[dd] / mean;
                    }
                }
                else
                {
                    for (int dd = 0; dd < noOfRegressors; dd++)
                    {
                        beta[dd] = 0.0;
                        pSigChanges[dd] = 0.0;
                        chisq = p = 0.0;
                    }
                }
       
                // put values into output volume
                int yy = 0;
                // betas
                for (int dd = 0; dd < noOfRegressors; dd++)
                {
                    scalarsInOutput->SetComponent(vox, yy++, beta[dd]);
                }
                // chisq and p
                scalarsInOutput->SetComponent(vox, yy++, chisq);
                scalarsInOutput->SetComponent(vox, yy++, p);
                // % signal changes
                for (int dd = 0; dd < noOfRegressors; dd++)
                {
                    scalarsInOutput->SetComponent(vox, yy++, pSigChanges[dd]);
                }

                vox++;

                // progress bar update
                if (!(count%target))
                {
                    UpdateProgress(count / (100.0*target));
                }
                count++;
            }
        } 
    }

    delete [] beta;
    delete [] pSigChanges;

    GeneralLinearModel::Free();
    tc->Delete();
}
*/


//add by qinshuo--20140918
//multiple input
int vtkGLMEstimator::FillInputPortInformation( int port, vtkInformation* info )
{
  if(port == 0)
  {
    info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkImageData" );
    info->Set(vtkAlgorithm::INPUT_IS_REPEATABLE(), 1);
    return 1;
  }
 
  vtkErrorMacro("This filter does not have more than 1 input port!");
  return 0;
}
 
int vtkGLMEstimator::RequestData(vtkInformation *vtkNotUsed(request),
                                             vtkInformationVector **inputVector,
                                             vtkInformationVector *outputVector)
{
  // get the info objects
  vtkInformation *inInfo0 = inputVector[0]->GetInformationObject(0);
  vtkInformation *inInfo1 = inputVector[0]->GetInformationObject(1);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);
 
  // get the output
  vtkImageData *output = vtkImageData::SafeDownCast(
      outInfo->Get(vtkDataObject::DATA_OBJECT()));
 
  // get the input
  vtkImageData *input0 = vtkImageData::SafeDownCast(
      inInfo0->Get(vtkDataObject::DATA_OBJECT()));
 
  vtkImageData *input1 = vtkImageData::SafeDownCast(
    inInfo1->Get(vtkDataObject::DATA_OBJECT()));
 
/****************************************************/
  
	std::cout<<"testing Estimator RequestData()"<<std::endl;
    std::cout<<"number of input connections: "<<this->GetNumberOfInputConnections(0)<<std::endl;
	std::cout<<"number of input ports: "<<this->GetNumberOfInputPorts()<<std::endl;
	
	int numberOfInputs;
#if (VTK_MAJOR_VERSION >= 5)
    numberOfInputs = this->GetNumberOfInputConnections(0);
#else
    numberOfInputs = this->NumberOfInputs;
#endif						//this->GetInput(0)
    if (numberOfInputs == 0 || Getinputx(0,inputVector) == NULL)
    {
        vtkErrorMacro( << "No input image data in this filter.");
        return 0;
    }

    // compute global means and grand mean for the sequence
    if (this->GlobalEffect > 0)
    {
        ComputeMeans(inputVector);
    }

    // Sets up properties for output vtkImageData
    int noOfRegressors = ((vtkGLMDetector *)this->Detector)->GetDesignMatrix()->GetNumberOfComponents();
    int imgDim[3];  
    int vox;
        
    //this->GetInput(0)
	Getinputx(0,inputVector)->GetDimensions(imgDim);
    //output->SetOrigin(this->GetInput(0)->GetOrigin());
	output->SetOrigin(Getinputx(0,inputVector)->GetOrigin());
    //output->SetSpacing(this->GetInput(0)->GetSpacing());
	output->SetSpacing(Getinputx(0,inputVector)->GetSpacing());
    // The scalar components hold the following:
    // for each regressor: beta value
    // plus chisq (the sum of squares of the residuals from the best-fit)
    // plus the correlation coefficient at lag 1 generated from error modeling.
    // plus one % signal change for each regressors
#if VTK_MAJOR_VERSION <= 5
	output->SetScalarType(VTK_FLOAT);
	output->SetNumberOfScalarComponents(noOfRegressors * 2 + 2);
	output->SetDimensions(imgDim[0], imgDim[1], imgDim[2]);
    output->AllocateScalars();
#else
	output->SetDimensions(imgDim[0], imgDim[1], imgDim[2]);
	output->AllocateScalars(VTK_FLOAT, noOfRegressors * 2 + 2);
#endif
    // Array holding time course of a voxel
    vtkFloatArray *tc = vtkFloatArray::New();
    tc->SetNumberOfTuples(numberOfInputs);
    tc->SetNumberOfComponents(1);

    // for progress update (bar)
    unsigned long count = 0;
    unsigned long target = (unsigned long)(imgDim[0]*imgDim[1]*imgDim[2] / 100.0);
    target++;

    // Use memory allocation for MS Windows VC++ compiler.
    // beta[noOfRegressors] is not allowed by MS Windows VC++ compiler.
    float *beta = new float [noOfRegressors];

    // array of % signal change; one for each beta
    float *pSigChanges = new float [noOfRegressors];

    vox = 0;
    vtkDataArray *scalarsInOutput = output->GetPointData()->GetScalars();
    // Voxel iteration through the entire image volume
    for (int kk = 0; kk < imgDim[2]; kk++)
    {
        for (int jj = 0; jj < imgDim[1]; jj++)
        {
            for (int ii = 0; ii < imgDim[0]; ii++)
            {
                // Gets time course for this voxel
                float total = 0.0;
                float scaledTotal = 0.0;
                for (int i = 0; i < numberOfInputs; i++)
                {
                    //short ---- by qinshuo 2014.12.2
					float *value 
                       // = (short *)this->GetInput(i)->GetScalarPointer(ii, jj, kk);
					   //short ---- by qinshuo 2014.12.2
						=(float *)Getinputx(i,inputVector)->GetScalarPointer(ii,jj,kk);
                    // time course is scaled by user option
                    float scale = 1.0;
                    if (this->GlobalEffect == 1)
                    {
                        scale = 100.0 / this->GrandMean;
                    }
                    else if (this->GlobalEffect == 2)
                    {
                        scale = 100.0 / this->GlobalMeans[i];
                    }
                    else if (this->GlobalEffect == 3)
                    {
                        scale = (100.0 / this->GlobalMeans[i]) * (100.0 / this->GrandMean);
                    }

                    float v = scale * (*value);
                    scaledTotal += v;
                    tc->SetComponent(i, 0, v);
                    total += *value;
                }   

                float chisq, p;
                if ((total/numberOfInputs) > this->LowerThreshold)
                {
                    // first pass parameter estimates without modeling autocorrelation structure.
                    ((vtkGLMDetector *)this->Detector)->DisableAR1Modeling ( );
                    ((vtkGLMDetector *)this->Detector)->FitModel( tc, beta, &chisq );
                    // for testing
                    p = 0.0;
                    if ( 0 ) {
                        ((vtkGLMDetector *)this->Detector)->ComputeResiduals ( tc, beta );
                        // second pass parameter estimates, with whitened temporal autocorrelation.
                        p = ((vtkGLMDetector *)this->Detector)->ComputeCorrelationCoefficient ( );
                        ((vtkGLMDetector *)this->Detector)->PreWhitenDataAndResiduals ( tc, p );
                        ((vtkGLMDetector *)this->Detector)->EnableAR1Modeling ( );
                        ((vtkGLMDetector *)this->Detector)->FitModel( tc, beta, &chisq );
                    }
                     // now have all we need to compute inferences

                    // compute % signal changes for all betas
                    float mean = scaledTotal / numberOfInputs;
                    for (int dd = 0; dd < noOfRegressors; dd++)
                    {
                        pSigChanges[dd] = 100 * beta[dd] / mean;
                    }
                }
                else
                {
                    for (int dd = 0; dd < noOfRegressors; dd++)
                    {
                        beta[dd] = 0.0;
                        pSigChanges[dd] = 0.0;
                        chisq = p = 0.0;
                    }
                }
       
                // put values into output volume
                int yy = 0;
                // betas
                for (int dd = 0; dd < noOfRegressors; dd++)
                {
                    scalarsInOutput->SetComponent(vox, yy++, beta[dd]);
                }
                // chisq and p
                scalarsInOutput->SetComponent(vox, yy++, chisq);
                scalarsInOutput->SetComponent(vox, yy++, p);
                // % signal changes
                for (int dd = 0; dd < noOfRegressors; dd++)
                {
                    scalarsInOutput->SetComponent(vox, yy++, pSigChanges[dd]);
                }

                vox++;

                // progress bar update
                if (!(count%target))
                {
                    UpdateProgress(count / (100.0*target));
                }
                count++;
            }
        } 
    }

    delete [] beta;
    delete [] pSigChanges;

    GeneralLinearModel::Free();
    tc->Delete();


  return 1;
}



vtkImageData* vtkGLMEstimator::Getinputx(int i,vtkInformationVector **inputVector)
{
	vtkInformation *infox=inputVector[0]->GetInformationObject(i);
	return vtkImageData::SafeDownCast(
      infox->Get(vtkDataObject::DATA_OBJECT()));
}



