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
 * Filename: parseArg.h
 * 
 * $Id: parseArg.h,v 1.2 2008/05/09 08:23:23 falkmn Exp $ 
 */
#ifndef _PARSEARG_H_
#define _PARSEARG_H_

class ParseArguments
{
public:
    ParseArguments(int argc=0, char **argv=NULL, const char *progName=NULL);
    ~ParseArguments(void);

    // print program usage on stdout
    void printUsage(void);

    void setProgramName(const char *progName);

    void setArguments(int argc, char **argv)
    {
        _argc = argc;
        _argv = argv;
    }

    const char* getVolFileName(void) { return _volFileName; }
    const char* getNoiseFileName(void) { return _noiseFileName; }
    const char* getTfFileName(void) { return _tfFileName; }
    const char* getLicFilterFileName(void) { return _licFilterFileName; }
    const char* getRedirectFileName(void) { return _redirectFile; }
    const char* getHaltonFileName(void) { return _haltonFileName; }

    const bool getGradientsFlag(void) { return _useGradients; }
    const bool getLambda2Flag(void) { return _useLambda2; }

    // parse the given command arguments
    // short arguments have the form of 
    //    -h,  -f 10
    // long arguments 
    //    --help, --filtersize=10
    //
    // return true if successful
    bool parse(void);

protected:
    // parse long arguments (leading --)
    // idx points to the argument to be parsed (argv[idx])
    // return true if successful
    bool parseLongArgs(int idx, int len);


private:
    
    char *_progName;

    int _argc;
    char **_argv;

    char *_volFileName;
    char *_noiseFileName;
    char *_tfFileName;
    char *_licFilterFileName;
    char *_redirectFile;
    char *_haltonFileName;

    bool _useGradients;
    bool _useLambda2;
};

#endif // _PARSEARG_H_
