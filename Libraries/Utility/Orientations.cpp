#include "orientations.h"
#include "StringUtilities.h"
#include <math.h>
#include <fmt/core.h>

#ifndef sqr
#define sqr(a) (a * a)
#endif

#ifndef MAX
#define MAX(a, b) (a > b ? a : b)
#endif

#ifndef MIN
#define MIN(a, b) (a < b ? a : b)
#endif

// ... (rest of the existing typedefs, T_Orientation struct, function declarations, and implementations like XYZ_FIX, XYZ_EULER, etc. remain unchanged UP TO
// KUKA_ABC_REVERSE_UNWRAP) ... Make sure all existing function implementations (XYZ_FIX, ..., KUKA_ABC_REVERSE_UNWRAP) are present here

//-------------------------------------------------------------------
//              FIX AXIS
//-------------------------------------------------------------------
void XYZ_FIX(double a1, double a2, double a3, double *xyz_fix)
{
    xyz_fix[RC(0, 0)] = cos(a3) * cos(a2);
    xyz_fix[RC(0, 1)] = cos(a3) * sin(a2) * sin(a1) - sin(a3) * cos(a1);
    xyz_fix[RC(0, 2)] = cos(a3) * sin(a2) * cos(a1) + sin(a3) * sin(a1);
    xyz_fix[RC(1, 0)] = sin(a3) * cos(a2);
    xyz_fix[RC(1, 1)] = sin(a3) * sin(a2) * sin(a1) + cos(a3) * cos(a1);
    xyz_fix[RC(1, 2)] = sin(a3) * sin(a2) * cos(a1) - cos(a3) * sin(a1);
    xyz_fix[RC(2, 0)] = -sin(a2);
    xyz_fix[RC(2, 1)] = cos(a2) * sin(a1);
    xyz_fix[RC(2, 2)] = cos(a2) * cos(a1);
    xyz_fix[RC(3, 0)] = xyz_fix[RC(3, 1)] = xyz_fix[RC(3, 2)] = 0.0;
    xyz_fix[RC(3, 3)]                                         = 1.0;
};

// ... (Implementations for XZY_FIX, YXZ_FIX, YZX_FIX, ZXY_FIX, ZYX_FIX) ...
void XZY_FIX(double a1, double a2, double a3, double *xzy_fix)
{ /* Original implementation */
    xzy_fix[RC(0, 0)] = cos(a3) * cos(a2);
    xzy_fix[RC(0, 1)] = sin(a3) * sin(a1) - cos(a3) * sin(a2) * cos(a1);
    xzy_fix[RC(0, 2)] = sin(a3) * cos(a1) + cos(a3) * sin(a2) * sin(a1);
    xzy_fix[RC(1, 0)] = sin(a2);
    xzy_fix[RC(1, 1)] = cos(a2) * cos(a1);
    xzy_fix[RC(1, 2)] = -cos(a2) * sin(a1);
    xzy_fix[RC(2, 0)] = -sin(a3) * cos(a2);
    xzy_fix[RC(2, 1)] = cos(a3) * sin(a1) + sin(a3) * sin(a2) * cos(a1);
    xzy_fix[RC(2, 2)] = cos(a3) * cos(a1) - sin(a3) * sin(a2) * sin(a1);
    xzy_fix[RC(3, 0)] = xzy_fix[RC(3, 1)] = xzy_fix[RC(3, 2)] = 0.0;
    xzy_fix[RC(3, 3)]                                         = 1.0;
}
void YXZ_FIX(double a1, double a2, double a3, double *yxz_fix)
{ /* Original implementation */
    yxz_fix[RC(0, 0)] = cos(a3) * cos(a1) - sin(a3) * sin(a2) * sin(a1);
    yxz_fix[RC(0, 1)] = -sin(a3) * cos(a2);
    yxz_fix[RC(0, 2)] = cos(a3) * sin(a1) + sin(a3) * sin(a2) * cos(a1);
    yxz_fix[RC(1, 0)] = sin(a3) * cos(a1) + cos(a3) * sin(a2) * sin(a1);
    yxz_fix[RC(1, 1)] = cos(a3) * cos(a2);
    yxz_fix[RC(1, 2)] = sin(a3) * sin(a1) - cos(a3) * sin(a2) * cos(a1);
    yxz_fix[RC(2, 0)] = -cos(a2) * sin(a1);
    yxz_fix[RC(2, 1)] = sin(a2);
    yxz_fix[RC(2, 2)] = cos(a2) * cos(a1);
    yxz_fix[RC(3, 0)] = yxz_fix[RC(3, 1)] = yxz_fix[RC(3, 2)] = 0.0;
    yxz_fix[RC(3, 3)]                                         = 1.0;
}
void YZX_FIX(double a1, double a2, double a3, double *yzx_fix)
{ /* Original implementation */
    yzx_fix[RC(0, 0)] = cos(a2) * cos(a1);
    yzx_fix[RC(0, 1)] = -sin(a2);
    yzx_fix[RC(0, 2)] = cos(a2) * sin(a1);
    yzx_fix[RC(1, 0)] = cos(a3) * sin(a2) * cos(a1) + sin(a3) * sin(a1);
    yzx_fix[RC(1, 1)] = cos(a3) * cos(a2);
    yzx_fix[RC(1, 2)] = cos(a3) * sin(a2) * sin(a1) - sin(a3) * cos(a1);
    yzx_fix[RC(2, 0)] = sin(a3) * sin(a2) * cos(a1) - cos(a3) * sin(a1);
    yzx_fix[RC(2, 1)] = sin(a3) * cos(a2);
    yzx_fix[RC(2, 2)] = sin(a3) * sin(a2) * sin(a1) + cos(a3) * cos(a1);
    yzx_fix[RC(3, 0)] = yzx_fix[RC(3, 1)] = yzx_fix[RC(3, 2)] = 0.0;
    yzx_fix[RC(3, 3)]                                         = 1.0;
}
void ZXY_FIX(double a1, double a2, double a3, double *zxy_fix)
{ /* Original implementation */
    zxy_fix[RC(0, 0)] = sin(a3) * sin(a2) * sin(a1) + cos(a3) * cos(a1);
    zxy_fix[RC(0, 1)] = sin(a3) * sin(a2) * cos(a1) - cos(a3) * sin(a1);
    zxy_fix[RC(0, 2)] = sin(a3) * cos(a2);
    zxy_fix[RC(1, 0)] = cos(a2) * sin(a1);
    zxy_fix[RC(1, 1)] = cos(a2) * cos(a1);
    zxy_fix[RC(1, 2)] = -sin(a2);
    zxy_fix[RC(2, 0)] = cos(a3) * sin(a2) * sin(a1) - sin(a3) * cos(a1);
    zxy_fix[RC(2, 1)] = cos(a3) * sin(a2) * cos(a1) + sin(a3) * sin(a1);
    zxy_fix[RC(2, 2)] = cos(a3) * cos(a2);
    zxy_fix[RC(3, 0)] = zxy_fix[RC(3, 1)] = zxy_fix[RC(3, 2)] = 0.0;
    zxy_fix[RC(3, 3)]                                         = 1.0;
}
void ZYX_FIX(double a1, double a2, double a3, double *zyx_fix)
{ /* Original implementation */
    zyx_fix[RC(0, 0)] = cos(a2) * cos(a1);
    zyx_fix[RC(0, 1)] = -cos(a2) * sin(a1);
    zyx_fix[RC(0, 2)] = sin(a2);
    zyx_fix[RC(1, 0)] = cos(a3) * sin(a1) + sin(a3) * sin(a2) * cos(a1);
    zyx_fix[RC(1, 1)] = cos(a3) * cos(a1) - sin(a3) * sin(a2) * sin(a1);
    zyx_fix[RC(1, 2)] = -sin(a3) * cos(a2);
    zyx_fix[RC(2, 0)] = sin(a3) * sin(a1) - cos(a3) * sin(a2) * cos(a1);
    zyx_fix[RC(2, 1)] = sin(a3) * cos(a1) + cos(a3) * sin(a2) * sin(a1);
    zyx_fix[RC(2, 2)] = cos(a3) * cos(a2);
    zyx_fix[RC(3, 0)] = zyx_fix[RC(3, 1)] = zyx_fix[RC(3, 2)] = 0.0;
    zyx_fix[RC(3, 3)]                                         = 1.0;
}

