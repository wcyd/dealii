// ---------------------------------------------------------------------
//
// Copyright (C) 2004 - 2017 by the deal.II authors
//
// This file is part of the deal.II library.
//
// The deal.II library is free software; you can use it, redistribute
// it, and/or modify it under the terms of the GNU Lesser General
// Public License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
// The full text of the license can be found in the file LICENSE at
// the top level of the deal.II distribution.
//
// ---------------------------------------------------------------------


#include <deal.II/base/qprojector.h>
#include <deal.II/base/tensor_product_polynomials.h>
#include <deal.II/base/tensor_product_polynomials_const.h>
#include <deal.II/base/tensor_product_polynomials_bubbles.h>
#include <deal.II/base/polynomials_p.h>
#include <deal.II/base/polynomials_piecewise.h>
#include <deal.II/base/polynomials_rannacher_turek.h>
#include <deal.II/fe/fe_poly.h>
#include <deal.II/fe/fe_values.h>
#include <deal.II/fe/fe_poly.templates.h>

DEAL_II_NAMESPACE_OPEN


template <>
void
FE_Poly<TensorProductPolynomials<1>,1,2>::
fill_fe_values (const Triangulation<1,2>::cell_iterator &,
                const CellSimilarity::Similarity                           cell_similarity,
                const Quadrature<1>                                       &quadrature,
                const Mapping<1,2>                                        &mapping,
                const Mapping<1,2>::InternalDataBase                      &mapping_internal,
                const dealii::internal::FEValues::MappingRelatedData<1,2> &mapping_data,
                const FiniteElement<1,2>::InternalDataBase                &fe_internal,
                dealii::internal::FEValues::FiniteElementRelatedData<1,2> &output_data) const
{
  // convert data object to internal
  // data for this class. fails with
  // an exception if that is not
  // possible
  Assert (dynamic_cast<const InternalData *> (&fe_internal) != nullptr, ExcInternalError());
  const InternalData &fe_data = static_cast<const InternalData &> (fe_internal);

  // transform gradients and higher derivatives. there is nothing to do
  // for values since we already emplaced them into output_data when
  // we were in get_data()
  if (fe_data.update_each & update_gradients && cell_similarity != CellSimilarity::translation)
    for (unsigned int k=0; k<this->dofs_per_cell; ++k)
      mapping.transform (make_array_view(fe_data.shape_gradients, k),
                         mapping_covariant,
                         mapping_internal,
                         make_array_view(output_data.shape_gradients, k));

  if (fe_data.update_each & update_hessians && cell_similarity != CellSimilarity::translation)
    {
      for (unsigned int k=0; k<this->dofs_per_cell; ++k)
        mapping.transform (make_array_view(fe_data.shape_hessians, k),
                           mapping_covariant_gradient,
                           mapping_internal,
                           make_array_view(output_data.shape_hessians, k));

      for (unsigned int k=0; k<this->dofs_per_cell; ++k)
        for (unsigned int i=0; i<quadrature.size(); ++i)
          for (unsigned int j=0; j<2; ++j)
            output_data.shape_hessians[k][i] -=
              mapping_data.jacobian_pushed_forward_grads[i][j]
              * output_data.shape_gradients[k][i][j];
    }

  if (fe_data.update_each & update_3rd_derivatives && cell_similarity != CellSimilarity::translation)
    {
      for (unsigned int k=0; k<this->dofs_per_cell; ++k)
        mapping.transform (make_array_view(fe_data.shape_3rd_derivatives, k),
                           mapping_covariant_hessian,
                           mapping_internal,
                           make_array_view(output_data.shape_3rd_derivatives, k));

      for (unsigned int k=0; k<this->dofs_per_cell; ++k)
        correct_third_derivatives(output_data, mapping_data, quadrature.size(), k);
    }
}



