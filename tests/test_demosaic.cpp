/*
 * File Name: test_demosaic.cpp
 * Description:
 * Author: Dan.Cao <caodan@linuxtoy.cn>
 *
 */

#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <getopt.h>

#include "raw_bayer_demosaic.h"

ISP_USE_NAMESPACE

void show_usage(char *name)
{
    printf("usage: %s\n", name);
    printf("   --input,-i     input file\n");
    printf("   --output,-o    output file\n");
    printf("   --width,-w     image width (pixels)\n");
    printf("   --stride,-s    image stride (bytes)\n");
    printf("   --height,-v    image height (pixels)\n");
    printf("   --format,-f    bayer mem format: RAW8, RAW10_UNPACKED, RAW12_UNPACKED, RAW14_UNPACKED, RAW16\n");
    printf("   --cfa,-c       color filter array: RGGB, GBRG, GRBG, BGGR\n");
    printf("   --help,-h      this helpful message\n");
}

struct test_demosaic_args {
    char *inFileName;
    char *outFileName;
    int width;
    int height;
    int stride;
    BayerCFAPattern_e cfa;
    RawFormat_e rawFmt;

    uint8_t *bayerData;
};

static std::string bayerPattern2String(BayerCFAPattern_e cfa)
{
    std::string str;

    switch (cfa) {
    case BAYER_CFA_RGGB:
        str = "RGGB";
        break;
    case BAYER_CFA_BGGR:
        str = "BGGR";
        break;
    case BAYER_CFA_GRBG:
        str = "GRBG";
        break;
    case BAYER_CFA_GBRG:
        str = "GBRG";
        break;
    default:
        str = "Invalid CFA";
        break;
    }

    return str;
}

static std::string rawFormat2String(RawFormat_e format)
{
    std::string str;

    switch (format) {
    case FORMAT_RAW8:
        str = "RAW8";
        break;
    case FORMAT_RAW10_UNPACKED:
        str = "RAW10_UNPACKED";
        break;
    case FORMAT_RAW12_UNPACKED:
        str = "RAW12_UNPACKED";
        break;
    case FORMAT_RAW14_UNPACKED:
        str = "RAW14_UNPACKED";
        break;
    case FORMAT_RAW16:
        str = "RAW16";
        break;
    default:
        str = "Invalid Raw format";
        break;
    }

    return str;
}

static int parse_arg(int argc, char *argv[],
        struct test_demosaic_args *demosaic_arg)
{
    int optidx = 0;

    struct option longopt[] = {
        {"input",  required_argument, NULL,'i'},
        {"output", required_argument, NULL,'o'},
        {"width",  required_argument, NULL,'w'},
        {"height", required_argument, NULL,'v'},
        {"stride", required_argument, NULL,'s'},
        {"format", required_argument, NULL,'f'},
        {"cfa",    required_argument, NULL,'c'},
        {"help",   no_argument,       NULL,'h'},
        {0,0,0,0}
    };

    char c;
    while ((c=getopt_long(argc,argv,"i:o:w:v:s:f:c:h",longopt,&optidx)) != -1) {
        switch (c) {
        case 'i':
            demosaic_arg->inFileName = strdup(optarg);
            break;
        case 'o':
            demosaic_arg->outFileName = strdup(optarg);
            break;
        case 'w':
            demosaic_arg->width = strtol(optarg, NULL, 10);
            break;
        case 'v':
            demosaic_arg->height = strtol(optarg, NULL, 10);
            break;
        case 's':
            demosaic_arg->stride = strtol(optarg, NULL, 10);
            break;
        case 'f':
            if(strcmp(optarg, "RAW8") == 0)
                demosaic_arg->rawFmt = FORMAT_RAW8;
            else if(strcmp(optarg, "RAW10_UNPACKED") == 0)
                demosaic_arg->rawFmt = FORMAT_RAW10_UNPACKED;
            else if(strcmp(optarg, "RAW12_UNPACKED") == 0)
                demosaic_arg->rawFmt = FORMAT_RAW12_UNPACKED;
            else if(strcmp(optarg, "RAW14_UNPACKED") == 0)
                demosaic_arg->rawFmt = FORMAT_RAW14_UNPACKED;
            else if(strcmp(optarg, "RAW16_UNPACKED") == 0)
                demosaic_arg->rawFmt = FORMAT_RAW16;
            else {
                std::cout << "Invalid format " << optarg << std::endl;
                return -1;
            }
            break;
        case 'c':
            if(strcmp(optarg, "RGGB") == 0)
                demosaic_arg->cfa = BAYER_CFA_RGGB;
            else if(strcmp(optarg, "BGGR") == 0)
                demosaic_arg->cfa = BAYER_CFA_BGGR;
            else if(strcmp(optarg, "GRBG") == 0)
                demosaic_arg->cfa = BAYER_CFA_GRBG;
            else if(strcmp(optarg, "GBRG") == 0)
                demosaic_arg->cfa = BAYER_CFA_GBRG;
            else {
                std::cout << "Invalid CFA " << optarg << std::endl;
                return -1;
            }
            break;
        case 'h':
            show_usage(argv[0]);
            exit(0);
        default:
            std::cout << "bad arg" << std::endl;
            exit(0);
            break;
        }
    }

