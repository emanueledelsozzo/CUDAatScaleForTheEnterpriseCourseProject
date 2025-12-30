/* Copyright (c) 2019, NVIDIA CORPORATION. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *  * Neither the name of NVIDIA CORPORATION nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#define WINDOWS_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#pragma warning(disable : 4819)
#endif

#include <Exceptions.h>
#include <ImageIO.h>
#include <ImagesCPU.h>
#include <ImagesNPP.h>
#include <vector>
#include <tuple>

#include <string.h>
#include <fstream>
#include <iostream>
#include <dirent.h>
#include <algorithm>
#include <cctype>

#include <cuda_runtime.h>
#include <npp.h>

#include <helper_cuda.h>
#include <helper_string.h>


std::tuple<std::string, std::string> parseInputAndOutput(int argc, char *argv[]){

    std::string sFilename;
    char *filePath;

    // parse input file
    if (checkCmdLineFlag(argc, (const char **)argv, "input"))
    {
        getCmdLineArgumentString(argc, (const char **)argv, "input", &filePath);
        sFilename = filePath;
    } else 
    {
        std::cout << std::endl << std::endl << "Please provide the input file: " << std::endl
        << " -input INPUT_FILE: path to input image" << std::endl << std::endl;
        exit(EXIT_FAILURE);
    }

    // if we specify the filename at the command line, then we only test
    // sFilename[0].
    int fileErrors = 0;
    std::ifstream infile(sFilename.data(), std::ifstream::in);

    // check input file
    if (infile.good())
    {
        std::cout << "edgeDetector opened: <" << sFilename.data()
                  << "> successfully!" << std::endl;
        fileErrors = 0;
        infile.close();
    }
    else
    {
        std::cout << "edgeDetector unable to open: <" << sFilename.data() << ">"
                  << std::endl;
        fileErrors++;
        infile.close();
    }

    if (fileErrors > 0)
    {
        exit(EXIT_FAILURE);
    }

    std::string sResultFilename = sFilename;

    std::string::size_type dot = sResultFilename.rfind('.');

    if (dot != std::string::npos)
    {
        sResultFilename = sResultFilename.substr(0, dot);
    }

    sResultFilename += "_edgeDetector.pgm";

    // parse output file
    if (checkCmdLineFlag(argc, (const char **)argv, "output"))
    {
        char *outputFilePath;
        getCmdLineArgumentString(argc, (const char **)argv, "output",
                                 &outputFilePath);
        sResultFilename = outputFilePath;
    }

    return {sFilename, sResultFilename};

}


std::tuple<Npp16s, Npp16s> parseThresholds(int argc, char *argv[]){

    Npp16s lowThreshold = 85;
    Npp16s highThreshold = 255;

    // parse low threshold
    if (checkCmdLineFlag(argc, (const char **)argv, "low_threshold"))
    {
        lowThreshold = getCmdLineArgumentInt(argc, (const char **)argv, "low_threshold");
    }

    // parse high threshold
    if (checkCmdLineFlag(argc, (const char **)argv, "high_threshold"))
    {
        highThreshold = getCmdLineArgumentInt(argc, (const char **)argv, "high_threshold");
    }

    // check both thresholds are positive
    if (lowThreshold < 0 || highThreshold < 0){
        std::cout << std::endl << std::endl << "Thresholds cannot be negative" << std::endl;
        exit(EXIT_FAILURE);
    }

    // check low threshold is smaller than high threshold
    if (lowThreshold > highThreshold)
    {
        std::cout << std::endl << std::endl << "low_threshold must be smaller than high_threshold" << std::endl;
        exit(EXIT_FAILURE);
    }

    return {lowThreshold, highThreshold};

}


NppiDifferentialKernel parseKernel(int argc, char *argv[]){

    NppiDifferentialKernel kernel = NPP_FILTER_SOBEL;

    // parse kernel
    if (checkCmdLineFlag(argc, (const char **)argv, "kernel"))
    {
        char *selectedKernel;
        std::string sKernel;
        getCmdLineArgumentString(argc, (const char **)argv, "kernel",
                                 &selectedKernel);
        sKernel = selectedKernel;

        // convert the string to lowercase in-place
        std::transform(sKernel.begin(), sKernel.end(), sKernel.begin(), 
                   [](unsigned char c){ return std::tolower(c); }); // use a lambda to handle potential type issues
    
        if (sKernel == "sobel")
        {
            kernel = NPP_FILTER_SOBEL;
        } else if (sKernel == "scharr")
        {
            kernel = NPP_FILTER_SCHARR;
        } else {
            std::cout << std::endl << std::endl << 
            "Please select one of the supported kernels: sobel or scharr" << std::endl << std::endl;
            exit(EXIT_FAILURE);
        }

    }

    return kernel;

}


std::tuple<std::string, std::string, Npp16s, Npp16s, NppiDifferentialKernel> parseArguments(int argc, char *argv[])
{

    std::string sFilename, sResultFilename;
    Npp16s lowThreshold, highThreshold;
    NppiDifferentialKernel kernel;

    // helper function
    if (checkCmdLineFlag(argc, (const char **)argv, "help"))
    {
        std::cout << std::endl << std::endl << "Usage:" << std::endl
        << " -input INPUT_FILE: path to input image [required]" << std::endl
        << " -output OUTPUT_FILE: path to output image [default: INPUT_FILE_edgeDetector.pgm]" << std::endl
        << " -low_threshold=LOW_THRESHOLD: low hysteresis threshold [default: 85]" << std::endl
        << " -high_threshold=HIGH_THRESHOLD: high hysteresis threshold [default: 255]" << std::endl 
        << " -kernel KERNEL: differential filter - the supported kernels sobel and scharr [default: sobel]" << std::endl 
        << std::endl;
        exit(EXIT_SUCCESS);

    }

    // parse all arguments
    std::tie(sFilename, sResultFilename) = parseInputAndOutput(argc, argv);
    std::tie(lowThreshold, highThreshold) = parseThresholds(argc, argv);
    kernel = parseKernel(argc, argv);

    return {sFilename, sResultFilename, lowThreshold, highThreshold, kernel};
}


    bool printfNPPinfo(int argc, char *argv[])
{
    const NppLibraryVersion *libVer = nppGetLibVersion();

    printf("NPP Library Version %d.%d.%d\n", libVer->major, libVer->minor,
           libVer->build);

    int driverVersion, runtimeVersion;
    cudaDriverGetVersion(&driverVersion);
    cudaRuntimeGetVersion(&runtimeVersion);

    printf("  CUDA Driver  Version: %d.%d\n", driverVersion / 1000,
           (driverVersion % 100) / 10);
    printf("  CUDA Runtime Version: %d.%d\n", runtimeVersion / 1000,
           (runtimeVersion % 100) / 10);

    // min spec is SM 1.0 devices
    bool bVal = checkCudaCapabilities(1, 0);
    return bVal;
}


int main(int argc, char *argv[])
{
    printf("%s Starting...\n\n", argv[0]);

    try
    {

        findCudaDevice(argc, (const char **)argv);

        if (printfNPPinfo(argc, argv) == false)
        {
            exit(EXIT_SUCCESS);
        }

        std::string sFilename, sResultFilename;
        Npp16s lowThreshold, highThreshold;
        NppiDifferentialKernel kernel;

        // parse arguments to the application
        std::tie(sFilename, sResultFilename, lowThreshold, highThreshold, kernel) = 
            parseArguments(argc, argv);

        // declare a host image object for an 8-bit grayscale image
        npp::ImageCPU_8u_C1 oHostSrc;
        // load gray-scale image from disk
        npp::loadImage(sFilename, oHostSrc);
        // declare a device image and copy construct from the host image,
        // i.e. upload host to device
        npp::ImageNPP_8u_C1 oDeviceSrc(oHostSrc);

        NppiSize oSrcSize = {(int)oDeviceSrc.width(), (int)oDeviceSrc.height()};
        NppiPoint oSrcOffset = {0, 0};

        // create struct with ROI size
        NppiSize oSizeROI = {(int)oDeviceSrc.width(), (int)oDeviceSrc.height()};
        // allocate device image of appropriately reduced size
        npp::ImageNPP_8u_C1 oDeviceDst(oSizeROI.width, oSizeROI.height);
        
        // declare memory buffer
        int nBufferSize = 0;
        Npp8u *pDeviceBuffer = 0;

        // get necessary scratch buffer size and allocate that much device memory
        NPP_CHECK_NPP(nppiFilterCannyBorderGetBufferSize(
            oSizeROI, &nBufferSize));

        cudaMalloc((void **)&pDeviceBuffer, nBufferSize);

        if ((nBufferSize > 0) && (pDeviceBuffer != 0))
        {

            // run edge detector
            NPP_CHECK_NPP(nppiFilterCannyBorder_8u_C1R(
                oDeviceSrc.data(), oDeviceSrc.pitch(), oSrcSize, oSrcOffset,
                oDeviceDst.data(), oDeviceDst.pitch(), oSizeROI, kernel,
                NPP_MASK_SIZE_3_X_3, lowThreshold, highThreshold, nppiNormL2,
                NPP_BORDER_REPLICATE, pDeviceBuffer));
        }

        // declare a host image for the result
        npp::ImageCPU_8u_C1 oHostDst(oDeviceDst.size());
        // and copy the device result data into it
        oDeviceDst.copyTo(oHostDst.data(), oHostDst.pitch());

        saveImage(sResultFilename, oHostDst);
        std::cout << "Saved image: " << sResultFilename << std::endl;

        // free device memory
        nppiFree(oDeviceSrc.data());
        nppiFree(oDeviceDst.data());
        cudaFree(pDeviceBuffer);

        exit(EXIT_SUCCESS);
    }
    catch (npp::Exception &rException)
    {
        std::cerr << "Program error! The following exception occurred: \n";
        std::cerr << rException << std::endl;
        std::cerr << "Aborting." << std::endl;

        exit(EXIT_FAILURE);
    }
    catch (...)
    {
        std::cerr << "Program error! An unknow type of exception occurred. \n";
        std::cerr << "Aborting." << std::endl;

        exit(EXIT_FAILURE);
        return -1;
    }

    return 0;
}
