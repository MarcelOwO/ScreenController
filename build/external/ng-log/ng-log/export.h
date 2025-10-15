
#ifndef NGLOG_EXPORT_H
#define NGLOG_EXPORT_H

#ifdef NGLOG_STATIC_DEFINE
#  define NGLOG_EXPORT
#  define NGLOG_NO_EXPORT
#else
#  ifndef NGLOG_EXPORT
#    ifdef ng_log_EXPORTS
        /* We are building this library */
#      define NGLOG_EXPORT 
#    else
        /* We are using this library */
#      define NGLOG_EXPORT 
#    endif
#  endif

#  ifndef NGLOG_NO_EXPORT
#    define NGLOG_NO_EXPORT 
#  endif
#endif

#ifndef NGLOG_DEPRECATED
#  define NGLOG_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef NGLOG_DEPRECATED_EXPORT
#  define NGLOG_DEPRECATED_EXPORT NGLOG_EXPORT NGLOG_DEPRECATED
#endif

#ifndef NGLOG_DEPRECATED_NO_EXPORT
#  define NGLOG_DEPRECATED_NO_EXPORT NGLOG_NO_EXPORT NGLOG_DEPRECATED
#endif

/* NOLINTNEXTLINE(readability-avoid-unconditional-preprocessor-if) */
#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef NGLOG_NO_DEPRECATED
#    define NGLOG_NO_DEPRECATED
#  endif
#endif

#endif /* NGLOG_EXPORT_H */
