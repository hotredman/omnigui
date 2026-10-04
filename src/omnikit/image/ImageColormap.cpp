#include "ImageColormap.hpp"

namespace OmniKit {

const uint8_t* GetTurboLUT() {
    static uint8_t lut[256 * 3];
    static bool init = false;
    if (!init) {
        auto cl = [](float v) {
            v = v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v);
            return static_cast<uint8_t>(v * 255.0f + 0.5f);
        };
        for (int i = 0; i < 256; i++) {
            float x = i / 255.0f;
            float x2 = x * x;
            float x3 = x2 * x;
            float x4 = x2 * x2;
            float x5 = x4 * x;

            float r = 0.13572138f + 4.61539260f * x - 42.66032258f * x2 + 132.13108234f * x3
                      - 152.94239396f * x4 + 59.28637943f * x5;
            float g = 0.09140261f + 2.19418839f * x + 4.84296658f * x2 - 14.18503333f * x3
                      + 4.27729857f * x4 + 2.82956604f * x5;
            float b = 0.10667330f + 12.64194608f * x - 60.58204836f * x2 + 110.36276771f * x3
                      - 89.90310912f * x4 + 27.34824973f * x5;

            lut[3 * i]     = cl(r);
            lut[3 * i + 1] = cl(g);
            lut[3 * i + 2] = cl(b);
        }
        init = true;
    }
    return lut;
}

} // namespace OmniKit
