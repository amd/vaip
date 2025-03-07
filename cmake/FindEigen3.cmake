#
#  Copyright (C) 2023 – 2024 Advanced Micro Devices, Inc. All rights reserved.
#  Licensed under the MIT License.
#
if(NOT Eigen3_FOUND)
  if(NOT TARGET Eigen3::Eigen)
    FetchContent_Declare(
      Eigen3
      URL ${DEP_URL_eigen}
      URL_HASH SHA1=${DEP_SHA1_eigen}
      OVERRIDE_FIND_PACKAGE
    )
  endif()
endif()
set(Eigen3_FOUND TRUE)
