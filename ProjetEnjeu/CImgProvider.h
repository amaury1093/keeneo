#pragma once


class CImgProvider
{
public:
	CImgProvider() {}
	CImgProvider(const std::string& videoName)
	{
		open(videoName);
	}


	bool open(const std::string& videoName, unsigned int maxTries = 4);	// Tries to open a video file with a maximum number of tries in case temporary failure happens

	void close()    { m_reader.release(); }	// You should call close() if you intend to reuse CImgProvider object to open another video

	// Video images size methods
	int  getW  ()   { return m_reader.isOpened() ? static_cast<int>(m_reader.get(CV_CAP_PROP_FRAME_WIDTH)) : 0; }
	int  getH  ()   { return m_reader.isOpened() ? static_cast<int>(m_reader.get(CV_CAP_PROP_FRAME_HEIGHT)): 0; }

	std::string getCodec();	// It can be useful to adapt image processing algorithms to video codec compression that was used

	bool getNext(cv::Mat &out);	// Gets next image in video file or returns false if end of video has been reached
								// out MUST NOT be used for processing as it is highly temporary

private:
	cv::VideoCapture m_reader;
};

