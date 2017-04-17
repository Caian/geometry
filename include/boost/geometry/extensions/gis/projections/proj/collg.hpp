#ifndef BOOST_GEOMETRY_PROJECTIONS_COLLG_HPP
#define BOOST_GEOMETRY_PROJECTIONS_COLLG_HPP

// Boost.Geometry - extensions-gis-projections (based on PROJ4)
// This file is automatically generated. DO NOT EDIT.

// Copyright (c) 2008-2015 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// This file is converted from PROJ4, http://trac.osgeo.org/proj
// PROJ4 is originally written by Gerald Evenden (then of the USGS)
// PROJ4 is maintained by Frank Warmerdam
// PROJ4 is converted to Boost.Geometry by Barend Gehrels

// Last updated version of proj: 4.9.1

// Original copyright notice:

// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.

#include <boost/geometry/util/math.hpp>

#include <boost/geometry/extensions/gis/projections/impl/base_static.hpp>
#include <boost/geometry/extensions/gis/projections/impl/base_dynamic.hpp>
#include <boost/geometry/extensions/gis/projections/impl/projects.hpp>
#include <boost/geometry/extensions/gis/projections/impl/factory_entry.hpp>

namespace boost { namespace geometry { namespace projections
{
    #ifndef DOXYGEN_NO_DETAIL
    namespace detail { namespace collg
    {

            static const double FXC = 1.12837916709551257390;
            static const double FYC = 1.77245385090551602729;
            static const double ONEEPS = 1.0000001;

            // template class, using CRTP to implement forward/inverse
            template <typename Geographic, typename Cartesian, typename Parameters>
            struct base_collg_spheroid : public base_t_fi<base_collg_spheroid<Geographic, Cartesian, Parameters>,
                     Geographic, Cartesian, Parameters>
            {

                 typedef double geographic_type;
                 typedef double cartesian_type;


                inline base_collg_spheroid(const Parameters& par)
                    : base_t_fi<base_collg_spheroid<Geographic, Cartesian, Parameters>,
                     Geographic, Cartesian, Parameters>(*this, par) {}

                // FORWARD(s_forward)  spheroid
                // Project coordinates from geographic (lon, lat) to cartesian (x, y)
                inline void fwd(geographic_type& lp_lon, geographic_type& lp_lat, cartesian_type& xy_x, cartesian_type& xy_y) const
                {
                    if ((xy_y = 1. - sin(lp_lat)) <= 0.)
                        xy_y = 0.;
                    else
                        xy_y = sqrt(xy_y);
                    xy_x = FXC * lp_lon * xy_y;
                    xy_y = FYC * (1. - xy_y);
                }

                // INVERSE(s_inverse)  spheroid
                // Project coordinates from cartesian (x, y) to geographic (lon, lat)
                inline void inv(cartesian_type& xy_x, cartesian_type& xy_y, geographic_type& lp_lon, geographic_type& lp_lat) const
                {
                    lp_lat = xy_y / FYC - 1.;
                    if (fabs(lp_lat = 1. - lp_lat * lp_lat) < 1.)
                        lp_lat = asin(lp_lat);
                    else if (fabs(lp_lat) > ONEEPS) throw proj_exception();
                    else    lp_lat = lp_lat < 0. ? -geometry::math::half_pi<double>() : geometry::math::half_pi<double>();
                    if ((lp_lon = 1. - sin(lp_lat)) <= 0.)
                        lp_lon = 0.;
                    else
                        lp_lon = xy_x / (FXC * sqrt(lp_lon));
                }

                static inline std::string get_name()
                {
                    return "collg_spheroid";
                }

            };

            // Collignon
            template <typename Parameters>
            void setup_collg(Parameters& par)
            {
                par.es = 0.;
            }

        }} // namespace detail::collg
    #endif // doxygen

    /*!
        \brief Collignon projection
        \ingroup projections
        \tparam Geographic latlong point type
        \tparam Cartesian xy point type
        \tparam Parameters parameter type
        \par Projection characteristics
         - Pseudocylindrical
         - Spheroid
        \par Example
        \image html ex_collg.gif
    */
    template <typename Geographic, typename Cartesian, typename Parameters = parameters>
    struct collg_spheroid : public detail::collg::base_collg_spheroid<Geographic, Cartesian, Parameters>
    {
        inline collg_spheroid(const Parameters& par) : detail::collg::base_collg_spheroid<Geographic, Cartesian, Parameters>(par)
        {
            detail::collg::setup_collg(this->m_par);
        }
    };

    #ifndef DOXYGEN_NO_DETAIL
    namespace detail
    {

        // Factory entry(s)
        template <typename Geographic, typename Cartesian, typename Parameters>
        class collg_entry : public detail::factory_entry<Geographic, Cartesian, Parameters>
        {
            public :
                virtual base_v<Geographic, Cartesian>* create_new(const Parameters& par) const
                {
                    return new base_v_fi<collg_spheroid<Geographic, Cartesian, Parameters>, Geographic, Cartesian, Parameters>(par);
                }
        };

        template <typename Geographic, typename Cartesian, typename Parameters>
        inline void collg_init(detail::base_factory<Geographic, Cartesian, Parameters>& factory)
        {
            factory.add_to_factory("collg", new collg_entry<Geographic, Cartesian, Parameters>);
        }

    } // namespace detail
    #endif // doxygen

}}} // namespace boost::geometry::projections

#endif // BOOST_GEOMETRY_PROJECTIONS_COLLG_HPP

