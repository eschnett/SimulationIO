// SWIG interface file

%module H5

%{
  #include <H5Cpp.h>
%}

%include <std_string.i>

enum {
  H5F_ACC_TRUNC, H5F_ACC_EXCL, H5F_ACC_RDONLY, H5F_ACC_RDWR, H5F_ACC_DEBUG
};
namespace H5 {
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
