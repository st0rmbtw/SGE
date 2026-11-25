#ifndef SGE_PROFILE_HPP_
#define SGE_PROFILE_HPP_

#if SGE_PROFILE_ENABLED
    #include <tracy/Tracy.hpp>
#else
    #include "macros.hpp"

    #define ZoneNamed(x,y) SGE_UNUSED(x,y)
    #define ZoneNamedN(x,y,z) SGE_UNUSED(x,y,z)
    #define ZoneNamedC(x,y,z) SGE_UNUSED(x,y,z)
    #define ZoneNamedNC(x,y,z,w) SGE_UNUSED(x,y,z,w)

    #define ZoneTransient(x,y) SGE_UNUSED(x,y)
    #define ZoneTransientN(x,y,z) SGE_UNUSED(x,y,z)

    #define ZoneScoped SGE_UNUSED()
    #define ZoneScopedN(x) SGE_UNUSED(x)
    #define ZoneScopedC(x) SGE_UNUSED(x)
    #define ZoneScopedNC(x,y) SGE_UNUSED(x,y)

    #define ZoneText(x,y) SGE_UNUSED(x,y)
    #define ZoneTextV(x,y,z) SGE_UNUSED(x,y,z)
    #define ZoneTextF(x,...) SGE_UNUSED(x,__VA_ARGS__)
    #define ZoneTextVF(x,y,...) SGE_UNUSED(x,y,__VA_ARGS__)
    #define ZoneName(x,y) SGE_UNUSED(x,y)
    #define ZoneNameV(x,y,z) SGE_UNUSED(x,y,z)
    #define ZoneNameF(x,...) SGE_UNUSED(x,__VA_ARGS__)
    #define ZoneNameVF(x,y,...) SGE_UNUSED(x,y,__VA_ARGS__)
    #define ZoneColor(x) SGE_UNUSED(x)
    #define ZoneColorV(x,y) SGE_UNUSED(x,y)
    #define ZoneValue(x) SGE_UNUSED(x)
    #define ZoneValueV(x,y) SGE_UNUSED(x,y)
    #define ZoneIsActive (false)
    #define ZoneIsActiveV(x) (false)

    #define FrameMark SGE_UNUSED()
    #define FrameMarkNamed(x) SGE_UNUSED(x)
    #define FrameMarkStart(x) SGE_UNUSED(x)
    #define FrameMarkEnd(x) SGE_UNUSED(x)

    #define FrameImage(x,y,z,w,a) SGE_UNUSED(x,y,z, w, a)

    #define TracyLockable( type, varname ) type varname
    #define TracyLockableN( type, varname, desc ) type varname
    #define TracySharedLockable( type, varname ) type varname
    #define TracySharedLockableN( type, varname, desc ) type varname
    #define LockableBase( type ) type
    #define SharedLockableBase( type ) type
    #define LockMark(x) SGE_UNUSED(x)
    #define LockableName(x,y,z) SGE_UNUSED(x,y,z)

    #define TracyPlot(x,y) SGE_UNUSED(x,y)
    #define TracyPlotConfig(x,y,z,w,a) SGE_UNUSED(x,y,z,w,a)

    #define TracyMessage(x,y) SGE_UNUSED(x,y)
    #define TracyMessageL(x) SGE_UNUSED(x)
    #define TracyMessageC(x,y,z) SGE_UNUSED(x,y,z)
    #define TracyMessageLC(x,y) SGE_UNUSED(x,y)
    #define TracyAppInfo(x,y) SGE_UNUSED(x,y)

    #define TracyAlloc(x,y) SGE_UNUSED(x,y)
    #define TracyFree(x) SGE_UNUSED(x)
    #define TracySecureAlloc(x,y) SGE_UNUSED(x,y)
    #define TracySecureFree(x) SGE_UNUSED(x)

    #define TracyAllocN(x,y,z) SGE_UNUSED(x,y,z)
    #define TracyFreeN(x,y) SGE_UNUSED(x,y)
    #define TracySecureAllocN(x,y,z) SGE_UNUSED(x,y,z)
    #define TracySecureFreeN(x,y) SGE_UNUSED(x,y)

    #define ZoneNamedS(x,y,z) SGE_UNUSED(x,y,z)
    #define ZoneNamedNS(x,y,z,w) SGE_UNUSED(x,y,z,w)
    #define ZoneNamedCS(x,y,z,w) SGE_UNUSED(x,y,z,w)
    #define ZoneNamedNCS(x,y,z,w,a) SGE_UNUSED(x,y,z,w)

    #define ZoneTransientS(x,y,z) SGE_UNUSED(x,y,z)
    #define ZoneTransientNS(x,y,z,w) SGE_UNUSED(x,y,z,w)

    #define ZoneScopedS(x) SGE_UNUSED(x)
    #define ZoneScopedNS(x,y) SGE_UNUSED(x,y)
    #define ZoneScopedCS(x,y) SGE_UNUSED(x,y)
    #define ZoneScopedNCS(x,y,z) SGE_UNUSED(x,y,z)

    #define TracyAllocS(x,y,z) SGE_UNUSED(x,y,z)
    #define TracyFreeS(x,y) SGE_UNUSED(x,y)
    #define TracySecureAllocS(x,y,z) SGE_UNUSED(x,y,z)
    #define TracySecureFreeS(x,y) SGE_UNUSED(x,y)

    #define TracyAllocNS(x,y,z,w) SGE_UNUSED(x,y,z,w)
    #define TracyFreeNS(x,y,z) SGE_UNUSED(x,y,z)
    #define TracySecureAllocNS(x,y,z,w) SGE_UNUSED(x,y,z,w)
    #define TracySecureFreeNS(x,y,z) SGE_UNUSED(x,y,z)

    #define TracyMessageS(x,y,z) SGE_UNUSED(x,y,z)
    #define TracyMessageLS(x,y) SGE_UNUSED(x,y)
    #define TracyMessageCS(x,y,z,w) SGE_UNUSED(x,y,z,w)
    #define TracyMessageLCS(x,y,z) SGE_UNUSED(x,y,z)

    #define TracySourceCallbackRegister(x,y) SGE_UNUSED(x,y)
    #define TracyParameterRegister(x,y) SGE_UNUSED(x,y)
    #define TracyParameterSetup(x,y,z,w) SGE_UNUSED(x,y,z,w)
    #define TracyIsConnected (false)
    #define TracyIsStarted (false)
    #define TracySetProgramName(x) SGE_UNUSED(x)

    #define TracyFiberEnter(x) SGE_UNUSED(x)
    #define TracyFiberEnterHint(x,y) SGE_UNUSED(x,y)
    #define TracyFiberLeave SGE_UNUSED()
#endif

#endif