/**
 * @file raw_bayer_demosaic.cpp
 *
 * @brief bayer demosaic algorithm implement
 *
 * @author Dan.Cao <caodan@linuxtoy.cn>
 */
#include <cmath>

#include "raw_bayer_demosaic.h"

ISP_USE_NAMESPACE

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

#ifndef CLAMP
#define CLAMP(_v, _min, _max)  MIN(MAX(_v, _min), _max)
#endif

Demosaic::Demosaic(enum DemosaicInterPolation method)
    : m_interpolationMethod (method)
    , m_gammaLUT(nullptr)
{
    float kFactor = 0.33;

    m_gammaLUT = new uint8_t[256];
    for (int i = 0; i < 256; i++) {
        float f = (i + 0.5f) / 255;

        f = static_cast<float>(pow(f, kFactor));

        m_gammaLUT[i] = static_cast<uint8_t>(f * 255.0f - 0.5f);
    }
}

Demosaic::Demosaic()
    : Demosaic(BILINEAR_INTERPOLATION)
{
    delete m_gammaLUT;
}

int Demosaic::bayer2RGB(const BayerImageData &bayerImage,
                         BayerCFAPattern_e cfa, RGBImageData &rgbImage) const
{
    if (m_interpolationMethod == BILINEAR_INTERPOLATION)
        bayer2RGB_BilinearInterpolation(bayerImage, cfa, rgbImage);
    else {
        std::cout << "Unsupport alg " << m_interpolationMethod << std::endl;
        return -1;
    }

    return 0;
}

static inline uint16_t bayerAt(const BayerImageData &bayerImage,
		int col, int row)
{
    uint16_t value;

    /* clamp the border
     * col may out of the image area 1 or 2 cols
     * row may out of the image area 1 or 2 row
     */
    if (row < 0)
        row += 2;
    else if (row >= bayerImage.height)
        row -= 2;
    if (col < 0)
        col += 2;
    else if (col >= bayerImage.width)
        col -= 2;

    if (bayerImage.format == FORMAT_RAW8) {
        uint8_t *data = reinterpret_cast<uint8_t *>((intptr_t)bayerImage.imageData + row * bayerImage.stride);

        value = data[col];
    } else {
        uint16_t *data = reinterpret_cast<uint16_t *>((intptr_t)bayerImage.imageData + row * bayerImage.stride);

        if (bayerImage.format == FORMAT_RAW10_UNPACKED)
            value =  data[col] >> 6;
        else if (bayerImage.format == FORMAT_RAW12_UNPACKED)
            value =  data[col] >> 4;
        else if (bayerImage.format == FORMAT_RAW14_UNPACKED)
            value =  data[col] >> 2;
        else
            value =  data[col];
    }

    return value;
}

/* BilinearInterpolation Pixel R */
static void BI_Pixel_R(int row, int col,
                       const BayerImageData &bayerImage,
                       uint16_t &R, uint16_t &G, uint16_t &B)
{
        /* Get G */
        uint16_t G1 = bayerAt(bayerImage, col,     row - 1);
        uint16_t G2 = bayerAt(bayerImage, col + 1, row    );
        uint16_t G3 = bayerAt(bayerImage, col,     row + 1);
        uint16_t G4 = bayerAt(bayerImage, col - 1, row    );

        /* Get R */
        uint16_t R1 = bayerAt(bayerImage, col,     row - 2);
        uint16_t R2 = bayerAt(bayerImage, col + 2, row    );
        uint16_t R3 = bayerAt(bayerImage, col,     row + 2);
        uint16_t R4 = bayerAt(bayerImage, col - 2, row    );

        /* Get B */
        uint16_t B1 = bayerAt(bayerImage, col - 1, row - 1);
        uint16_t B2 = bayerAt(bayerImage, col + 1, row - 1);
        uint16_t B3 = bayerAt(bayerImage, col + 1, row + 1);
        uint16_t B4 = bayerAt(bayerImage, col - 1, row + 1);

        R = bayerAt(bayerImage, col, row);

        if (abs(R1 - R3) < abs(R2 - R4)) {
            G = static_cast<uint32_t>(G1 + G3) / 2;
        } else if (abs(R1 - R3) > abs(R2 - R4)) {
            G = static_cast<uint32_t>(G2 + G4) / 2;
        } else {
            G = static_cast<uint32_t>(G1 + G2 + G3 + G4) / 4;
        }

        B = static_cast<uint32_t>(B1 + B2 + B3 + B4) / 4;
}

