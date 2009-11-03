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

#include "common.hh"

#include <roboptim/core/io.hh>
#include <roboptim/core/finite-difference-gradient.hh>

#include <roboptim/trajectory/frontal-speed.hh>
#include <roboptim/trajectory/orthogonal-speed.hh>
#include <roboptim/trajectory/spline.hh>
#include <roboptim/trajectory/state-function.hh>

using namespace roboptim;

int run_test ()
{
  const unsigned orderMax = 1;
  Spline::vector_t params (12);

  // Initial position.
  params[0] = 0.,  params[1] = 0., params[2] = 0.;
  // Control point 1.
  params[3] = 25.,  params[4] = 100., params[5] = 0.;
  // Control point 2.
  params[6] = 75.,  params[7] = 0., params[8] = 0.;
  // Final position.
  params[9] = 100., params[10] = 100., params[11] = 0.;

  Spline::interval_t timeRange = Spline::makeInterval (0., 4.);
  Spline spline (timeRange, 3, params, "before");

  for (unsigned i = 0; i < 10; ++i)
    {
      const StableTimePoint timePoint = i / 10. * tMax;
      const double t = timePoint.getTime (spline.timeRange ());

      boost::shared_ptr<DerivableFunction> frontalSpeed (new FrontalSpeed ());
      StateFunction<Spline> stateFunction
	(spline, frontalSpeed, timePoint, orderMax);

      boost::shared_ptr<DerivableFunction> orthogonalSpeed
	(new OrthogonalSpeed ());
      StateFunction<Spline> orthoStateFunction (spline, orthogonalSpeed,
						timePoint, orderMax);

      std::cout << "State cost evaluation:" << std::endl
		<< stateFunction (params) << std::endl
		<< "State cost gradient:" << std::endl
		<< stateFunction.gradient (params) << std::endl
		<< "Trajectory state (splitted):" << std::endl;
      for (unsigned o = 0; o <= orderMax; ++o)
	std::cout << spline.derivative (t, o) << std::endl;
      std::cout << "Trajectory state (one call):" << std::endl
		<< spline.state (t, orderMax) << std::endl
		<< "Trajectory state variation (splitted):" << std::endl;
      for (unsigned o = 0; o <= orderMax; ++o)
	std::cout << spline.variationDerivWrtParam (t, o) << std::endl;
      std::cout << "Trajectory state (one call):" << std::endl
		<< spline.variationStateWrtParam (t, orderMax) << std::endl;

      try
	{
	  std::cout << "Check frontal speed gradient." << std::endl;
	  checkGradientAndThrow (*frontalSpeed, 0, spline.state (t, orderMax));

	  std::cout << "Check orthogonal speed gradient." << std::endl;
	  checkGradientAndThrow (*orthogonalSpeed, 0,
				 spline.state (t, orderMax));

	  std::cout << "Check state cost gradient." << std::endl;
	  checkGradientAndThrow (stateFunction, 0, params);
	}
      catch (BadGradient& bg)
	{
	  std::cout << bg << std::endl;
	}
      std::cout << "\n\n" << std::endl;
    }

  return 0;
}

GENERATE_TEST ()