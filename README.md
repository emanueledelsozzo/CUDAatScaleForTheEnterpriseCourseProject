# Edge Detection using NVIDIA NPP with CUDA

## Overview

This project demonstrates the use of NVIDIA Performance Primitives (NPP) library with CUDA to perform edge detection. The goal is to utilize GPU acceleration to efficiently detect the edges of a given image, leveraging the computational power of modern GPUs. The project is a part of the CUDA at Scale for the Enterprise course and serves as a way for understanding how to implement basic image processing operations using CUDA and NPP.

## Code Organization

```bin/```
This folder will hold the binary/executable code that is built automatically or manually.

```datasets/```
This folder contains example datasets and a preprocessing script. Specifically, it includes a pgm version of the aerials dataset (aerials_pgm), whose original version is available from the USC Viterbi School of Engineering's SIPI Image Database (https://sipi.usc.edu/database/database.php?volume=aerials), a version processed by the edge detector application (aerials_out), and a Lena pgm image (Lena.pgm). This folder also contains a Python script to convert images to pgm (convert_to_pgm.py).

```Common/```
This folder contains CUDA libraries.

```src/```
This folder contains the source code.

```README.md```
This file holds the description of the project.

```Makefile```
A Makefile to build the project's code.

```run_single_image.sh```
An example script to run the executable code on a single image.

```run_multiple_images.sh```
An example script to run the executable code on multiple images from the datasets folder.

## Key Concepts

Performance Strategies, Image Processing, NPP Library

## Supported SM Architectures

[SM 3.5 ](https://developer.nvidia.com/cuda-gpus)  [SM 3.7 ](https://developer.nvidia.com/cuda-gpus)  [SM 5.0 ](https://developer.nvidia.com/cuda-gpus)  [SM 5.2 ](https://developer.nvidia.com/cuda-gpus)  [SM 6.0 ](https://developer.nvidia.com/cuda-gpus)  [SM 6.1 ](https://developer.nvidia.com/cuda-gpus)  [SM 7.0 ](https://developer.nvidia.com/cuda-gpus)  [SM 7.2 ](https://developer.nvidia.com/cuda-gpus)  [SM 7.5 ](https://developer.nvidia.com/cuda-gpus)  [SM 8.0 ](https://developer.nvidia.com/cuda-gpus)  [SM 8.6 ](https://developer.nvidia.com/cuda-gpus)

## Supported OSes

Linux, Windows

## Supported CPU Architecture

x86_64, ppc64le, armv7l

## CUDA APIs involved

## Dependencies needed to build/run
[FreeImage](https://freeimage.sourceforge.io), [NPP](https://developer.nvidia.com/npp)

## Prerequisites

Download and install the [CUDA Toolkit 11.4](https://developer.nvidia.com/cuda-downloads) for your corresponding platform.
Make sure the dependencies mentioned in [Dependencies]() section above are installed.

## Build and Run

### Windows
The Windows samples are built using the Visual Studio IDE. Solution files (.sln) are provided for each supported version of Visual Studio, using the format:
```
*_vs<version>.sln - for Visual Studio <version>
```
Each individual sample has its own set of solution files in its directory:

To build/examine all the samples at once, the complete solution files should be used. To build/examine a single sample, the individual sample solution files should be used.
> **Note:** Some samples require that the Microsoft DirectX SDK (June 2010 or newer) be installed and that the VC++ directory paths are properly set up (**Tools > Options...**). Check DirectX Dependencies section for details."

### Linux
The Linux samples are built using makefiles. To use the makefiles, change the current directory to the sample directory you wish to build, and run make:
```
$ make
```
The samples makefiles can take advantage of certain options:
*  **TARGET_ARCH=<arch>** - cross-compile targeting a specific architecture. Allowed architectures are x86_64, ppc64le, armv7l.
    By default, TARGET_ARCH is set to HOST_ARCH. On a x86_64 machine, not setting TARGET_ARCH is the equivalent of setting TARGET_ARCH=x86_64.<br/>
`$ make TARGET_ARCH=x86_64` <br/> `$ make TARGET_ARCH=ppc64le` <br/> `$ make TARGET_ARCH=armv7l` <br/>
    See [here](http://docs.nvidia.com/cuda/cuda-samples/index.html#cross-samples) for more details.
*   **dbg=1** - build with debug symbols
    ```
    $ make dbg=1
    ```
*   **SMS="A B ..."** - override the SM architectures for which the sample will be built, where `"A B ..."` is a space-delimited list of SM architectures. For example, to generate SASS for SM 50 and SM 60, use `SMS="50 60"`.
    ```
    $ make SMS="50 60"
    ```

*  **HOST_COMPILER=<host_compiler>** - override the default g++ host compiler. See the [Linux Installation Guide](http://docs.nvidia.com/cuda/cuda-installation-guide-linux/index.html#system-requirements) for a list of supported host compilers.
```
    $ make HOST_COMPILER=g++
```


## Running the Program
After building the project, you can run the program using the following command:

```bash
make run
```

This command will execute the compiled binary, applying edge detection to the input image (Lena.pgm), and save the result as Lena_edgeDetector.pgm in the datasets/ folder.

If you wish to run the binary directly with custom input/output files, you can use:

```bash
./bin/edgeDetector -input datasets/Lena.pgm -output datasets/Lena_edgeDetector.pgm
```

You can run this command to see the supported input arguments:

```bash
./bin/edgeDetector -help
```

The code supports the following input arguments:

```
-input INPUT_FILE: path to input image [required]
-output OUTPUT_FILE: path to output image [default: INPUT_FILE_edgeDetector.pgm]
-low_threshold=LOW_THRESHOLD: low hysteresis threshold [default: 85]
-high_threshold=HIGH_THRESHOLD: high hysteresis threshold [default: 255]
-kernel KERNEL: differential filter - the supported kernels sobel and scharr [default: sobel]
```

- Cleaning Up
To clean up the compiled binaries and other generated files, run:

```bash
make clean
```

This will remove all files in the bin/ and build/ folders.

## References (for more details)
[NPP FilterCannyBorder](https://docs.nvidia.com/cuda/archive/9.2/npp/group__image__filter__canny__border.html) [Canny Edge Detector](https://en.wikipedia.org/wiki/Canny_edge_detector)
