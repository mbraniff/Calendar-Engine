FROM debian:trixie

  RUN apt-get update && apt-get install -y --no-install-recommends \
      build-essential cmake ninja-build gdb clangd git pkg-config file \
      mesa-utils \
      libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev libxext-dev \
      libwayland-dev libwayland-bin libxkbcommon-dev \
      libgl1-mesa-dev libegl-dev libgles-dev \
   && rm -rf /var/lib/apt/lists/*