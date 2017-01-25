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
 * Filename: parseArg.cpp
 * 
 * $Id: parseArg.cpp,v 1.2 2008/05/09 08:23:24 falkmn Exp $ 
 */

#include <string.h>
#include <iostream>
#include "parseArg.h"


ParseArguments::ParseArguments(int argc, char **argv, const char *progName) 
    : _progName(NULL),_argc(argc),_argv(argv),
      _volFileName(NULL),
      _noiseFileName(NULL),_tfFileName(NULL),
      _licFilterFileName(NULL),_redirectFile(NULL),
      _haltonFileName(NULL),_useGradients(false),
      _useLambda2(false)
{
    setProgramName(progName);
}


ParseArguments::~ParseArguments(void)
{
    delete [] _progName;

    delete [] _volFileName;
    delete [] _noiseFileName;
    delete [] _tfFileName;
    delete [] _licFilterFileName;
    delete [] _redirectFile;
    delete [] _haltonFileName;
}

void ParseArguments::printUsage(void)
{
    std::cerr << "\nUsage:  "
              << (_progName ? _progName : (_argv ? _argv[0] : "executable"))
              << " <volfilename.dat> [-h | --help] "
              << "[-g | --gradient] \n" /*[-l | --lambda2]*/
              << "\t\t\t\t[-f <file> | --filter=<file>]\n"
              << "\t\t\t\t[-n <file> | --noise=<file>]\n"
              << "\t\t\t\t[-t <file> | --transfer=<file>]\n"
        //        << "\t\t\t\t[-r <file> | --redirect=<file>]\n"
        //        << "\t\t\t\t[-s <file> | --halton=<file>]\n\n"
              << "\t-h | --help \tShow usage\n"
              << "\t-g | --gradient\tUse noise gradients\n"
        //        << "\t-l | --lambda2 \tLoad lambda2 volume\n"
              << "\t-f <png>\tFilter kernel stored in PNG file\n"
              << "\t--filter=<png>\n"
              << "\t-n <noisefile>\tUse given noise for LIC\n"
              << "\t--noise=<noisefile>\n"
              << "\t-t <png>\tTransfer function stored in PNG file\n"
              << "\t--transfer=<png>\n"
        //        << "\t-r <file>\tRedirect output to file\n"
        //        << "\t--redirect=<file>\n"
        //        << "\t-s <file>\tHalton sequence for camera positions\n"
        //        << "\t--halton=<file>\n"
              << std::endl;
}


void ParseArguments::setProgramName(const char *progName)
{
    if (_progName)
    {
        delete [] _progName;
        _progName = NULL;
    }

    if (!progName)
        return;

    _progName = new char[strlen(progName)+1];
    strcpy(_progName, progName);
}


