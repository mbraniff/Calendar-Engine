FROM debian:trixie

  RUN apt-get update && apt-get install -y --no-install-recommends \
      build-essential cmake ninja-build gdb clangd git \
      mesa-utils \
      libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev \
      libgl1-mesa-dev \
   && rm -rf /var/lib/apt/lists/*