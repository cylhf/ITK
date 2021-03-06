// This is core/vnl/vnl_matlab_filewrite.h
#ifndef vnl_matlab_filewrite_h_
#define vnl_matlab_filewrite_h_
//:
//  \file
//  \author David Capel, Oxford RRG
//  \date   17 August 1998
//
// \verbatim
// Modifications
// LSB (Manchester) 23/3/01  Tidied documentation
//   Feb.2002 - Peter Vanroose - brief doxygen comment placed on single line
// \endverbatim

#include <string>
#include <fstream>
#include <complex>
#ifdef _MSC_VER
#  include <vcl_msvc_warnings.h>
#endif

#include "vnl_vector.h"
#include "vnl_matrix.h"
#include "vnl/vnl_export.h"

//: Code to perform MATLAB binary file operations
//    vnl_matlab_filewrite is a collection of I/O functions for reading/writing
//    matrices in the compact MATLAB binary format (.mat)

class VNL_EXPORT vnl_matlab_filewrite
{
 public:
  vnl_matlab_filewrite (char const* file_name, char const *basename = nullptr);

  //: Add scalar/vector/matrix variable to the MAT file using specified variable name.
  // If no name is given, variables will be generated by
  // appending 0,1,2 etc to the given basename.
  void write(double v, char const* variable_name = nullptr);

  void write(vnl_vector<double> const & v, char const* variable_name = nullptr);
  void write(vnl_vector<std::complex<double> > const & v, char const* variable_name = nullptr);

  void write(vnl_matrix<float> const & M, char const* variable_name = nullptr);
  void write(vnl_matrix<double> const & M, char const* variable_name = nullptr);
  void write(vnl_matrix<std::complex<float> > const & M, char const* variable_name = nullptr);
  void write(vnl_matrix<std::complex<double> > const & M, char const* variable_name = nullptr);

  void write(double const * const *M, int rows, int cols, char const* variable_name = nullptr);

 protected:
  std::string basename_;
  int variable_int_;
  std::fstream out_;

  std::string make_var_name(char const* variable_name);
};

#endif // vnl_matlab_filewrite_h_