template <>
void
FE_Poly<TensorProductPolynomials<2>,2,3>::
fill_fe_values (const Triangulation<2,3>::cell_iterator &,
                const CellSimilarity::Similarity                           cell_similarity,
                const Quadrature<2>                                       &quadrature,
                const Mapping<2,3>                                        &mapping,
                const Mapping<2,3>::InternalDataBase                      &mapping_internal,
                const dealii::internal::FEValues::MappingRelatedData<2,3> &mapping_data,
                const FiniteElement<2,3>::InternalDataBase                &fe_internal,
                dealii::internal::FEValues::FiniteElementRelatedData<2,3> &output_data) const
{

  // assert that the following dynamics
  // cast is really well-defined.
  Assert (dynamic_cast<const InternalData *> (&fe_internal) != nullptr, ExcInternalError());
  const InternalData &fe_data = static_cast<const InternalData &> (fe_internal);

  // transform gradients and higher derivatives. there is nothing to do
  // for values since we already emplaced them into output_data when
  // we were in get_data()
  if (fe_data.update_each & update_gradients && cell_similarity != CellSimilarity::translation)
    for (unsigned int k=0; k<this->dofs_per_cell; ++k)
      mapping.transform (make_array_view(fe_data.shape_gradients, k),
                         mapping_covariant,
                         mapping_internal,
                         make_array_view(output_data.shape_gradients, k));

  if (fe_data.update_each & update_hessians && cell_similarity != CellSimilarity::translation)
    {
      for (unsigned int k=0; k<this->dofs_per_cell; ++k)
        mapping.transform (make_array_view(fe_data.shape_hessians, k),
                           mapping_covariant_gradient,
                           mapping_internal,
                           make_array_view(output_data.shape_hessians, k));

      for (unsigned int k=0; k<this->dofs_per_cell; ++k)
        for (unsigned int i=0; i<quadrature.size(); ++i)
          for (unsigned int j=0; j<3; ++j)
            output_data.shape_hessians[k][i] -=
              mapping_data.jacobian_pushed_forward_grads[i][j]
              * output_data.shape_gradients[k][i][j];
    }

  if (fe_data.update_each & update_3rd_derivatives && cell_similarity != CellSimilarity::translation)
    {
      for (unsigned int k=0; k<this->dofs_per_cell; ++k)
        mapping.transform (make_array_view(fe_data.shape_3rd_derivatives, k),
                           mapping_covariant_hessian,
                           mapping_internal,
                           make_array_view(output_data.shape_3rd_derivatives, k));

      for (unsigned int k=0; k<this->dofs_per_cell; ++k)
        correct_third_derivatives(output_data, mapping_data, quadrature.size(), k);
    }
}


template <>
void
FE_Poly<PolynomialSpace<1>,1,2>::
fill_fe_values (const Triangulation<1,2>::cell_iterator &,
                const CellSimilarity::Similarity                           cell_similarity,
                const Quadrature<1>                                       &quadrature,
                const Mapping<1,2>                                        &mapping,
                const Mapping<1,2>::InternalDataBase                      &mapping_internal,
                const dealii::internal::FEValues::MappingRelatedData<1,2> &mapping_data,
                const FiniteElement<1,2>::InternalDataBase                &fe_internal,
                dealii::internal::FEValues::FiniteElementRelatedData<1,2> &output_data) const
{
  // convert data object to internal
  // data for this class. fails with
  // an exception if that is not
  // possible

  Assert (dynamic_cast<const InternalData *> (&fe_internal) != nullptr, ExcInternalError());
  const InternalData &fe_data = static_cast<const InternalData &> (fe_internal);

  // transform gradients and higher derivatives. there is nothing to do
  // for values since we already emplaced them into output_data when
  // we were in get_data()
  if (fe_data.update_each & update_gradients && cell_similarity != CellSimilarity::translation)
    for (unsigned int k=0; k<this->dofs_per_cell; ++k)
      mapping.transform (make_array_view(fe_data.shape_gradients, k),
                         mapping_covariant,
                         mapping_internal,
                         make_array_view(output_data.shape_gradients, k));

  if (fe_data.update_each & update_hessians && cell_similarity != CellSimilarity::translation)
    {
      for (unsigned int k=0; k<this->dofs_per_cell; ++k)
        mapping.transform (make_array_view(fe_data.shape_hessians, k),
                           mapping_covariant_gradient,
                           mapping_internal,
                           make_array_view(output_data.shape_hessians, k));

      for (unsigned int k=0; k<this->dofs_per_cell; ++k)
        for (unsigned int i=0; i<quadrature.size(); ++i)
          for (unsigned int j=0; j<2; ++j)
            output_data.shape_hessians[k][i] -=
              mapping_data.jacobian_pushed_forward_grads[i][j]
              * output_data.shape_gradients[k][i][j];
    }

  if (fe_data.update_each & update_3rd_derivatives && cell_similarity != CellSimilarity::translation)
    {
      for (unsigned int k=0; k<this->dofs_per_cell; ++k)
        mapping.transform (make_array_view(fe_data.shape_3rd_derivatives, k),
                           mapping_covariant_hessian,
                           mapping_internal,
                           make_array_view(output_data.shape_3rd_derivatives, k));

      for (unsigned int k=0; k<this->dofs_per_cell; ++k)
        correct_third_derivatives(output_data, mapping_data, quadrature.size(), k);
    }
}


