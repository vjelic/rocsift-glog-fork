<!-- Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved -->

<a name="readme-top"></a>

<!-- PROJECT LOGO -->
<br />
<div align="center">
  <a href="https://github.com/ROCm/rocsift">
    <img src="images/logo.png" alt="Logo" width="80" height="80">
  </a>

<h3 align="center">Rocsift</h3>

  <p align="center">
    A C99 ROCm™ Debug API
    <br />
    <a href="https://rocm.github.io/rocsift/"<strong>Explore the docs »</strong></a>
    <br />
    <br />
    <a href="https://github.com/ROCm/rocsift/issues/new?labels=bug&template=bug-report---.md">Report a Bug</a>
    ·
    <a href="https://github.com/ROCm/rocsift/issues/new?labels=enhancement&template=feature-request---.md">Request a Feature</a>
  </p>
</div>
<!-- ABOUT THE PROJECT -->

## About Rocsift

Rocsift is a C99 API which provides a simple interface to debug AMD ROCm™ GPUs. It wraps AMDGPU SysFS and DebugFS files to provide functions to detect devices, access registers, memory, dump runlists, and more.

<p align="right"><a href="#readme-top">back to top</a></p>

### Built With

<div align="center">

<a href=""></a>[![ROCm™][rocm]][rocm-url]</a>
<a href=""></a>[![C++][C++]][C++-url]</a>
<a href=""></a>[![Cmake][Cmake]][Cmake-url]</a>
<a href=""></a>[![Linux][Linux]][Linux-url]</a>

</div>

<p align="right"><a href="#readme-top">back to top</a></p>

## Getting Started

### Build From Source

#### Prerequisites

- Cmake
- C++20 compiler

#### Build

```sh
sudo apt install cmake
git clone --recursive https://github.com/ROCm/rocsift
cd rocsift && mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j
```

Either `make install` or `make -j` package and install the built packages

<p align="right"><a href="#readme-top">back to top</a></p>

## Usage

In the following code block, we will read the contents of the 32-bit register located at MMIO address 0x8012, which happens to be the GRBM_STATUS register on many AMD GPUs.
First, we initialize sift. Then, we find the GPU partition we want to talk to. Some GPUs have a reconfigurable number of partitions, but most only have a single partition.
After, we read the register and print the value. Finally, we destroy sift.

```c++
#include "rocsift/sift.h"


int main () {
  sift_status_t rc = sift_init();
  if (rc != SIFT_STATUS_SUCCESS) {
    fprintf(stderr, "Failed to initialize sift");
    sift_destroy();
    return -1;
  }

  int partition = 0; // partition number
  sift_aperture_t aperture = SIFT_APERTURE_MMIO;
  uint64_t addr = 0x8012;

  int num_parts;
  rc = sift_get_partition_count(&num_parts);
  if (rc != SIFT_STATUS_SUCCESS) {
    fprintf(stderr, "failed to get partition count\n");
    sift_destroy();
    return -1;
  }

  sift_partition_t part;
  rc = sift_get_partition(partition, &part);
  if (rc != SIFT_STATUS_SUCCESS) {
    fprintf(stderr, "failed to get partition %d\n", partition);
    sift_destroy();
    return -1;
  }
  uint32_t val;
  rc = sift_partition_read_reg32(part, &val, aperture, addr);
  if (rc != SIFT_STATUS_SUCCESS) {
    fprintf(stderr, "failed to read aperture %s address 0x%016lx\n",
    aperture_str.c_str(), addr);
    sift_destroy();
  return -1;
  }
  printf("0x%08x\n", val);

  if (sift_destroy() != SIFT_STATUS_SUCCESS) {
    fprintf(stderr, "failed to destroy sift\n");
    return -1;
  };
  return 0;
}
```

For more examples, see [rocsift-tools][rocsift-tools-cmd-url].

<p align="right"><a href="#readme-top">back to top</a></p>

<!-- ENVIRONMENT VARIABLES -->

## Environment Variables

#### ROCSIFT_DEVID_OVERRIDE

Allows the user to override the device ID assigned in kfd/node with the value specified in this env variable.

EXAMPLE: Override the device id to 0x74A1 for the device with domain 0, bus 3, device 4, and function 1.

```
ROCSIFT_DEVID_OVERRIDE="0:03:04.1->0x74a1"
```

NOTE: If you need to override multiple devices, you can supply them comma separated like so:

```
ROCSIFT_DEVID_OVERRIDE="0:83:00.0->0x73a5,0:03:00.0->0x66a2,0:04:00.0->0x7388"
```

#### ROCSIFT_LOG_LEVEL

Can be set to any one of the following values:

```
trace
debug
info
warn
error
critical
off
```

<!-- ROADMAP -->

## Roadmap

- [x] Register access
- [x] Memory access
- [x] Runlists
- [x] Virtual address translation
- [ ] Wavefront dumps

See the [open issues](https://github.com/ROCm/rocsift/issues) for a full list of proposed features (and known issues).

<p align="right"><a href="#readme-top">back to top</a></p>

<!-- CONTRIBUTING -->

## Contributing

Any contributions you make are **greatly appreciated**.

If you have a suggestion that would make this better, please fork the repo and create a pull request. You can also simply open an issue with the tag "enhancement".
Don't forget to give the project a star! Thanks again!

1. Fork the Project
2. Create your Feature Branch (`git checkout -b feature/AmazingFeature`)
3. Commit your Changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the Branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

<p align="right"><a href="#readme-top">back to top</a></p>

<!-- LICENSE -->

## License

See `LICENSE.md` for more information.

<p align="right"><a href="#readme-top">back to top</a></p>

<!-- CONTACT -->

## Contact

Project Link: [https://github.com/ROCm/rocsift](https://github.com/ROCm/rocsift)

<p align="right"><a href="#readme-top">back to top</a></p>

[rocsift-tools-cmd-url]: https://github.com/ROCm/rocsift/tree/main/cmd
[rocm]: https://img.shields.io/badge/ROCm%E2%84%A2-grey?style=for-the-badge&logo=amd&labelColor=ED1C24
[rocm-url]: https://rocm.docs.amd.com
[C++]: https://img.shields.io/badge/c++-%2300599C.svg?style=for-the-badge&logo=c%2B%2B&logoColor=white
[C++-url]: https://isocpp.org
[CMake]: https://img.shields.io/badge/CMake-%23008FBA.svg?style=for-the-badge&logo=cmake&logoColor=white
[CMake-url]: https://cmake.org
[Linux]: https://img.shields.io/badge/Linux-FCC624?style=for-the-badge&logo=linux&logoColor=black
[Linux-url]: https://www.linux.org
