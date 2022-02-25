#ifndef HDF5FILES_HANDLE_H
#define HDF5FILES_HANDLE_H


#include "hdf5.h"
#include "string.h"
#include <iostream>
#include <vector>
#include <type_traits>

#define TRUE            1
#define FALSE           0

namespace utility
{
template <typename T>
struct SignalSaveType
{
    std::string name;
    std::vector<T> data;
};

class Hdf5Handle
{
public:
    Hdf5Handle() {}
    ~Hdf5Handle() {}

    void creat(std::string file)
    {
        file_id_ = creat_h5file(file.c_str(), H5F_ACC_TRUNC);
        group_id_header_ = creat_h5group(file_id_, "/Header");
        close_h5group(group_id_header_);
        group_id_ = creat_h5group(file_id_, "/Signal");
        close_h5group(group_id_);
    }

    void close()
    {
        close_h5file(file_id_);
    }

    template <typename T>
    void write(std::string signal, std::vector<T> data, const char *group)
    {
        hid_t group_id = open_h5group(file_id_, group);

        int rank = 1;
        hsize_t dims[] = {data.size()};
        hid_t dataspace_id, dataset_id;

        hid_t type_id ;
        if(std::is_same<T,int32_t>::value) {
            type_id =  H5T_NATIVE_INT;
        }
        else if(std::is_same<T,u_int32_t>::value) {
            type_id = H5T_NATIVE_UINT;
        }
        else if(std::is_same<T,u_int16_t>::value) {
            type_id = H5T_NATIVE_USHORT;
        }
        else if(std::is_same<T,int16_t>::value) {
            type_id = H5T_NATIVE_SHORT;
        }
        else if(std::is_same<T,float>::value) {
            type_id = H5T_NATIVE_FLOAT;
        }
        else if(std::is_same<T,double>::value) {
            type_id = H5T_NATIVE_DOUBLE;
        }
        else {
            type_id = H5T_NATIVE_DOUBLE;
        }

        dataspace_id = creat_h5dataspace(rank, dims);
        dataset_id = creat_h5dataset(group_id, signal.c_str(), type_id, dataspace_id);

        // write data
        write_hdf5(dataset_id,type_id,&data.front());
        close_h5dataspace(dataspace_id);
        close_h5dataset(dataset_id);

        // close group
        close_h5group(group_id);
    }

    bool open(std::string file)
    {
        file_id_ = open_h5file(file.c_str(),H5F_ACC_RDONLY);
        if(file_id_ > 0)
            return 1;
        else
            return 0;
    }

    std::vector<std::string> get_list(const char *group)
    {
        std::vector<std::string> list;

        hid_t group_id = open_h5group(file_id_, group);

        H5G_info_t grp_info;
        H5Gget_info(group_id, &grp_info);

        // get signal name
        char name[32];
        for(int i=0; i<grp_info.nlinks;i++) {
            H5Lget_name_by_idx(group_id, ".", H5_INDEX_NAME, H5_ITER_NATIVE,(hsize_t)i, name,32,H5P_DEFAULT);
            list.push_back(name);
        }

        close_h5group(group_id);
        return list;
    }

    H5T_class_t get_class(const std::string & dataset, const char *group)
    {
        hid_t group_id = open_h5group(file_id_, group);

        hid_t dataset_id = H5Dopen2(group_id, dataset.c_str(), H5P_DEFAULT);

        hid_t datatype = H5Dget_type(dataset_id);
        H5T_class_t t_class = H5Tget_class(datatype);

        close_h5dataset(dataset_id);
        close_h5group(group_id);
        return t_class;
    }

    template <typename T>
    std::vector<T> get_data(std::string list, const char *group);

private:
    hid_t file_id_;
    hid_t group_id_header_;
    hid_t group_id_;

    /* write */
    hid_t creat_h5file(const char *name, unsigned int flag)
    {
        hid_t file_id = H5Fcreate(name, flag, H5P_DEFAULT, H5P_DEFAULT);
        return file_id;
    }
    herr_t close_h5file(hid_t file_id)
    {
        return H5Fclose(file_id);
    }
    hid_t creat_h5group(hid_t file, const char * group)
    {
        hid_t group_id = H5Gcreate2(file, group, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
        return group_id;
    }
    herr_t close_h5group(hid_t group_id)
    {
        return H5Gclose(group_id);
    }
    hid_t creat_h5dataspace(int rank, const hsize_t *dims)
    {
        hid_t space_id = H5Screate_simple(rank, dims, NULL);
        return space_id;
    }
    herr_t close_h5dataspace(hid_t dataspace_id)
    {
        return H5Sclose(dataspace_id);
    }
    hid_t creat_h5dataset(hid_t loc_id, const char *name, hid_t type_id, hid_t space_id)
    {
        hid_t dataset_id = H5Dcreate2(loc_id, name, type_id,space_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
        return dataset_id;
    }
    herr_t close_h5dataset(hid_t dataset_id)
    {
        return H5Dclose(dataset_id);
    }
    herr_t write_hdf5(hid_t dataset_id,hid_t type_id,const void *buf)
    {
        herr_t status = H5Dwrite(dataset_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, buf);
        return status;
    }
    herr_t creat_attribute_str(hid_t dataset_id, const char *attr_name, const char * buf)
    {
        hid_t aid1 = H5Screate(H5S_SCALAR);
        hid_t stype = H5Tcopy(H5T_C_S1);
        H5Tset_size(stype, strlen(buf));
        H5Tset_strpad(stype, H5T_STR_NULLPAD);
        hid_t attr = H5Acreate2(dataset_id,attr_name, stype, aid1, H5P_DEFAULT, H5P_DEFAULT);
        herr_t ret = H5Awrite(attr,  stype, buf);

        // close attribute and file dataspaces, and datatype
        ret = H5Sclose(aid1);
        ret = H5Tclose(stype);
        ret = H5Aclose(attr);
        return ret;
    }

    /* read */
    hid_t open_h5file(const char *name, unsigned int flag)
    {
        return H5Fopen(name, flag, H5P_DEFAULT);
    }
    hid_t open_h5group(hid_t file_id, const char *name)
    {
        return H5Gopen2(file_id, name, H5P_DEFAULT);
    }
    hid_t open_h5dataset(hid_t file_id, const char *name)
    {
        return H5Dopen2(file_id, name, H5P_DEFAULT);
    }
    herr_t read_hdf5(hid_t dset_id, hid_t mem_type_id, void *buf/*out*/)
    {
        return H5Dread(dset_id, mem_type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, buf);

    }

}; // class

} // namespace

#endif // HDF5FILES_HANDLE_H
