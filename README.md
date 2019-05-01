## TF Trusted

TF Trusted allows you to run most Tensorflow models inside of an [Intel SGX](https://software.intel.com/en-us/sgx) device. It leverages a Tensorflow custom operation to send gRPC messages into the Intel SGX device via [Asylo](https://asylo.dev/) where the model is then run by Tensorflow Lite.

This project's goal is to make it easy to experiment with running TensorFlow models inside secure enclaves. This library is not production-ready and is provided for research and experimentation only.

We're always looking for contributors, if you're learning about how you can help improve the project, please check out our [contributing guidelines](CONTRIBUTING.md).

## Getting Started

To get started, clone this repository and then install the following dependencies.

#### Install Bazel

Bazel is required to build this custom operation. It can be downloaded from [here](https://docs.bazel.build/versions/master/install.html).

#### Python and Tensorflow

TF Trusted also requires python 3.5, 3.6 be installed along with tensorflow 1.13.1. You can install these using your favourite python version manager. We recommend using conda.

#### Install Docker

On Linux we need to build the custom operation using a docker container provided by TensorFlow.

Run one of the following commands to install docker for Ubuntu. Or use your desired package manager.

```
$ sudo snap install docker

$ sudo apt install docker.io
```

#### Build TF Trusted Custom Op

Follow the instructions for building the TensorFlow custom operation located [here](tf_trusted_custom_op/README.md).

### Build and Run TF Trusted

First, we will run TF Trusted in simulation mode. This makes it easy for testing new programs on with Asylo because you don't actually need the enclaves devices on the host machine.

We use a docker container to build TF Trusted and then run it.

```
$ docker run -it --rm \
  -v bazel-cache:/root/.cache/bazel \
  -v `pwd`:/opt/my-project \
  -w /opt/my-project \
  -p 50051:50051/tcp -p 50051:50051/udp \
  gcr.io/asylo-framework/asylo:buildenv-v0.3.4 \
  bazel run --config=enc-sim //tf_trusted \
  --incompatible_disallow_filetype=false --incompatible_disallow_data_transition=false
```

#### Run a Model

In another shell run the following with the correct options for the model you're using:

```
cd tf_trusted_custom_op
python model_run.py --model_file <location of protobuf model> \
                    --input_file <location of input file, npy format> \
                    --input_name <input placeholder node name> \
                    --output_name <output node name>
```

The input and output names are needed by the Tensorflow Lite converter to convert the model in the proper format. These can be retrieved the examining the model using a graph visualizer such at [Netron](https://github.com/lutzroeder/netron).

You should now see output!

### Running on an Intel SGX Device.

Next, we will run TF Trusted on an Intel SGX Device. This runs the program with encryption so that no one can learn about what the device is computing. It also allows a third party to remotely attest to the identity of the enclave.

When building enclave programs it's important to run them on an actual enclave or you might not detect performance issues or other bugs.

When running on a machine with an Intel SGX device there are some extra dependencies that need to be installed.

#### Install Intel SGX driver, SDK and PSW.

Driver can be installed with the following instructions:

https://github.com/01org/linux-sgx-driver

SDK/PSW can be installed with the following instructions:

https://github.com/intel/linux-sgx

#### Run AESM Service

The Architecture Enclave Service Manager (AESM) allows the Intel SGX device to be used by the host operating system. We can start the AESM service with:

```
service aesmd start
```

#### Build and Run TF Trusted

Now we can run a similar command as before. We just need to point the docker container to the SGX device, the aesmd socket and tell bazel inside the asylo docker container to use the SGX device.

```
$ docker run -it --rm --device=/dev/isgx \
  -v /var/run/aesmd/aesm.socket:/var/run/aesmd/aesm.socket \
  -v bazel-cache:/root/.cache/bazel \
  -v `pwd`:/opt/my-project \
  -w /opt/my-project  -p 50051:50051/tcp -p 50051:50051/udp \
  gcr.io/asylo-framework/asylo \
  bazel run --config=sgx --define=SGX_SIM=0 //tf_trusted \
  --incompatible_disallow_filetype=false --incompatible_disallow_data_transition=false
```

#### Run a Model

In another shell run the following with the correct options for the model you're using:

```
cd tf_trusted_custom_op
python model_run.py --model_file <location of protobuf model> \
                    --input_file <location of input file, npy format> \
                    --input_name <input placeholder node name> \
                    --output_name <output node name>
```


#### Install TF Trusted custom op

To be able to run the `model_run.py` script from anywhere on your machine you can install it with pip:

```
pip install -e .
```