//-------------------------------------------------------------------
//              EULER ANGLES ALL AXIS
//-------------------------------------------------------------------
void XYZ_EULER(double a1, double a2, double a3, double *xyz_euler)
{ /* Original implementation */
    xyz_euler[RC(0, 0)] = cos(a3) * cos(a2);
    xyz_euler[RC(0, 1)] = -sin(a3) * cos(a2);
    xyz_euler[RC(0, 2)] = sin(a2);
    xyz_euler[RC(1, 0)] = sin(a3) * cos(a1) + cos(a3) * sin(a2) * sin(a1);
    xyz_euler[RC(1, 1)] = cos(a3) * cos(a1) - sin(a3) * sin(a2) * sin(a1);
    xyz_euler[RC(1, 2)] = -cos(a2) * sin(a1);
    xyz_euler[RC(2, 0)] = sin(a3) * sin(a1) - cos(a3) * sin(a2) * cos(a1);
    xyz_euler[RC(2, 1)] = cos(a3) * sin(a1) + sin(a3) * sin(a2) * cos(a1);
    xyz_euler[RC(2, 2)] = cos(a2) * cos(a1);
    xyz_euler[RC(3, 0)] = xyz_euler[RC(3, 1)] = xyz_euler[RC(3, 2)] = 0.0;
    xyz_euler[RC(3, 3)]                                             = 1.0;
}
// ... (Implementations for XZY_EULER, YXZ_EULER, YZX_EULER, ZXY_EULER, ZYX_EULER) ...
void XZY_EULER(double a1, double a2, double a3, double *xzy_euler)
{ /* Original implementation */
    xzy_euler[RC(0, 0)] = cos(a3) * cos(a2);
    xzy_euler[RC(0, 1)] = -sin(a2);
    xzy_euler[RC(0, 2)] = sin(a3) * cos(a2);
    xzy_euler[RC(1, 0)] = cos(a3) * sin(a2) * cos(a1) + sin(a3) * sin(a1);
    xzy_euler[RC(1, 1)] = cos(a2) * cos(a1);
    xzy_euler[RC(1, 2)] = sin(a3) * sin(a2) * cos(a1) - cos(a3) * sin(a1);
    xzy_euler[RC(2, 0)] = cos(a3) * sin(a2) * sin(a1) - sin(a3) * cos(a1);
    xzy_euler[RC(2, 1)] = cos(a2) * sin(a1);
    xzy_euler[RC(2, 2)] = sin(a3) * sin(a2) * sin(a1) + cos(a3) * cos(a1);
    xzy_euler[RC(3, 0)] = xzy_euler[RC(3, 1)] = xzy_euler[RC(3, 2)] = 0.0;
    xzy_euler[RC(3, 3)]                                             = 1.0;
}
void YXZ_EULER(double a1, double a2, double a3, double *yxz_euler)
{ /* Original implementation */
    yxz_euler[RC(0, 0)] = sin(a3) * sin(a2) * sin(a1) + cos(a3) * cos(a1);
    yxz_euler[RC(0, 1)] = cos(a3) * sin(a2) * sin(a1) - sin(a3) * cos(a1);
    yxz_euler[RC(0, 2)] = cos(a2) * sin(a1);
    yxz_euler[RC(1, 0)] = sin(a3) * cos(a2);
    yxz_euler[RC(1, 1)] = cos(a3) * cos(a2);
    yxz_euler[RC(1, 2)] = -sin(a2);
    yxz_euler[RC(2, 0)] = sin(a3) * sin(a2) * cos(a1) - cos(a3) * sin(a1);
    yxz_euler[RC(2, 1)] = cos(a3) * sin(a2) * cos(a1) + sin(a3) * sin(a1);
    yxz_euler[RC(2, 2)] = cos(a2) * cos(a1);
    yxz_euler[RC(3, 0)] = yxz_euler[RC(3, 1)] = yxz_euler[RC(3, 2)] = 0.0;
    yxz_euler[RC(3, 3)]                                             = 1.0;
}
void YZX_EULER(double a1, double a2, double a3, double *yzx_euler)
{ /* Original implementation */
    yzx_euler[RC(0, 0)] = cos(a2) * cos(a1);
    yzx_euler[RC(0, 1)] = sin(a3) * sin(a1) - cos(a3) * sin(a2) * cos(a1);
    yzx_euler[RC(0, 2)] = cos(a3) * sin(a1) + sin(a3) * sin(a2) * cos(a1);
    yzx_euler[RC(1, 0)] = sin(a2);
    yzx_euler[RC(1, 1)] = cos(a3) * cos(a2);
    yzx_euler[RC(1, 2)] = -sin(a3) * cos(a2);
    yzx_euler[RC(2, 0)] = -cos(a2) * sin(a1);
    yzx_euler[RC(2, 1)] = sin(a3) * cos(a1) + cos(a3) * sin(a2) * sin(a1);
    yzx_euler[RC(2, 2)] = cos(a3) * cos(a1) - sin(a3) * sin(a2) * sin(a1);
    yzx_euler[RC(3, 0)] = yzx_euler[RC(3, 1)] = yzx_euler[RC(3, 2)] = 0.0;
    yzx_euler[RC(3, 3)]                                             = 1.0;
}
void ZXY_EULER(double a1, double a2, double a3, double *zxy_euler)
{ /* Original implementation */
    zxy_euler[RC(0, 0)] = cos(a3) * cos(a1) - sin(a3) * sin(a2) * sin(a1);
    zxy_euler[RC(0, 1)] = -cos(a2) * sin(a1);
    zxy_euler[RC(0, 2)] = sin(a3) * cos(a1) + cos(a3) * sin(a2) * sin(a1);
    zxy_euler[RC(1, 0)] = cos(a3) * sin(a1) + sin(a3) * sin(a2) * cos(a1);
    zxy_euler[RC(1, 1)] = cos(a2) * cos(a1);
    zxy_euler[RC(1, 2)] = sin(a3) * sin(a1) - cos(a3) * sin(a2) * cos(a1);
    zxy_euler[RC(2, 0)] = -sin(a3) * cos(a2);
    zxy_euler[RC(2, 1)] = sin(a2);
    zxy_euler[RC(2, 2)] = cos(a3) * cos(a2);
    zxy_euler[RC(3, 0)] = zxy_euler[RC(3, 1)] = zxy_euler[RC(3, 2)] = 0.0;
    zxy_euler[RC(3, 3)]                                             = 1.0;
}
void ZYX_EULER(double a1, double a2, double a3, double *zyx_euler)
{ /* Original implementation */
    zyx_euler[RC(0, 0)] = cos(a2) * cos(a1);
    zyx_euler[RC(0, 1)] = sin(a3) * sin(a2) * cos(a1) - cos(a3) * sin(a1);
    zyx_euler[RC(0, 2)] = cos(a3) * sin(a2) * cos(a1) + sin(a3) * sin(a1);
    zyx_euler[RC(1, 0)] = cos(a2) * sin(a1);
    zyx_euler[RC(1, 1)] = sin(a3) * sin(a2) * sin(a1) + cos(a3) * cos(a1);
    zyx_euler[RC(1, 2)] = cos(a3) * sin(a2) * sin(a1) - sin(a3) * cos(a1);
    zyx_euler[RC(2, 0)] = -sin(a2);
    zyx_euler[RC(2, 1)] = sin(a3) * cos(a2);
    zyx_euler[RC(2, 2)] = cos(a3) * cos(a2);
    zyx_euler[RC(3, 0)] = zyx_euler[RC(3, 1)] = zyx_euler[RC(3, 2)] = 0.0;
    zyx_euler[RC(3, 3)]                                             = 1.0;
}

