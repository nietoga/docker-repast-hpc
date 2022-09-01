# Docker-powered Repast HPC development environment

## Introduction:
This repo contains the dockerfiles housing the instructions to generate docker-powered [Repast for High-Performance Computing](https://repast.github.io/repast_hpc.html) (**Repast HPC**) development environments.

The current Repast HPC version supported by the docker images is 2.3.1; whereas we used ubuntu 22.10 and alpine 3.16.2 as the base images in the dockerfiles.

## Usage:
### To pull (download) any of the images:
Make sure you have Docker Engine on your machine and run one of the following commands.

```powershell
# Ubuntu.22.10-powered image:
docker pull ghcr.io/w-mohammed/repast-hpc:2.3.1-ubuntu22.10
# alpine3.16.2-powered image:
docker pull ghcr.io/w-mohammed/repast-hpc:2.3.1-alpine3.16.2
```

### To run the Repast HPC development environment mapping a working directory: 
This image is primarily intended as a development environment that the user can employ to develop their Agent-Based Models (ABM)s. To get started with your development: 
- navigate to your projects working directory (where your model files are or are to be saved), and
- run one of the commands below (depending on which Linux distribution you prefer to work with and the image you pulled earlier).

```powershell
# Linux users:
## Ubuntu.22.10-powered image:
docker run --rm -it -v $(pwd):/project ghcr.io/w-mohammed/repast-hpc:2.3.1-ubuntu22.10
## alpine3.16.2-powered image:
docker run --rm -it -v $(pwd):/project ghcr.io/w-mohammed/repast-hpc:2.3.1-alpine3.16.2
# Windows users (PowerShell):
## Ubuntu.22.10-powered image:
docker run --rm -it -v $(PWD):/project ghcr.io/w-mohammed/repast-hpc:2.3.1-ubuntu22.10
## alpine3.16.2-powered image:
docker run --rm -it -v $(PWD):/project ghcr.io/w-mohammed/repast-hpc:2.3.1-alpine3.16.2
```

### To use the images as base layers in a dockerfile:
It is also possible to use any of the supported images as a base layer in other dockerfiles. 

```dockerfile
# Ubuntu.22.10-powered image:
FROM ghcr.io/w-mohammed/repast-hpc:2.3.1-ubuntu22.10
# alpine3.16.2-powered image:
FROM ghcr.io/w-mohammed/repast-hpc:2.3.1-alpine3.16.2
```

## Build:
Users can build any of the images from hosted dockerfiles:
- clone the repository using `git clone https://github.com/W-Mohammed/docker-repast-hpc.git`,
- navigate to the `docker-repast-hpc` folder, and 
- of the two commands below, call the one that corresponds to the linux distribution in which you are interested.

```powershell
# Ubuntu.22.10-powered image:
docker build --squash --tag repast-hpc ./alpine/
# alpine3.16.2-powered image:
docker build --squash --tag repast-hpc ./ubuntu
```
