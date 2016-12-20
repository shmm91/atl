#ifndef __AWESOMO_ESTIMATION_KF_HPP__
#define __AWESOMO_ESTIMATION_KF_HPP__

#include "awesomo_core/utils/utils.hpp"


namespace awesomo {

class KalmanFilter {
public:
  bool initialized;

  VecX mu;

  MatX B;
  MatX R;

  MatX C;
  MatX Q;

  MatX S;
  MatX I;
  MatX K;

  VecX mu_p;
  MatX S_p;

  KalmanFilter(void);
  int init(VecX mu, MatX R, MatX C, MatX Q);
  int estimate(MatX A, VecX y);
};

}  // end of awesomo namespace
#endif
