# Candela 3

## Docker Builder Image

Generate using:

```bash
docker build --target github -t kvnnap/clang:github .devcontainer/linux/
docker push kvnnap/clang:github
```

## WSL

```Dockerfile
FROM ubuntu:25.10

RUN export debian_frontend=noninteractive \
 && apt update \
 && apt install -y software-properties-common \
 && add-apt-repository ppa:kisak/kisak-mesa \
 && apt update \
 && apt install -y vulkan-tools
```

```bash
docker build -t kvnnap/vulkantest:latest .
docker run --rm -it --gpus all \
 -v /usr/lib/wsl:/usr/lib/wsl:ro \
 -v /tmp/.X11-unix:/tmp/.X11-unix:ro \
 -v $XDG_RUNTIME_DIR/$WAYLAND_DISPLAY:/tmp/$WAYLAND_DISPLAY:ro \
 -e LD_LIBRARY_PATH=/usr/lib/wsl/lib \
 -e XDG_RUNTIME_DIR=/tmp \
 -e WAYLAND_DISPLAY=$WAYLAND_DISPLAY \
 -e DISPLAY=$DISPLAY \
    kvnnap/vulkantest:latest
```