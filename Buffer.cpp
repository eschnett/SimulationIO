#include "Buffer.hpp"

namespace SimulationIO {

shared_ptr<dconcatenation_t> dconcatenation_t::make(int dim) {
  switch (dim) {
  case 0:
    return make_shared<concatenation_t<0>>();
  case 1:
    return make_shared<concatenation_t<1>>();
  case 2:
    return make_shared<concatenation_t<2>>();
  case 3:
    return make_shared<concatenation_t<3>>();
  case 4:
    return make_shared<concatenation_t<4>>();
  default:
    assert(0);
  }
}
}
