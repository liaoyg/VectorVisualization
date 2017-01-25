/*
* Copyright (c) 2008  Martin Falk <falk@visus.uni-stuttgart.de>
*                     Visualization Research Center (VISUS),
*                     Universität Stuttgart, Germany
*
* This source code is distributed as part of the publication
* "Output-Sensitive 3D Line Integral Convolution".
* Sample images and movies of this application can be found
* at http://www.vis.uni-stuttgart.de/texflowvis .
* This file may be distributed, modified, and used free of charge
* as long as this copyright notice is included in its original
* form. Commercial use is strictly prohibited. However we request
* that acknowledgement is given to the following article
*
*     M. Falk, D. Weiskopf.
*     Output-Sensitive 3D Line Integral Convolution,
*     IEEE Transactions on Visualization and Computer Graphics, 2008.
*
* Filename: imageUtils.h
*
* $Id: imageUtils.h,v 1.2 2008/05/09 08:23:23 falkmn Exp $
*/
#ifndef _IMAGEUTILS_H_
#define _IMAGEUTILS_H_


struct Image
{
	Image(void)
	{
		imgData = NULL;
		width = 0;
		height = 0;
		channel = 1;
		gamma = 1.0;
		maxVal = 255;
	}

	// data is allocated with new when reading images
	// data is not freed implicitely!
	unsigned char *imgData;
	int width;
	int height;

	// only used for png
	int channel;
	double gamma;

	// only used for ppm
	int maxVal;
};

bool pngRead(const char *fileName, Image *img);
bool pngWrite(const char *fileName, const Image *img, bool invert = false);

bool ppmRead(const char *filename, Image *img);
bool ppmWrite(const char *filename, const Image *img);


#endif // _IMAGEUTILS_H_
