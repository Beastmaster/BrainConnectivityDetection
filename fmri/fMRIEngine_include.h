//head files for fMRIEngine
#include "vtkFMRIEngineConfigure.h"
#include "vtkMultipleInputsImageFilter.h"
#include "vtkImageData.h"
#include "FMRIEngineConstants.h"              
#include "vtkGLMDetector.h"
#include "GeneralLinearModel.h"
#include "vtkGLMEstimator.h"
#include "vtkActivationDetector.h"
#include "vtkGLMVolumeGenerator.h"
#include "vtkActivationEstimator.h"
#include "vtkIsingActivationThreshold.h"
#include "vtkActivationFalseDiscoveryRate.h"
#include "vtkIsingActivationTissue.h"
#include "vtkActivationRegionStats.h"
#include "vtkIsingConditionalDistribution.h"
#include "vtkActivationVolumeCaster.h"//zero out values out of range of low/high threshold
#include "vtkIsingMeanfieldApproximation.h"
#include "vtkActivationVolumeGenerator.h"
#include "vtkLabelMapWhitening.h"
#include "vtkCDF.h"            
#include "vtkMultipleInputsImageFilter.h"
#include "vtkFMRIEngineConfigure.h"
#include "vtkParzenDensityEstimation.h"
#include "SignalModel.h"
#include <iostream>

//Descirption:headfile used by ReadFile()
//Info : by qinshuo
#include <vtkImageData.h>
#include <vtkMetaImageReader.h>
#include <vtkMetaImageWriter.h>
#include <vtkXMLImageDataReader.h>
#include <vtkXMLImageDataWriter.h>
#include <vtkDICOMImageReader.h>


//headfiles for write designmatrix as test
#include <fstream>

//newly added function: get timecourses
#include "vtkGetTimecourse.h"