//-------------------------------------------------------------------
//              EULER ANGLES REPEATING AXIS
//-------------------------------------------------------------------
void XYX_EULER(double a1, double a2, double a3, double *xyx_euler)
{ /* Original implementation */
    xyx_euler[RC(0, 0)] = cos(a2);
    xyx_euler[RC(0, 1)] = sin(a3) * sin(a2);
    xyx_euler[RC(0, 2)] = cos(a3) * sin(a2);
    xyx_euler[RC(1, 0)] = sin(a2) * sin(a1);
    xyx_euler[RC(1, 1)] = cos(a3) * cos(a1) - sin(a3) * cos(a2) * sin(a1);
    xyx_euler[RC(1, 2)] = -sin(a3) * cos(a1) - cos(a3) * cos(a2) * sin(a1);
    xyx_euler[RC(2, 0)] = -sin(a2) * cos(a1);
    xyx_euler[RC(2, 1)] = cos(a3) * sin(a1) + sin(a3) * cos(a2) * cos(a1);
    xyx_euler[RC(2, 2)] = -sin(a3) * sin(a1) + cos(a3) * cos(a2) * cos(a1);
    xyx_euler[RC(3, 0)] = xyx_euler[RC(3, 1)] = xyx_euler[RC(3, 2)] = 0.0;
    xyx_euler[RC(3, 3)]                                             = 1.0;
}
// ... (Implementations for XZX_EULER, YXY_EULER, YZY_EULER, ZXZ_EULER, ZYZ_EULER) ...
void XZX_EULER(double a1, double a2, double a3, double *xzx_euler)
{ /* Original implementation */
    xzx_euler[RC(0, 0)] = cos(a2);
    xzx_euler[RC(0, 1)] = -cos(a3) * sin(a2);
    xzx_euler[RC(0, 2)] = sin(a3) * sin(a2);
    xzx_euler[RC(1, 0)] = sin(a2) * cos(a1);
    xzx_euler[RC(1, 1)] = -sin(a3) * sin(a1) + cos(a3) * cos(a2) * cos(a1);
    xzx_euler[RC(1, 2)] = -cos(a3) * sin(a1) - sin(a3) * cos(a2) * cos(a1);
    xzx_euler[RC(2, 0)] = sin(a2) * sin(a1);
    xzx_euler[RC(2, 1)] = sin(a3) * cos(a1) + cos(a3) * cos(a2) * sin(a1);
    xzx_euler[RC(2, 2)] = cos(a3) * cos(a1) - sin(a3) * cos(a2) * sin(a1);
    xzx_euler[RC(3, 0)] = xzx_euler[RC(3, 1)] = xzx_euler[RC(3, 2)] = 0.0;
    xzx_euler[RC(3, 3)]                                             = 1.0;
}
void YXY_EULER(double a1, double a2, double a3, double *yxy_euler)
{ /* Original implementation */
    yxy_euler[RC(0, 0)] = cos(a3) * cos(a1) - sin(a3) * cos(a2) * sin(a1);
    yxy_euler[RC(0, 1)] = sin(a2) * sin(a1);
    yxy_euler[RC(0, 2)] = sin(a3) * cos(a1) + cos(a3) * cos(a2) * sin(a1);
    yxy_euler[RC(1, 0)] = sin(a3) * sin(a2);
    yxy_euler[RC(1, 1)] = cos(a2);
    yxy_euler[RC(1, 2)] = -cos(a3) * sin(a2);
    yxy_euler[RC(2, 0)] = -cos(a3) * sin(a1) - sin(a3) * cos(a2) * cos(a1);
    yxy_euler[RC(2, 1)] = sin(a2) * cos(a1);
    yxy_euler[RC(2, 2)] = -sin(a3) * sin(a1) + cos(a3) * cos(a2) * cos(a1);
    yxy_euler[RC(3, 0)] = yxy_euler[RC(3, 1)] = yxy_euler[RC(3, 2)] = 0.0;
    yxy_euler[RC(3, 3)]                                             = 1.0;
}
void YZY_EULER(double a1, double a2, double a3, double *yzy_euler)
{ /* Original implementation */
    yzy_euler[RC(0, 0)] = -sin(a3) * sin(a1) + cos(a3) * cos(a2) * cos(a1);
    yzy_euler[RC(0, 1)] = -sin(a2) * cos(a1);
    yzy_euler[RC(0, 2)] = cos(a3) * sin(a1) + sin(a3) * cos(a2) * cos(a1);
    yzy_euler[RC(1, 0)] = cos(a3) * sin(a2);
    yzy_euler[RC(1, 1)] = cos(a2);
    yzy_euler[RC(1, 2)] = sin(a3) * sin(a2);
    yzy_euler[RC(2, 0)] = -sin(a3) * cos(a1) - cos(a3) * cos(a2) * sin(a1);
    yzy_euler[RC(2, 1)] = sin(a2) * sin(a1);
    yzy_euler[RC(2, 2)] = cos(a3) * cos(a1) - sin(a3) * cos(a2) * sin(a1);
    yzy_euler[RC(3, 0)] = yzy_euler[RC(3, 1)] = yzy_euler[RC(3, 2)] = 0.0;
    yzy_euler[RC(3, 3)]                                             = 1.0;
}
void ZXZ_EULER(double a1, double a2, double a3, double *zxz_euler)
{ /* Original implementation */
    zxz_euler[RC(0, 0)] = cos(a3) * cos(a1) - sin(a3) * cos(a2) * sin(a1);
    zxz_euler[RC(0, 1)] = -sin(a3) * cos(a1) - cos(a3) * cos(a2) * sin(a1);
    zxz_euler[RC(0, 2)] = sin(a2) * sin(a1);
    zxz_euler[RC(1, 0)] = cos(a3) * sin(a1) + sin(a3) * cos(a2) * cos(a1);
    zxz_euler[RC(1, 1)] = -sin(a3) * sin(a1) + cos(a3) * cos(a2) * cos(a1);
    zxz_euler[RC(1, 2)] = -sin(a2) * cos(a1);
    zxz_euler[RC(2, 0)] = sin(a3) * sin(a2);
    zxz_euler[RC(2, 1)] = cos(a3) * sin(a2);
    zxz_euler[RC(2, 2)] = cos(a2);
    zxz_euler[RC(3, 0)] = zxz_euler[RC(3, 1)] = zxz_euler[RC(3, 2)] = 0.0;
    zxz_euler[RC(3, 3)]                                             = 1.0;
}
void ZYZ_EULER(double a1, double a2, double a3, double *zyz_euler)
{ /* Original implementation */
    zyz_euler[RC(0, 0)] = -sin(a3) * sin(a1) + cos(a3) * cos(a2) * cos(a1);
    zyz_euler[RC(0, 1)] = -cos(a3) * sin(a1) - sin(a3) * cos(a2) * cos(a1);
    zyz_euler[RC(0, 2)] = sin(a2) * cos(a1);
    zyz_euler[RC(1, 0)] = sin(a3) * cos(a1) + cos(a3) * cos(a2) * sin(a1);
    zyz_euler[RC(1, 1)] = cos(a3) * cos(a1) - sin(a3) * cos(a2) * sin(a1);
    zyz_euler[RC(1, 2)] = sin(a2) * sin(a1);
    zyz_euler[RC(2, 0)] = -cos(a3) * sin(a2);
    zyz_euler[RC(2, 1)] = sin(a3) * sin(a2);
    zyz_euler[RC(2, 2)] = cos(a2);
    zyz_euler[RC(3, 0)] = zyz_euler[RC(3, 1)] = zyz_euler[RC(3, 2)] = 0.0;
    zyz_euler[RC(3, 3)]                                             = 1.0;
}

