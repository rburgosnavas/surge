//-------------------------------------------------------------------------------------------------------
//	Copyright 2005 Claes Johanson & Vember Audio
//-------------------------------------------------------------------------------------------------------
#pragma once
#include "globals.h"
#include <vt_dsp/basic_dsp.h>
#include <vt_dsp/halfratefilter.h>

#define setzero(x) memset(x, 0, sizeof(*x))

class quadr_osc
{
public:
   quadr_osc()
   {
      r = 0;
      i = -1;
   }
   inline void set_rate(float w)
   {
      dr = cos(w);
      di = sin(w);
      // normalize vector
      double n = 1 / sqrt(r * r + i * i);
      r *= n;
      i *= n;
   }
   inline void set_phase(float w)
   {
      r = sin(w);
      i = -cos(w);
   }
   inline void process()
   {
      float lr = r, li = i;
      r = dr * lr - di * li;
      i = dr * li + di * lr;
   }

public:
   float r, i;

private:
   float dr, di;
};

template <class T, bool first_run_checks = true> class lipol
{
public:
   lipol()
   {
      reset();
   }
   void reset()
   {
      if (first_run_checks)
         first_run = true;
      new_v = 0;
      v = 0;
      dv = 0;
      setBlockSize(block_size);
   }
   inline void newValue(T f)
   {
      v = new_v;
      new_v = f;
      if (first_run_checks && first_run)
      {
         v = f;
         first_run = false;
      }
      dv = (new_v - v) * bs_inv;
   }
   inline T getTargetValue()
   {
      return new_v;
   }

   inline void instantize()
   {
      v = new_v;
      dv = 0;
   }
   inline void process()
   {
      v += dv;
   }
   void setBlockSize(int n)
   {
      bs_inv = 1 / (T)n;
   }
   T v;
   T new_v;
   T dv;

private:
   T bs_inv;
   bool first_run;
};

template <class T, bool first_run_checks = true> class lag
{
public:
   lag(T lp)
   {
      this->lp = lp;
      lpinv = 1 - lp;
      v = 0;
      target_v = 0;
      if (first_run_checks)
         first_run = true;
   }
   lag()
   {
      lp = 0.004;
      lpinv = 1 - lp;
      v = 0;
      target_v = 0;
      if (first_run_checks)
         first_run = true;
   }
   void setRate(T lp)
   {
      this->lp = lp;
      lpinv = 1 - lp;
   }
   inline void newValue(T f)
   {
      target_v = f;
      if (first_run_checks && first_run)
      {
         v = target_v;
         first_run = false;
      }
   }
   inline void startValue(T f)
   {
      target_v = f;
      v = f;
      if (first_run_checks && first_run)
      {
         first_run = false;
      }
   }
   inline void instantize()
   {
      v = target_v;
   }
   inline T getTargetValue()
   {
      return target_v;
   }
   inline void process()
   {
      v = v * lpinv + target_v * lp;
   }
   // void setBlockSize(int n){ bs_inv = 1/(T)n; }
   T v;
   T target_v;

private:
   bool first_run;
   // T bs_inv;
   T lp, lpinv;
};

/*inline float db2linear(float db)
{
        return powf(10.f,0.05f*db);
}*/

inline void flush_denormal(double& d)
{
   if (fabs(d) < 1E-30)
      d = 0;
}

inline bool within_range(int lo, int value, int hi)
{
   return ((value >= lo) && (value <= hi));
}

//#define limit_range(x,low,high) (max(low, min(x, high)))

inline float lerp(float a, float b, float x)
{
   return (1 - x) * a + x * b;
}

inline void trixpan(
    float& L,
    float& R,
    float x) // panning that always lets both channels through unattenuated (seperate hard-panning)
{
   if (x < 0.f)
   {
      L = L - x * R;
      R = (1.f + x) * R;
   }
   else
   {
      R = x * L + R;
      L = (1.f - x) * L;
   }
}

inline double tanh_fast(double in)
{
   double x = fabs(in);
   const double a = 2 / 3;
   double xx = x * x;
   double denom = 1 + x + xx + a * x * xx;
   return ((in > 0) ? 1 : -1) * (1 - 1 / denom);
}

inline float tanh_fast(float in)
{
   const float a = 2 / 3;
   //#if PPC
   float x = fabs(in);
   float xx = x * x;
   float denom = 1 + x + xx + a * x * xx;
#if PPC
   return ((in > 0.f) ? 1.f : -1.f) * (1.f - 1.f / denom);
#else
   _mm_store_ss(&denom, _mm_rcp_ss(_mm_load_ss(&denom)));
   return ((in > 0.f) ? 1.f : -1.f) * (1.f - denom);
#endif
   /*#else
           int sign = 0x80000000;
           int mask = 0x7fffffff;
           __m128 one = _mm_load_ss(1.f);
           __m128 xin = _mm_load_ss(&in);
           __m128 x = _mm_and_ss(xin,_mm_load_ss((float*)&mask));
           __m128 xx = _mm_mul_ss(x,x);
           __m128 denom = _mm_add_ss(_mm_add_ss(one,x),
   _mm_add_ss(xx,_mm_mul_ss(xx,_mm_mul_ss(_mm_load_ss(&a),x)));
           
   #endif*/
}

