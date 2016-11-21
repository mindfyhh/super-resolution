// The total variation regularization cost implementation. Total variation is
// effectively defined as the gradient value at each pixel. Its purpose is to
// add denoising by imposing smoothness in the estimated image (smaller changes
// between neighrboing pixels in the x and y directions).

#ifndef SRC_SOLVERS_TV_REGULARIZER_H_
#define SRC_SOLVERS_TV_REGULARIZER_H_

#include <vector>

#include "solvers/regularizer.h"

#include "opencv2/core/core.hpp"

namespace super_resolution {

class TotalVariationRegularizer : public Regularizer {
 public:
  using Regularizer::Regularizer;  // Inherit Regularizer constructor.

  // Implementation of total variation regularization.
  virtual std::vector<double> ComputeResiduals(
      const double* image_data) const;
};

}  // namespace super_resolution

#endif  // SRC_SOLVERS_TV_REGULARIZER_H_