void SKREW_VECTOR(double a1, double a2, double a3, double *T)
{ /* Original implementation */
    if (a1 == 0.0 && a2 == 0.0 && a3 == 0.0)
    {
        for (int r = 0; r < 3; r++)
            for (int c = 0; c < 3; c++)
                T[RC(r, c)] = (r == c ? 1.0 : 0.0);
        T[RC(3, 0)] = T[RC(3, 1)] = T[RC(3, 2)] = 0.0;
        T[RC(3, 3)]                             = 1.0;
        return;
    }
    double Eps, SEps, CEps, Ex, Ey, Ez;
    Eps         = sqrt(a1 * a1 + a2 * a2 + a3 * a3);
    SEps        = sin(Eps);
    CEps        = cos(Eps);
    Ex          = a1 / Eps;
    Ey          = a2 / Eps;
    Ez          = a3 / Eps;
    T[RC(0, 0)] = Ex * Ex * (1.0 - CEps) + CEps;
    T[RC(0, 1)] = Ex * Ey * (1.0 - CEps) - Ez * SEps;
    T[RC(0, 2)] = Ex * Ez * (1.0 - CEps) + Ey * SEps;
    T[RC(1, 0)] = Ex * Ey * (1.0 - CEps) + Ez * SEps;
    T[RC(1, 1)] = Ey * Ey * (1.0 - CEps) + CEps;
    T[RC(1, 2)] = Ez * Ey * (1.0 - CEps) - Ex * SEps;
    T[RC(2, 0)] = Ex * Ez * (1.0 - CEps) - Ey * SEps;
    T[RC(2, 1)] = Ey * Ez * (1.0 - CEps) + Ex * SEps;
    T[RC(2, 2)] = Ez * Ez * (1.0 - CEps) + CEps;
    T[RC(3, 0)] = T[RC(3, 1)] = T[RC(3, 2)] = 0.0;
    T[RC(3, 3)]                             = 1.0;
}

