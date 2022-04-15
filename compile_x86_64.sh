#!/bin/bash
export PROJECT_ROOT_DIR=$(cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )

DEPLOY_IP="192.168.40.99"
DEPLOY_USERNAME="nvidia"
DEPLOY_PASSWORD="nvidia"
DEPLOY_PATH="~/aicc/"

TOOLCHAIN_CMAKE=${PROJECT_ROOT_DIR}/cmake/linux_x86_64_gnu.cmake
BUILD_DIR=${PROJECT_ROOT_DIR}/build_x86_64
INSTALL_DIR=${PROJECT_ROOT_DIR}/output_x86_64

function query_cpu_corees() {
  cat /proc/cpuinfo| grep "processor"| wc -l
}

function build_make() {
  if [ ! -f "${TOOLCHAIN_CMAKE}" ]; then
    echo -e "\e[33m[WARN] ${TOOLCHAIN_CMAKE} not exist\e[0m"
    mkdir -p ${BUILD_DIR} && cd ${BUILD_DIR} &&
    cmake -G "Unix Makefiles" -DCMAKE_SYSTEM_NAME="Linux" -DCMAKE_SYSTEM_PROCESSOR="x86_64"   \
                              -DCMAKE_C_COMPILER=/home/toolchain/linux-x86-64-gnu/bin/gcc     \
                              -DCMAKE_CXX_COMPILER=/home/toolchain/linux-x86-64-gnu/bin/g++   \
                              -DCMAKE_INSTALL_PREFIX=${INSTALL_DIR}                           \
                              ${COMPILE_ARGS}                                                 \
                              ../$1 && make -j8
  else
    echo -e "\e[32m[INFO] use toolchain is ${TOOLCHAIN_CMAKE}\e[0m"
    mkdir -p ${BUILD_DIR} && cd ${BUILD_DIR} &&
    cmake -G "Unix Makefiles" -DCMAKE_TOOLCHAIN_FILE="${TOOLCHAIN_CMAKE}"                     \
                              -DAA_PLATFORM_MODULES_PATH=${AA_PLATFORM_MODULES_PATH}          \
                              -DCMAKE_INSTALL_PREFIX=${INSTALL_DIR} ../$1 && make -j8
  fi

}


function make_install() {
  cd ${BUILD_DIR}
  make install
}


function make_clean() {
  rm -rf ${BUILD_DIR}/*
  rm -rf ${INSTALL_DIR}/*
}

function deploy() {
  CMD="sshpass -p $3 scp -r ${INSTALL_DIR}/ $2@$1:$4"
  echo -e "\033[33m${CMD}\033[0m"
  ${CMD}
}

function main() {
  if  [ ! -n "$1" ] ;then
    echo -e "\e[32m[INFO] ready cmake build\e[0m"
    build_make ${CMAKE_ROOT_DIR}
  fi
  
  while getopts icud: option; #选项后面的冒号表示该选项需要参数
  do
    case "$option" in
      d)
        echo -e "\e[32m[INFO] cmake root dir $OPTARG\e[0m" #参数存在$optarg中
        CMAKE_ROOT_DIR=$OPTARG
        ;;
      i)
        echo -e "\e[32m[INFO] make install\e[0m"
        build_make ${CMAKE_ROOT_DIR}
        make_install
        # sed -i 's#${AA_PLATFORM_MODULES_PATH}#'''${AA_PLATFORM_MODULES_PATH}'''#g' ${AA_PLATFORM_MODULES_PATH}/adas-packages.json 
        ;;
      c)
        echo -e "\e[32m[INFO] make clean\e[0m"
        make_clean
        ;;
      u)
        echo -e "\e[32m[INFO] deploy\e[0m"
        deploy ${DEPLOY_IP} ${DEPLOY_USERNAME} ${DEPLOY_PASSWORD} ${DEPLOY_PATH}
        ;;

      ?) #当有不认识的选项的时候arg为?
        echo -e "\e[32m Usage: args [-b] [-i] [-c] [-t n] \e[0m"
        echo -e "\e[32m empty means make\e[0m"
        echo -e "\e[32m -i means install\e[0m"
        echo -e "\e[32m -c means clean\e[0m"
        echo -e "\e[32m -u means deploy\e[0m"
        echo -e "\e[32m -d cmake root directory\e[0m"
    esac
  done

}

main "$@"