__forceinline double tanh_faster1(double x)
{
   const double a = -1 / 3, b = 2 / 15;
   // return tanh(x);
   double xs = x * x;
   double y = 1 + xs * a + xs * xs * b;
   return y * x;
}

__forceinline float clamp01(float in)
{
   if (in > 1.0f)
      return 1.0f;
   if (in < 0.0f)
      return 0.0f;
   return in;
}

__forceinline float clamp1bp(float in)
{
   if (in > 1.0f)
      return 1.0f;
   if (in < -1.0f)
      return -1.0f;
   return in;
}

// anv�nd custom format (x^3 ?) internt, men spara som decibel i xml-datan
// bollocks to it
inline float amp_to_linear(float x)
{
   x = max(0.f, x);
   return x * x * x;
}
inline float linear_to_amp(float x)
{
   return powf(limit_range(x, 0.0000000001f, 1.f), 1.f / 3.f);
}
inline float amp_to_db(float x)
{
   return limit_range((float)(6.f * 3.f * log(x) / log(2.f)), -192.f, 96.f);
}
inline float db_to_amp(float x)
{
   return limit_range(powf((10.f / 3.f), 0.05f * x), 0.f, 1.f);
}

inline double sincf(double x)
{
   if (x == 0)
      return 1;
   return (sin(M_PI * x)) / (M_PI * x);
}

inline double sinc(double x)
{
   if (fabs(x) < 0.0000000000000000000001)
      return 1.0;
   return (sin(x) / x);
}

inline double blackman(int i, int n)
{
   // if (i>=n) return 0;
   return (0.42 - 0.5 * cos(2 * M_PI * i / (n - 1)) + 0.08 * cos(4 * M_PI * i / (n - 1)));
}

inline double symmetric_blackman(double i, int n)
{
   // if (i>=n) return 0;
   i -= (n / 2);
   return (0.42 - 0.5 * cos(2 * M_PI * i / (n)) + 0.08 * cos(4 * M_PI * i / (n)));
}

inline double blackman(double i, int n)
{
   // if (i>=n) return 0;
   return (0.42 - 0.5 * cos(2 * M_PI * i / (n - 1)) + 0.08 * cos(4 * M_PI * i / (n - 1)));
}

inline double blackman_harris(int i, int n)
{
   // if (i>=n) return 0;
   return (0.35875 - 0.48829 * cos(2 * M_PI * i / (n - 1)) + 0.14128 * cos(4 * M_PI * i / (n - 1)) -
           0.01168 * cos(6 * M_PI * i / (n - 1)));
}

inline double symmetric_blackman_harris(double i, int n)
{
   // if (i>=n) return 0;
   i -= (n / 2);
   // return (0.42 - 0.5*cos(2*M_PI*i/(n)) + 0.08*cos(4*M_PI*i/(n)));
   return (0.35875 - 0.48829 * cos(2 * M_PI * i / (n)) + 0.14128 * cos(4 * M_PI * i / (n - 1)) -
           0.01168 * cos(6 * M_PI * i / (n)));
}

inline double blackman_harris(double i, int n)
{
   // if (i>=n) return 0;
   return (0.35875 - 0.48829 * cos(2 * M_PI * i / (n - 1)) + 0.14128 * cos(4 * M_PI * i / (n - 1)) -
           0.01168 * cos(6 * M_PI * i / (n - 1)));
}

float correlated_noise(float lastval, float correlation);
float correlated_noise_mk2(float& lastval, float correlation);
float drift_noise(float& lastval);
float correlated_noise_o2(float lastval, float& lastval2, float correlation);
float correlated_noise_o2mk2(float& lastval, float& lastval2, float correlation);

inline double hanning(int i, int n)
{
   if (i >= n)
      return 0;
   return 0.5 * (1 - cos(2 * M_PI * i / (n - 1)));
}

inline double hamming(int i, int n)
{
   if (i >= n)
      return 0;
   return 0.54 - 0.46 * cos(2 * M_PI * i / (n - 1));
}

inline char* float_to_str(float value, char* str)
{
   if (!str)
      return 0;
   sprintf(str, "%f", value);
   return str;
}

inline char* yes_no(int value, char* str)
{
   if (!str)
      return 0;
   if (value)
      sprintf(str, "yes");
   else
      sprintf(str, "no");
   return str;
}

inline bool is_yes(const char* str)
{
   if (!str)
      return 0;
#if WINDOWS
   if (_stricmp(str, "yes") == 0)
      return 1;
#else
   if (strcasecmp(str, "yes") == 0)
      return 1;
#endif
   return 0;
}