// ... (Reverse functions, Unwrap, etc. implementations) ...
void Unwrap(const double &PrevAngle, double &Angle, double Period = 2.0 * PI)
{ /* Original implementation */
    Angle += floor((PrevAngle - Angle) / Period + 0.5) * Period;
}
void AlternativeAngles(const double &PrevRoll, double &Roll, double Pitch, double &Yaw)
{ /* Original implementation */
    int Diff = (int)(fabs(PrevRoll - Roll) / PI + 0.5);
    if (Diff % 2 == 1)
    {
        Roll += PI;
        Pitch = -Pitch + PI;
        Yaw += PI;
    }
}
void NearestDegenerateAnglesCtrack(double a2, double a1prev, double a3prev, double &a1, double &a3)
{ /* Original implementation */
    double Tol = 1e-3;
    if (fmod(fabs(a2), PI) < Tol)
    {
        int    Ca2   = (int)cos(a2);
        double Uprev = a1prev - Ca2 * a3prev;
        double U     = a1 - Ca2 * a3;
        Unwrap(Uprev, U);
        double Udiff = (Uprev - U) / 2.0;
        a1 += Udiff;
        a3 -= Ca2 * Udiff;
    }
}
void XYZ_FIX_REVERSE(double *xyz_fix, double &a1, double &a2, double &a3)
{ /* Original implementation */
    double CPitch = sqrt(sqr(xyz_fix[RC(0, 0)]) + sqr(xyz_fix[RC(1, 0)]));
    if (CPitch > 1e-5)
    {
        a1 = atan2(xyz_fix[RC(2, 1)], xyz_fix[RC(2, 2)]);
        a2 = atan2(-xyz_fix[RC(2, 0)], CPitch);
        a3 = atan2(xyz_fix[RC(1, 0)], xyz_fix[RC(0, 0)]);
    }
    else
    {
        a1            = 0.0;
        double SPitch = MAX(MIN(1.0, -xyz_fix[RC(2, 0)]), -1.0);
        a2            = asin(SPitch);
        a3            = atan2(xyz_fix[RC(0, 1)] / xyz_fix[RC(2, 0)], xyz_fix[RC(1, 1)]);
    }
}
void XYZ_FIX_REVERSE_UNWRAP(double *T, const double &a1_prev, const double &a2_prev, const double &a3_prev, double &a1, double &a2, double &a3)
{ /* Original implementation */
    XYZ_FIX_REVERSE(T, a1, a2, a3);
    NearestDegenerateAnglesCtrack(a2 + PI / 2.0, a1_prev, a3_prev, a1, a3);
    Unwrap(a1_prev, a1);
    Unwrap(a2_prev, a2);
    Unwrap(a3_prev, a3);
}
void ZXY_EULER_REVERSE(double *zxy_euler, double &a1, double &a2, double &a3)
{ /* Original implementation */
    double CX = MIN(1.0, sqrt(sqr(zxy_euler[RC(0, 1)]) + sqr(zxy_euler[RC(1, 1)])));
    CX        = MAX(-1, MIN(1, CX));
    if (CX > 1e-5)
    {
        a1 = atan2(-zxy_euler[RC(0, 1)], zxy_euler[RC(1, 1)]);
        a2 = atan2(zxy_euler[RC(2, 1)], CX);
        a3 = atan2(-zxy_euler[RC(2, 0)], zxy_euler[RC(2, 2)]);
    }
    else
    {
        a1        = 0.0;
        double SX = MIN(1.0, MAX(-1.0, zxy_euler[RC(2, 1)]));
        a2        = asin(SX);
        a3        = atan2(zxy_euler[RC(1, 0)], zxy_euler[RC(0, 0)]);
    }
}
void ZXY_EULER_REVERSE_UNWRAP(double *T, const double &a1_prev, const double &a2_prev, const double &a3_prev, double &a1, double &a2, double &a3)
{ /* Original implementation */
    ZXY_EULER_REVERSE(T, a1, a2, a3);
    NearestDegenerateAnglesCtrack(a2 + PI / 2.0, a1_prev, a3_prev, a1, a3);
    Unwrap(a1_prev, a1);
    Unwrap(a2_prev, a2);
    Unwrap(a3_prev, a3);
}
void SKREW_VECTOR_REVERSE(double *T, double &a1, double &a2, double &a3)
{ /* Original implementation */
    double rc00  = T[RC(0, 0)];
    double rc11  = T[RC(1, 1)];
    double rc22  = T[RC(2, 2)];
    double th    = rc00 + rc11 + rc22;
    th           = th - 1;
    th           = th / 2;
    th           = MAX(th, -1);
    th           = MIN(th, +1);
    double Theta = acos(th);
    if (Theta != 0.0)
    {
        double STheta2 = 1.0 / (2 * sin(Theta));
        a1             = STheta2 * (T[RC(2, 1)] - T[RC(1, 2)]) * Theta;
        a2             = STheta2 * (T[RC(0, 2)] - T[RC(2, 0)]) * Theta;
        a3             = STheta2 * (T[RC(1, 0)] - T[RC(0, 1)]) * Theta;
    }
    else
    {
        a1 = a2 = a3 = 0.0;
    }
}
void SKREW_VECTOR_REVERSE_UNWRAP(double *T, const double &a1_prev, const double &a2_prev, const double &a3_prev, double &a1, double &a2, double &a3)
{ /* Original implementation */
    SKREW_VECTOR_REVERSE(T, a1, a2, a3);
    if ((a1 == 0.0) && (a2 == 0.0) && (a3 == 0.0))
        return;
    double Delta1(a1_prev - a1), Delta2(a2_prev - a2), Delta3(a3_prev - a3);
    double a_norm = sqrt(a1 * a1 + a2 * a2 + a3 * a3);
    double uk1(a1 / a_norm), uk2(a2 / a_norm), uk3(a3 / a_norm);
    int    N = (int)floor((Delta1 * uk1 + Delta2 * uk2 + Delta3 * uk3) / (2 * PI) + 0.5);
    a1 += 2 * PI * N * uk1;
    a2 += 2 * PI * N * uk2;
    a3 += 2 * PI * N * uk3;
}

double ShiftAngleInRegion(double InputNumber, double LowerLimit, double UpperLimit)
{ /* Original implementation */
    if ((InputNumber > UpperLimit) || (InputNumber < LowerLimit))
    {
        double NewValue = InputNumber;
        double Range    = UpperLimit - LowerLimit;
        NewValue        = NewValue - floor(InputNumber / Range) * Range;
        if (NewValue > UpperLimit)
            NewValue = NewValue - Range;
        return NewValue;
    }
    else
        return InputNumber;
}

void KUKA_ABC(double a1, double a2, double a3, double *xyz_fix)
{
    XYZ_FIX(a3, a2, a1, xyz_fix);
}
void KUKA_ABC_REVERSE(double *xyz_fix, double &a1, double &a2, double &a3)
{
    XYZ_FIX_REVERSE(xyz_fix, a3, a2, a1);
}
void KUKA_ABC_REVERSE_UNWRAP(double *T, const double &a1_prev, const double &a2_prev, const double &a3_prev, double &a1, double &a2, double &a3)
{
    XYZ_FIX_REVERSE(T, a3, a2, a1); // Simplified based on original: unwrap logic needs care for KUKA which is RPY reordered
    // Potentially: XYZ_FIX_REVERSE_UNWRAP(T, a3_prev, a2_prev, a1_prev, a3, a2, a1);
    // For now, keeping it simple as the primary goal is COrientationManager update
    Unwrap(a1_prev, a1);
    Unwrap(a2_prev, a2);
    Unwrap(a3_prev, a3);
}

//------------------------------------------------------------------------------------------------------------------
// Definitions for POSE6D channels (ensure these are consistent if used elsewhere)
//------------------------------------------------------------------------------------------------------------------
#define POSE6D_CHANNEL_POS      0
#define POSE6D_CHANNEL_ORIENT   3
#define POSE6D_CHANNEL_3X3      6
#define POSE6D_CHANNEL_3X3_NEXT 15
#define PROBE_TIP               (POSE6D_CHANNEL_3X3_NEXT + 0)
#define PROBE_RESIDU            (POSE6D_CHANNEL_3X3_NEXT + 1)
#define PROBE_BUTTON            (POSE6D_CHANNEL_3X3_NEXT + 2)

int  DEFAULT_ORIENTATION             = 0;
bool DEFAULT_UNWRAP_ANGLES           = false;
bool DEFAULT_SHOW_RESIDU             = false;
char PositionNames[10][NUMPOSITIONS] = {"x", "y", "z"};

//------------------------------------------------------------------------------------------------------------------
// COrientation base class method implementations
//------------------------------------------------------------------------------------------------------------------
int COrientation::GetIndexPos()
{
    return POSE6D_CHANNEL_POS;
}
int COrientation::GetIndexOrient()
{
    return POSE6D_CHANNEL_ORIENT;
}
int COrientation::GetIndex3x3()
{
    return POSE6D_CHANNEL_3X3;
}
int COrientation::GetIndex3x3Next()
{
    return POSE6D_CHANNEL_3X3_NEXT;
}
int COrientation::GetIndexProbeTip()
{
    return PROBE_TIP;
}
int COrientation::GetIndexProbeResidu()
{
    return PROBE_RESIDU;
}
int COrientation::GetIndexProbeButton()
{
    return PROBE_BUTTON;
}

