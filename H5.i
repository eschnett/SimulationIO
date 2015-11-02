// SWIG interface file

%module H5

%{
#include <H5Cpp.h>

#include <algorithm>
%}

%include <std_string.i>
%include <std_vector.i>

%template(vector_int) std::vector<int>;

enum {
  H5F_ACC_TRUNC, H5F_ACC_EXCL, H5F_ACC_RDONLY, H5F_ACC_RDWR, H5F_ACC_DEBUG
};
namespace H5 {
  struct DataSpace {
    %extend {
      static DataSpace make(const std::vector<int>& dims) {
        std::vector<hsize_t> dims1(dims.size());
        std::copy(dims.begin(), dims.end(), dims1.begin());
        return H5::DataSpace(dims1.size(), dims1.data());
      }
    }
  };
  %nodefaultctor PredType;
  struct PredType {
    static const PredType NATIVE_INT;
    static const PredType NATIVE_FLOAT;
    static const PredType NATIVE_DOUBLE;
    static const PredType NATIVE_HSIZE;
    static const PredType NATIVE_HSSIZE;
  };
  struct DataType {
    DataType(const PredType& predtype);
  };
  struct DataSet {
    int getId() const;
  };
  struct Group;
  %nodefaultctor CommonFG;
  struct CommonFG {
    Group createGroup(const std::string& name);
  };
  struct H5File: CommonFG {
    H5File(const std::string& filename, unsigned int flags);
    void close();
  };
  struct Group: CommonFG {
  };
}
