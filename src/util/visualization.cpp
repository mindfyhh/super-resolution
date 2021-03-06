#include "util/visualization.h"

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

#include "image/image_data.h"

#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"

#include "glog/logging.h"

namespace super_resolution {
namespace util {
namespace {

// The size of a displayed image for the DisplayImage function if rescale is
// set to true.
constexpr int kDisplayWidthPixels = 1250;
constexpr int kDisplayHeightPixels = 850;

// Appearance variables for the selection rectangle the user can draw.
static const cv::Scalar kSelectionRectangleColor(0, 255, 255);  // yellow
constexpr int kSelectionRectangleLineThickness = 1;

// Returns a scaling factor for the image based on the size ratio of the
// display width and height values. This computes the scale such that the
// resized image will always fit within the maximum size parameters.
double GetResizeScale(const cv::Size& image_size) {
  const double scale_x =
      static_cast<double>(kDisplayWidthPixels) /
      static_cast<double>(image_size.width);
  const double scale_y =
      static_cast<double>(kDisplayHeightPixels) /
      static_cast<double>(image_size.height);
  return std::min(scale_x, scale_y);
}

// A state for the OpenCV window mouse callback that allows tracking dragging
// and rectangle position over time.
struct WindowInteractionStatus {
  WindowInteractionStatus(const cv::Mat image, const std::string window_name)
      : original_image(image), window_name(window_name) {}

  const cv::Mat original_image;
  const std::string window_name;

  int drag_start_x = 0;
  int drag_start_y = 0;
  bool dragging = false;
  bool is_zoomed_in = false;
};

// Callback function for OpenCV's window. This implements logic that allows the
// user to zoom in to sections of the image by drawing rectangles.
void DisplayWindowMouseCallback(
    const int event,
    const int x_pos,
    const int y_pos,
    const int flags,
    void *ptr) {

  WindowInteractionStatus* status =
      reinterpret_cast<WindowInteractionStatus*>(ptr);

  // If image is zoomed in and right button is pressed, zooms the image out.
  if (event == CV_EVENT_RBUTTONDOWN && status->is_zoomed_in) {
    cv::imshow(status->window_name, status->original_image);
    status->is_zoomed_in = false;
  }

  // If left button is pressed and the image isn't already zoomed in, start
  // dragging.
  if (event == CV_EVENT_LBUTTONDOWN && !status->is_zoomed_in) {
    status->drag_start_x = x_pos;
    status->drag_start_y = y_pos;
    status->dragging = true;
  }

  // Dragging was interrupted if the status is set to dragging but the left
  // button is no longer being pushed down. This can happen if the mouse goes
  // off screen. In this case, cancel the drag-to-zoom-in operation.
  if (status->dragging && !(flags & CV_EVENT_FLAG_LBUTTON)) {
    cv::imshow(status->window_name, status->original_image);
    status->dragging = false;
  }

  // If left button is released during dragging, perform the zoom.
  if (event == CV_EVENT_LBUTTONUP && status->dragging) {
    const int left_x = std::min(x_pos, status->drag_start_x);
    const int top_y = std::min(y_pos, status->drag_start_y);
    const int selection_width = std::abs(x_pos - status->drag_start_x);
    const int selection_height = std::abs(y_pos - status->drag_start_y);
    const cv::Rect selection(left_x, top_y, selection_width, selection_height);
    cv::Mat cropped_image = status->original_image(selection);
    const double scale = GetResizeScale(cropped_image.size());
    cv::resize(cropped_image, cropped_image, cv::Size(), scale, scale);
    cv::imshow(status->window_name, cropped_image);
    status->is_zoomed_in = true;
    status->dragging = false;
  } else if (status->dragging) {
    // If dragging, draw a rectangle to indicate the user's current selection.
    cv::Mat selection_image = status->original_image.clone();
    cv::rectangle(
        selection_image,
        cv::Point(status->drag_start_x, status->drag_start_y),
        cv::Point(x_pos, y_pos),
        kSelectionRectangleColor,
        kSelectionRectangleLineThickness);
    cv::imshow(status->window_name, selection_image);
  }
}

}  // namespace

void DisplayImage(
    const ImageData& image,
    const std::string& window_name,
    const bool rescale) {

  ImageData display_image = ImageData(image.GetVisualizationImage());
  if (rescale) {
    display_image.ResizeImage(GetResizeScale(display_image.GetImageSize()));
  }

  cv::namedWindow(window_name, CV_WINDOW_AUTOSIZE);
  WindowInteractionStatus status(
      display_image.GetVisualizationImage(), window_name);
  cv::setMouseCallback(window_name, DisplayWindowMouseCallback, &status);
  cv::imshow(window_name, status.original_image);
  std::cout << "Displaying image. Press any key to continue." << std::endl;
  cv::waitKey(0);
  cv::destroyWindow(window_name);
}

void DisplayImagesSideBySide(
    const std::vector<ImageData>& images,
    const std::string& window_name,
    const bool rescale) {

  CHECK_GT(images.size(), 0) << "Provide at least one image to display.";

  // Concatenate the images side-by-side.
  int width = 0;
  int height = 0;
  for (const ImageData& image : images) {
    const cv::Size image_size = image.GetImageSize();
    width += image_size.width;
    height = std::max(height, image_size.height);
  }

  const int image_type = (images[0].GetNumChannels() < 3) ? CV_8UC1 : CV_8UC3;
  cv::Mat stitched_images(height, width, image_type);
  int x_pos = 0;
  for (const ImageData& image : images) {
    const cv::Size image_size = image.GetImageSize();
    cv::Mat next_region = stitched_images(
        cv::Rect(x_pos, 0, image_size.width, image_size.height));
    image.GetVisualizationImage().copyTo(next_region);
    x_pos += image_size.width;
  }

  // Create an ImageData (force normalization since this is with unsigned
  // values) and display normally.
  ImageData stitched_image_data(stitched_images, NORMALIZE_IMAGE);
  DisplayImage(stitched_image_data, window_name, rescale);
}

}  // namespace util
}  // namespace super_resolution