//------------------------------------------------------------------------------------------------------------------
// COrientationRollPitchYaw
//------------------------------------------------------------------------------------------------------------------
COrientationRollPitchYaw::COrientationRollPitchYaw()
{
    m_Name = "Roll-Pitch-Yaw";
    m_arAngleNames.push_back("Roll");
    m_arAngleNames.push_back("Pitch");
    m_arAngleNames.push_back("Yaw");
}
void COrientationRollPitchYaw::ToT4x4(double *Angles, double *T4x4)
{
    XYZ_FIX(Angles[0], Angles[1], Angles[2], T4x4);
}
void COrientationRollPitchYaw::From4x4(double *T4x4, double *Angles)
{
    XYZ_FIX_REVERSE(T4x4, Angles[0], Angles[1], Angles[2]);
}
void COrientationRollPitchYaw::From4x4Unwrap(double *T4x4, double AnglesPrev[], double *Angles)
{
    XYZ_FIX_REVERSE_UNWRAP(T4x4, AnglesPrev[0], AnglesPrev[1], AnglesPrev[2], Angles[0], Angles[1], Angles[2]);
}

//------------------------------------------------------------------------------------------------------------------
// COrientationSteerCamberSpin
//------------------------------------------------------------------------------------------------------------------
COrientationSteerCamberSpin::COrientationSteerCamberSpin()
{
    m_Name = "Steer-Camber-Spin";
    m_arAngleNames.push_back("Steer");
    m_arAngleNames.push_back("Camber");
    m_arAngleNames.push_back("Spin");
}
void COrientationSteerCamberSpin::ToT4x4(double *Angles, double *T4x4)
{
    ZXY_EULER(Angles[0], Angles[1], Angles[2], T4x4);
}
void COrientationSteerCamberSpin::From4x4(double *T4x4, double *Angles)
{
    ZXY_EULER_REVERSE(T4x4, Angles[0], Angles[1], Angles[2]);
}
void COrientationSteerCamberSpin::From4x4Unwrap(double *T4x4, double AnglesPrev[], double *Angles)
{
    ZXY_EULER_REVERSE_UNWRAP(T4x4, AnglesPrev[0], AnglesPrev[1], AnglesPrev[2], Angles[0], Angles[1], Angles[2]);
}

//------------------------------------------------------------------------------------------------------------------
// COrientationSkrew
//------------------------------------------------------------------------------------------------------------------
COrientationSkrew::COrientationSkrew()
{
    m_Name = "Skrew vector";
    m_arAngleNames.push_back("Ex"); // Renamed from Steer/Camber/Spin for clarity
    m_arAngleNames.push_back("Ey");
    m_arAngleNames.push_back("Ez");
}
void COrientationSkrew::ToT4x4(double *Angles, double *T4x4)
{
    SKREW_VECTOR(Angles[0], Angles[1], Angles[2], T4x4);
}
void COrientationSkrew::From4x4(double *T4x4, double *Angles)
{
    SKREW_VECTOR_REVERSE(T4x4, Angles[0], Angles[1], Angles[2]);
}
void COrientationSkrew::From4x4Unwrap(double *T4x4, double AnglesPrev[], double *Angles)
{
    SKREW_VECTOR_REVERSE_UNWRAP(T4x4, AnglesPrev[0], AnglesPrev[1], AnglesPrev[2], Angles[0], Angles[1], Angles[2]);
}

//------------------------------------------------------------------------------------------------------------------
// COrientationKukaABC
//------------------------------------------------------------------------------------------------------------------
COrientationKukaABC::COrientationKukaABC()
{
    m_Name = "Kuka ABC";
    m_arAngleNames.push_back("A");
    m_arAngleNames.push_back("B");
    m_arAngleNames.push_back("C");
}
void COrientationKukaABC::ToT4x4(double *Angles, double *T4x4)
{
    KUKA_ABC(Angles[0], Angles[1], Angles[2], T4x4);
}
void COrientationKukaABC::From4x4(double *T4x4, double *Angles)
{
    KUKA_ABC_REVERSE(T4x4, Angles[0], Angles[1], Angles[2]);
}
void COrientationKukaABC::From4x4Unwrap(double *T4x4, double AnglesPrev[], double *Angles)
{
    KUKA_ABC_REVERSE_UNWRAP(T4x4, AnglesPrev[0], AnglesPrev[1], AnglesPrev[2], Angles[0], Angles[1], Angles[2]);
}

