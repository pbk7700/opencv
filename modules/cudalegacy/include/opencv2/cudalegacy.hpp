/*M///////////////////////////////////////////////////////////////////////////////////////
//
//  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
//  By downloading, copying, installing or using the software you agree to this license.
//  If you do not agree to this license, do not download, install,
//  copy or use the software.
//
//
//                           License Agreement
//                For Open Source Computer Vision Library
//
// Copyright (C) 2000-2008, Intel Corporation, all rights reserved.
// Copyright (C) 2009, Willow Garage Inc., all rights reserved.
// Third party copyrights are property of their respective owners.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistribution's of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//
//   * Redistribution's in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
//   * The name of the copyright holders may not be used to endorse or promote products
//     derived from this software without specific prior written permission.
//
// This software is provided by the copyright holders and contributors "as is" and
// any express or implied warranties, including, but not limited to, the implied
// warranties of merchantability and fitness for a particular purpose are disclaimed.
// In no event shall the Intel Corporation or contributors be liable for any direct,
// indirect, incidental, special, exemplary, or consequential damages
// (including, but not limited to, procurement of substitute goods or services;
// loss of use, data, or profits; or business interruption) however caused
// and on any theory of liability, whether in contract, strict liability,
// or tort (including negligence or otherwise) arising in any way out of
// the use of this software, even if advised of the possibility of such damage.
//
//M*/

#ifndef __OPENCV_CUDALEGACY_HPP__
#define __OPENCV_CUDALEGACY_HPP__

#include "opencv2/core/cuda.hpp"
#include "opencv2/cudalegacy/NCV.hpp"
#include "opencv2/cudalegacy/NPP_staging.hpp"
#include "opencv2/cudalegacy/NCVPyramid.hpp"
#include "opencv2/cudalegacy/NCVHaarObjectDetection.hpp"
#include "opencv2/cudalegacy/NCVBroxOpticalFlow.hpp"
#include "opencv2/video/background_segm.hpp"

/**
  @addtogroup cuda
  @{
    @defgroup cudalegacy Legacy support
  @}
*/

namespace cv { namespace cuda {

//! @addtogroup cudalegacy
//! @{

class CV_EXPORTS ImagePyramid : public Algorithm
{
public:
    virtual void getLayer(OutputArray outImg, Size outRoi, Stream& stream = Stream::Null()) const = 0;
};

CV_EXPORTS Ptr<ImagePyramid> createImagePyramid(InputArray img, int nLayers = -1, Stream& stream = Stream::Null());

////////////////////////////////////////////////////
// GMG

/** @brief Background/Foreground Segmentation Algorithm.

The class discriminates between foreground and background pixels by building and maintaining a model
of the background. Any pixel which does not fit this model is then deemed to be foreground. The
class implements algorithm described in @cite Gold2012 .
 */
class CV_EXPORTS BackgroundSubtractorGMG : public cv::BackgroundSubtractor
{
public:
    using cv::BackgroundSubtractor::apply;
    virtual void apply(InputArray image, OutputArray fgmask, double learningRate, Stream& stream) = 0;

    virtual int getMaxFeatures() const = 0;
    virtual void setMaxFeatures(int maxFeatures) = 0;

    virtual double getDefaultLearningRate() const = 0;
    virtual void setDefaultLearningRate(double lr) = 0;

    virtual int getNumFrames() const = 0;
    virtual void setNumFrames(int nframes) = 0;

    virtual int getQuantizationLevels() const = 0;
    virtual void setQuantizationLevels(int nlevels) = 0;

    virtual double getBackgroundPrior() const = 0;
    virtual void setBackgroundPrior(double bgprior) = 0;

    virtual int getSmoothingRadius() const = 0;
    virtual void setSmoothingRadius(int radius) = 0;

    virtual double getDecisionThreshold() const = 0;
    virtual void setDecisionThreshold(double thresh) = 0;

    virtual bool getUpdateBackgroundModel() const = 0;
    virtual void setUpdateBackgroundModel(bool update) = 0;

    virtual double getMinVal() const = 0;
    virtual void setMinVal(double val) = 0;

    virtual double getMaxVal() const = 0;
    virtual void setMaxVal(double val) = 0;
};

/** @brief Creates GMG Background Subtractor

@param initializationFrames Number of frames of video to use to initialize histograms.
@param decisionThreshold Value above which pixel is determined to be FG.
 */
CV_EXPORTS Ptr<cuda::BackgroundSubtractorGMG>
    createBackgroundSubtractorGMG(int initializationFrames = 120, double decisionThreshold = 0.8);

////////////////////////////////////////////////////
// FGD

/** @brief The class discriminates between foreground and background pixels by building and maintaining a model
of the background.

Any pixel which does not fit this model is then deemed to be foreground. The class implements
algorithm described in @cite FGD2003 .
@sa BackgroundSubtractor
 */
class CV_EXPORTS BackgroundSubtractorFGD : public cv::BackgroundSubtractor
{
public:
    /** @brief Returns the output foreground regions calculated by findContours.

    @param foreground_regions Output array (CPU memory).
     */
    virtual void getForegroundRegions(OutputArrayOfArrays foreground_regions) = 0;
};

struct CV_EXPORTS FGDParams
{
    int Lc;  //!< Quantized levels per 'color' component. Power of two, typically 32, 64 or 128.
    int N1c; //!< Number of color vectors used to model normal background color variation at a given pixel.
    int N2c; //!< Number of color vectors retained at given pixel.  Must be > N1c, typically ~ 5/3 of N1c.
    //!< Used to allow the first N1c vectors to adapt over time to changing background.

    int Lcc;  //!< Quantized levels per 'color co-occurrence' component.  Power of two, typically 16, 32 or 64.
    int N1cc; //!< Number of color co-occurrence vectors used to model normal background color variation at a given pixel.
    int N2cc; //!< Number of color co-occurrence vectors retained at given pixel.  Must be > N1cc, typically ~ 5/3 of N1cc.
    //!< Used to allow the first N1cc vectors to adapt over time to changing background.

    bool is_obj_without_holes; //!< If TRUE we ignore holes within foreground blobs. Defaults to TRUE.
    int perform_morphing;     //!< Number of erode-dilate-erode foreground-blob cleanup iterations.
    //!< These erase one-pixel junk blobs and merge almost-touching blobs. Default value is 1.

    float alpha1; //!< How quickly we forget old background pixel values seen. Typically set to 0.1.
    float alpha2; //!< "Controls speed of feature learning". Depends on T. Typical value circa 0.005.
    float alpha3; //!< Alternate to alpha2, used (e.g.) for quicker initial convergence. Typical value 0.1.

    float delta;   //!< Affects color and color co-occurrence quantization, typically set to 2.
    float T;       //!< A percentage value which determines when new features can be recognized as new background. (Typically 0.9).
    float minArea; //!< Discard foreground blobs whose bounding box is smaller than this threshold.

    //! default Params
    FGDParams();
};

/** @brief Creates FGD Background Subtractor

@param params Algorithm's parameters. See @cite FGD2003 for explanation.
 */
CV_EXPORTS Ptr<cuda::BackgroundSubtractorFGD>
    createBackgroundSubtractorFGD(const FGDParams& params = FGDParams());

//! @}

}}

#endif /* __OPENCV_CUDALEGACY_HPP__ */
