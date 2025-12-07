#if defined(__STDC_VERSION__)
#   if __STDC_VERSION__ >= 202311L
        // Since C23:
        // Do nothing, static_assert is already defined
#   elif __STDC_VERSION__ >= 201112L
#       define static_assert _Static_assert 
#   else
#       error ERROR: Cannot build with this old of a version of C. Please use C11 or later
#   endif 
#else
#   error ERROR: Cannot build with this old of a version of C. Please use C11 or later
#endif