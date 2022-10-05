# Dockerfile

# This dockerfile contains the instructions required to build a Repast HPC image powered by Ubuntu Linux. 

# Declare the version of ubuntu's base image:
ARG UBUNTU_VERSION="22.10"

# Start from an ubuntu official image:
FROM ubuntu:${UBUNTU_VERSION}

# Declare image metadata:
LABEL org.opencontainers.image.title="Repast for High Performance Computing (Repast HPC) 2.3.1 docker image"
LABEL org.opencontainers.image.version="1.2"
LABEL org.opencontainers.image.authors="Wael Mohammed <mrwaelmohammed@gmail.com>"
LABEL org.opencontainers.image.url="https://github.com/W-Mohammed/docker-repast-hpc/pkgs/container/repast-hpc"
LABEL org.opencontainers.image.source="https://github.com/W-Mohammed/docker-repast-hpc/ubuntu"
LABEL org.opencontainers.image.base.name="https://hub.docker.com/layers/ubuntu/library/ubuntu/22.10/images/sha256-75d11da998393ed4a786917130c205967123f7ca8cddce2889fd9e3a0a69fcdd?context=explore"
LABEL org.opencontainers.image.description="This image provides a Repast HPC development environment.\
    The authors of this image adapted it from https://github.com/eze1981/repast-hpc/blob/master/Dockerfile.\
    This image preserves build-essential."

# Install system dependencies:
RUN apt update -y && \
    apt upgrade -y git \
        build-essential \
        zlib1g-dev \
        libgtest-dev \
        curl

# Repast HPC (rhpc):
## Declare the version of Repast HPC (rhpc) to be installed:
ARG RHPC_VERSION="2.3.1"

## Set a temporary working directory:
WORKDIR /tmp/repast-hpc-src

# Download the rhpc compressed file:
RUN curl -O -L https://github.com/Repast/repast.hpc/releases/download/v${RHPC_VERSION}/repast_hpc-${RHPC_VERSION}.tgz && \
    tar -xvzf repast_hpc-${RHPC_VERSION}.tgz 

# Move working directory to the installation folder:
WORKDIR /tmp/repast-hpc-src/repast_hpc-${RHPC_VERSION}/MANUAL_INSTALL

# Add MPICH bin to the PATH:
ENV PATH="/root/sfw/MPICH/bin:${PATH}"

# Install repast hpc dependencies:
RUN ./install.sh mpich && \
    ./install.sh netcdf && \
    ./install.sh boost

# Install repast hpc:
RUN ./install.sh rhpc

# Remove unneeded files:
WORKDIR /
RUN rm -rf tmp

# Setup a project file:
## Create the "project" folder as a working directory:
WORKDIR /project

## Copy rhpc example files to a folder called "examples" in the "project" directory:
RUN cp -v -T -r /root/sfw/repast_hpc-${RHPC_VERSION}/bin /project/examples

# Declare start-up command:
CMD ["/bin/bash"]