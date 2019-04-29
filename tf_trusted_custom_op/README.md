## TF Trusted Custom Operation

These instructions guide you through building the TF Trusted custom operation that is required to run models directly inside of an Intel SGX device.

### MacOS Build

#### Install Bazel

Bazel is required to build this custom operation. It can be downloaded from [here](https://docs.bazel.build/versions/master/install.html).

#### Python and Tensorflow

This example also requires python 3.5, 3.6 be installed along with tensorflow 1.13.1. You can install these using your favourite python version manager. We recommend using conda.

#### Build Custom Operation

Run the following commands to build the custom operation.

```
$ ./configure.sh
$ bazel build model_enclave_op.so
$ cp bazel-bin/model_enclave_op.so .
```

### Linux Build

#### Install Docker

On Linux we need to build the custom operation using a docker container provided by TensorFlow.

Run one of the following commands to install docker for Ubuntu. Or use your desired package manager.

```
$ sudo snap install docker

$ sudo apt install docker.io
```

#### Start Container

For this command to work correctly we need to be in the root directory of this repo.

```
$ cd ..
$ sudo docker run -it -v `pwd`:/opt/my-project \
  -w /opt/my-project/tf_trusted_custom_op \
  tensorflow/tensorflow:custom-op /bin/bash
```

#### Build Custom Operation

Run the following commands to build the custom operation.

```
$ ./configure.sh
$ bazel build model_enclave_op.so
$ cp bazel-bin/model_enclave_op.so .
```

We can now exit out of the docker container.

```
exit
```