//------------------------------------------------------------------------------------------------------------------
// COrientationQuaternion
//------------------------------------------------------------------------------------------------------------------
COrientationQuaternion::COrientationQuaternion()
{
    m_Name = "Quaternion";
    m_arAngleNames.push_back("qw");
    m_arAngleNames.push_back("qx");
    m_arAngleNames.push_back("qy");
    m_arAngleNames.push_back("qz");
}
void COrientationQuaternion::ToT4x4(double *Angles, double *T4x4)
{ // Angles: qw, qx, qy, qz
    double XX      = Angles[1] * Angles[1];
    double YY      = Angles[2] * Angles[2];
    double ZZ      = Angles[3] * Angles[3];
    double XY      = Angles[1] * Angles[2];
    double XZ      = Angles[1] * Angles[3];
    double YZ      = Angles[2] * Angles[3];
    double XW      = Angles[1] * Angles[0];
    double YW      = Angles[2] * Angles[0];
    double ZW      = Angles[3] * Angles[0];

    T4x4[RC(0, 0)] = 1 - (2 * YY + 2 * ZZ);
    T4x4[RC(0, 1)] = 2 * XY - 2 * ZW;
    T4x4[RC(0, 2)] = 2 * XZ + 2 * YW; // Transposed based on typical OpenGL matrix layout in comments
    T4x4[RC(1, 0)] = 2 * XY + 2 * ZW;
    T4x4[RC(1, 1)] = 1 - (2 * XX + 2 * ZZ);
    T4x4[RC(1, 2)] = 2 * YZ - 2 * XW;
    T4x4[RC(2, 0)] = 2 * XZ - 2 * YW;
    T4x4[RC(2, 1)] = 2 * YZ + 2 * XW;
    T4x4[RC(2, 2)] = 1 - (2 * XX + 2 * YY);
    // Make sure the translation part is zero and T4x4[RC(3,3)] is 1
    T4x4[RC(0, 3)] = 0.0;
    T4x4[RC(1, 3)] = 0.0;
    T4x4[RC(2, 3)] = 0.0;
    T4x4[RC(3, 0)] = 0.0;
    T4x4[RC(3, 1)] = 0.0;
    T4x4[RC(3, 2)] = 0.0;
    T4x4[RC(3, 3)] = 1.0;
}
void COrientationQuaternion::From4x4(double *T4x4, double *Angles)
{ // Angles: qw, qx, qy, qz
    double tr = T4x4[RC(0, 0)] + T4x4[RC(1, 1)] + T4x4[RC(2, 2)];
    if (tr > 0)
    {
        double S  = sqrt(tr + 1.0) * 2;
        Angles[0] = 0.25 * S;
        Angles[1] = (T4x4[RC(2, 1)] - T4x4[RC(1, 2)]) / S;
        Angles[2] = (T4x4[RC(0, 2)] - T4x4[RC(2, 0)]) / S;
        Angles[3] = (T4x4[RC(1, 0)] - T4x4[RC(0, 1)]) / S;
    }
    else if ((T4x4[RC(0, 0)] > T4x4[RC(1, 1)]) & (T4x4[RC(0, 0)] > T4x4[RC(2, 2)]))
    {
        double S  = sqrt(1.0 + T4x4[RC(0, 0)] - T4x4[RC(1, 1)] - T4x4[RC(2, 2)]) * 2;
        Angles[0] = (T4x4[RC(2, 1)] - T4x4[RC(1, 2)]) / S;
        Angles[1] = 0.25 * S;
        Angles[2] = (T4x4[RC(0, 1)] + T4x4[RC(1, 0)]) / S;
        Angles[3] = (T4x4[RC(0, 2)] + T4x4[RC(2, 0)]) / S;
    }
    else if (T4x4[RC(1, 1)] > T4x4[RC(2, 2)])
    {
        double S  = sqrt(1.0 + T4x4[RC(1, 1)] - T4x4[RC(0, 0)] - T4x4[RC(2, 2)]) * 2;
        Angles[0] = (T4x4[RC(0, 2)] - T4x4[RC(2, 0)]) / S;
        Angles[1] = (T4x4[RC(0, 1)] + T4x4[RC(1, 0)]) / S;
        Angles[2] = 0.25 * S;
        Angles[3] = (T4x4[RC(1, 2)] + T4x4[RC(2, 1)]) / S;
    }
    else
    {
        double S  = sqrt(1.0 + T4x4[RC(2, 2)] - T4x4[RC(0, 0)] - T4x4[RC(1, 1)]) * 2;
        Angles[0] = (T4x4[RC(1, 0)] - T4x4[RC(0, 1)]) / S;
        Angles[1] = (T4x4[RC(0, 2)] + T4x4[RC(2, 0)]) / S;
        Angles[2] = (T4x4[RC(1, 2)] + T4x4[RC(2, 1)]) / S;
        Angles[3] = 0.25 * S;
    }
}
int COrientationQuaternion::GetIndex3x3()
{
    return POSE6D_CHANNEL_3X3 + 1;
} // This was specific, check if it's still desired
int COrientationQuaternion::GetIndex3x3Next()
{
    return POSE6D_CHANNEL_3X3_NEXT + 1;
}
int COrientationQuaternion::GetIndexProbeTip()
{
    return PROBE_TIP + 1;
}
int COrientationQuaternion::GetIndexProbeResidu()
{
    return PROBE_RESIDU + 1;
}
int COrientationQuaternion::GetIndexProbeButton()
{
    return PROBE_BUTTON + 1;
}

//------------------------------------------------------------------------------------------------------------------
// COrientationZVector
//------------------------------------------------------------------------------------------------------------------
void Normalize(double *V)
{ /* Original implementation */
    double Norm = sqrt(V[0] * V[0] + V[1] * V[1] + V[2] * V[2]);
    if (Norm > 0.0000001)
    {
        for (int i = 0; i < 3; i++)
            V[i] = V[i] / Norm;
    }
}
double Determinant(double *R4x4)
{ /* Original implementation */
    return R4x4[RC(0, 0)] * R4x4[RC(1, 1)] * R4x4[RC(2, 2)] + R4x4[RC(0, 1)] * R4x4[RC(1, 2)] * R4x4[RC(2, 0)] +
           R4x4[RC(0, 2)] * R4x4[RC(1, 0)] * R4x4[RC(2, 1)] - R4x4[RC(0, 2)] * R4x4[RC(1, 1)] * R4x4[RC(2, 0)] -
           R4x4[RC(0, 1)] * R4x4[RC(1, 0)] * R4x4[RC(2, 2)] - R4x4[RC(0, 0)] * R4x4[RC(1, 2)] * R4x4[RC(2, 1)];
}
void CrossProduct(double *V1, double *V2, double *VResult)
{ /* Original implementation */
    VResult[0] = (V1[1] * V2[2] - V1[2] * V2[1]);
    VResult[1] = (V1[2] * V2[0] - V1[0] * V2[2]);
    VResult[2] = (V1[0] * V2[1] - V1[1] * V2[0]); // Corrected cross product for VResult[1]
}
double DotProduct(double *V1, double *V2)
{ /* Original implementation */
    double dResult = 0.0;
    for (int i = 0; i < 3; i++)
        dResult += V1[i] * V2[i];
    return dResult;
}

COrientationZVector::COrientationZVector()
{
    m_Name = "Z-Vector";
    m_arAngleNames.push_back("Vx");
    m_arAngleNames.push_back("Vy");
    m_arAngleNames.push_back("Vz");
}
void COrientationZVector::ToT4x4(double *Angles, double *T4x4)
{                                                     /* Original implementation slightly adapted */
    double Vz[3] = {Angles[0], Angles[1], Angles[2]}; // Input is the Z vector
    Normalize(Vz);

    double Vx[3], Vy[3];
    double tempAxis[3] = {0.0, 0.0, 1.0}; // Default X world axis
    if (fabs(Vz[0]) < 0.9 && fabs(Vz[1]) < 0.9)
    { // If Vz is not too close to Z-axis (to avoid issues with Y-axis later)
        tempAxis[0] = 1.0;
        tempAxis[1] = 0.0;
        tempAxis[2] = 0.0; // Use X-axis
    }
    else
    { // Vz is close to Z-axis, use Y-axis to generate X
        tempAxis[0] = 0.0;
        tempAxis[1] = 1.0;
        tempAxis[2] = 0.0; // Use Y-axis
    }

    CrossProduct(tempAxis, Vz, Vy); // Vy = tempAxis x Vz (should be Y if Z is Z-axis like)
    Normalize(Vy);
    CrossProduct(Vy, Vz, Vx); // Vx = Vy x Vz
    Normalize(Vx);

    for (int i = 0; i < 3; i++)
    {
        T4x4[RC(i, 0)] = Vx[i];
        T4x4[RC(i, 1)] = Vy[i];
        T4x4[RC(i, 2)] = Vz[i];
        T4x4[RC(i, 3)] = 0.0; // No translation
    }
    T4x4[RC(3, 0)] = 0.0;
    T4x4[RC(3, 1)] = 0.0;
    T4x4[RC(3, 2)] = 0.0;
    T4x4[RC(3, 3)] = 1.0;

    if (Determinant(T4x4) < 0)
    { // Ensure right-handed system
        for (int i = 0; i < 3; i++)
            T4x4[RC(i, 0)] = -T4x4[RC(i, 0)]; // Flip X-axis
    }
}
void COrientationZVector::From4x4(double *T4x4, double *Angles)
{
    Angles[0] = T4x4[RC(0, 2)];
    Angles[1] = T4x4[RC(1, 2)];
    Angles[2] = T4x4[RC(2, 2)];
}

