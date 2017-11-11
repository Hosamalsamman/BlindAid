#pragma once

#include "DetectBase.h"
#include "CaptureBase.h"

namespace Vision
{
  namespace TrafficLight
  {
    class Parameters : public IParameters
    {
    public:
      enum Mode
      {
        BlobDetector,
        DeepLearning
      };

      Parameters() { InitBlobParams(); }

      bool Valid()
      {
        return true;
      }

      const Mode &GetMode() const { return _mode; }
      void SetMode(Mode mode) { _mode = mode; }

      float GetUpperRegionToAnalyze() { return _topRegionToAnalyze; }
      void SetLowerRegionToAnalyzer(float topRegionToAnalyzeR) { _topRegionToAnalyze = topRegionToAnalyzeR; }

      cv::Scalar GetColorRange(int color, int index) { return _colorRanges[color][index]; }
      void SetColorRange(int color, int index, cv::Scalar scalar) { _colorRanges[color][index] = scalar; }

      cv::SimpleBlobDetector::Params *GetBlobParams() { return &_blobParams; }

    private:
      void InitBlobParams()
      {
        _blobParams.filterByArea = true;
        _blobParams.minArea = 4 * 4;
        _blobParams.maxArea = 60 * 60;
        _blobParams.filterByCircularity = true;
        _blobParams.minCircularity = 0.1f;
        _blobParams.filterByConvexity = true;
        _blobParams.minConvexity = 0.8f;
        _blobParams.filterByInertia = true;
        _blobParams.minInertiaRatio = 0.5f;
        _blobParams.filterByColor = true;
        _blobParams.blobColor = 255;
      }

      // detection mode to use (CV or AI).
      Mode _mode = BlobDetector;

      // upper region to inspect for traffic lights.
      float _topRegionToAnalyze = 0.5f;

      // HSV color range for red/green/yellow masking.
      cv::Scalar _colorRanges[3][2] = {
        { cv::Scalar(150, 150, 180), cv::Scalar(10, 255, 255) },
        { cv::Scalar(40, 150, 180), cv::Scalar(80, 255, 255) },
        { cv::Scalar(10, 110, 160), cv::Scalar(40, 255, 255) } };

      // parameters class for blobdetector.
      cv::SimpleBlobDetector::Params _blobParams;
    };

    class Data : public IData
    {
    public:
      void Clear() { _results.clear(); }

      bool Valid()
      {
        return true;
      }

      std::vector<Circle>* GetRegions() { return &_results; }
      void PushBack(Circle &result) { _results.push_back(result); }
      int Size() { return (int)_results.size(); }
      Circle& At(int i) { return _results.at(i); }

    private:
      std::vector<Circle> _results;
    };

    class Base : public DetectBase<Parameters, Capture::Data, Data>
    {
    public:
      static Base *Base::MakeTrafficLight(IParameters *params, IData *input, IData *output);
      Base(IParameters *params, IData *input, IData *output);

    protected:

    };
  }
}