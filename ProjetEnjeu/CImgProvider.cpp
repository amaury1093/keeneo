#include "PrecHeaders.h"
#include "CImgProvider.h"

bool CImgProvider::open(const std::string& videoName, unsigned int maxTries /*= 4*/)
{
	bool failedLoad = true;
	unsigned int cpt = 0;
	while (failedLoad && cpt <= maxTries)
	{
		failedLoad = !m_reader.open(videoName);
		cpt++;
	}
	if (failedLoad)
	{
		std::cerr << "failed to open file " << videoName << " after " << maxTries << " tries" << std::endl;
		return false;
	}
	return true;
}

std::string CImgProvider::getCodec()
{
	std::string res;
	if (m_reader.isOpened())
	{
		int codec = m_reader.get(CV_CAP_PROP_FOURCC);
		char code[5]; code[4] = 0; memcpy(&code,&codec,4);
		res = std::string(code);
	}
	return res;
}

bool CImgProvider::getNext(cv::Mat& out)
{
	if (m_reader.isOpened())
	{
		m_reader >> out;
	}
	return !out.empty();
}

