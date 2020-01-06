#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc_c.h>

#include <iostream>
#include <vector>

using namespace std;

double mx = 0, my = 0;
int dx = 0, dy = 0, horizBar_x = 0, vertiBar_y = 0;
bool clickVertiBar = false, clickHorizBar = false, needScroll = false;
CvRect rect_bar_horiz, rect_bar_verti;

void help() {
  printf("This program demonstrated the use of the cvSetMouseCallback /n"
         "for viewing large image with scroll bar in a small window/n"
         "Call:/n"
         "./Image_ScrollBar [<img_filename default im.jpg> <window_width "
         "default 1400> <window_height default "
         "700>]");
}

void mouse_callback(int event, int x, int y, int flags, void *param) {
  if (needScroll) {
    switch (event) {
    case CV_EVENT_LBUTTONDOWN:
      mx = x, my = y;
      dx = 0, dy = 0;
      if (x >= rect_bar_horiz.x &&
          x <= rect_bar_horiz.x + rect_bar_horiz.width &&
          y >= rect_bar_horiz.y &&
          y <= rect_bar_horiz.y + rect_bar_horiz.height) {
        clickHorizBar = true;
      }
      if (x >= rect_bar_verti.x &&
          x <= rect_bar_verti.x + rect_bar_verti.width &&
          y >= rect_bar_verti.y &&
          y <= rect_bar_verti.y + rect_bar_verti.height) {
        clickVertiBar = true;
      }
      break;
    case CV_EVENT_MOUSEMOVE:
      if (clickHorizBar) {
        dx = fabs(x - mx) > 1 ? (int)(x - mx) : 0;
        dy = 0;
      }
      if (clickVertiBar) {
        dx = 0;
        dy = fabs(y - my) > 1 ? (int)(y - my) : 0;
      }
      mx = x, my = y;
      break;
    case CV_EVENT_LBUTTONUP:
      mx = x, my = y;
      dx = 0, dy = 0;
      clickHorizBar = false;
      clickVertiBar = false;
      break;
    default:
      dx = 0, dy = 0;
      break;
    }
  }
}

void myShowImageScroll(char *title, IplImage *src_img, int winWidth = 1400,
                       int winHeight = 700) {
  IplImage *dst_img;
  CvRect rect_dst, rect_src;
  int imgWidth = src_img->width, imgHeight = src_img->height, barWidth = 25;
  double scale_w = (double)imgWidth / (double)winWidth,
         scale_h = (double)imgHeight / (double)winHeight;

  if (scale_w < 1)
    winWidth = imgWidth + barWidth;
  if (scale_h < 1)
    winHeight = imgHeight + barWidth;

  int showWidth = winWidth, showHeight = winHeight;
  int src_x = 0, src_y = 0;
  int horizBar_width = 0, horizBar_height = 0, vertiBar_width = 0,
      vertiBar_height = 0;

  needScroll = scale_w > 1.0 || scale_h > 1.0 ? true : false;
  if (needScroll) {
    dst_img = cvCreateImage(cvSize(winWidth, winHeight), src_img->depth,
                            src_img->nChannels);
    cvZero(dst_img);
    if (scale_w > 1.0) {
      showHeight = winHeight - barWidth;
      horizBar_width = (int)((double)winWidth / scale_w);
      horizBar_height = winHeight - showHeight;
      horizBar_x = min(max(0, horizBar_x + dx), winWidth - horizBar_width);
      rect_bar_horiz =
          cvRect(horizBar_x, showHeight + 1, horizBar_width, horizBar_height);
      cvRectangleR(dst_img, rect_bar_horiz, cvScalarAll(255), -1);
    }
    if (scale_h > 1.0) {
      showWidth = winWidth - barWidth;
      vertiBar_width = winWidth - showWidth;
      vertiBar_height = (int)((double)winHeight / scale_h);
      vertiBar_y = min(max(0, vertiBar_y + dy), winHeight - vertiBar_height);
      rect_bar_verti =
          cvRect(showWidth + 1, vertiBar_y, vertiBar_width, vertiBar_height);
      cvRectangleR(dst_img, rect_bar_verti, cvScalarAll(255), -1);
    }

    showWidth = min(showWidth, imgWidth);
    showHeight = min(showHeight, imgHeight);
    rect_dst = cvRect(0, 0, showWidth, showHeight);
    cvSetImageROI(dst_img, rect_dst);
    src_x = (int)((double)horizBar_x * scale_w);
    src_y = (int)((double)vertiBar_y * scale_h);
    src_x = min(src_x, imgWidth - showWidth);
    src_y = min(src_y, imgHeight - showHeight);
    rect_src = cvRect(src_x, src_y, showWidth, showHeight);
    cvSetImageROI(src_img, rect_src);
    cvCopy(src_img, dst_img);

    cvResetImageROI(dst_img);
    cvResetImageROI(src_img);
    cvShowImage(title, dst_img);

    cvReleaseImage(&dst_img);
  } else {
    cvShowImage(title, src_img);
  }
}

int main(int argc, char **argv) {
  help();
  const char *filename = argc > 1 ? argv[1] : "im.jpg";
  int width = 1400, height = 700;
  if (4 == argc) {
    sscanf(argv[2], "%u", &width);
    sscanf(argv[3], "%u", &height);
  }

  cvNamedWindow("Image Scroll Bar", 1);
  cvSetMouseCallback("Image Scroll Bar", mouse_callback);

  IplImage *image = cvLoadImage(filename, CV_LOAD_IMAGE_COLOR);
  if (!image) {
    fprintf(stderr,
            "Can not load %s and/or %s/n"
            "Usage: Image_ScrollBar [<img_filename default im.jpg>]/n",
            filename);
    exit(-1);
  }

  while (1) {
    myShowImageScroll("Image Scroll Bar", image, width, height);

    int KEY = cvWaitKey(10);
    if ((char)KEY == 27)
      break;
  }
  cvDestroyWindow("Image Scroll Bar");

  return 0;
}
