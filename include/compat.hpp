#ifndef DST_COMPAT_HPP
#define DST_COMPAT_HPP

#ifdef __has_include
# if __has_include(<version>)
#   include <version>
# endif
#endif

namespace std
{

#if __has_cpp_attribute(__cpp_lib_interpolate)
#else
	template <typename T1, typename T2>
	T1 lerp(T1 v0, T1 v1, T2 t)
	{
  		return (1 - t) * v0 + t * v1;
  	}
#endif

}

#endif /* DST_COMPAT_HPP */
