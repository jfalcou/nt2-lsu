//==============================================================================
//         Copyright 2003 - 2011   LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2011   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
#ifndef NT2_CORE_DETAILS_COMPONENTS_HPP_INCLUDED
#define NT2_CORE_DETAILS_COMPONENTS_HPP_INCLUDED

#include <hpx/hpx_fwd.hpp>
#include <hpx/include/components.hpp>
#include <hpx/include/applier.hpp>
#include <hpx/include/async.hpp>
#include <hpx/include/local_lcos.hpp>

#include <nt2/include/functions/run.hpp>
#include <nt2/include/functions/numel.hpp>
#include <boost/simd/sdk/simd/native.hpp>
#include <boost/fusion/include/at.hpp>

namespace nt2 { namespace server
{
  class transforms_server
    : public hpx::components::managed_component_base<transforms_server>
  {
  public:
    transforms_server() {}
    
    template<class A0, class A1>
    void nt2_nD_loopnest( A0& a0
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
    void nt2_1D_loopnest( A0& a0
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


    template <class A0, class A1>
    struct nt2_nD_loopnest_action
      : hpx::actions::make_action< void(transforms_server::*)(A0&,A1&,std::size_t const&,std::size_t const&)
                                 , &transforms_server::template nt2_nD_loopnest<A0,A1>
                                 , boost::mpl::false_
                                 , nt2_nD_loopnest_action<A0,A1> 
                                 > {};

    template <class A0, class A1>
    struct nt2_1D_loopnest_action
      : hpx::actions::make_action< void(transforms_server::*)(A0&,A1&,std::size_t const&,std::size_t const&)
                                 , &transforms_server::template nt2_1D_loopnest<A0,A1>
                                 , boost::mpl::false_
                                 , nt2_1D_loopnest_action<A0,A1> 
                                 > {};

  };

} }

HPX_SERIALIZATION_REGISTER_TEMPLATE_ACTION( (template <class A0, class A1>)
                                          , (nt2::server::transforms_server::nt2_nD_loopnest_action<A0,A1>)
                                          )

HPX_SERIALIZATION_REGISTER_TEMPLATE_ACTION( (template <class A0, class A1>)
                                          , (nt2::server::transforms_server::nt2_1D_loopnest_action<A0,A1>)
                                          )


namespace nt2 { namespace stubs 
{ 
  struct transforms_server
    : hpx::components::stub_base<nt2::server::transforms_server>
  {
    template<class A0, class A1>
    static void nt2_nD_loopnest_async( hpx::naming::id_type const& gid
                                     , A0& a0
                                     , A1& a1
                                     , std::size_t const& outer_begin
                                     , std::size_t const& outer_end
                                     )
    {
      typedef typename server::transforms_server::nt2_nD_loopnest_action<A0,A1> action_type;
      hpx::apply<action_type>(gid, a0, a1, outer_begin, outer_end);
    }

    template<class A0, class A1>
    static void nt2_1D_loopnest_async( hpx::naming::id_type const& gid
                                     , A0& a0
                                     , A1& a1
                                     , std::size_t const& begin
                                     , std::size_t const& end
                                     )
    {
      typedef server::transforms_server::nt2_1D_loopnest_action<A0,A1> action_type;
      hpx::apply<action_type>(gid, a0, a1, begin, end);
    }

    template<class A0, class A1>
    static void nt2_nD_loopnest_sync( hpx::naming::id_type const& gid
                                     , A0& a0
                                     , A1& a1
                                     , std::size_t const& outer_begin
                                     , std::size_t const& outer_end
                                     )
    {
      typedef server::transforms_server::nt2_nD_loopnest_action<A0,A1> action_type;
      hpx::async<action_type>(gid, a0, a1, outer_begin, outer_end).get;
    }

    template<class A0, class A1>
    static void nt2_1D_loopnest_sync( hpx::naming::id_type const& gid
                                     , A0& a0
                                     , A1& a1
                                     , std::size_t const& begin
                                     , std::size_t const& end
                                     )
    {
      typedef server::transforms_server::nt2_1D_loopnest_action<A0,A1> action_type;
      hpx::async<action_type>(gid, a0, a1, begin, end).get();
    }
  };

} }


namespace nt2{ namespace details
{
  class transform_actions
    : public hpx::components::
      client_base< transform_actions
                 , stubs::transforms_server
                 >
  {
    typedef hpx::components::
    client_base< transform_actions
               , stubs::transforms_server
               > base_type;

  public:
    transform_actions()
    {}

    transform_actions(hpx::naming::id_type const& gid)
      : base_type(gid)
    {}

    template<class A0, class A1>
    void nt2_nD_loopnest_async( A0& a0
                              , A1& a1
                              , std::size_t const& outer_begin
                              , std::size_t const& outer_end
                              )
    {
      BOOST_ASSERT(this->gid_);
      this->base_type::nt2_nD_loopnest_async(this->gid_, a0, a1, outer_begin, outer_end);
    }

    template<class A0, class A1>
    void nt2_1D_loopnest_async( A0& a0
                              , A1& a1
                              , std::size_t const& begin
                              , std::size_t const& end
                              )
    {
      BOOST_ASSERT(this->gid_);
      this->base_type::nt2_1D_loopnest_async(this->gid_, a0, a1, begin, end);
    }

    template<class A0, class A1>
    void nt2_nD_loopnest_sync( A0& a0
                              , A1& a1
                              , std::size_t const& outer_begin
                              , std::size_t const& outer_end
                              )
    {
      BOOST_ASSERT(this->gid_);
      this->base_type::nt2_nD_loopnest_sync(this->gid_, a0, a1, outer_begin, outer_end);
    }

    template<class A0, class A1>
    void nt2_1D_loopnest_sync( A0& a0
                              , A1& a1
                              , std::size_t const& begin
                              , std::size_t const& end
                              )
    {
      BOOST_ASSERT(this->gid_);
      this->base_type::nt2_1D_loopnest_sync(this->gid_, a0, a1, begin, end);
    }
  };

} }

#endif
