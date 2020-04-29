/**
 * @file imagesignalprocessor.h
 *
 * @brief provide the image process algorithms
 *
 * @author Dan.Cao <caodan@linuxtoy.cn>
 */
#ifndef IMAGESIGNALPROCESSOR_H
#define IMAGESIGNALPROCESSOR_H

#include <cstddef>
#include <iostream>

#define BEGIN_NAMESPACE_ISP namespace ImageSignalProcessor {

#define END_NAMESPACE_ISP }

#define ISP_USE_NAMESPACE using namespace ImageSignalProcessor;


BEGIN_NAMESPACE_ISP

/**
 * @brief bayer color filter array pattern
 */
enum BayerCFAPattern_e {
    BAYER_CFA_RGGB = 0,
    BAYER_CFA_BGGR,
    BAYER_CFA_GRBG,
    BAYER_CFA_GBRG,
};

/**
 * @brief raw image format in memory
 */
enum RawFormat_e {
    FORMAT_RAW8 = 0,
    FORMAT_RAW10_UNPACKED,
    FORMAT_RAW12_UNPACKED,
    FORMAT_RAW14_UNPACKED,
    FORMAT_RAW16,
};

/**
 * @brief describe the bayer image data
 */
struct BayerImageData {
    int width;        /*!< image width */
    int stride;       /*!< image stride for each row */
    int height;       /*!< image height */
    RawFormat_e format; /*!< image memory format */

    void *imageData;  /*!< point to the image data */

    BayerImageData()
        : width (0)
        , stride (0)
        , height (0)
        , format (FORMAT_RAW12_UNPACKED)
        , imageData (nullptr)
    {
    }

    size_t imageSize() { return height * stride;}

    int init(int w, int h, RawFormat_e f) {
        int s;

        switch (f) {
        case FORMAT_RAW8:
            s = w;
            break;
        case FORMAT_RAW10_UNPACKED:
        case FORMAT_RAW12_UNPACKED:
        case FORMAT_RAW14_UNPACKED:
        case FORMAT_RAW16:
            s = w * 2;
            break;
        default:
            std::cout << "Invalid format " << f << std::endl;
            return -1;
        }
        stride = s;
        width = w;
        height = h;
        format = f;

        return 0;
    }

    void setImageData(void *data) {
        imageData = data;
    }
};

enum RGBFormat_e {
    FORMAT_RGB32,   /*!< The image is stored using a 32-bit RGB format (0xffRRGGBB). */
    FORMAT_ARGB32,  /*!< The image is stored using a 32-bit ARGB format (0xAARRGGBB). */
    FORMAT_RGB888,  /*!< The image is stored using a 24-bit RGB format (8-8-8). */
    FORMAT_RGBA8888 /*!< The image is stored using a 32-bit byte-ordered RGBA format (8-8-8-8).
                         Unlike ARGB32 this is a byte-ordered format,
                         which means the 32bit encoding differs between big endian and little endian
                         architectures, being respectively (0xRRGGBBAA) and (0xAABBGGRR).
                         The order of the colors is the same on any architecture
                         if read as bytes 0xRR,0xGG,0xBB,0xAA. */
};

/**
 * @brief describe the RGB image data
 */
struct RGBImageData {
    int width;        /*!< image width */
    int stride;       /*!< image stride for each row */
    int height;       /*!< image height */
    RGBFormat_e format;

    void *imageData;  /*!< point to the image data */

    RGBImageData()
        : width (0)
        , stride (0)
        , height (0)
        , format (FORMAT_RGB32)
        , imageData (nullptr)
    {
    }

    size_t imageSize() { return height * stride;}

    int init(int w, int h, RGBFormat_e f) {
        int s;

        switch (f) {
        case FORMAT_RGB32:
        case FORMAT_ARGB32:
        case FORMAT_RGBA8888:
            s = w * 4;
            break;
        case FORMAT_RGB888:
            s = w * 3;
            break;
        default:
            std::cout << "Invalid format " << f << std::endl;
            return -1;
        }
        stride = s;
        width = w;
        height = h;
        format = f;

        return 0;
    }

    void setImageData(void *data) {
        imageData = data;
    }
};


END_NAMESPACE_ISP


#endif // IMAGESIGNALPROCESSOR_H
