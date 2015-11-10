// SWIG interface file

%module H5

%{
#include <H5Cpp.h>

#include <algorithm>
%}

%include <std_string.i>
%include <std_vector.i>

%template(vector_double) std::vector<double>;
%template(vector_int) std::vector<int>;

enum {
  H5F_ACC_TRUNC, H5F_ACC_EXCL, H5F_ACC_RDONLY, H5F_ACC_RDWR, H5F_ACC_DEBUG
};
namespace H5 {
  struct DataSpace {
    %extend {
      static DataSpace make(const std::vector<int>& idims) {
        std::vector<hsize_t> dims(dims.size());
        std::copy(idims.begin(), idims.end(), dims.begin());
        return H5::DataSpace(dims.size(), dims.data());
      }
    }
    %extend {
      std::vector<int> getSimpleExtentDims() const {
        auto ndims = self->getSimpleExtentNdims();
        std::vector<hsize_t> dims(ndims);
        self->getSimpleExtentDims(dims.data());
        std::vector<int> idims(ndims);
        std::copy(dims.begin(), dims.end(), idims.begin());
        return idims;
      }
    }
    int getSimpleExtentNdims() const;
    int getSimpleExtentNpoints() const;
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
    DataSpace getSpace() const;
    DataType getDataType() const;
    %extend {
      std::vector<int> read_int() const {
        auto dataspace = self->getSpace();
        auto npoints = dataspace.getSimpleExtentNpoints();
        std::vector<int> buf(npoints);
        self->read(buf.data(), H5::DataType(H5::PredType::NATIVE_INT));
        return buf;
      }
    }
    %extend {
      std::vector<double> read_double() const {
        auto dataspace = self->getSpace();
        auto npoints = dataspace.getSimpleExtentNpoints();
        std::vector<double> buf(npoints);
        self->read(buf.data(), H5::DataType(H5::PredType::NATIVE_DOUBLE));
        return buf;
      }
    }
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
