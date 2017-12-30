#include "DepthObstacle.h"

using namespace std;
using namespace cv;

namespace Vision
{
  namespace DepthObstacle
  {
    Detect::Detect(IParameters *params, IData *input, IData *output) : IDetect(params, input, output)
    {

    }

    void Detect::Process()
    {
      MaskShadows();
      SetCenterPoint();
      SeparateRegions();
      FindMaxInRegions();
    }

    void Detect::MaskShadows()
    {
      inRange(*_input->GetDepthImage(), _params->GetMinDistance(), _params->GetMaxDistance(), _maskImage);
    }

    void Detect::SetCenterPoint()
    {
      switch (_params->GetMode())
      {
      case(Parameters::Mode::Dynamic):
        DetectHand();
        break;
      case(Parameters::Mode::Static):
        _output->SetHandPosition(_params->GetDefaultHandPosition());
        break;
      case(Parameters::Mode::Reduced):
        _output->SetHandPosition(Point(_input->GetDepthImage()->cols / 2, _input->GetDepthImage()->rows * 1 / 3));
        break;
      }
    }

    void Detect::DetectHand()
    {
      // TODO: detect position of hand
      // use blobfinder hardcoded to expected color and size range of dot on glove, and limit to lower third of frame.

      Mat handDotMask;
      inRange(*_input->GetHsvImage(), _params->GetHandDotHsvRange(0), _params->GetHandDotHsvRange(1), handDotMask);

      Ptr<SimpleBlobDetector> sbd = SimpleBlobDetector::create(_params->GetSbdParams());

      vector<KeyPoint> keyPoints;
      sbd->detect(*_input->GetDepthImage(), keyPoints);

      if (keyPoints.size() == 0)
        _output->SetHandPosition(Point(_input->GetDepthImage()->cols / 2, _input->GetDepthImage()->rows / 2));
      else
        _output->SetHandPosition(keyPoints.at(0).pt);
    }

    void Detect::SeparateRegions()
    {
      Point tl;
      Point br;

      for (int i = 0; i < VERT_REGIONS; ++i)
        for (int j = 0; j < HORZ_REGIONS; ++j)
        {
          tl.x = _output->GetHandPosition().x + (int)((i - 2.5) * _input->GetDepthImage()->cols * _params->GetCenterRegionsWidth() * _params->GetHorizontalCoverage());
          br.x = _output->GetHandPosition().x + (int)((i - 1.5) * _input->GetDepthImage()->cols * _params->GetCenterRegionsWidth() * _params->GetHorizontalCoverage());
          tl.y = _output->GetHandPosition().y + (int)((j - 1.5) * _input->GetDepthImage()->rows * _params->GetCenterRegionHeight() * _params->GetVerticalCoverage());
          br.y = _output->GetHandPosition().y + (int)((j - 0.5) * _input->GetDepthImage()->rows * _params->GetCenterRegionHeight() * _params->GetVerticalCoverage());

          if (_params->GetSnapToEdges())
          {
            if (i == 0) tl.x = (int)(_input->GetDepthImage()->cols * (1 - _params->GetHorizontalCoverage()) / 2);
            if (i == VERT_REGIONS - 1) br.x = (int)(_input->GetDepthImage()->cols * (1 + _params->GetHorizontalCoverage()) / 2);
            if (j == 0) tl.y = (int)(_input->GetDepthImage()->rows * (1 - _params->GetVerticalCoverage()) / 2);
            if (j == HORZ_REGIONS - 1) br.y = (int)(_input->GetDepthImage()->rows * (1 + _params->GetVerticalCoverage()) / 2);
          }

          _output->SetRegionBounds(i, j, Rect(tl, br) & Rect(0, 0, _input->GetDepthImage()->cols, _input->GetDepthImage()->rows));
        }
    }

    void Detect::FindMaxInRegions()
    {
      Mat hist;
      int size[] = { _params->GetHistogramBins() };
      float range[] = { _params->GetMinDistance(), _params->GetMaxDistance() };
      const float* ranges[] = { range };
      int channels[] = { 0 };

      float sum = 0;
      float total = 0;

      for (int i = 0; i < HORZ_REGIONS; ++i)
        for (int j = 0; j < VERT_REGIONS; ++j)
        {
          calcHist(&(*_input->GetDepthImage())(_output->GetRegionBounds(j, i)), 1, channels, _maskImage(_output->GetRegionBounds(j, i)), hist, 1, size, ranges, true, false);
          normalize(hist, hist, 0, hist.rows, NORM_MINMAX, -1, Mat());

          sum = (float)cv::sum(hist)[0];
          total = 0;

          for (int k = 0; k < _params->GetHistogramBins(); ++k)
          {
            total += hist.at<float>(k);

            if (total > sum * _params->GetPercentileToIgnore())
            {
              _output->SetRegionIntensity(j, i, (int)(_params->GetMinDistance() + k * (_params->GetMaxDistance() - _params->GetMinDistance()) / (float)_params->GetHistogramBins()));
              break;
            }
          }
        }
    }
  }
}