/* BilinearInterpolation Pixel Gr */
static void BI_Pixel_Gr(int row, int col,
                       const BayerImageData &bayerImage,
                       uint16_t &R, uint16_t &G, uint16_t &B)
{
        uint16_t R1 = bayerAt(bayerImage, col - 1, row);
        uint16_t R2 = bayerAt(bayerImage, col + 1, row);

        uint16_t B1 = bayerAt(bayerImage, col, row - 1);
        uint16_t B2 = bayerAt(bayerImage, col, row + 1);

        R = static_cast<uint32_t>(R1 + R2) / 2;
        G = bayerAt(bayerImage, col, row);
        B = static_cast<uint32_t>(B1 + B2) / 2;
}

/* BilinearInterpolation Pixel Gb */
static void BI_Pixel_Gb(int row, int col,
                       const BayerImageData &bayerImage,
                       uint16_t &R, uint16_t &G, uint16_t &B)
{
        uint16_t R1 = bayerAt(bayerImage, col, row - 1);
        uint16_t R2 = bayerAt(bayerImage, col, row + 1);

        uint16_t B1 = bayerAt(bayerImage, col - 1, row);
        uint16_t B2 = bayerAt(bayerImage, col + 1, row);

        R = static_cast<uint32_t>(R1 + R2) / 2;
        G = bayerAt(bayerImage, col, row);
        B = static_cast<uint32_t>(B1 + B2) / 2;
}

static void BI_Pixel_B(int row, int col,
                       const BayerImageData &bayerImage,
                       uint16_t &R, uint16_t &G, uint16_t &B)
{
        uint16_t G1 = bayerAt(bayerImage, col,     row - 1);
        uint16_t G2 = bayerAt(bayerImage, col + 1, row);
        uint16_t G3 = bayerAt(bayerImage, col,     row + 1);
        uint16_t G4 = bayerAt(bayerImage, col - 1, row);

        uint16_t B1 = bayerAt(bayerImage, col,     row - 2);
        uint16_t B2 = bayerAt(bayerImage, col + 2, row);
        uint16_t B3 = bayerAt(bayerImage, col,     row + 2);
        uint16_t B4 = bayerAt(bayerImage, col - 2, row);

        uint16_t R1 = bayerAt(bayerImage, col - 1, row - 1);
        uint16_t R2 = bayerAt(bayerImage, col + 1, row - 1);
        uint16_t R3 = bayerAt(bayerImage, col + 1, row + 1);
        uint16_t R4 = bayerAt(bayerImage, col - 1, row + 1);

        R = (R1 + R2 + R3 + R4) / 4;

        if (abs(B1 - B3) < abs(B2 - B4)) {
            G = (G1 + G3) / 2;
        } else if (abs(B1 - B3) > abs(B2 - B4)) {
            G = (G2 + G4) / 2;
        } else {
            G = (G1 + G2 + G3 + G4) / 4;
        }

        B = bayerAt(bayerImage, col, row);
}

void Demosaic::bayerRGGB2RGB_BI(int row, int col,
                              const BayerImageData &bayerImage,
                              BayerCFAPattern_e cfa,
                              uint16_t &R, uint16_t &G, uint16_t &B) const
{
    if (row % 2 == 0 && col % 2 == 0) {
        BI_Pixel_R(row, col, bayerImage, R, G, B);
    } else if (row % 2 == 0 && col % 2 == 1) {
        BI_Pixel_Gr(row, col, bayerImage, R, G, B);
    } else if (row % 2 == 1 && col % 2 == 0) {
        BI_Pixel_Gb(row, col, bayerImage, R, G, B);
    } else {
        BI_Pixel_B(row, col, bayerImage, R, G, B);
    }
}

void Demosaic::bayerGRBG2RGB_BI(int row, int col,
                              const BayerImageData &bayerImage,
                              BayerCFAPattern_e cfa,
                              uint16_t &R, uint16_t &G, uint16_t &B) const
{
    if (row % 2 == 0 && col % 2 == 0) {
        BI_Pixel_Gr(row, col, bayerImage, R, G, B);
    } else if (row % 2 == 0 && col % 2 == 1) {
        BI_Pixel_R(row, col, bayerImage, R, G, B);
    } else if (row % 2 == 1 && col % 2 == 0) {
        BI_Pixel_B(row, col, bayerImage, R, G, B);
    } else {
        BI_Pixel_Gb(row, col, bayerImage, R, G, B);
    }
}