    if (demosaic_arg->inFileName == nullptr) {
        std::cout << "Invalid arg: input file not set" << std::endl;
        return -1;
    }
    if (demosaic_arg->outFileName == nullptr) {
        std::cout << "Invalid arg: output file not set" << std::endl;
        return -1;
    }
    if (demosaic_arg->width == 0) {
        std::cout << "Invalid arg: width not set" << std::endl;
        return -1;
    }
    if (demosaic_arg->height == 0) {
        std::cout << "Invalid arg: height not set" << std::endl;
        return -1;
    }

    if (demosaic_arg->stride == 0) {
        if (demosaic_arg->rawFmt == FORMAT_RAW8)
            demosaic_arg->stride = demosaic_arg->width;
        else
            demosaic_arg->stride = demosaic_arg->width * 2;
    }

    std::cout << "Input  File: " << std::string(demosaic_arg->inFileName) << std::endl;
    std::cout << "Output File: " << std::string(demosaic_arg->outFileName) << std::endl;
    std::cout << demosaic_arg->width << " x " << demosaic_arg->height << "  "
        << "stride = " << demosaic_arg->stride << "  "
        << rawFormat2String(demosaic_arg->rawFmt) << "  "
        << bayerPattern2String(demosaic_arg->cfa)
        << std::endl;

    /* load input file */
    size_t bayerImageSize = demosaic_arg->stride * demosaic_arg->height;
    FILE *input_fp = fopen(demosaic_arg->inFileName, "r");
    if (input_fp == NULL) {
        std::cout << "Fail to open " << demosaic_arg->inFileName << std::endl;
        return -1;
    }
    fseek(input_fp, 0, SEEK_END);
    if (ftell(input_fp) < static_cast<long>(bayerImageSize)) {
        std::cout << "input file size tool small, less than "
            << bayerImageSize << std::endl;
        fclose(input_fp);
        return -1;
    }
    fseek(input_fp, 0, SEEK_SET);

    uint8_t *bayerData = new uint8_t[bayerImageSize];
    if (fread(bayerData, 1, bayerImageSize, input_fp) != bayerImageSize) {
        std::cout << "Fail to read " << bayerImageSize << " bytes from "
            << demosaic_arg->inFileName << std::endl;
        delete bayerData;
        fclose(input_fp);
        return -1;
    }
    fclose(input_fp);

    demosaic_arg->bayerData = bayerData;

    return 0;
}

int main(int argc, char *argv[])
{
    struct test_demosaic_args demosaic_arg;
    int rc;

    memset(&demosaic_arg, 0, sizeof(demosaic_arg));

    rc = parse_arg(argc, argv, &demosaic_arg);
    if (rc)
        return rc;

    BayerImageData bayerImageData;
    bayerImageData.init(demosaic_arg.width, demosaic_arg.height, demosaic_arg.rawFmt);
    bayerImageData.setImageData(demosaic_arg.bayerData);

    RGBImageData rgbImageData;
    rc = rgbImageData.init(demosaic_arg.width, demosaic_arg.height,
            FORMAT_RGB888);
    if (rc) {
        std::cout << "Fail to init rgb image data" << std::endl;
        return rc;
    }
    auto rgbDataSize = rgbImageData.imageSize();
    auto rgbData = new uint8_t[rgbDataSize];
    rgbImageData.setImageData(rgbData);

    Demosaic demosaic;

    rc = demosaic.bayer2RGB(bayerImageData, demosaic_arg.cfa, rgbImageData);
    if (rc) {
        std::cout << "Fail to convert bayer to RGB" << std::endl;
        delete rgbData;
        delete demosaic_arg.bayerData;;
        return -1;
    }

    FILE *output_fp = fopen(demosaic_arg.outFileName, "w+");
    if (output_fp == NULL) {
        std::cout << "Fail to open " << demosaic_arg.outFileName << std::endl;
        return -1;
    }

    /* PNM header */
    //fprintf(output_fp, "P6\n%i %i\n255\n", demosaic_arg.width, demosaic_arg.height);

    fwrite(rgbData, 1, rgbDataSize, output_fp);
    fclose(output_fp);

    delete rgbData;
    delete demosaic_arg.bayerData;;

    return 0;
}
