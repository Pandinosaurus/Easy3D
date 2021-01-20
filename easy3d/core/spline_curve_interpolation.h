/**
 * Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
 * https://3d.bk.tudelft.nl/liangliang/
 *
 * This file is part of Easy3D. If it is useful in your research/work,
 * I would be grateful if you show your appreciation by citing it:
 * ------------------------------------------------------------------
 *      Liangliang Nan.
 *      Easy3D: a lightweight, easy-to-use, and efficient C++
 *      library for processing and rendering 3D data. 2018.
 * ------------------------------------------------------------------
 * Easy3D is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License Version 3
 * as published by the Free Software Foundation.
 *
 * Easy3D is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef EASY3D_CORE_SPLINE_CURVE_INTERPOLATION_H
#define EASY3D_CORE_SPLINE_CURVE_INTERPOLATION_H


#include <vector>
#include <cassert>

#include <easy3d/core/spline_interpolation.h>


namespace easy3d {


    /**
     * @brief Cubic spline curve interpolation for arbitrary dimensions.
     * \details This is a wrapper of SplineInterpolation. It can be instantiated with any point type (1D, 2D, 3D etc.).
     * @tparam Point_t: type of a point.
     * @tparam Real_t: floating point representation of the points (float, double etc.)
     * Example usage:
     *      \code
     *          const int resolution = 1000;    // Number of line subdivisions to display the spline
     *          SplineCurveInterpolation<vec3> interpolator;
     *          interpolator.set_boundary(...);
     *          interpolator.set_points(points);
     *          for (int i = 0; i < resolution; ++i) {
     *              const vec3 p = interpolator.eval_f(float(i) / float(resolution - 1));
     *              std::cout << p << std::endl;
     *          }
     *      \endcode
     * @class SplineCurveInterpolation easy3d/core/spline_curve_interpolation.h
     */
    template<typename Point_t>
    class SplineCurveInterpolation {
    public:
        typedef typename Point_t::FT FT;

        enum BoundaryType {
            first_deriv = 1,
            second_deriv = 2
        };

    public:

        /// Constructor.
        /// Sets default boundary condition to be zero curvature at both ends
        SplineCurveInterpolation() : left_(second_deriv), right_(second_deriv),
                                left_value_(0.0), right_value_(0.0),
                                linear_extrapolation_(false), dim_(0), total_length_(0.0) {
        }

        /// Sets the boundary condition (optional).
        /// \attention If called, it has to come before set_points().
        void set_boundary(BoundaryType left, FT left_value,
                          BoundaryType right, FT right_value,
                          bool linear_extrapolation = false);

        /// Set the position of the point samples on the curve.
        /// \c cubic_spline true for cubic spline interpolation; \c false for linear interpolation.
        /// \attention The \c points have to be ordered along the curve.
        void set_points(const std::vector<Point_t> &points, bool cubic_spline = true);

        /// Evaluate position of the spline
        /// @param u : curve parameter ranging from [0; 1]
        Point_t eval_f(FT u) const;

    private:
        // -------------------------------------------------------------------------
        /// @name Class tools
        // -------------------------------------------------------------------------
        BoundaryType left_, right_;
        FT left_value_, right_value_;
        bool linear_extrapolation_;

        std::size_t dim_;
        std::vector< SplineInterpolation<FT> > interpolators_;
        FT total_length_;
    };



    // Cubic spline interpolation implementation
    // -----------------------

    template<typename Point_t>
    void SplineCurveInterpolation<Point_t>::set_boundary(typename SplineCurveInterpolation<Point_t>::BoundaryType left,
                                                         FT left_value,
                                                         typename SplineCurveInterpolation<Point_t>::BoundaryType right,
                                                         FT right_value,
                                                         bool linear_extrapolation) {
        assert(interpolators_.size() == 0);  // set_points() must not have happened yet
        left_ = left;
        right_ = right;
        left_value_ = left_value;
        right_value_ = right_value;
        linear_extrapolation_ = linear_extrapolation;
    }


    template<typename Point_t>
    void SplineCurveInterpolation<Point_t>::set_points(const std::vector<Point_t> &points, bool cubic_spline) {
        if (points.empty())
            return;

        dim_ = points[0].dimension();

        // an ND curve is represented in the parametric form: x1(t), x2(t), x3(t)...
        std::vector<FT> T(points.size());
        std::vector< std::vector<FT> > coords(dim_, std::vector<FT>(points.size()));

        FT t = 0.0;
        for (std::size_t i = 0; i < points.size(); ++i) {
            const auto &p = points[i];
            if (i > 0)
                t += distance(points[i-1], p);
            T[i] = t;
            for (std::size_t j = 0; j<dim_; ++j)
                coords[j][i] = p[j];
        }
        total_length_ = t;

        // class instantiation
        interpolators_.resize(dim_);
        for (std::size_t i=0; i<dim_; ++i) {
            // set boundary condition
            interpolators_[i].set_boundary(
                    left_ == first_deriv ? SplineInterpolation<FT>::first_deriv : SplineInterpolation<FT>::second_deriv,
                    left_value_,
                    right_ == first_deriv ? SplineInterpolation<FT>::first_deriv : SplineInterpolation<FT>::second_deriv,
                    right_value_,
                    cubic_spline
                    );
            // set data
            interpolators_[i].set_data(T, coords[i]);
        }
    }


    template<typename Point_t>
    Point_t SplineCurveInterpolation<Point_t>::eval_f(FT u) const {
        Point_t p;
        for (std::size_t i=0; i<dim_; ++i)
            p[i] = interpolators_[i](u * total_length_);
        return p;
    }


}   // namespace easy3d


#endif  // EASY3D_CORE_SPLINE_CURVE_INTERPOLATION_H
