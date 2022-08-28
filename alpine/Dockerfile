# Dockerfile

# This dockerfile contains the instructions required to build a Repast HPC image powered by Alpine Linux. 

# Declare the version of alpine's base image:
ARG ALPINE_VERSION="3.16.2"

# Start from an alpine official image:
FROM alpine:${ALPINE_VERSION}

# Declare image metadata:
LABEL org.opencontainers.image.title="Repast for High Performance Computing (Repast HPC) 2.3.1 docker image"
LABEL org.opencontainers.image.authors="Wael Mohammed <mrwaelmohammed@gmail.com>"
LABEL org.opencontainers.image.url="https://github.com/W-Mohammed/docker-repast-hpc/pkgs/container/repast-hpc"
LABEL org.opencontainers.image.source="https://github.com/W-Mohammed/docker-repast-hpc/alpine"
LABEL org.opencontainers.image.base.name="https://hub.docker.com/layers/alpine/library/alpine/3.16.2/images/sha256-1304f174557314a7ed9eddb4eab12fed12cb0cd9809e4c28f29af86979a3c870?context=explore"
LABEL org.opencontainers.image.description="This image provides a Repast HPC development environment.\
    The authors of this image adapted it from https://github.com/eze1981/repast-hpc/blob/master/Dockerfile "

# Install system dependencies:
RUN apk add --no-cache --update \
    bash \
    openssl \
    ca-certificates \
    curl \
    gcc \
    make \
    g++ \
    zlib-dev \
    git

# Repast HPC (rhpc):
## Declare the version of rhpc to be installed:
ARG RHPC_VERSION="2.3.1"

## Create temporary directory and set as working directory:
WORKDIR /tmp/repast-hpc-src

## Download, build, and install rhpc:
RUN curl -O -L github.com/Repast/repast.hpc/releases/download/v${RHPC_VERSION}/repast_hpc-${RHPC_VERSION}.tgz && \
    tar -xvzf repast_hpc-${RHPC_VERSION}.tgz

## Create temporary directory and set as working directory:
WORKDIR /tmp/repast-hpc-src/repast_hpc-${RHPC_VERSION}/MANUAL_INSTALL

## Add MPICH bin to the PATH:
ENV PATH="/root/sfw/MPICH/bin:${PATH}"

## Install repast hpc dependencies:
RUN ./install.sh mpich && \
    ./install.sh netcdf && \
    ./install.sh boost

## Install repast rhpc:
RUN ./install.sh rhpc

# Remove unneeded files:
WORKDIR /
RUN rm -rf /tmp/*

# Setup a project file:
## Create the "project" folder as a working directory:
WORKDIR /project

## Copy rhpc example files to a folder called "examples" in the "project" directory:
RUN cp -v -T -r /root/sfw/repast_hpc-${RHPC_VERSION}/bin /project/examples

# Declare start-up command:
CMD ["/bin/sh"]