template <>
void
FE_Poly<PolynomialSpace<2>,2,3>::
fill_fe_values (const Triangulation<2,3>::cell_iterator &,
                const CellSimilarity::Similarity                           cell_similarity,
                const Quadrature<2>                                       &quadrature,
                const Mapping<2,3>                                        &mapping,
                const Mapping<2,3>::InternalDataBase                      &mapping_internal,
                const dealii::internal::FEValues::MappingRelatedData<2,3> &mapping_data,
                const FiniteElement<2,3>::InternalDataBase                &fe_internal,
                dealii::internal::FEValues::FiniteElementRelatedData<2,3> &output_data) const
{
  Assert (dynamic_cast<const InternalData *> (&fe_internal) != nullptr, ExcInternalError());
  const InternalData &fe_data = static_cast<const InternalData &> (fe_internal);

  // transform gradients and higher derivatives. there is nothing to do
  // for values since we already emplaced them into output_data when
  // we were in get_data()
  if (fe_data.update_each & update_gradients && cell_similarity != CellSimilarity::translation)
    for (unsigned int k=0; k<this->dofs_per_cell; ++k)
      mapping.transform (make_array_view(fe_data.shape_gradients, k),
                         mapping_covariant,
                         mapping_internal,
                         make_array_view(output_data.shape_gradients, k));

  if (fe_data.update_each & update_hessians && cell_similarity != CellSimilarity::translation)
    {
      for (unsigned int k=0; k<this->dofs_per_cell; ++k)
        mapping.transform (make_array_view(fe_data.shape_hessians, k),
                           mapping_covariant_gradient,
                           mapping_internal,
                           make_array_view(output_data.shape_hessians, k));

      for (unsigned int k=0; k<this->dofs_per_cell; ++k)
        for (unsigned int i=0; i<quadrature.size(); ++i)
          for (unsigned int j=0; j<3; ++j)
            output_data.shape_hessians[k][i] -=
              mapping_data.jacobian_pushed_forward_grads[i][j]
              * output_data.shape_gradients[k][i][j];
    }

  if (fe_data.update_each & update_3rd_derivatives && cell_similarity != CellSimilarity::translation)
    {
      for (unsigned int k=0; k<this->dofs_per_cell; ++k)
        mapping.transform (make_array_view(fe_data.shape_3rd_derivatives, k),
                           mapping_covariant_hessian,
                           mapping_internal,
                           make_array_view(output_data.shape_3rd_derivatives, k));

      for (unsigned int k=0; k<this->dofs_per_cell; ++k)
        correct_third_derivatives(output_data, mapping_data, quadrature.size(), k);
    }
}


#include "fe_poly.inst"

DEAL_II_NAMESPACE_CLOSE
