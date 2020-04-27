/**
 * @file raw_bayer_demosaic.h
 *
 * @brief bayer demosaic algorithm
 *
 * @author Dan.Cao <caodan@linuxtoy.cn>
 */
#ifndef RAWBAYERDEMOSAIC_H
#define RAWBAYERDEMOSAIC_H

#include "imagesignalprocessor.h"

BEGIN_NAMESPACE_ISP

/**
 * @brief demosaic algorithm
 */
class Demosaic {

public:
    enum DemosaicInterPolation {
        BILINEAR_INTERPOLATION = 0,
    };

    Demosaic();
    Demosaic(enum DemosaicInterPolation);

    /**
     * @brief convert bayer image to RGB image
     * @param[in]  bayerImage
     * @param[in]  cfa
     * @param[out] rgbImage
     */
    int bayer2RGB(const BayerImageData &bayerImage, BayerCFAPattern_e cfa,
                   RGBImageData &rgbImage) const;

private:
    enum DemosaicInterPolation m_interpolationMethod;
    uint8_t *m_gammaLUT;

    int bayer2RGB_BilinearInterpolation(const BayerImageData &bayerImage,
                              BayerCFAPattern_e cfa,
                              RGBImageData &rgbImage) const;
};

END_NAMESPACE_ISP

#endif // RAWBAYERDEMOSAIC_H
