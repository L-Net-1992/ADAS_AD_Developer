#include "hdf5files_handle.h"

namespace utility
{

template <>
std::vector<int> Hdf5Handle::get_data<int>(std::string list, const char *group)
{
    std::vector<int> ret;
    hid_t group_id = open_h5group(file_id_, group);

    hid_t dataset_id = H5Dopen2(group_id, list.c_str(), H5P_DEFAULT);
    hid_t dataspace_id = H5Dget_space(dataset_id);
    int rank = H5Sget_simple_extent_ndims(dataspace_id);

    hsize_t dims_out[]={0};
    H5Sget_simple_extent_dims(dataspace_id, dims_out, NULL);
    ret.resize(dims_out[0]*rank);
    H5Dread(dataset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, ret.data());

    // close
    close_h5dataspace(dataspace_id);
    close_h5dataset(dataset_id);

    // close group
    close_h5group(group_id);

    // return tmp;
    return ret;
}

template <>
std::vector<float> Hdf5Handle::get_data<float>(std::string list, const char *group)
{
    std::vector<float> ret;
    hid_t group_id = open_h5group(file_id_, group);

    hid_t dataset_id = H5Dopen2(group_id, list.c_str(), H5P_DEFAULT);
    hid_t dataspace_id = H5Dget_space(dataset_id);
    int rank = H5Sget_simple_extent_ndims(dataspace_id);

    hsize_t dims_out[]={0};
    H5Sget_simple_extent_dims(dataspace_id, dims_out, NULL);
    ret.resize(dims_out[0]*rank);
    H5Dread(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, ret.data());

    // close
    close_h5dataspace(dataspace_id);
    close_h5dataset(dataset_id);

    // close group
    close_h5group(group_id);

    // return tmp;
    return ret;
}

} //namespace
