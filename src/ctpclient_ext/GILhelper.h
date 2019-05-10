#include <boost/python.hpp>

/// @brief RAII class used to lock and unlock the GIL.
class GIL_lock
{
  PyGILState_STATE _state;
public:
  GIL_lock() {
    _state = PyGILState_Ensure();
  }

  ~GIL_lock() {
    PyGILState_Release(_state);
  }
};