bool ParseArguments::parse(void)
{
    int idx = 1;   // index of current argument 
    int len;       // length of current argument

    if (_argc == 1)
    {
        return false;
    }

    while (idx < _argc)
    {
        len = (int)strlen(_argv[idx]);

        if ((len > 2) && (_argv[idx][0] == '-')
            && (_argv[idx][1] == '-')) // e.g. --help
        {
            if (!parseLongArgs(idx, len))
            {
                std::cerr << "Unrecognized argument: " << _argv[idx] << std::endl;
                return false;
            }
        }
        else if ((len == 2) && (_argv[idx][0] == '-')) // e.g. -h
        {
            // short arguments
            switch (_argv[idx][1])
            {
            case 'h':
                printUsage();
                exit(0);        
                break;
            case 'g':
                _useGradients = true;
                break;
            case 'l':
                _useLambda2 = true;
                break;
            case 'f':
                if (idx+1 < _argc)
                {
                    if (_argv[idx+1][0] != '-')
                    {
                        _licFilterFileName = new char[strlen(_argv[idx+1])+1];
                        strcpy(_licFilterFileName, _argv[idx+1]);
                        ++idx;
                    }
                    else
                    {
                        std::cerr << "Invalid argument: " << _argv[idx]
                                  << std::endl;
                        return false;
                    }
                }
                else
                {
                    std::cerr << "Missing filename:  filter kernel (png)" 
                              << std::endl;
                    return false;
                }
                break;
            case 'n':
                if (idx+1 < _argc)
                {
                    if (_argv[idx+1][0] != '-')
                    {
                        _noiseFileName = new char[strlen(_argv[idx+1])+1];
                        strcpy(_noiseFileName, _argv[idx+1]);
                        ++idx;
                    }
                    else
                    {
                        std::cerr << "Invalid argument: " << _argv[idx] 
                                  << std::endl;
                        return false;
                    }
                }
                else
                {
                    std::cerr << "Missing filename:  noise" << std::endl;
                    return false;
                }
                break;
            case 't':
                if (idx+1 < _argc)
                {
                    if (_argv[idx+1][0] != '-')
                    {
                        _tfFileName = new char[strlen(_argv[idx+1])+1];
                        strcpy(_tfFileName, _argv[idx+1]);
                        ++idx;
                    }
                    else
                    {
                        std::cerr << "Invalid argument: " << _argv[idx] 
                                  << std::endl;
                        return false;
                    }
                }
                else
                {
                    std::cerr << "Missing filename:  transfer function (png)" 
                              << std::endl;
                    return false;
                }
                break;
            case 'r':
                if (idx+1 < _argc)
                {
                    if (_argv[idx+1][0] != '-')
                    {
                        _redirectFile = new char[strlen(_argv[idx+1])+1];
                        strcpy(_redirectFile, _argv[idx+1]);
                        ++idx;
                    }
                    else
                    {
                        std::cerr << "Invalid argument: " << _argv[idx] 
                                  << std::endl;
                        return false;
                    }
                }
                else
                {
                    std::cerr << "Missing filename:  redirection" 
                              << std::endl;
                    return false;
                }
                break;
            case 's':
                if (idx+1 < _argc)
                {
                    if (_argv[idx+1][0] != '-')
                    {
                        _haltonFileName = new char[strlen(_argv[idx+1])+1];
                        strcpy(_haltonFileName, _argv[idx+1]);
                        ++idx;
                    }
                    else
                    {
                        std::cerr << "Invalid argument: " << _argv[idx] 
                                  << std::endl;
                        return false;
                    }
                }
                else
                {
                    std::cerr << "Missing filename:  halton sequence" 
                              << std::endl;
                    return false;
                }
                break;
            default:
                std::cerr << "Unknown argument: " << _argv[idx] 
                          << std::endl;
                return false;
                break;
            }
        }
        else // no leading '-'
        {
            // first fill volume filename. Abort if more to come
            if (!_volFileName)
            {
                _volFileName = new char[strlen(_argv[idx])+1];
                strcpy(_volFileName, _argv[idx]);
            }
            else
            {
                std::cerr << "Unknown arguments (too much filenames)." 
                          << std::endl;
                return false;
            }
        }
        ++idx;
    }
    return true;
}


bool ParseArguments::parseLongArgs(int idx, int len)
{
    if (strcmp(&_argv[idx][2], "help") == 0)
    {
        printUsage();
        exit(0);
    }
    else if (strncmp(&_argv[idx][2], "filter", 6) == 0)
    {
        if ((len > 9) && (_argv[idx][8] == '='))
        {
            _licFilterFileName = new char[strlen(&_argv[idx][9])+1];
            strcpy(_licFilterFileName, &_argv[idx][9]);
        }
        else
        {
            std::cerr << "Missing filename:  filter kernel (png)" 
                      << std::endl;
            return false;
        }
    }
    else if (strncmp(&_argv[idx][2], "noise", 5) == 0)
    {
        if ((len > 9) && (_argv[idx][7] == '='))
        {
            _noiseFileName = new char[strlen(&_argv[idx][8])+1];
            strcpy(_noiseFileName, &_argv[idx][8]);
        }
        else
        {
            std::cerr << "Missing filename:  noise" << std::endl;
            return false;
        }
    }
    else if (strncmp(&_argv[idx][2], "transfer", 8) == 0)
    {
        if ((len > 11) && (_argv[idx][10] == '='))
        {
            _tfFileName = new char[strlen(&_argv[idx][11])+1];
            strcpy(_tfFileName, &_argv[idx][11]);
        }
        else
        {
            std::cerr << "Missing filename:  transfer function (png)" 
                      << std::endl;
            return false;
        }
    }
    else if (strncmp(&_argv[idx][2], "redirect", 8) == 0)
    {
        if ((len > 11) && (_argv[idx][10] == '='))
        {
            _redirectFile = new char[strlen(&_argv[idx][11])+1];
            strcpy(_redirectFile, &_argv[idx][11]);
        }
        else
        {
            std::cerr << "Missing filename:  redirection" << std::endl;
            return false;
        }
    }
    else if (strncmp(&_argv[idx][2], "halton", 6) == 0)
    {
        if ((len > 9) && (_argv[idx][8] == '='))
        {
            _haltonFileName = new char[strlen(&_argv[idx][9])+1];
            strcpy(_haltonFileName, &_argv[idx][9]);
        }
        else
        {
            std::cerr << "Missing filename:  halton sequence" << std::endl;
            return false;
        }
    }
    else if (strncmp(&_argv[idx][2], "gradient", 8) == 0)
    {
        _useGradients = true;
    }
    else if (strncmp(&_argv[idx][2], "lambda2", 7) == 0)
    {
        _useLambda2 = true;
    }
    else
    {
        return false;
    }

    return true;
}
