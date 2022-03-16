set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)


set(TOOLCHAIN_ROOT_PATH /home/toolchain/linux-x86-64-gnu)
# set(CMAKE_SYSROOT ${TOOLCHAIN_ROOT_PATH}/aarch64-none-linux-gnu/libc)

# specify the cross compiler
set(CMAKE_C_COMPILER   ${TOOLCHAIN_ROOT_PATH}/bin/gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_ROOT_PATH}/bin/g++)

# where is the target environment 
# set(CMAKE_FIND_ROOT_PATH /usr)

# search for programs in the build host directories (not necessary)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# for libraries and headers in the target directories
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)


# configure Boost and Qt
# set(QT_QMAKE_EXECUTABLE ./)
# set(BOOST_ROOT ./)