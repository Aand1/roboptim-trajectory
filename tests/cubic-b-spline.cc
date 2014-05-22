// Copyright (C) 2009 by Thomas Moulard, AIST, CNRS, INRIA.
//
// This file is part of the roboptim.
//
// roboptim is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// roboptim is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with roboptim.  If not, see <http://www.gnu.org/licenses/>.

#undef NDEBUG

#include "shared-tests/fixture.hh"

#include <roboptim/core/finite-difference-gradient.hh>

#include <roboptim/core/visualization/gnuplot.hh>
#include <roboptim/core/visualization/gnuplot-commands.hh>
#include <roboptim/core/visualization/gnuplot-function.hh>

#include <roboptim/trajectory/visualization/trajectory.hh>

#include <roboptim/trajectory/fwd.hh>
#include <roboptim/trajectory/cubic-b-spline.hh>

using namespace roboptim;
using namespace roboptim::visualization;
using namespace roboptim::visualization::gnuplot;

struct SplineDerivWrtParameters : public DifferentiableFunction
{
  SplineDerivWrtParameters (const CubicBSpline& spline, value_type t)
    : DifferentiableFunction
      (spline.parameters ().size (), spline.outputSize (),
       "spline differentiable w.r.t parameters"),
      spline_ (spline),
      t_ (t)
  {}

  virtual void
  impl_compute (result_t& result, const argument_t& x)
    const
  {
#ifndef ROBOPTIM_DO_NOT_CHECK_ALLOCATION
    Eigen::internal::set_is_malloc_allowed (true);
#endif //! ROBOPTIM_DO_NOT_CHECK_ALLOCATION

    CubicBSpline spline (spline_);
    spline.setParameters (x);
    result = spline (t_);
  }

  virtual void
  impl_gradient (gradient_t& gradient,
		 const argument_t& x,
		 size_type functionId = 0)
    const
  {
#ifndef ROBOPTIM_DO_NOT_CHECK_ALLOCATION
    Eigen::internal::set_is_malloc_allowed (true);
#endif //! ROBOPTIM_DO_NOT_CHECK_ALLOCATION

    CubicBSpline spline (spline_);
    spline.setParameters (x);
    gradient = spline.variationConfigWrtParam (t_).row (functionId);
  }

private:
  const CubicBSpline& spline_;
  value_type t_;
};

BOOST_FIXTURE_TEST_SUITE (trajectory, TestSuiteConfiguration)

