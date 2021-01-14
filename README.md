# diligent-graph

## Build

The following was tested on Ubuntu 16.04, 18.04, 20.04

### Install dependencies

Ubuntu 16.04 & 18.04

```bash
sudo apt install mesa-common-dev libgl1-mesa-dev libeigen3-dev libassimp-dev libxkbcommon-dev libxkbcommon-x11-dev	
```

Ubuntu 20.04:

```shell
sudo apt install mesa-common-dev libgl1-mesa-dev libeigen3-dev libassimp-dev libxkbcommon-dev libxkbcommon-x11-dev libx11-xcb-dev
```

### Build for yourself

```
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make
```

### Build and create a DEB package

```
mkdir build && cd build
export DISTRO_CODENAME=bionic # or xenial, focal, etc.
export GITHUB_RUN_NUMBER=10  # increase for every build
cmake -DCMAKE_BUILD_TYPE=Release ..
cpack
```

