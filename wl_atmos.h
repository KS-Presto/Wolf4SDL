// WL_ATMOS.H

#ifndef _WL_ATMOS_H_
#define _WL_ATMOS_H_

#if defined(USE_STARSKY) || defined(USE_RAIN) || defined(USE_SNOW)
    void Init3DPoints (void);
#endif

#ifdef USE_STARSKY
    void DrawStarSky (void);
#endif

#ifdef USE_RAIN
    void DrawRain (void);
#endif

#ifdef USE_SNOW
    void DrawSnow (void);
#endif

#endif