BOOST_AUTO_TEST_CASE (trajectory_cubic_b_spline)
{
  using namespace roboptim::visualization::gnuplot;

  typedef Function::value_type value_type;

  boost::shared_ptr<boost::test_tools::output_test_stream>
    output = retrievePattern("cubic-b-spline");

  CubicBSpline::vector_t params (16);

  // Initial position.
  params[0] = 0.,  params[1] = 0.;
  params[2] = 0.,  params[3] = 0.;
  params[4] = 0.,  params[5] = 0.;
  // Control point 3.
  params[6] = 25.,  params[7] = 50.;
  // Control point 4.
  params[8] = 50.,  params[9] = 25.;
  // Final position.
  params[10] = 100., params[11] = 100.;
  params[12] = 100., params[13] = 100.;
  params[14] = 100., params[15] = 100.;

  // First 2D spline
  CubicBSpline spline_2d_1 (std::make_pair (0., 5.), 2,
                            params, "Cubic B-spline 2D (1)");

  // Second 2D spline (change some parameters)
  params[7] = 75.;
  CubicBSpline spline_2d_2 (std::make_pair (0., 5.), 2,
                            params, "Cubic B-spline 2D (2)");

  Gnuplot gnuplot = Gnuplot::make_interactive_gnuplot ();
  discreteInterval_t interval (0., 5., 0.01);

  // FIRST PART: TEST 1D CUBIC B-SPLINES

  CubicBSpline::vector_t params_1d (8);

  // Initial position.
  params_1d[0] = 0.;
  params_1d[1] = 0.;
  params_1d[2] = 0.;
  // Control point 3.
  params_1d[3] = 50.;
  // Control point 4.
  params_1d[4] = 50.;
  // Final position.
  params_1d[5] = 0.;
  params_1d[6] = 0.;
  params_1d[7] = 0.;

  // First 1D spline
  CubicBSpline spline_1d_1 (std::make_pair (0., 5.), 1,
                            params_1d, "Cubic B-spline 1D (1)");

  // Second 1D spline (change some parameters)
  params_1d[0] = 30.;
  params_1d[3] = 75.;
  params_1d[4] = 25.;
  params_1d[7] = 30.;
  CubicBSpline spline_1d_2 (std::make_pair (0., 5.), 1,
                            params_1d, "Cubic B-spline 1D (2)");

  // Test spline additions
  CubicBSpline spline_1d_3 = spline_1d_1 + spline_1d_2;
  CubicBSpline spline_1d_4 = spline_1d_1;
  spline_1d_4 += spline_1d_2;

  BOOST_CHECK_THROW
    (CubicBSpline spline_1d_5 = spline_1d_1 + spline_2d_2;
     std::cout << spline_1d_5 << std::endl,
     std::runtime_error);

  // Gnuplot export and compare to pattern
  (*output)
    << (gnuplot
        << set ("multiplot layout 3,2")

        << comment (spline_2d_1)

        << comment ("Values:")
        << comment (spline_2d_1 (0.))
        << comment (spline_2d_1 (2.5))
        << comment (spline_2d_1 (5.))

        << comment ("1st derivative:")
        << comment (spline_2d_1.derivative (0., 1))
        << comment (spline_2d_1.derivative (2.5, 1))
        << comment (spline_2d_1.derivative (5., 1))

        << comment ("2nd derivative:")
        << comment (spline_2d_1.derivative (0., 2))
        << comment (spline_2d_1.derivative (2.5, 2))
        << comment (spline_2d_1.derivative (5., 2))
        << plot_xy (spline_2d_1, interval)

        << comment (spline_2d_2)

        << comment ("Values:")
        << comment (spline_2d_2 (0.))
        << comment (spline_2d_2 (2.5))
        << comment (spline_2d_2 (5.))

        << comment ("1st derivative:")
        << comment (spline_2d_2.derivative (0., 1))
        << comment (spline_2d_2.derivative (2.5, 1))
        << comment (spline_2d_2.derivative (5., 1))

        << comment ("2nd derivative:")
        << comment (spline_2d_2.derivative (0., 2))
        << comment (spline_2d_2.derivative (2.5, 2))
        << comment (spline_2d_2.derivative (5., 2))
        << plot_xy (spline_2d_2, interval)

        << plot (spline_1d_1, interval)
        << plot (spline_1d_2, interval)

        << plot (spline_1d_3, interval)
        << plot (spline_1d_4, interval)

        << unset ("multiplot"));


  std::cout << output->str () << std::endl;

  BOOST_CHECK (output->match_pattern ());

  // Check gradients with finite-differences

  for (value_type t = 0.5; t < 5.; t += 0.5)
    {
      try
        {
          Function::vector_t x (1);
          x[0] = t;
          checkGradientAndThrow (spline_2d_1, 0, x);

          SplineDerivWrtParameters splineDerivWrtParams (spline_2d_1, t);
          checkGradientAndThrow
            (splineDerivWrtParams, 0, spline_2d_1.parameters ());
        }
      catch (BadGradient<EigenMatrixDense>& bg)
        {
          std::cerr << bg << std::endl;
          BOOST_CHECK(false);
        }
    }

  for (value_type x = 0.; x < 10.; x += 0.25)
    {
      Function::vector_t params = spline_2d_1.parameters ();
      params[0 * 2] = 321;
      params[1 * 2] = 123;
      params[2 * 2] =
        6. * (x - 1./6. * params[0 * 2] - 2. / 3. * params[1 * 2]);

      assert (1. / 6. * params[0 * 2]
              + 2. / 3. * params[1 * 2]
              + 1. / 6. * params[2 * 2] - x < 1e-8);

      spline_2d_1.setParameters (params);
      if (std::fabs (spline_2d_1 (0.)[0] - x) >= 1e-8)
        std::cout << "# " << spline_2d_1 (0.)[0] << " != " << x << std::endl;
    }

  discreteInterval_t window (0., 5., 0.01);
  for (value_type t = boost::get<0> (window); t < boost::get<1> (window);
       t += boost::get<2> (window))
    {
      BOOST_CHECK_SMALL (std::abs (spline_1d_3 (t)[0]
				   - (spline_1d_1 (t)[0] + spline_1d_2 (t)[0])),
			 1e-8);
      BOOST_CHECK_SMALL (std::abs (spline_1d_4 (t)[0]
				   - (spline_1d_1 (t)[0] + spline_1d_2 (t)[0])),
			 1e-8);
    }

  // Non-regression test (compilation)
  std::vector<CubicBSpline> vec;
  vec.push_back(CubicBSpline (std::make_pair (0., 5.), 2, params,
			      "cubic-b-spline"));
}

BOOST_AUTO_TEST_SUITE_END ()
