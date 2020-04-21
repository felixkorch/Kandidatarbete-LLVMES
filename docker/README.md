# Docker environment for LLVMES

This directory contains a Dockerfile for Fedora Linux.
The Docker container is generated from a template in the top of this directory.

## Prerequisites
1. You need to install `docker` and `docker-compose`.
2. Make sure `docker` is running as a daemon.

## Docker Usage

1. In the root of this project, run `docker-compose up -d` to deploy the build
   environment. The first time you run this command, the Docker container will
   be built before it is deployed.
2. When the above step is done, one can execute commands from within the Docker
   container by prefixing them with `docker-compose exec llvmes`. 

   E.g. to build the project, execute `docker-compose exec llvmes ./autobuild`
   from the project root.

The working directory with LLVMES (this project) will automatically be mounted under the same
path in the container. Any changes you make in the project will automatically be
reflected inside of the Docker container.

:warning: The Docker daemon runs as root by default, and therefore any directories
or files created from within the Docker container is owned by the root user. To
fix this, consult a guide for your own distribution/init system on how to run
root-less Docker containers.

## About the templates

Each directory represents another Docker container. 
The `config.yaml` file is read and all of the fields become
a dictionary in the Python code and are variables in the template. 

## Generating Dockerfiles

Just run `./generate` in the `Docker` directory.

## Acknowledgment
The Docker workflow and structure as presented here originates from [Remacs - A
community drived port of Emacs to Rust](https://github.com/remacs/remacs). Huge thanks!
