//==============================================================================
//         Copyright 2003 - 2011   LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2011   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================

#include <nt2/include/functions/run.hpp>
#include <nt2/include/functions/numel.hpp>
#include <boost/simd/sdk/simd/native.hpp>
#include <boost/fusion/include/at.hpp>

template<class A0, class A1>
void nt2::server::transforms_server::
     nt2_nD_loopnest( A0& a0
                    , A1& a1
                    , std::size_t const& outer_begin
                    , std::size_t const& outer_end
                    )
{
  typedef typename meta::
    strip< typename meta::scalar_of<A0>::type >::type stype;
  
  typedef boost::simd::native<stype, BOOST_SIMD_DEFAULT_EXTENSION>
    target_type;
  
  static const std::size_t N  = target_type::static_size;
  const std::size_t in_sz     = boost::fusion::at_c<0>(a0.extent());
  const std::size_t in_sz_bnd = (in_sz/N)*N;
  
  for(std::size_t j=outer_begin; j<outer_end; ++j)
  {
    std::size_t it = j*in_sz;
    // Process all vectorizable chunks
    for(std::size_t m=it+in_sz_bnd; it != m; it+=N)
      nt2::run(a0, it, nt2::run(a1, it, meta::as_<target_type>()));
    // Process the scalar epilogue
    for(std::size_t m=it+in_sz-in_sz_bnd; it != m; ++it)
      nt2::run(a0, it, nt2::run(a1, it, meta::as_<stype>()));
  }
}

template<class A0, class A1>
void nt2::server::transforms_server::
     nt2_1D_loopnest( A0& a0
                    , A1& a1
                    , std::size_t const& begin
                    , std::size_t const& end
                    )
{
  typedef typename meta::
    strip< typename meta::scalar_of<A0>::type >::type stype;
  
  typedef boost::simd::native<stype, BOOST_SIMD_DEFAULT_EXTENSION>
    target_type;
  
  static const std::size_t N = target_type::static_size;
  const std::size_t simd_bound = ((end-begin)/N)*N;
  
  // Process all vectorizable chunks
  for(std::size_t i=begin; i<simd_bound; i+=N)
  {
    nt2::run(a0, i, nt2::run(a1, i, meta::as_<target_type>()));
  }
  
  // Process scalar epilogue
  for(std::size_t i=simd_bound; i<end; ++i)
  {
    nt2::run(a0, i, nt2::run(a1, i, meta::as_<stype>()));
  }
}
