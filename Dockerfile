# Dockerfile

# This dockerfile contains the instructions required to build a Repast HPC image. 

# Declare the version of ubuntu's base image:
ARG UBUNTU_VERSION="22.10"

# Start from an ubuntu official image:
FROM ubuntu:${UBUNTU_VERSION}

# Declare maintainer's name:
LABEL "developer"="Wael Mohammed <mrwaelmohammed@gmail.com>"
LABEL "adapted from"="https://github.com/eze1981/repast-hpc/blob/master/Dockerfile"
LABEL org.opencontainers.image.source="https://github.com/W-Mohammed/docker-repast-hpc"

# Add MPICH bin to the PATH:
ENV PATH="/root/sfw/MPICH/bin:${PATH}"

# Set a temporary working directory:
WORKDIR /temp/repast-hpc-src

# Install system dependencies:
RUN apt update -y && \
    apt upgrade -y git \
        build-essential \
        zlib1g-dev \
        libgtest-dev && \
    apt-get install wget

# Declare the version of Repast HPC (rhpc) to be installed:
ARG RHPC_VERSION="2.3.1"

# Download Repast HPC (rhpc) compressed file:
RUN wget https://github.com/Repast/repast.hpc/releases/download/v${RHPC_VERSION}/repast_hpc-${RHPC_VERSION}.tgz && \
    tar -xvzf repast_hpc-${RHPC_VERSION}.tgz 

# Move working directory to installation folder:
WORKDIR /temp/repast-hpc-src/repast_hpc-${RHPC_VERSION}/MANUAL_INSTALL

# Install repast hpc dependencies:
RUN ./install.sh curl && \
    ./install.sh mpich && \
    ./install.sh netcdf && \
    ./install.sh boost

# Install repast hpc:
RUN ./install.sh rhpc

# Remove unneeded files:
RUN apt remove -y git build-essential && \
    apt autoremove -y

# Pass the libraries directories to ld
RUN ldconfig /root/sfw/CURL/lib/ \
    /root/sfw/MPICH/lib/ \
    /root/sfw/NetCDF/lib/ \
    /root/sfw/NetCDF-cxx/lib/ \
    /root/sfw/repast_hpc-2.3.1/lib/ \
    /root/sfw/Boost/Boost_1.61/lib/

# Remove unneeded files:
WORKDIR /
RUN rm -rf temp

# Copy rhpc example files to an examples folder in a project directory:
WORKDIR /project
RUN cp -v -T -r /root/sfw/repast_hpc-${RHPC_VERSION}/bin /project/examples

# Declare start-up command:
CMD ["/bin/sh"]