void Demosaic::bayerGBRG2RGB_BI(int row, int col,
                              const BayerImageData &bayerImage,
                              BayerCFAPattern_e cfa,
                              uint16_t &R, uint16_t &G, uint16_t &B) const
{
    if (row % 2 == 0 && col % 2 == 0) {
        BI_Pixel_Gb(row, col, bayerImage, R, G, B);
    } else if (row % 2 == 0 && col % 2 == 1) {
        BI_Pixel_B(row, col, bayerImage, R, G, B);
    } else if (row % 2 == 1 && col % 2 == 0) {
        BI_Pixel_R(row, col, bayerImage, R, G, B);
    } else {
        BI_Pixel_Gr(row, col, bayerImage, R, G, B);
    }
}

void Demosaic::bayerBGGR2RGB_BI(int row, int col,
                              const BayerImageData &bayerImage,
                              BayerCFAPattern_e cfa,
                              uint16_t &R, uint16_t &G, uint16_t &B) const
{
    if (row % 2 == 0 && col % 2 == 0) {
        BI_Pixel_B(row, col, bayerImage, R, G, B);
    } else if (row % 2 == 0 && col % 2 == 1) {
        BI_Pixel_Gb(row, col, bayerImage, R, G, B);
    } else if (row % 2 == 1 && col % 2 == 0) {
        BI_Pixel_Gr(row, col, bayerImage, R, G, B);
    } else {
        BI_Pixel_R(row, col, bayerImage, R, G, B);
    }
}

int Demosaic::bayer2RGB_BilinearInterpolation(const BayerImageData &bayerImage,
                              BayerCFAPattern_e cfa,
                              RGBImageData &rgbImage) const
{
    register int shift;

    switch (bayerImage.format) {
    case FORMAT_RAW8:
        shift = 0;
        break;
    case FORMAT_RAW10_UNPACKED:
        shift = 2;
        break;
    case FORMAT_RAW12_UNPACKED:
        shift = 4;
        break;
    case FORMAT_RAW14_UNPACKED:
        shift = 6;
        break;
    case FORMAT_RAW16:
        shift = 8;
        break;
    default:
        std::cout << "Invalid bayer mem format " << bayerImage.format << std::endl;
        return -1;
    }

    if (cfa == BAYER_CFA_RGGB) {
        for (int row = 0; row < bayerImage.height; row++) {
            uint8_t *rgbLineBuf = reinterpret_cast<uint8_t *>((intptr_t)rgbImage.imageData + row * rgbImage.stride);

            for (int col = 0; col < bayerImage.width; col++) {
                uint16_t R, G, B;

                bayerRGGB2RGB_BI(row, col, bayerImage, cfa, R, G, B);

                rgbLineBuf[col * 3]     = CLAMP((R >> shift), 0, 255);
                rgbLineBuf[col * 3 + 1] = CLAMP((G >> shift), 0, 255);
                rgbLineBuf[col * 3 + 2] = CLAMP((B >> shift), 0, 255);
            }
        }
    } else if (cfa == BAYER_CFA_BGGR) {
        for (int row = 0; row < bayerImage.height; row++) {
            uint8_t *rgbLineBuf = reinterpret_cast<uint8_t *>((intptr_t)rgbImage.imageData + row * rgbImage.stride);

            for (int col = 0; col < bayerImage.width; col++) {
                uint16_t R, G, B;

                bayerBGGR2RGB_BI(row, col, bayerImage, cfa, R, G, B);

                rgbLineBuf[col * 3]     = (R >> shift);
                rgbLineBuf[col * 3 + 1] = (G >> shift);
                rgbLineBuf[col * 3 + 2] = (B >> shift);
            }
        }
    } else if (cfa == BAYER_CFA_GRBG) {
        for (int row = 0; row < bayerImage.height; row++) {
            uint8_t *rgbLineBuf = reinterpret_cast<uint8_t *>((intptr_t)rgbImage.imageData + row * rgbImage.stride);

            for (int col = 0; col < bayerImage.width; col++) {
                uint16_t R, G, B;

                bayerGRBG2RGB_BI(row, col, bayerImage, cfa, R, G, B);

                rgbLineBuf[col * 3]     = (R >> shift);
                rgbLineBuf[col * 3 + 1] = (G >> shift);
                rgbLineBuf[col * 3 + 2] = (B >> shift);
            }
        }
    } else {
        for (int row = 0; row < bayerImage.height; row++) {
            uint8_t *rgbLineBuf = reinterpret_cast<uint8_t *>((intptr_t)rgbImage.imageData + row * rgbImage.stride);

            for (int col = 0; col < bayerImage.width; col++) {
                uint16_t R, G, B;

                bayerGBRG2RGB_BI(row, col, bayerImage, cfa, R, G, B);

                rgbLineBuf[col * 3]     = (R >> shift);
                rgbLineBuf[col * 3 + 1] = (G >> shift);
                rgbLineBuf[col * 3 + 2] = (B >> shift);
            }
        }
    }

    return 0;
}
