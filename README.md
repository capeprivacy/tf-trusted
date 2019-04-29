## TF Trusted

TF Trusted allows you to run most Tensorflow models inside of an [Intel SGX](https://software.intel.com/en-us/sgx) device. It leverages a Tensorflow custom operation to send gRPC messages into the Intel SGX device via [Asylo](https://asylo.dev/) where the model is then run by Tensorflow Lite.

The goal of this project is to provide an easy to use framework to experiment with running models securely inside secure enclaves. It **shouldn't** be used in production and is provided for research and experimentation only.

Please see the [contributing](CONTRIBUTING.md) document on how you can help improve the project.

## Getting Started

First clone this repo and follow the instructions [here](tf_trusted_custom_op/README.md) to build the required custom operation.

#### Build and Run TF Trusted

Here we use a docker container to build TF Trusted and then run it.

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

### Running on Intel SGX Device.

If running on a machine with a SGX Device you run the following to install the needed dependencies.

#### Install Intel SGX driver, SDK and PSW.

Driver can be installed with the following instructions:

https://github.com/01org/linux-sgx-driver

SDK/PSW can be installed with the following instructions:

https://github.com/intel/linux-sgx

#### Run aesmd Service

The aesmd service manages the SGX device.

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