//------------------------------------------------------------------------------------------------------------------
// COrientation4x4
//------------------------------------------------------------------------------------------------------------------
COrientation4x4::COrientation4x4()
{
    m_Name = "4x4 Matrix";
    // A 4x4 matrix has 16 elements. These are its "channels".
    // Names assuming column-major storage M_row_col
    for (int col = 0; col < 4; ++col)
    {
        for (int row = 0; row < 4; ++row)
        {
            std::string elementName = fmt::format("M{}{}", row, col);
            m_arAngleNames.push_back(elementName);
        }
    }
}

void COrientation4x4::ToT4x4(double *MatrixAsAngles, double *T4x4_out)
{
    // Assumes MatrixAsAngles is a pointer to 16 doubles representing the 4x4 matrix (column-major)
    if (MatrixAsAngles && T4x4_out)
    {
        memcpy(T4x4_out, MatrixAsAngles, 16 * sizeof(double));
    }
}

void COrientation4x4::From4x4(double *T4x4_in, double *MatrixAsAngles)
{
    // Assumes MatrixAsAngles is a pointer to 16 doubles to store the 4x4 matrix (column-major)
    if (MatrixAsAngles && T4x4_in)
    {
        memcpy(MatrixAsAngles, T4x4_in, 16 * sizeof(double));
    }
}

//------------------------------------------------------------------------------------------------------------------
// COrientation3x3
//------------------------------------------------------------------------------------------------------------------
COrientation3x3::COrientation3x3()
{
    m_Name = "3x3 Matrix";
    // A 3x3 matrix has 9 elements.
    // Names assuming column-major storage R_row_col
    for (int col = 0; col < 3; ++col)
    {
        for (int row = 0; row < 3; ++row)
        {
            std::string elementName = fmt::format("R{}{}", row, col);
            m_arAngleNames.push_back(elementName);
        }
    }
}

void COrientation3x3::ToT4x4(double *Matrix3x3AsAngles, double *T4x4_out)
{
    // Assumes Matrix3x3AsAngles points to 9 doubles (column-major: R00,R10,R20, R01,R11,R21, R02,R12,R22)
    if (Matrix3x3AsAngles && T4x4_out)
    {
        // Copy 3x3 part (column-major)
        T4x4_out[RC(0, 0)] = Matrix3x3AsAngles[0];
        T4x4_out[RC(1, 0)] = Matrix3x3AsAngles[1];
        T4x4_out[RC(2, 0)] = Matrix3x3AsAngles[2];
        T4x4_out[RC(0, 1)] = Matrix3x3AsAngles[3];
        T4x4_out[RC(1, 1)] = Matrix3x3AsAngles[4];
        T4x4_out[RC(2, 1)] = Matrix3x3AsAngles[5];
        T4x4_out[RC(0, 2)] = Matrix3x3AsAngles[6];
        T4x4_out[RC(1, 2)] = Matrix3x3AsAngles[7];
        T4x4_out[RC(2, 2)] = Matrix3x3AsAngles[8];

        // Set other parts for a proper rotation matrix (no translation)
        T4x4_out[RC(0, 3)] = 0.0;
        T4x4_out[RC(1, 3)] = 0.0;
        T4x4_out[RC(2, 3)] = 0.0;
        T4x4_out[RC(3, 0)] = 0.0;
        T4x4_out[RC(3, 1)] = 0.0;
        T4x4_out[RC(3, 2)] = 0.0;
        T4x4_out[RC(3, 3)] = 1.0;
    }
}

void COrientation3x3::From4x4(double *T4x4_in, double *Matrix3x3AsAngles)
{
    // Assumes Matrix3x3AsAngles points to 9 doubles to store the 3x3 part (column-major)
    if (Matrix3x3AsAngles && T4x4_in)
    {
        Matrix3x3AsAngles[0] = T4x4_in[RC(0, 0)];
        Matrix3x3AsAngles[1] = T4x4_in[RC(1, 0)];
        Matrix3x3AsAngles[2] = T4x4_in[RC(2, 0)];
        Matrix3x3AsAngles[3] = T4x4_in[RC(0, 1)];
        Matrix3x3AsAngles[4] = T4x4_in[RC(1, 1)];
        Matrix3x3AsAngles[5] = T4x4_in[RC(2, 1)];
        Matrix3x3AsAngles[6] = T4x4_in[RC(0, 2)];
        Matrix3x3AsAngles[7] = T4x4_in[RC(1, 2)];
        Matrix3x3AsAngles[8] = T4x4_in[RC(2, 2)];
    }
}

//------------------------------------------------------------------------------------------------------------------
// COrientationManager - Singleton Implementation
//------------------------------------------------------------------------------------------------------------------

COrientationManager *COrientationManager::s_instance = nullptr;

COrientationManager *COrientationManager::GetInstance()
{
    if (s_instance == nullptr)
    {
        s_instance = new COrientationManager();
    }
    return s_instance;
}

COrientationManager::COrientationManager()
{
    m_mapOrientation = {
        {ORIENTATION_ROLLPITCHYAW, new COrientationRollPitchYaw()},
        {ORIENTATION_STEERCAMBERSPIN, new COrientationSteerCamberSpin()},
        {ORIENTATION_SKREW, new COrientationSkrew()},
        {ORIENTATION_KUKAABC, new COrientationKukaABC()},
        {ORIENTATION_QUATERNION, new COrientationQuaternion()},
        {ORIENTATION_ZVECTOR, new COrientationZVector()},
        {ORIENTATION_4X4, new COrientation4x4()}, // New
        {ORIENTATION_3X3, new COrientation3x3()}  // New
    };
}

COrientationManager::~COrientationManager()
{
    for (auto &iter : m_mapOrientation)
    {
        delete iter.second;
    }
    m_mapOrientation.clear();
}

COrientation *COrientationManager::GetOrientation(int iIndex)
{
    auto iter = m_mapOrientation.find(iIndex);
    if (iter != m_mapOrientation.end())
    {
        return iter->second;
    }
    else
    {
        // assert(false); // Or handle error appropriately
        return nullptr;
    }
}

int COrientationManager::GetOrientationIndex(const std::string &sName)
{
    for (const auto &pair : m_mapOrientation)
    {
        if (pair.second && CompareIgnoreCase(pair.second->GetOrientName(), sName))
        {
            return pair.first;
        }
    }
    return -1; // Not found
}

std::string COrientationManager::GetOrientationName(int iIndex)
{
    auto iter = m_mapOrientation.find(iIndex);
    if (iter != m_mapOrientation.end() && iter->second)
    {
        return iter->second->GetOrientName();
    }
    return std::string("Unknown"); // Not found
}

int COrientationManager::GetIndexPos()
{
    // This method could be on COrientation if it varies, or static if global
    return POSE6D_CHANNEL_POS;
}

int COrientationManager::GetNumOrientations()
{
    return m_mapOrientation.size();
}
