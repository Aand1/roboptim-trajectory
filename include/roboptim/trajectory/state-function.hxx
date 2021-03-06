// Copyright (C) 2009 by Florent Lamiraux, Thomas Moulard, AIST, CNRS, INRIA.
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

#ifndef ROBOPTIM_TRAJECTORY_STATE_COST_HXX
# define ROBOPTIM_TRAJECTORY_STATE_COST_HXX

# include <stdexcept>
# include <boost/format.hpp>

# include <roboptim/core/alloc.hh>

namespace roboptim
{
namespace trajectory
{
  template <typename T>
  StateFunction<T>::StateFunction (const trajectory_t& trajectory,
				   boost::shared_ptr<DerivableFunction> function,
				   const StableTimePoint tpt,
				   size_type order)
    : DerivableFunction (trajectory.parameters ().size (),
			 function->outputSize (),
			 (boost::format ("state cost using function ``%s'' at t=%f * tMax")
			  % function->getName ()
			  % tpt.getAlpha ()).str ()),
      trajectory_ (trajectory),
      function_ (function),
      tpt_ (tpt),
      order_ (order)
  {
    if (function_->inputSize () != trajectory_.outputSize () * (order + 1))
      {
	boost::format fmt
	  ("failed to build state cost object:"
	   " state function input size is %d, expected size is %d");
	fmt % function_->inputSize ()
	  % (trajectory_.outputSize () * (order + 1));
	throw std::runtime_error (fmt.str ());
      }
  }

  template <typename T>
  StateFunction<T>::~StateFunction()
  {
  }

  template <typename T>
  typename StateFunction<T>::size_type
  StateFunction<T>::order () const
  {
    return order_;
  }

  template <typename T>
  void
  StateFunction<T>::impl_compute (result_ref res,
				  const_argument_ref p) const
  {
#ifndef ROBOPTIM_DO_NOT_CHECK_ALLOCATION
    bool cur_malloc_allowed = is_malloc_allowed ();
    set_is_malloc_allowed (true);
#endif //! ROBOPTIM_DO_NOT_CHECK_ALLOCATION

    static boost::shared_ptr<trajectory_t> updatedTrajectory =
      boost::shared_ptr<trajectory_t> (trajectory_.clone ());
    updatedTrajectory->setParameters (p);
    (*function_) (res, updatedTrajectory->state (tpt_, this->order_));

#ifndef ROBOPTIM_DO_NOT_CHECK_ALLOCATION
    set_is_malloc_allowed (cur_malloc_allowed);
#endif //! ROBOPTIM_DO_NOT_CHECK_ALLOCATION
  }

  template <typename T>
  void
  StateFunction<T>::impl_gradient (gradient_ref grad,
				   const_argument_ref p,
				   size_type i) const
  {
#ifndef ROBOPTIM_DO_NOT_CHECK_ALLOCATION
    bool cur_malloc_allowed = is_malloc_allowed ();
    set_is_malloc_allowed (true);
#endif //! ROBOPTIM_DO_NOT_CHECK_ALLOCATION

    static boost::shared_ptr<trajectory_t> updatedTrajectory =
      boost::shared_ptr<trajectory_t> (trajectory_.clone ());
    updatedTrajectory->setParameters (p);
    grad = function_->gradient
      (updatedTrajectory->state (tpt_, this->order_), i) *
      updatedTrajectory->variationStateWrtParam (tpt_, this->order_);

#ifndef ROBOPTIM_DO_NOT_CHECK_ALLOCATION
    set_is_malloc_allowed (cur_malloc_allowed);
#endif //! ROBOPTIM_DO_NOT_CHECK_ALLOCATION
  }

} // end of namespace trajectory.
} // end of namespace roboptim.

#endif //! ROBOPTIM_TRAJECTORY_STATE_COST_HXX
