#pragma once

#include "baseUnits.h"
#include <vector> // Required for std::vector
#include <map>    // Required for std::map

#define RC(r, c) ((r) + 4 * (c))

extern int  DEFAULT_ORIENTATION;
extern bool DEFAULT_UNWRAP_ANGLES;
extern bool DEFAULT_SHOW_RESIDU;

const double DEG2RAD        = 0.017453292519943;
const double RAD2DEG        = 57.2957795130833;
const double PI             = 3.14159265358979323846264338327950288419716939937510;
const int    MAX_NUM_ANGLES = 4; // This remains, but COrientation4x4/3x3 will handle more "channels"

const int   NUMPOSITIONS    = 3;
extern char PositionNames[10][NUMPOSITIONS];

const int ORIENTATION_ROLLPITCHYAW    = 0;
const int ORIENTATION_STEERCAMBERSPIN = 10;
const int ORIENTATION_SKREW           = 18;
const int ORIENTATION_KUKAABC         = 19;
const int ORIENTATION_QUATERNION      = 20;
const int ORIENTATION_ZVECTOR         = 21;
const int ORIENTATION_4X4             = 22; // New convention
const int ORIENTATION_3X3             = 23; // New convention

////////////////////////////////////////////////////////////////////////////////////////////////
// ... (rest of the existing function declarations for XYZ_FIX, etc.) ...
////////////////////////////////////////////////////////////////////////////////////////////////
void XYZ_FIX(double a1, double a2, double a3, double *xyz_fix);
void XZY_FIX(double a1, double a2, double a3, double *xzy_fix);
void YXZ_FIX(double a1, double a2, double a3, double *yxz_fix);
void YZX_FIX(double a1, double a2, double a3, double *yzx_fix);
void ZXY_FIX(double a1, double a2, double a3, double *zxy_fix);
void ZYX_FIX(double a1, double a2, double a3, double *zyx_fix);

void XYZ_EULER(double a1, double a2, double a3, double *xyz_euler);
void XZY_EULER(double a1, double a2, double a3, double *xzy_euler);
void YXZ_EULER(double a1, double a2, double a3, double *yxz_euler);
void YZX_EULER(double a1, double a2, double a3, double *yzx_euler);
void ZXY_EULER(double a1, double a2, double a3, double *zxy_euler);
void ZYX_EULER(double a1, double a2, double a3, double *zyx_euler);

void XYX_EULER(double a1, double a2, double a3, double *xyx_euler);
void XZX_EULER(double a1, double a2, double a3, double *xzx_euler);
void YXY_EULER(double a1, double a2, double a3, double *yxy_euler);
void YZY_EULER(double a1, double a2, double a3, double *yzy_euler);
void ZXZ_EULER(double a1, double a2, double a3, double *zxz_euler);
void ZYZ_EULER(double a1, double a2, double a3, double *zyz_euler);

void SKREW_VECTOR(double a1, double a2, double a3, double *skrew_vector);
void SKREW_VECTOR_REVERSE(double *T, double &a1, double &a2, double &a3);
void SKREW_VECTOR_REVERSE_UNWRAP(double *T, const double &a1_prev, const double &a2_prev, const double &a3_prev, double &a1, double &a2, double &a3);

void XYZ_FIX_REVERSE(double *T, double &a1, double &a2, double &a3);

double ShiftAngleInRegion(double InputNumber, double LowerLimit = -180.0, double UpperLimit = 180.0);

class COrientation
{
  public:
    COrientation() { ; };
    virtual ~COrientation() { ; }; // Added virtual destructor

  public:
    int                 GetNumOrientChannels() { return m_arAngleNames.size(); };
    std::string         GetOrientName() { return m_Name; };
    std::string         GetAngleName(int i) { return (i >= 0 && i < m_arAngleNames.size()) ? m_arAngleNames[i] : "Error"; }; // Bounds check
    virtual std::string GetBaseUnit() { return UNIT_ANGLE; };

  public: // to and from T4x4
    virtual void ToT4x4(double *Angles, double *T4x4) { ; };
    virtual void From4x4(double *T4x4, double *Angles) { ; };
    virtual void From4x4Unwrap(double *T4x4, double AnglesPrev[], double *Angles) { From4x4(T4x4, Angles); };

