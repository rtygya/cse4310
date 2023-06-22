
// include necessary dependencies
#include <iostream>
#include <cstdio>
#include "opencv2/opencv.hpp"

// configuration parameters
#define NUM_COMNMAND_LINE_ARGUMENTS 1
#define DISPLAY_WINDOW_NAME "Image"

class GraphicsEditor {
	public:
		cv::Mat imageOrig;
		static int mode;
	
	public:
		GraphicsEditor(char *file)
		{
			imageOrig = cv::imread(file, cv::IMREAD_COLOR);
		
			// check for file error
			if(!imageOrig.data)
			{
				std::cout << "Error while opening file " << file << std::endl;
			}
			else
			{
				cv::imshow(DISPLAY_WINDOW_NAME, imageOrig);
				cv::setMouseCallback(DISPLAY_WINDOW_NAME, clickCallback, &imageOrig);
				cv::waitKey();
			}
		}

		static void clickCallback(int event, int x, int y, int flags, void* param)
		{
			// cast userdata to a cv::Mat
			cv::Mat imageIn = *(cv::Mat *)param;
			int currentMode = mode;
			// handle the mouse event types
			if(event == cv::EVENT_RBUTTONDOWN)
			{
				std::cout << "RIGHT CLICK (" << x << ", " << y << ")" << std::endl;
				if (mode == 5)
				{
					mode = 1;
				}
				else
				{
					mode++;
				}

				switch (mode)
				{
					case 1:
						std::cout << "Eyedropper activated" << std::endl;
						break;
					case 2:
						std::cout << "Crop activated" << std::endl;
						break;
					case 3:
						std::cout << "Pencil activated" << std::endl;
						break;
					case 4:
						std::cout << "Paint bucket activated" << std::endl;
						break;
					case 5:
						std::cout << "Reset activated" << std::endl;
						break;
					default:
						std::cout << "Right click to toggle" << std::endl;
				}
			}
		}
};

int main(int argc, char **argv)
{
    // validate and parse the command line arguments
    if(argc != NUM_COMNMAND_LINE_ARGUMENTS + 1)
    {
        std::printf("USAGE: %s <image_path> \n", argv[0]);
        return 0;
    }
    else
    {
		GraphicsEditor g(argv[1]);
		
    }
}