  public: // getting indices into matrices, these are 0-based
    virtual int GetIndexPos();
    virtual int GetIndexOrient();
    virtual int GetIndex3x3();
    virtual int GetIndex3x3Next();
    virtual int GetIndexProbeTip();
    virtual int GetIndexProbeResidu();
    virtual int GetIndexProbeButton();

  protected:
    std::string              m_Name;
    std::vector<std::string> m_arAngleNames;
};

class COrientationRollPitchYaw : public COrientation
{
  public:
    COrientationRollPitchYaw();
    void ToT4x4(double *Angles, double *T4x4) override;
    void From4x4(double *T4x4, double *Angles) override;
    void From4x4Unwrap(double *T4x4, double AnglesPrev[], double *Angles) override;
};

class COrientationSteerCamberSpin : public COrientation
{
  public:
    COrientationSteerCamberSpin();
    void ToT4x4(double *Angles, double *T4x4) override;
    void From4x4(double *T4x4, double *Angles) override;
    void From4x4Unwrap(double *T4x4, double *AnglesPrev, double *Angles) override;
};

class COrientationSkrew : public COrientation
{
  public:
    COrientationSkrew();
    void ToT4x4(double *Angles, double *T4x4) override;
    void From4x4(double *T4x4, double *Angles) override;
    void From4x4Unwrap(double *T4x4, double *AnglesPrev, double *Angles) override;
};

class COrientationKukaABC : public COrientation
{
  public:
    COrientationKukaABC();
    void ToT4x4(double *Angles, double *T4x4) override;
    void From4x4(double *T4x4, double *Angles) override;
    void From4x4Unwrap(double *T4x4, double *AnglesPrev, double *Angles) override;
};

class COrientationQuaternion : public COrientation
{
  public:
    COrientationQuaternion();
    void        ToT4x4(double *Angles, double *T4x4) override;
    void        From4x4(double *T4x4, double *Angles) override;
    std::string GetBaseUnit() override { return ""; };
    int         GetIndex3x3() override;
    int         GetIndex3x3Next() override;
    int         GetIndexProbeTip() override;
    int         GetIndexProbeResidu() override;
    int         GetIndexProbeButton() override;
};

void   Normalize(double *V);
double Determinant(double *R4x4);
void   CrossProduct(double *V1, double *V2, double *VResult);
double DotProduct(double *V1, double *V2);

class COrientationZVector : public COrientation
{
  public:
    COrientationZVector();
    void        ToT4x4(double *Angles, double *T4x4) override;
    void        From4x4(double *T4x4, double *Angles) override;
    std::string GetBaseUnit() override { return ""; };
};

// New COrientation derived classes for 4x4 and 3x3
class COrientation4x4 : public COrientation
{
  public:
    COrientation4x4();
    void        ToT4x4(double *MatrixAsAngles, double *T4x4) override;
    void        From4x4(double *T4x4, double *MatrixAsAngles) override;
    std::string GetBaseUnit() override { return ""; };
};

class COrientation3x3 : public COrientation
{
  public:
    COrientation3x3();
    void        ToT4x4(double *Matrix3x3AsAngles, double *T4x4) override;
    void        From4x4(double *T4x4, double *Matrix3x3AsAngles) override;
    std::string GetBaseUnit() override { return ""; };
};

class COrientationManager
{
  public:
    static COrientationManager *GetInstance(); // Singleton access

    COrientation                        *GetOrientation(int iIndex);
    int                                  GetOrientationIndex(const std::string &sName); // New method
    std::string                          GetOrientationName(int iIndex);                // New method
    int                                  GetIndexPos();
    int                                  GetNumOrientations();
    const std::map<int, COrientation *> &GetMapOrientations() const { return m_mapOrientation; }; // New method to access the map

  private:
    COrientationManager(); // Private constructor
    ~COrientationManager();
    COrientationManager(const COrientationManager &)            = delete; // No copy
    COrientationManager &operator=(const COrientationManager &) = delete; // No assignment

    static COrientationManager   *s_instance; // Singleton instance
    std::map<int, COrientation *> m_mapOrientation;
};

inline COrientationManager *GetOrientationManager()
{
    return COrientationManager::GetInstance();
}; 